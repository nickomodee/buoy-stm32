#include <sys/types.h>
#include <errno.h>

// defined in linker script
extern char _end;
char *heap_end = 0;

void *_sbrk(ptrdiff_t incr) {
    extern char _estack;  // end of the stack pointer fromo linker
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_end;
    }

    prev_heap_end = heap_end;

    if (heap_end + incr > &_estack) {
        errno = ENOMEM;
        return (void *)-1;
    }

    heap_end += incr;
    return (void *)prev_heap_end;
}