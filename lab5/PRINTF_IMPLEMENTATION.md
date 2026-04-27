# Printf Implementation for Lab5 Assignment 1

## Overview
This is an implementation of the `printf()` function integrated with the existing STDIO class in the kernel. The implementation uses variable arguments (varargs) to handle multiple format specifiers and outputs formatted strings to the screen.

## Files Created

### 1. `/home/lht/dev/study/os/lab5/src/kernel/printf.cpp`
Main implementation of the printf function with the following components:

#### Helper Function: `printf_add_to_buffer()`
- Adds a single character to the internal buffer
- Automatically flushes the buffer (32 characters max) when full
- Returns the number of characters printed to screen

#### Main Function: `printf(const char *const fmt, ...)`
- Entry point for formatted printing
- Parses format string and extracts variable arguments
- Supports multiple format specifiers

### 2. `/home/lht/dev/study/os/lab5/src/kernel/stdio.cpp`
Implementation of the STDIO class methods:
- `initialize()` - Sets up screen memory at 0xb8000
- `print()` - Overloaded methods for character/string output
- `moveCursor()` - Updates cursor position
- `getCursor()` - Retrieves current cursor position
- `rollUp()` - Scrolls screen when bottom is reached

### 3. `/home/lht/dev/study/os/lab5/src/utils/stdlib.cpp`
Utility functions used by printf:
- `itos()` - Converts integer to string in specified base (2-36)
- `swap()` - Template function for swapping values

## Supported Format Specifiers

| Specifier | Description | Example |
|-----------|-------------|---------|
| `%d` | Decimal integer (supports negative) | `printf("%d", -1234)` outputs `-1234` |
| `%c` | Single character | `printf("%c", 'N')` outputs `N` |
| `%s` | Null-terminated string | `printf("%s", "Hello")` outputs `Hello` |
| `%x` | Hexadecimal integer (uppercase A-F) | `printf("%x", 255)` outputs `FF` |
| `%%` | Literal percent character | `printf("%%")` outputs `%` |

## Implementation Details

### Format Parsing Algorithm
1. Iterate through each character in the format string
2. When `%` is encountered, check the next character:
   - `%` → Output literal `%`
   - `c` → Extract and output character argument
   - `s` → Extract and output string argument
   - `d` → Extract integer, convert to decimal, output
   - `x` → Extract integer, convert to hexadecimal, output
3. Regular characters are accumulated in a buffer
4. Buffer is flushed when full (32 bytes) or when strings are printed

### Buffer Management
- Uses a 32-character buffer to optimize screen output
- When buffer fills, it's null-terminated and sent to STDIO::print()
- Buffer is flushed at end of printf() call
- Ensures efficient screen I/O operations

### Negative Number Handling
- Checks if value is negative and format is `%d`
- Outputs minus sign separately
- Converts absolute value to decimal string
- Works with the itos() function's modulo-based conversion

### Variable Arguments Processing
Uses standard `<stdarg.h>` macros:
- `va_start()` - Initialize argument list
- `va_arg()` - Extract next argument
- `va_end()` - Clean up argument list

## Testing

The implementation has been tested with:
1. Basic compilation - All source files compile without errors
2. Format specifier verification - Each format specifier produces correct output
3. Edge cases:
   - Negative numbers in decimal format
   - Large hexadecimal values
   - Empty strings
   - Multiple format specifiers in one call

Example test case (from setup.cpp):
```c
printf("print percentage: %%\n"
       "print char \"N\": %c\n"
       "print string \"Hello World!\": %s\n"
       "print decimal: \"-1234\": %d\n"
       "print hexadecimal \"0x7abcdef0\": %x\n",
       'N', "Hello World!", -1234, 0x7abcdef0);
```

Expected output:
```
print percentage: %
print char "N": N
print string "Hello World!": Hello World!
print decimal: "-1234": -1234
print hexadecimal "0x7abcdef0": 7ABCDEF0
```

## Architecture Notes

### Integration with STDIO
- `printf()` is declared in `stdio.h`
- Uses global `extern STDIO stdio` instance
- Calls `stdio.print()` methods for screen output
- Respects screen dimensions (80×25 characters)

### Screen Output
- Screen memory: 0xb8000 (80×25 text mode)
- Each character: 2 bytes (character + color attribute)
- Automatic scrolling when bottom line reached
- Default color: 0x07 (white on black)

### Varargs Mechanism
The implementation uses macro-based argument extraction:
```c
#define _INTSIZEOF(n) ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap, v) (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap, type) (*(type *)((ap += _INTSIZEOF(type)) - _INTSIZEOF(type)))
```

## Compilation

```bash
cd /home/lht/dev/study/os/lab5
make compile-printf      # Compile printf implementation
make compile-all         # Compile all files including test
make clean              # Remove object files
```

## Performance Characteristics

- Time Complexity: O(n) where n is the length of format string + total argument size
- Space Complexity: O(1) - Uses fixed buffers
- Screen I/O: Buffered (32 characters) to reduce port I/O operations

## Known Limitations

1. No support for width specifiers (e.g., `%10d`)
2. No support for precision specifiers (e.g., `%.2f`)
3. Float format specifier (`%f`) not implemented
4. No support for long/short modifiers (`%ld`, `%hd`)
5. Hexadecimal output always uppercase (A-F)
6. Limited to 32-character buffer size

## Future Enhancements

- Add width and precision specifiers
- Implement floating-point support
- Add support for more format specifiers
- Improve performance with larger buffers
