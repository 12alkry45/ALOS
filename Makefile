CROSS = ~/opt/cross/bin/i686-elf-
CC = $(CROSS)gcc
LD = $(CROSS)ld
GDB = $(CROSS)gdb
AR = $(CROSS)ar

CFLAGS = -O0 -g -ffreestanding -Wall -Wextra -fno-exceptions -m32
CFLAGS += -I include
CFLAGS += -MMD -MP

LDFLAGS = -T linker.ld -m elf_i386

BUILD_DIR = build

C_DIRS = kernel drivers arch lib mm init include test fs
C_SOURCES = $(foreach dir,$(C_DIRS),$(wildcard $(dir)/*.c))

ASM_SOURCES = $(wildcard arch/*.asm boot/boot.asm )

obj_from_c = $(patsubst %.c,$(BUILD_DIR)/%.o,$(1))
obj_from_asm = $(patsubst %.asm,$(BUILD_DIR)/%.o,$(1))

C_OBJS = $(call obj_from_c,$(C_SOURCES))
ASM_OBJS = $(call obj_from_asm,$(ASM_SOURCES))

KERNEL_OBJS = $(ASM_OBJS) $(C_OBJS)
C_DEPS = $(C_OBJS:.o=.d)

LIB_SOURCES = $(wildcard lib/*.c)
LIB_OBJS = $(call obj_from_c,$(LIB_SOURCES))
LIB_TARGET = $(BUILD_DIR)/lib/lib.a

KERNEL_ELF = $(BUILD_DIR)/kernel.elf 
ISO_IMAGE = $(BUILD_DIR)/os-image.iso

-include $(C_DEPS)

.PHONY: all run debug clean

all: $(ISO_IMAGE)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.asm
	@mkdir -p $(@D)
	nasm $< -f elf -o $@

$(LIB_TARGET): $(LIB_OBJS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^

$(KERNEL_ELF): $(KERNEL_OBJS) $(LIB_TARGET)
	@mkdir -p $(@D)
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS) $(LIB_TARGET)

$(ISO_IMAGE) : $(KERNEL_ELF)
	@mkdir -p $(BUILD_DIR)/iso/boot/grub
	cp $(KERNEL_ELF) $(BUILD_DIR)/iso/boot/kernel.elf
	cp grub.cfg $(BUILD_DIR)/iso/boot/grub/grub.cfg
	i686-elf-grub-mkrescue -o $@ $(BUILD_DIR)/iso 2>/dev/null
	@echo "ISO is created: $@"

run: $(ISO_IMAGE)
	qemu-system-i386 -cdrom $< -display cocoa,zoom-to-fit=on

kernel.dis: $(KERNEL_ELF)
	objdump -d -M intel $< > $@

debug: $(ISO_IMAGE) $(KERNEL_ELF)
	qemu-system-i386 -s -S -cdrom $(ISO_IMAGE) -d guest_errors,int -display cocoa,zoom-to-fit=on &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file $(KERNEL_ELF)"

clean:
	rm -rf $(BUILD_DIR)
