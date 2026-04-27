/**
 * Simple test program to verify printf implementation
 * This tests each format specifier independently
 */

#include "stdio.h"
#include "os_type.h"

// Declare the global STDIO instance
STDIO stdio;

// Helper: Test a single format specifier
void test_format(const char *description, const char *fmt, ...)
{
    printf("Test: %s\n", description);
    printf("Result: ");
    
    // This is tricky without proper varargs forwarding
    // For now, we'll test each one separately
}

int main()
{
    // Initialize STDIO
    stdio.initialize();
    
    printf("=== Printf Implementation Test Suite ===\n");
    printf("\n");
    
    // Test 1: Literal percent
    printf("Test 1 - Literal %%: %%\n");
    
    // Test 2: Character
    printf("Test 2 - Character 'N': %c\n", 'N');
    
    // Test 3: String
    printf("Test 3 - String: %s\n", "Hello World!");
    
    // Test 4: Negative decimal
    printf("Test 4 - Negative decimal: %d\n", -1234);
    
    // Test 5: Positive decimal
    printf("Test 5 - Positive decimal: %d\n", 5678);
    
    // Test 6: Hexadecimal
    printf("Test 6 - Hexadecimal: %x\n", 0x7abcdef0);
    
    // Test 7: Multiple arguments
    printf("Test 7 - Multiple args: char=%c, string=%s, dec=%d, hex=%x\n", 
           'X', "Test", -42, 0xDEADBEEF);
    
    // Test 8: Combined (like the actual test case)
    printf("\nCombined Test (from setup.cpp):\n");
    printf("print percentage: %%\n"
           "print char \"N\": %c\n"
           "print string \"Hello World!\": %s\n"
           "print decimal: \"-1234\": %d\n"
           "print hexadecimal \"0x7abcdef0\": %x\n",
           'N', "Hello World!", -1234, 0x7abcdef0);
    
    printf("\n=== All Tests Complete ===\n");
    
    return 0;
}
