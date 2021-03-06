/*
 * Header for custom libc functions that I don't want to 
 * put into the default headers as not to confuse any
 * software that might use them sometime in the future
 */

#ifndef _NEKOSYS_H
#define _NEKOSYS_H

#define PATH_MAX 1024

#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        uint8_t *buffer;
        size_t width;
        size_t height;
        size_t pitch;
        size_t pixelStride;
    } FRAMEBUF;

    typedef struct
    {
        int dx;
        int dy;
        int dwheel;
        int buttons;
    } MOUSEPACKET;

    /* FIXME: hacky stdin readline - should be redone */
    int readln(char *dst, size_t maxSize);

    /* Processes */
    int spawnp(pid_t *pid, const char *path, int argc, const char **argv);
    int waitp(pid_t pid);

    /* Kernel framebuffer */
    int framebuf_acquire(FRAMEBUF *framebuf);
    int framebuf_release();
    int framebuf_flush(int x, int y, int w, int h);
    int framebuf_flush_all();

    /* Shared buffers / memory regions */
    int shbuf_create(size_t size);
    int shbuf_map(int bufid, void **mapped);
    int shbuf_unmap(int bufid);

    /* Connection pipes */
    int pipe_open(const char *name);
    int pipe_close(int pipeid);
    int pipe_recv(int pipeid, pid_t *src, size_t size, uint8_t *buffer);
    int pipe_send(int pipeid, pid_t dst, size_t size, uint8_t *data);

    /* Threads */
    int thread_create(void (*entryPoint)());
    int thread_join(int threadId);
    int thread_die(int retcode);

    /* Mouse */
    int mouse_poll(MOUSEPACKET *packet);
#ifdef __cplusplus
}
#endif

#endif