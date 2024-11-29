#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "vasprintf.h"

/**
 * vasprintf - Allocates a formatted string and returns it.
 * @strp: Pointer to the allocated buffer
 * @fmt: Format string
 * @ap: Variable argument list
 *
 * Returns: Number of characters written (excluding null terminator),
 *          or -1 on allocation failure.
 */
int vasprintf(char **strp, const char *fmt, va_list ap) {
    if (!strp || !fmt) {
        return -1;  // Invalid arguments
    }

    va_list ap_copy;
    va_copy(ap_copy, ap);  // Copy the va_list to calculate size

    // Calculate required size (excluding null terminator)
    int size = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (size < 0) {
        return -1;  // Formatting error
    }

    // Allocate the buffer
    *strp = (char *)malloc(size + 1);
    if (!*strp) {
        return -1;  // Allocation failed
    }

    // Format the string into the allocated buffer
    int result = vsnprintf(*strp, size + 1, fmt, ap);
    if (result < 0) {
        free(*strp);  // Cleanup on failure
        *strp = NULL;
    }

    return result;
}
