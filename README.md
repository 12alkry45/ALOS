# ALOS

**A simple 32-bit operating system kernel**

ALOS is a small Unix based OS written in C and NASM. It runs in 32-bit protected mode and now includes a simply virtual memory management subsystem with paging and a kernel heap, alongside basic hardware drivers and interrupt handling. Now it have a Virtual Filesystem layer, simple RAMFS filesystem, and GRUB multiboot booting.

## Current Features

*   **Bootloader**: GRUB multiboot with 16KB stack, passes multiboot info and magic to kernel_main, grub.cfg with single menuentry, linker.ld for ELF layout, no more custom bootloader.
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
*   **VFS Layer**: unified filesystem interface supporting up to 8 mounted filesystems, vnode abstraction with ref counting and operation table, path resolution up to 256 characters, open file table with 32 descriptors tracking position and mode, errno error reporting, mount/unmount by filesystem name
*   **RAMFS Implementation**: in-memory filesystem with tree structure, full read/write/lookup/create/remove/rename/mkdir/rmdir/readdir implementation, registered as "ramfs", mountable at any path.
*   **Debugging & Testing**:
    *   GDB + QEMU integration for step-by-step debugging.
    *   Simple kernel `panic`.
    *   A `test/` directory for memory allocation tests (not automatic).
*   **Build System**: A `Makefile` that builds all object files into a dedicated `build/` directory.

## Project Structure

```
ALOS/
├── arch/ # Architecture-specific code (GDT, IDT, ISR, interrupt handlers)
├── boot/ # Multiboot header assembly
├── drivers/ # Hardware drivers (VGA, keyboard)
├── fs/ # Filesystem implementations (ramfs, VFS core)
├── include/ # All kernel headers (arch/, drivers/, fs/, kernel/, lib/, mm/)
├── init/ # Kernel initialization (paging, heap, VFS)
├── kernel/ # Kernel anwer to typing
├── lib/ # Standard library (string, printf, bitmap, ordered_array)
├── mm/ # Memory management (paging, kheap, frame allocator)
├── test/ # Kernel test suite
├── build/ # Compiled object files
├── Makefile
├── linker.ld # Linker script for ELF kernel
├── grub.cfg # GRUB boot configuration
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
Builds kernel.elf, places it in build/, GRUB loads it via multiboot.

## Recent Feature Deep-Dive

*   **GRUB bootloader**: grub bootloader replaced custom bootsector, kernel is now a proper ELF (creted ISO image), file loaded by multiboot, no more real mode code in boot path.
*   **Filesystem**: VFS and RAMFS provide filesystem abstraction with working file creation, reading, writing, renaming, directory operations all stored in memory.
*   **Error handling**: track errors with errno codes throughout VFS operations

## Setup Note

This kernel is compiled with a custom `i686-elf-gcc` cross-compiler. This ensures the final binary has no dependencies on your host OS's libc or runtime, producing a clean, standalone kernel image.

### Prerequisites

To build and run ALOS, your system needs the following tools:

*   **Cross-Compiler Toolchain**: `i686-elf-gcc`, `i686-elf-ld`, `i686-elf-ar`, and `i686-elf-gdb`.
    *   *Note*: The `Makefile` expects this toolchain to be installed at `~/opt/cross/bin/`.
*   **Assembler**: `nasm`
*   **Build Tools**: `make` and `i686-elf-grub-mkrescue` (part of the grub cross-compiled tools for creating the bootable ISO image).
*   **Emulator**: `qemu` (configured to use the `-display cocoa` interface for macOS).

