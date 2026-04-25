CROSS = /home/alesha/opt/cross/bin/i686-elf-
CC = $(CROSS)gcc
LD = $(CROSS)ld
GDB = $(CROSS)gdb
AR = $(CROSS)ar

CFLAGS = -g -ffreestanding -Wall -Wextra -fno-exceptions -m32
CFLAGS += -I.
CFLAGS += -MMD -MP

LDFLAGS = -Ttext 0x1000

BUILD_DIR = build

C_DIRS = kernel drivers arch lib mm init
C_SOURCES = $(foreach dir,$(C_DIRS),$(wildcard $(dir)/*.c))

ASM_SOURCES = $(wildcard boot/kernel_entry.asm arch/*.asm)
BOOTSECT_ASM = $(wildcard boot/bootsect.asm)

obj_from_c = $(patsubst %.c,$(BUILD_DIR)/%.o,$(1))
obj_from_asm = $(patsubst %.asm,$(BUILD_DIR)/%.o,$(1))
bin_from_asm = $(patsubst %.asm,$(BUILD_DIR)/%.bin,$(1))

C_OBJS = $(call obj_from_c,$(C_SOURCES))
ASM_OBJS = $(call obj_from_asm,$(ASM_SOURCES))
BOOT_BIN = $(call bin_from_asm,$(BOOTSECT_ASM))

KERNEL_OBJS = $(ASM_OBJS) $(C_OBJS)
C_DEPS = $(C_OBJS:.o=.d)

LIB_SOURCES = $(wildcard lib/*.c)
LIB_OBJS = $(call obj_from_c,$(LIB_SOURCES))
LIB_TARGET = $(BUILD_DIR)/lib/lib.a

KERNEL_BIN = $(BUILD_DIR)/kernel.bin
KERNEL_ELF = $(BUILD_DIR)/kernel.elf 
OS_IMAGE = $(BUILD_DIR)/os-image.bin

-include $(C_DEPS)

.PHONY: all run debug clean

all: $(OS_IMAGE)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.bin: %.asm
	@mkdir -p $(@D)
	nasm $< -f bin -o $@

$(BUILD_DIR)/%.o: %.asm
	@mkdir -p $(@D)
	nasm $< -f elf -o $@

$(LIB_TARGET): $(LIB_OBJS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^

$(KERNEL_BIN): $(KERNEL_OBJS) $(LIB_TARGET)
	@mkdir -p $(@D)
	$(LD) $(LDFLAGS) --oformat binary -o $@ $^

$(KERNEL_ELF): $(KERNEL_OBJS) $(LIB_TARGET)
	@mkdir -p $(@D)
	$(LD) $(LDFLAGS) -o $@ $^

$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	@mkdir -p $(@D)
	cat $^ > $@

run: $(OS_IMAGE)
	qemu-system-x86_64 -fda $<

kernel.dis: $(KERNEL_BIN)
	ndisasm -b 32 $< > $@

debug: $(OS_IMAGE) $(KERNEL_ELF)
	qemu-system-i386 -s -fda $(OS_IMAGE) -d guest_errors,int &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file $(KERNEL_ELF)"

clean:
	rm -rf $(BUILD_DIR)
