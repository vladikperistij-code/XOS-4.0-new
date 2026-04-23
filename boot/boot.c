#include <efi.h>
#include <efilib.h>
#include "boot_info.h"

extern void mini_kernel_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, void* kernel_entry, BootInfo* binfo);

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
    Print(L"--- XOS 4.0 UEFI LOADER ---\n");

    // 1. Графіка
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);

    BootInfo binfo;
    binfo.FramebufferBase = (unsigned int*)gop->Mode->FrameBufferBase;
    binfo.FramebufferSize = gop->Mode->FrameBufferSize;
    binfo.ScreenWidth = gop->Mode->Info->HorizontalResolution;
    binfo.ScreenHeight = gop->Mode->Info->VerticalResolution;
    binfo.PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;

    // 2. Завантаження ядра
    EFI_LOADED_IMAGE_PROTOCOL *loaded_image;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *filesystem;
    EFI_FILE_PROTOCOL *root, *kernel_file;
    uefi_call_wrapper(BS->HandleProtocol, 3, ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&loaded_image);
    uefi_call_wrapper(BS->HandleProtocol, 3, loaded_image->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&filesystem);
    uefi_call_wrapper(filesystem->OpenVolume, 2, filesystem, &root);
    
    if (uefi_call_wrapper(root->Open, 5, root, &kernel_file, L"kernel.bin", EFI_FILE_MODE_READ, 0) != EFI_SUCCESS) {
        Print(L"kernel.bin not found!\n"); return EFI_NOT_FOUND;
    }

    UINTN kernel_size = 0x100000; 
    EFI_PHYSICAL_ADDRESS kernel_addr = 0x1000000; 
    uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, (kernel_size/4096)+1, &kernel_addr);
    uefi_call_wrapper(kernel_file->Read, 3, kernel_file, &kernel_size, (void*)kernel_addr);

    // 3. Отримання карти пам'яті (Memory Map)
    UINTN MapKey;
    UINTN MemoryMapSize = 0;
    EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    // Спочатку дізнаємось розмір
    uefi_call_wrapper(BS->GetMemoryMap, 5, &MemoryMapSize, NULL, &MapKey, &DescriptorSize, &DescriptorVersion);
    MemoryMapSize += 2 * DescriptorSize;
    uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, MemoryMapSize, (VOID**)&MemoryMap);
    // Отримуємо реальні дані
    uefi_call_wrapper(BS->GetMemoryMap, 5, &MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);

    // Записуємо в структуру для ядра
    binfo.MemoryMap = MemoryMap;
    binfo.MemoryMapSize = MemoryMapSize;

    Print(L"Ready to jump. Finalizing...\n");

    // 4. Перехід у mini_kernel
    mini_kernel_main(ImageHandle, SystemTable, (void*)kernel_addr, &binfo);

    return EFI_SUCCESS;
}
