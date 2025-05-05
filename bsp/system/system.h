
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* _sbrk(ptrdiff_t incr);
/* Variables */
extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

extern char* __env[];
extern char** environ;

struct stat;
struct tms;

/* Functions */
void initialise_monitor_handles();

int _getpid(void);

int _kill(int pid, int sig);

void _exit(int status);

int _read(int file, char* ptr, int len);

int _write(int file, char* ptr, int len);

int _close(int file);

int _fstat(int file, struct stat* st);

int _isatty(int file);

int _lseek(int file, int ptr, int dir);

int _open(char* path, int flags, ...);

int _wait(int* status);

int _unlink(char* name);

int _times(struct tms* buf);

int _stat(char* file, struct stat* st);

int _link(char* old, char*);

int _fork(void);

int _execve(char* name, char** argv, char** env);

#ifdef __cplusplus
}
#endif
