#include "fs.h"
#include "../drivers/vga/vga.h"
#include "../utils/mem.h"

static File files[MAX_FILES];

void fs_init() {
    for (int i = 0; i < MAX_FILES; i++) {
        files[i].used = 0;
    }
}

int fs_create(const char* name) {
    //чи файл з таким іменем уже існує
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, name) == 0) {
            return -1; // файл уже існує
        }
    }

    //не існує — створюємо новий
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].used) {
            strncpy(files[i].name, name, MAX_FILENAME);
            files[i].content[0] = '\0';
            files[i].used = 1;
            return 1; //створено
        }
    }

    return 0; // немає місця
}

int fs_write(const char* name, const char* text) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, name) == 0) {
            strncat(files[i].content, text, MAX_CONTENT_SIZE - strlen(files[i].content) - 1);
            return 1;
        }
    }
    return 0; // file not found
}

const char* fs_read(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, name) == 0) {
            return files[i].content;
        }
    }
    return NULL;
}

int fs_delete(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, name) == 0) {
            files[i].used = 0;
            return 1;
        }
    }
    return 0;
}

void fs_list() {
    vga_println("Files:");
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used)
            vga_println(files[i].name);
    }
}
