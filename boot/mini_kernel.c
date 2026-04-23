#include <efi.h>
#include <efilib.h>
#include "boot_info.h"

typedef void (*KernelEntry)(BootInfo*);

void mini_kernel_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, void* kernel_entry, BootInfo* binfo) {
    UINTN MapKey;
    UINTN MemMapSize = 0;
    EFI_MEMORY_DESCRIPTOR *MemMap = NULL;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
    EFI_STATUS status;

    // 1. Отримуємо останній актуальний MapKey перед самим виходом
    status = uefi_call_wrapper(BS->GetMemoryMap, 5, &MemMapSize, NULL, &MapKey, &DescriptorSize, &DescriptorVersion);
    MemMapSize += 2 * DescriptorSize;
    uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, MemMapSize, (VOID**)&MemMap);
    uefi_call_wrapper(BS->GetMemoryMap, 5, &MemMapSize, MemMap, &MapKey, &DescriptorSize, &DescriptorVersion);

    // 2. ВИХІД — тепер UEFI віддає контроль ядру
    status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey);
    
    if (status != EFI_SUCCESS) {
        // Якщо не вийшло з першого разу, пробуємо востаннє (це стандарт UEFI)
        uefi_call_wrapper(BS->GetMemoryMap, 5, &MemMapSize, MemMap, &MapKey, &DescriptorSize, &DescriptorVersion);
        uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey);
    }

    // 3. СТРИБОК — назад вороття немає!
    KernelEntry start = (KernelEntry)kernel_entry;
    start(binfo);

    // Ми ніколи не маємо сюди повернутися
    while(1) { __asm__ volatile ("hlt"); }
}
