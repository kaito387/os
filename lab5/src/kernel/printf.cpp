#include "stdio.h"
#include "os_type.h"
#include "stdarg.h"
#include "stdlib.h"

// Global STDIO instance for output
extern STDIO stdio;

// Helper function to add a character to the buffer and flush if needed
int printf_add_to_buffer(char *buffer, char c, int &idx, const int BUF_LEN)
{
    int counter = 0;

    buffer[idx] = c;
    ++idx;

    if (idx == BUF_LEN)
    {
        buffer[idx] = '\0';
        counter = stdio.print(buffer);
        idx = 0;
    }

    return counter;
}

/**
 * printf - Format and print a string to the screen
 * @fmt: Format string with optional format specifiers
 * @...: Variable arguments corresponding to format specifiers
 *
 * Supported format specifiers:
 *   %d - decimal integer (supports negative numbers)
 *   %c - single character
 *   %s - null-terminated string
 *   %x - hexadecimal integer (lowercase)
 *   %% - literal percent character
 *
 * Returns: Number of characters printed
 */
int printf(const char *const fmt, ...)
{
    const int BUF_LEN = 32;

    char buffer[BUF_LEN + 1];
    char number[33];

    int idx, counter;
    va_list ap;

    va_start(ap, fmt);
    idx = 0;
    counter = 0;

    for (int i = 0; fmt[i]; ++i)
    {
        if (fmt[i] != '%')
        {
            // Regular character, add to buffer
            counter += printf_add_to_buffer(buffer, fmt[i], idx, BUF_LEN);
        }
        else
        {
            i++;
            if (fmt[i] == '\0')
            {
                break;
            }

            switch (fmt[i])
            {
            case '%':
                // Literal percent character
                counter += printf_add_to_buffer(buffer, '%', idx, BUF_LEN);
                break;

            case 'c':
                // Single character
                counter += printf_add_to_buffer(buffer, va_arg(ap, char), idx, BUF_LEN);
                break;

            case 's':
                // String - flush buffer first, then print string directly
                buffer[idx] = '\0';
                idx = 0;
                counter += stdio.print(buffer);
                counter += stdio.print(va_arg(ap, const char *));
                break;

            case 'd':
            case 'x':
                // Decimal or hexadecimal integer
                int temp = va_arg(ap, int);

                if (temp < 0 && fmt[i] == 'd')
                {
                    // Handle negative decimal numbers
                    counter += printf_add_to_buffer(buffer, '-', idx, BUF_LEN);
                    temp = -temp;
                }

                // Convert number to string (base 10 for 'd', base 16 for 'x')
                itos(number, temp, (fmt[i] == 'd' ? 10 : 16));

                // Add each digit to buffer
                for (int j = 0; number[j]; ++j)
                {
                    counter += printf_add_to_buffer(buffer, number[j], idx, BUF_LEN);
                }
                break;
            }
        }
    }

    // Flush remaining buffer
    buffer[idx] = '\0';
    counter += stdio.print(buffer);

    return counter;
}
