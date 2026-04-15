# ALOS
A simple operating system based on UNIX concepts, written in **NASM** and **C**.

## Current Features
- **Hybrid Bootloader**: 16-bit real mode loader transitioning into 32-bit protected mode.
- **Kernel Entry**: Hand-written Assembly bridge to the C kernel.
- **Debug Ready**: Integrated with GDB and QEMU for low-level debugging.

## Project Structure
- `boot/`: Bootsector and GDT initialization.
- `kernel/`: Main OS logic.
- `drivers/`: Hardware communication (VGA, etc.).

## How to Build
Make sure you have the `i686-elf` toolchain installed at `~/opt/cross/bin/`.

### Run
```bash
make run
```

## Debug
This will launch QEMU and attach GDB automatically:
```bash
make debug
```

## Setup Note
The project uses a custom cross-compiler (i686-elf-gcc) to ensure the kernel is built without dependencies on the host operating system.
