#include "xfs.h"
#include "ata.h"
#include "../kernel/drivers/GPU.h"

xfs_file_t filesystem[MAX_FILES];

static int xfs_strcmp(const char* a, const char* b) {
    while (*a && *a == *b) {
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static void xfs_strcpy(char* dst, const char* src) {
    while ((*dst++ = *src++));
}

static void xfs_itoa(unsigned int value, char* buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    int len = 0;
    unsigned int temp = value;
    while (temp > 0) {
        buf[len++] = '0' + (temp % 10);
        temp /= 10;
    }

    for (int i = 0; i < len / 2; i++) {
        char c = buf[i];
        buf[i] = buf[len - 1 - i];
        buf[len - 1 - i] = c;
    }
    buf[len] = '\0';
}

void xfs_init() {
    for (int i = 0; i < MAX_FILES; i++) filesystem[i].used = 0;
}

int xfs_create(char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem[i].used == 0) {
            xfs_strcpy(filesystem[i].name, name);
            filesystem[i].used = 1;
            filesystem[i].size = 0;
            return i;
        }
    }
    return -1;
}

int xfs_write(char* name, char* content) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem[i].used && xfs_strcmp(filesystem[i].name, name) == 0) {
            int j = 0;
            while (content[j] != '\0' && j < MAX_FILE_SIZE - 1) {
                filesystem[i].data[j] = content[j];
                j++;
            }
            filesystem[i].data[j] = '\0';
            filesystem[i].size = j;
            return 0;
        }
    }
    return -1;
}

int xfs_find(char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem[i].used && xfs_strcmp(filesystem[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int xfs_count() {
    int count = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem[i].used) count++;
    }
    return count;
}

const char* xfs_get_name(int index) {
    if (index < 0 || index >= MAX_FILES) return "";
    return filesystem[index].name;
}

int xfs_get_size(int index) {
    if (index < 0 || index >= MAX_FILES) return 0;
    return filesystem[index].used ? filesystem[index].size : 0;
}

char* xfs_read_by_index(int index) {
    if (index < 0 || index >= MAX_FILES) return 0;
    return filesystem[index].used ? filesystem[index].data : 0;
}

int xfs_delete_index(int index) {
    if (index < 0 || index >= MAX_FILES) return -1;
    if (!filesystem[index].used) return -1;
    filesystem[index].used = 0;
    xfs_sync();
    return 0;
}

char* xfs_read(char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem[i].used && xfs_strcmp(filesystem[i].name, name) == 0) {
            return filesystem[i].data;
        }
    }
    return 0;
}

void xfs_list() {
    draw_string_scaled("--- XFS Files ---", 15, 110, 0xFFFFFFFF, 1);
    int y = 125;
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem[i].used) {
            draw_string_scaled(filesystem[i].name, 15, y, 0x00FFD700, 1);
            y += 15;
        }
    }
}

void xfs_remove(char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem[i].used && xfs_strcmp(filesystem[i].name, name) == 0) {
            filesystem[i].used = 0;
            xfs_sync();
            return;
        }
    }
}

void xfs_sync() {
    for (int i = 0; i < MAX_FILES; i++) {
        ata_write_sector(300 + i, (unsigned short*)&filesystem[i]);
    }
}

void xfs_load() {
    for (int i = 0; i < MAX_FILES; i++) {
        ata_read_sector(300 + i, (unsigned short*)&filesystem[i]);
    }
}

void xfs_print_info() {
    int used_count = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem[i].used) used_count++;
    }

    char used_buf[16];
    xfs_itoa(used_count, used_buf);

    draw_string_scaled("Disk size:", 15, 75, 0xFFFFFFFF, 1);
    draw_string_scaled("4 KB (8 sectors)", 95, 75, 0x00FFD700, 1);
    draw_string_scaled("Used files:", 15, 90, 0xFFFFFFFF, 1);
    draw_string_scaled(used_buf, 105, 90, 0x00FFD700, 1);
    draw_string_scaled("/8", 130, 90, 0xFFFFFFFF, 1);
}
