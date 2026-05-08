#ifndef XFS_H
#define XFS_H

#define MAX_FILES 8
#define MAX_FILENAME 32
#define MAX_FILE_SIZE 468 // 32(name) + 4(size) + 4(used) + 4(unused) + 468(data) = 512

typedef struct {
    char name[MAX_FILENAME];
    int size;
    int used;
    int reserved; 
    char data[MAX_FILE_SIZE];
} xfs_file_t;

extern xfs_file_t filesystem[MAX_FILES];

void xfs_init();
int xfs_create(char* name);
int xfs_write(char* name, char* content);
int xfs_find(char* name);
int xfs_count();
const char* xfs_get_name(int index);
int xfs_get_size(int index);
char* xfs_read_by_index(int index);
int xfs_delete_index(int index);
char* xfs_read(char* name);
void xfs_list();
void xfs_sync();
void xfs_load();
void xfs_remove(char* name);
void xfs_print_info();

#endif