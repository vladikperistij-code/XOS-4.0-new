#ifndef BOOT_INFO_H
#define BOOT_INFO_H

typedef struct {
    unsigned int* FramebufferBase;
    unsigned long long FramebufferSize;
    unsigned int ScreenWidth;
    unsigned int ScreenHeight;
    unsigned int PixelsPerScanLine;
    // ДОДАЙ ЦІ ДВА РЯДКИ:
    void* MemoryMap;
    unsigned long long MemoryMapSize;
} BootInfo;

#endif
