#include <stdio.h>

/*
 * Helpers
 */

#ifdef DEBUG
    #include <sys/timeb.h>
    #define MSG(...) {                                                          \
        struct timeb tp; ftime(&tp);                                            \
        char msg_buf[1000];                                                     \
        sprintf(msg_buf, __VA_ARGS__);                                          \
        fprintf(stderr, "[%ld.%d] %s\n", tp.time, tp.millitm, msg_buf);         \
        fflush(stderr);                                                         \
    }
#else
    #define MSG(m)
#endif

void print_usage();

void print_usage_error(char *error_msg);

