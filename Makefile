C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c lib/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h lib/*.h)
OBJ = ${C_SOURCES:.c=.o cpu/interrupt.o}

CC = /home/alesha/opt/cross/bin/i686-elf-gcc
LD = /home/alesha/opt/cross/bin/i686-elf-ld
GDB = /home/alesha/opt/cross/bin/i686-elf-gdb
CFLAGS = -g

all: run

kernel.bin: boot/kernel_entry.o ${OBJ}
	${LD} -o $@ -Ttext 0x1000 $^ --oformat binary

kernel.elf: boot/kernel_entry.o ${OBJ}
	${LD} -o $@ -Ttext 0x1000 $^

os-image.bin: boot/bootsect.bin kernel.bin
	cat $^ > $@

run: os-image.bin
	qemu-system-x86_64 -fda $<

kernel.dis: kernel.bin
	ndisasm -b 32 $< > $@

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -ffreestanding -c $< -o $@

%.bin: %.asm
	nasm $< -f bin -o $@

%.o: %.asm
	nasm $< -f elf -o $@

debug: os-image.bin kernel.elf
	qemu-system-i386 -s -fda os-image.bin &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

clean:
	rm -rf *.bin *.o *.dis os-image.bin *.elf
	rm -rf boot/*.bin boot/*.o kernel/*.o drivers/*.o cpu/*.o lib/*.o
