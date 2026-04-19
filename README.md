# ALOS

A simple operating system based on UNIX concepts, written in **NASM** and **C**.  

## Current Features

- **Hybrid Bootloader**: 16-bit real mode loader → 32-bit protected mode
- **Interrupt Handling**:
  - IDT (Interrupt Descriptor Table) setup
  - PIC (Programmable Interrupt Controller) remapping and handling
  - IRQ0 (timer interrupt) — basic tick handling
  - IRQ1 (keyboard interrupt) — key press/release handling
- **Drivers**:
  - VGA text mode driver (basic screen output)
- **Standard Library** (early stage):
  - Memory operations (`memory_copy`, `memory_set`)
  - String utilities (`strlen`, `strcmp`, `strcpy`, converting int and hex numbers)
- **Debug**: Integrated with GDB and QEMU

## Project Structure

```
ALOS/
├── boot/           # Bootsector and GDT initialization
├── cpu/            # IDT, interrupt handlers, ISR, timer
├── drivers/        # Hardware drivers (VGA, keyboard)
├── kernel/         # Main OS logic (kernel_main)
├── lib/            # Basic memory and string functions
├── Makefile
└── README.md
```

## How to Build

Make sure you have the `i686-elf` toolchain installed at `~/opt/cross/bin/`.

```bash
make run
```

## Debug

Launches QEMU and attaches GDB automatically:

```bash
make debug
```

## Recent Additions

- Full basic interrupt handling
- PIC remapping (IRQs 0–15 to IDT entries 32–47)
- Timer interrupt (IRQ0)
- Keyboard interrupt (IRQ1) — scancode reading and basic key handling
- `lib/` directory with memory and string helper functions
- basic VGA driver


## Setup Note

The project uses a custom cross-compiler (`i686-elf-gcc`) to ensure the kernel is built without 
dependencies on the host operating system.
