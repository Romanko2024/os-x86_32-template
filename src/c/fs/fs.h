#ifndef FS_H
#define FS_H

#define MAX_FILES 10
#define MAX_CONTENT_SIZE 2000
#define MAX_FILENAME 32

typedef struct {
    char name[MAX_FILENAME];
    char content[MAX_CONTENT_SIZE];
    int used;
} File;

// API файлової системи
void fs_init();
int fs_create(const char* name);
int fs_write(const char* name, const char* text);
const char* fs_read(const char* name);
int fs_delete(const char* name);
void fs_list();

#endif