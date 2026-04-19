#include "gui_render_queue.h"
#include <string.h>

static SDL_Renderer* g_renderer = NULL;

typedef struct {
    RenderCmd buffer[RENDER_QUEUE_CAPACITY];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    SDL_mutex* mutex;
    SDL_cond*  cond_push;
    SDL_cond*  cond_pop;
    int should_exit;
} RenderQueue;

static RenderQueue g_queue;

void render_queue_init(SDL_Renderer* renderer)
{
    g_renderer = renderer;

    memset(&g_queue, 0, sizeof(g_queue));
    g_queue.mutex     = SDL_CreateMutex();
    g_queue.cond_push = SDL_CreateCond();
    g_queue.cond_pop  = SDL_CreateCond();
    g_queue.should_exit = 0;
}

int render_queue_push(const RenderCmd* cmd)
{
    if (!cmd || !g_renderer) return -1;

    SDL_LockMutex(g_queue.mutex);

    while (g_queue.count == RENDER_QUEUE_CAPACITY) {
        if (g_queue.should_exit) {
            SDL_UnlockMutex(g_queue.mutex);
            return -1;
        }
        SDL_CondWaitTimeout(g_queue.cond_push, g_queue.mutex, 100);
    }

    g_queue.buffer[g_queue.tail] = *cmd;
    g_queue.tail = (g_queue.tail + 1) % RENDER_QUEUE_CAPACITY;
    g_queue.count++;

    SDL_CondSignal(g_queue.cond_pop);
    SDL_UnlockMutex(g_queue.mutex);
    return 0;
}

int render_queue_pop(RenderCmd* out_cmd)
{
    if (!out_cmd) return -1;

    SDL_LockMutex(g_queue.mutex);

    while (g_queue.count == 0) {
        if (g_queue.should_exit) {
            SDL_UnlockMutex(g_queue.mutex);
            return -1;
        }
        SDL_CondWait(g_queue.cond_pop, g_queue.mutex);
    }

    *out_cmd = g_queue.buffer[g_queue.head];
    g_queue.head = (g_queue.head + 1) % RENDER_QUEUE_CAPACITY;
    g_queue.count--;

    SDL_CondSignal(g_queue.cond_push);
    SDL_UnlockMutex(g_queue.mutex);
    return 0;
}

void render_queue_signal_exit(void)
{
    SDL_LockMutex(g_queue.mutex);
    g_queue.should_exit = 1;
    SDL_CondBroadcast(g_queue.cond_pop);
    SDL_CondBroadcast(g_queue.cond_push);
    SDL_UnlockMutex(g_queue.mutex);
}

void render_queue_destroy(void)
{
    render_queue_signal_exit();
    if (g_queue.cond_pop)  SDL_DestroyCond(g_queue.cond_pop);
    if (g_queue.cond_push) SDL_DestroyCond(g_queue.cond_push);
    if (g_queue.mutex)     SDL_DestroyMutex(g_queue.mutex);
    memset(&g_queue, 0, sizeof(g_queue));
}
