all: run

kernel.o: kernel.c
	../../../opt/cross/bin/i686-elf-gcc --freestanding -c $< -o $@

kernel_entry.o: kernel_entry.asm
	nasm $< -f elf -o $@

kernel.bin: kernel_entry.o kernel.o
	../../../opt/cross/bin/i686-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

boot_sect_main.bin: boot_sect_main.asm
	nasm $< -f bin -o $@

os-image.bin: boot_sect_main.bin kernel.bin
	cat $^ > $@

run: os-image.bin
	qemu-system-x86_64 -fda $<

kernel.dis: kernel.bin
	ndisasm -b 32 $< > $@

clean:
	rm *.bin *.o *.dis
