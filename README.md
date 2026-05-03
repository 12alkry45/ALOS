# ALOS

**A simple 32-bit operating system kernel**

ALOS is a small Unix based OS written in C and NASM. It runs in 32-bit protected mode and now includes a simply virtual memory management subsystem with paging and a kernel heap, alongside basic hardware drivers and interrupt handling.

## Current Features

*   **Bootloader**: Transitions from 16-bit real mode to 32-bit protected mode.
*   **Interrupt Handling**:
    *   Custom IDT setup and PIC remapping.
    *   Handlers for CPU exceptions (e.g., Page Faults) and hardware IRQs (Timer, Keyboard).
*   **Memory Management**:
    *   **Paging**: Identity mapping for the first memory, with support for allocating and freeing frames via a bitset.
    *   **Kernel Heap**: A `kheap` implementation supporting `kmalloc` and `kfree`, managed through an ordered array of holes.
    *   **Standard Library for Memory**: `memcpy` and `memset` in assembly, plus `kmalloc` returning a pointer to memory.
*   **Hardware Drivers**:
    *   **VGA Text Mode Driver**: For basic screen output with scrolling.
    *   **Keyboard Driver**: Reads scancodes and decodes basic key presses/releases.
*   **Standard Library**: `printf`-like formatted printing, `atoi` with base support, and robust string utilities.
*   **Debugging & Testing**:
    *   GDB + QEMU integration for step-by-step debugging.
    *   Simple kernel `panic`.
    *   A `test/` directory for memory allocation tests (not automatic).
*   **Build System**: A `Makefile` that builds all object files into a dedicated `build/` directory.

## Project Structure

```
ALOS/
├── arch/           # Architecture-specific code (former cpu/)
├── boot/           # Bootsector code
├── drivers/        # Hardware drivers (VGA, Keyboard)
├── include/        # All kernel header files (.h)
├── init/           # Kernel initialization routine
├── kernel/         # Core kernel logic (simple echo shell)
├── lib/            # Libc-like utilities (string, printf, bitmap, ordered_array)
├── mm/             # Memory management (paging, kheap, frame)
├── test/           # Kernel test suite (e.g., memory allocation tests)
├── build/          # Output directory for compiled object files
├── Makefile
└── README.md
```

## How to Build and Run

### Prerequisites

You need the `i686-elf` cross-compiler toolchain installed. The Makefile expects it to be in your PATH.

**To build the kernel and run it in QEMU:**
```bash
make run
```

### Debugging

**To build, launch QEMU, and attach GDB automatically:**
```bash
make debug
```
The build system will compile the kernel with debug symbols.

## Recent Feature Deep-Dive

Massive leap in core functionality:

*   **Memory management leap**: The kernel now manages physical memory with a bitset allocator and frames allocation and has a fully functional virtual memory area. The `kheap` allocator uses a best-fit policy with proper hole splitting and coalescing, tracked by a sorted array.
*   **Robust error handling**: A `panic` function has been introduced. When a fatal error like a double fault or a failed assertion is hit, the system halts cleanly and prints diagnostic information, rather than silently crashing.
*   **Formatted output**: The addition of `printf` makes it significantly easier to write debug logs, display complex information, and build out the user interface.
*   **Refactoring for growth**: The architecture-dependent code was moved from `cpu/` to `arch/` for clarity. All headers are centralized in `include/`, giving the project a polished, proper structure and simplifying module interaction.
*   **Some testing**: Memory allocation and freeing test (not automatic)

## Setup Note

This kernel is compiled with a custom `i686-elf-gcc` cross-compiler. This ensures the final binary has no dependencies on your host OS's libc or runtime, producing a clean, standalone kernel image.
