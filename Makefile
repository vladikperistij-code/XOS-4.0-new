# --- Компілятори та інструменти ---
CC = gcc
LD = ld
OBJCOPY = objcopy
AS = nasm

# --- Шляхи ---
BIN = bin
ISO_DIR = $(BIN)/iso
EFI_BOOT = $(ISO_DIR)/EFI/BOOT

# Шлях до OVMF (для Arch/Ubuntu/Debian)
OVMF = /usr/share/ovmf/OVMF.fd
# Якщо на Arch, зазвичай: /usr/share/edk2-ovmf/x64/OVMF_CODE.fd

# --- Прапорці компіляції ---
EFIINC = /usr/include/efi
CFLAGS_EFI = -I$(EFIINC) -I$(EFIINC)/x86_64 -I$(EFIINC)/protocol \
             -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wall \
             -DEFI_FUNCTION_WRAPPER -Iboot

CFLAGS_KERN = -ffreestanding -fno-stack-protector -fno-pie -m64 -Wall -Iboot -Ikernel/drivers

# --- Об'єкти ядра (ВАЖЛИВИЙ ПОРЯДОК) ---
KERNEL_OBJS = $(BIN)/kernel.o $(BIN)/gpu.o $(BIN)/ram.o $(BIN)/cpu.o \
              $(BIN)/rtc.o $(BIN)/idt.o $(BIN)/keyboard.o $(BIN)/mouse.o $(BIN)/interrupts.o

# --- Головні цілі ---
all: prepare $(BIN)/bootx64.efi $(BIN)/kernel.bin

prepare:
	@mkdir -p "$(EFI_BOOT)"
	@mkdir -p "$(BIN)"

# 1. Збірка завантажувача (UEFI)
$(BIN)/bootx64.efi: boot/boot.c boot/mini_kernel.c
	$(CC) $(CFLAGS_EFI) -c boot/boot.c -o $(BIN)/boot.o
	$(CC) $(CFLAGS_EFI) -c boot/mini_kernel.c -o $(BIN)/mini_kernel.o
	$(LD) -nostdlib -znocombreloc -T /usr/lib/elf_x86_64_efi.lds -shared -Bsymbolic \
		-L /usr/lib /usr/lib/crt0-efi-x86_64.o $(BIN)/boot.o $(BIN)/mini_kernel.o \
		-o $(BIN)/boot.so -lefi -lgnuefi
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .reloc \
		--target=efi-app-x86_64 $(BIN)/boot.so $@

# 2. Збірка Асемблерних переривань
$(BIN)/interrupts.o: kernel/drivers/interrupts.asm
	$(AS) -f elf64 kernel/drivers/interrupts.asm -o $@

# 3. Збірка драйверів та ядра
$(BIN)/kernel.o: kernel/kernel.c
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

$(BIN)/mouse.o: kernel/drivers/Mouse.c
	$(CC) $(CFLAGS_KERN) -c $< -o $@

# 4. Лінкування ядра (в чистому бінарному форматі)
$(BIN)/kernel.bin: $(KERNEL_OBJS)
	$(LD) -T kernel/linker.ld -static -nostdlib $(KERNEL_OBJS) -o $(BIN)/kernel.elf
	$(OBJCOPY) -O binary $(BIN)/kernel.elf $@

# --- Запуск та очищення ---
run: all
	cp "$(BIN)/bootx64.efi" "$(EFI_BOOT)/BOOTX64.EFI"
	cp "$(BIN)/kernel.bin" "$(ISO_DIR)/kernel.bin"
	qemu-system-x86_64 -bios $(OVMF) -drive file=fat:rw:"$(ISO_DIR)",format=raw -net none -m 256M

clean:
	rm -rf $(BIN)
