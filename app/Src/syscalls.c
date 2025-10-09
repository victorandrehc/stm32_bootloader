#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

// Dummy implementation of _write
int _write(int file, const char *ptr, int len) {
    // Optionally implement UART output here
    return len;  // Pretend we wrote everything
}

// Dummy implementation of _read
int _read(int file, char *ptr, int len) {
    errno = ENOSYS;
    return -1;
}

// Dummy implementation of _close
int _close(int file) {
    errno = ENOSYS;
    return -1;
}

// Dummy implementation of _lseek
int _lseek(int file, int ptr, int dir) {
    errno = ENOSYS;
    return -1;
}

// Dummy implementation of _fstat
int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

// Dummy implementation of _isatty
int _isatty(int file) {
    return 1;
}
