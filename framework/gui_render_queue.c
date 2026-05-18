#include "gui_user_include.h"

#if WITHTOUCHGUI

#include "gui_includes.h"
#include "gui_render_queue.h"
#include <string.h>

typedef struct {
    RenderCmd buffer[RENDER_QUEUE_CAPACITY];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    gui_mutex_t * mutex;
    gui_cond_t * cond_push;
    gui_cond_t * cond_pop;
    int should_exit;
} RenderQueue;

static RenderQueue g_queue;
RenderBatch_t * common_batch = NULL;

void render_queue_init(void)
{
    memset(&g_queue, 0, sizeof(g_queue));
    g_queue.mutex     = _gui_mutex_init();
    g_queue.cond_push = _gui_cond_init();
    g_queue.cond_pop  = _gui_cond_init();
    g_queue.should_exit = 0;

    common_batch = malloc(sizeof(RenderBatch_t));
    common_batch->batch = NULL;
    common_batch->idx = 0;
}

int render_queue_push(const RenderCmd* cmd)
{
    if (!cmd) return -1;

    _gui_mutex_lock(g_queue.mutex);

    while (g_queue.count == RENDER_QUEUE_CAPACITY) {
        if (g_queue.should_exit) {
            _gui_mutex_unlock(g_queue.mutex);
            return -1;
        }
        _gui_cond_wait(g_queue.cond_push, g_queue.mutex);
    }

    g_queue.buffer[g_queue.tail] = *cmd;
    g_queue.tail = (g_queue.tail + 1) % RENDER_QUEUE_CAPACITY;
    g_queue.count++;

    _gui_cond_signal(g_queue.cond_pop);
    _gui_mutex_unlock(g_queue.mutex);
    return 0;
}

int render_queue_push_batch(const RenderCmd* cmds, uint32_t count)
{
    if (!cmds || count == 0 || count > RENDER_QUEUE_CAPACITY)
        return -1;

    _gui_mutex_lock(g_queue.mutex);

    while (RENDER_QUEUE_CAPACITY - g_queue.count < count) {
        if (g_queue.should_exit) {
            _gui_mutex_unlock(g_queue.mutex);
            return -1;
        }
        _gui_cond_wait(g_queue.cond_push, g_queue.mutex);
    }

    for (uint32_t i = 0; i < count; ++i) {
        g_queue.buffer[g_queue.tail] = cmds[i];
        g_queue.tail = (g_queue.tail + 1) % RENDER_QUEUE_CAPACITY;
        g_queue.count++;
    }

    _gui_cond_signal(g_queue.cond_pop);
    _gui_mutex_unlock(g_queue.mutex);
    return 0;
}

int render_queue_pop(RenderCmd* out_cmd)
{
    if (!out_cmd) return -1;

    _gui_mutex_lock(g_queue.mutex);

    while (g_queue.count == 0) {
        if (g_queue.should_exit) {
            _gui_mutex_unlock(g_queue.mutex);
            return -1;
        }
        _gui_cond_wait(g_queue.cond_pop, g_queue.mutex);
    }

    *out_cmd = g_queue.buffer[g_queue.head];
    g_queue.head = (g_queue.head + 1) % RENDER_QUEUE_CAPACITY;
    g_queue.count--;

    _gui_cond_signal(g_queue.cond_push);
    _gui_mutex_unlock(g_queue.mutex);
    return 0;
}

void render_queue_signal_exit(void)
{
    _gui_mutex_lock(g_queue.mutex);
    g_queue.should_exit = 1;
    _gui_cond_broadcast(g_queue.cond_pop);
    _gui_cond_broadcast(g_queue.cond_push);
    _gui_mutex_unlock(g_queue.mutex);
}

void render_queue_destroy(void)
{
    render_queue_signal_exit();
    if (g_queue.cond_pop)  _gui_cond_destroy(g_queue.cond_pop);
    if (g_queue.cond_push) _gui_cond_destroy(g_queue.cond_push);
    if (g_queue.mutex)     _gui_mutex_destroy(g_queue.mutex);
    memset(&g_queue, 0, sizeof(g_queue));
}

RenderCmd * add_task(RenderBatch_t * render_batch)
{
	if (! common_batch->is_active) return NULL;

    render_batch->idx ++;
    RenderCmd * new_ptr = realloc(render_batch->batch, sizeof(RenderCmd) * render_batch->idx);
    GUI_MEM_ASSERT(new_ptr);

    render_batch->batch = new_ptr;
    return & render_batch->batch[render_batch->idx - 1];
}

#endif /* WITHTOUCHGUI */
