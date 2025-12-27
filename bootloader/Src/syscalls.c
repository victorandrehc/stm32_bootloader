#include "main.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
// Dummy implementation of _write
int _write(int file, const char *ptr, int len) {
    const char* prefix = "BOOTLOADER: ";
    const size_t prefix_len = strlen(prefix);
    HAL_UART_Transmit(&huart2, (uint8_t *)prefix, prefix_len, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

// Dummy implementation of _read
int _read(int file, char *ptr, int len) {
    HAL_UART_Receive(&huart2, (uint8_t *)ptr, 1, HAL_MAX_DELAY);
    return 1;
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
