#include <efi.h>
#include <efilib.h>
#include "boot_info.h"

typedef void (*KernelEntry)(BootInfo*);

void mini_kernel_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, void* kernel_entry, BootInfo* binfo) {
    UINTN MapKey;
    UINTN MemMapSize = binfo->MemoryMapSize;
    EFI_MEMORY_DESCRIPTOR *MemMap = (EFI_MEMORY_DESCRIPTOR*)binfo->MemoryMap;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    // Нам все одно треба актуальний MapKey для виходу
    uefi_call_wrapper(BS->GetMemoryMap, 5, &MemMapSize, MemMap, &MapKey, &DescriptorSize, &DescriptorVersion);

    // EXIT BOOT SERVICES
    if (uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey) != EFI_SUCCESS) {
        // Якщо не вийшло, пробуємо ще раз (стандарт UEFI)
        uefi_call_wrapper(BS->GetMemoryMap, 5, &MemMapSize, MemMap, &MapKey, &DescriptorSize, &DescriptorVersion);
        uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey);
    }

    // СТРИБОК В ЯДРО
    KernelEntry start = (KernelEntry)kernel_entry;
    start(binfo);

    while(1) { __asm__ volatile ("hlt"); }
}
