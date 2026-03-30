# Copilot Instructions for OS Lab 2

This repository contains operating system lab materials and assignments, focusing on boot loaders, UEFI applications, and x86/x86_64 assembly programming.

## Repository Structure

```
lab2/
├── assignment.pdf          # Lab assignment specifications (44 pages)
├── materials/
│   ├── assignment/         # Lab assignment code templates
│   │   ├── makefile        # Builds 32-bit x86 asm/C++ hybrid test
│   │   ├── test.cpp        # C++ test harness (do not modify)
│   │   ├── student.asm      # Assembly student submission
│   │   ├── head.include    # ASM boilerplate (global declarations)
│   │   └── end.include     # ASM boilerplate (function wrappers)
│   ├── uefi/               # UEFI bootloader example
│   │   ├── Makefile        # Builds UEFI EFI executable
│   │   └── example1.c      # UEFI application example
│   ├── src/                # Boot sector resources
│   │   ├── mbr.asm         # Master Boot Record example
│   │   └── hd.img          # Hard disk image (10MB)
│   ├── uefi.md             # UEFI educational documentation
│   ├── gallery/            # Screenshots/diagrams
│   └── make-os-from-scratch-cpf.tar.gz  # Reference materials archive
```

## Build Commands

### Assignment (32-bit x86 asm + C++)
```bash
cd materials/assignment
make run        # Compile, link, and execute test (nasm + g++ 32-bit)
make clean      # Remove build artifacts
```

**Key Details:**
- Uses NASM for x86-32 assembly (`nasm -f elf32`)
- Uses g++ for 32-bit C++ (`g++ -m32`)
- Requires: `gcc-multilib g++-multilib` packages (run `sudo apt install` if needed)
- Links assembly object files with C++ object files
- No traditional "tests" — execution is the validation

### UEFI Application
```bash
cd materials/uefi
make            # Builds example1.efi (UEFI executable)
```

**Key Details:**
- Uses GNU-EFI toolchain (via posix-uefi)
- Generates .efi files (EFI PE32+ executables)
- Requires UEFI bootloader environment to run

## Code Architecture

### Assignment Pattern (asm/C++ Bridge)

The assignment demonstrates x86-32 low-level control with C++ scaffolding:

1. **C++ Test Harness** (`test.cpp`)
   - Defines external C functions to be implemented in assembly
   - `extern "C" void your_function();` — main function to implement
   - `extern "C" void student_function();` — optional helper
   - Sets up data structures and calls assembly code

2. **Assembly Template** (`student.asm`)
   - Linked with C++ as object files
   - Must preserve ABI: push/pop callee-saved registers
   - Access C++ globals via `extern` keyword (a1, a2, if_flag, while_flag)
   - Can call C++ functions (like `my_random()`)

3. **Boilerplate Includes**
   - `head.include`: Global declarations, external variable imports
   - `end.include`: Function wrapper setup, calling convention definitions

**Example Build Flow:**
```
student.asm → (nasm -f elf32) → student.o
test.cpp    → (g++ -m32 -c)    → test.o
student.o + test.o → (g++ -m32 -o) → test (executable)
./test → output
```

### UEFI Application Pattern

UEFI applications differ from traditional OS apps:
- Entry point: `efi_status_t efi_main(efi_handle_t image, efi_system_table_t *st)` (not `main`)
- Access UEFI services through `SystemTable` parameter
- Compiled as Position-Independent Executable (PIE) in PE/COFF format (.efi)
- No libc; use UEFI-provided functions for I/O, memory, graphics

## Key Conventions

### x86-32 Assembly (Lab Assignment)

**Calling Convention:** cdecl (C calling convention for 32-bit)
- Arguments passed on stack (right-to-left)
- Return values in EAX (32-bit) or EDX:EAX (64-bit)
- Caller cleans up stack
- Callee must preserve: EBX, ESI, EDI, EBP, ESP

**Register Usage Pattern:**
```asm
your_function:
    push ebp            ; save base pointer
    mov ebp, esp        ; set up stack frame
    pushad              ; save all registers
    
    ; your implementation
    
    popad               ; restore all registers
    pop ebp
    ret
```

**Accessing C++ Variables:**
```asm
extern a1, a2           ; extern declarations
mov eax, [a1]          ; access global
mov dword [a1], 42     ; modify global
```

### UEFI Application Pattern

**Include Headers:**
```c
#include <uefi.h>       /* Standard UEFI API */
```

**Key UEFI Services (via SystemTable):**
- `ST->ConOut->OutputString()` — Print text
- `ST->ConOut->QueryMode()` / `SetMode()` — Video mode control
- `ST->BootServices->LocateProtocol()` — Find device protocols
- Graphics Output Protocol (GOP) for pixel graphics

## Debugging Notes

- **32-bit Assembly Issues:** If compilation fails with `Error: bits mismatch`, ensure NASM format is `-f elf32` (32-bit, not 64-bit)
- **Linker Errors:** Missing external symbols usually mean declarations don't match between .cpp and .asm files (check `extern` keywords)
- **Multilib Missing:** UEFI builds fail without 32-bit libraries—install `gcc-multilib g++-multilib`
- **UEFI Runtime:** EFI applications require a UEFI firmware environment (QEMU with OVMF, physical UEFI machine, or simulator)

## Educational Context

This lab teaches fundamental OS concepts:
1. **Boot Process:** MBR, bootloaders, firmware/kernel handoff
2. **Low-Level Programming:** x86 assembly, hardware initialization
3. **Firmware Interfaces:** BIOS vs. UEFI, system calls, interrupt vectors
4. **Hardware Abstraction:** How bootloaders discover memory, devices, and configuration

Reference materials in `materials/make-os-from-scratch-cpf.tar.gz` include working examples for each stage.
