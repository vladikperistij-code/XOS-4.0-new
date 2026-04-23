CC = gcc
LD = ld
OBJCOPY = objcopy
AS = nasm
BIN = bin

# Шляхи до EFI (для завантажувача)
EFIINC = /usr/include/efi
OVMF = /usr/share/ovmf/OVMF.fd

# Прапорці
CFLAGS_EFI = -I$(EFIINC) -I$(EFIINC)/x86_64 -I$(EFIINC)/protocol \
             -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wall \
             -DEFI_FUNCTION_WRAPPER -Iboot

CFLAGS_KERN = -ffreestanding -fno-stack-protector -fno-pie -m64 -Wall -Iboot

# Список об'єктних файлів ядра
KERNEL_OBJS = $(BIN)/kernel.o $(BIN)/gpu.o $(BIN)/ram.o $(BIN)/cpu.o \
              $(BIN)/rtc.o $(BIN)/idt.o $(BIN)/keyboard.o $(BIN)/interrupts.o

all: prepare $(BIN)/bootx64.efi $(BIN)/kernel.bin

prepare:
	@mkdir -p "$(BIN)/iso/EFI/BOOT"

# Збірка завантажувача
$(BIN)/bootx64.efi: boot/boot.c boot/mini_kernel.c
	$(CC) $(CFLAGS_EFI) -c boot/boot.c -o $(BIN)/boot.o
	$(CC) $(CFLAGS_EFI) -c boot/mini_kernel.c -o $(BIN)/mini_kernel.o
	$(LD) -nostdlib -znocombreloc -T /usr/lib/elf_x86_64_efi.lds -shared -Bsymbolic \
		-L /usr/lib /usr/lib/crt0-efi-x86_64.o $(BIN)/boot.o $(BIN)/mini_kernel.o \
		-o $(BIN)/boot.so -lefi -lgnuefi
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .reloc \
		--target=efi-app-x86_64 $(BIN)/boot.so $@

# Компіляція Асемблерних переривань
$(BIN)/interrupts.o: kernel/drivers/interrupts.asm
	$(AS) -f elf64 kernel/drivers/interrupts.asm -o $(BIN)/interrupts.o

# Компіляція Сі файлів ядра та драйверів
$(BIN)/%.o: kernel/%.c
	$(CC) $(CFLAGS_KERN) -c $< -o $@

$(BIN)/gpu.o: kernel/drivers/GPU.c
	$(CC) $(CFLAGS_KERN) -c $< -o $@

$(BIN)/ram.o: kernel/drivers/RAM.c
	$(CC) $(CFLAGS_KERN) -c $< -o $@

$(BIN)/cpu.o: kernel/drivers/CPU.c
	$(CC) $(CFLAGS_KERN) -c $< -o $@

$(BIN)/rtc.o: kernel/drivers/RTC.c
	$(CC) $(CFLAGS_KERN) -c $< -o $@

$(BIN)/idt.o: kernel/drivers/IDT.c
	$(CC) $(CFLAGS_KERN) -c $< -o $@

$(BIN)/keyboard.o: kernel/drivers/Keyboard.c
	$(CC) $(CFLAGS_KERN) -c $< -o $@

# Лінковка всього ядра разом
$(BIN)/kernel.bin: $(KERNEL_OBJS)
	$(LD) -T kernel/linker.ld -static -nostdlib $(KERNEL_OBJS) -o $(BIN)/kernel.elf
	$(OBJCOPY) -O binary $(BIN)/kernel.elf $@

run: all
	cp "$(BIN)/bootx64.efi" "$(BIN)/iso/EFI/BOOT/BOOTX64.EFI"
	cp "$(BIN)/kernel.bin" "$(BIN)/iso/kernel.bin"
	qemu-system-x86_64 -bios $(OVMF) -drive file=fat:rw:"$(BIN)/iso",format=raw -net none -m 256M -d int -D qemu.log
clean:
	rm -rf $(BIN)
