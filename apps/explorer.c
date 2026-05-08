#include "explorer.h"
#include "../xfs/xfs.h"
#include "../kernel/drivers/GPU.h"

#include "../kernel/kernel.h"

#define LIST_X (EXPLORER_X + 10)
#define LIST_Y (EXPLORER_Y + 32)
#define LIST_W 180
#define LIST_H 180
#define ROW_H 22

#define NAME_X (EXPLORER_X + 210)
#define NAME_Y (EXPLORER_Y + 32)
#define NAME_W 200
#define NAME_H 20

#define CONTENT_X (EXPLORER_X + 210)
#define CONTENT_Y (EXPLORER_Y + 60)
#define CONTENT_W 200
#define CONTENT_H 120

#define BTN_X (EXPLORER_X + 210)
#define BTN_Y (EXPLORER_Y + 190)
#define BTN_W 90
#define BTN_H 22

static int explorer_open = 0;
static int explorer_selected = -1;
static int explorer_input_mode = 0; // 0-none, 1-name, 2-content
static char explorer_filename[MAX_FILENAME];
static char explorer_content[MAX_FILE_SIZE];
static char explorer_status[64];

static int explorer_strlen(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

static void explorer_strncpy(char* dst, const char* src, int max) {
    int i = 0;
    while (i < max - 1 && src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

static void explorer_set_status(const char* msg) {
    explorer_strncpy(explorer_status, msg, sizeof(explorer_status));
}

static void explorer_int_to_str(int value, char* buf) {
    int i = 0;
    if (value == 0) {
        buf[i++] = '0';
    } else {
        char tmp[12];
        int j = 0;
        while (value > 0) {
            tmp[j++] = (value % 10) + '0';
            value /= 10;
        }
        while (j > 0) buf[i++] = tmp[--j];
    }
    buf[i] = '\0';
}

static void explorer_draw_button(int x, int y, const char* label) {
    fill_rect(x, y, BTN_W, BTN_H, 0x002E3A55);
    fill_rect(x, y, BTN_W, 1, 0x00FFFFFF);
    fill_rect(x, y + BTN_H - 1, BTN_W, 1, 0x00FFFFFF);
    fill_rect(x, y, 1, BTN_H, 0x00FFFFFF);
    fill_rect(x + BTN_W - 1, y, 1, BTN_H, 0x00FFFFFF);
    draw_string_scaled(label, x + 8, y + 5, 0x00FFFFFF, 1);
}

static void explorer_draw_text_box(int x, int y, int w, int h, int active) {
    fill_rect(x, y, w, h, 0x001A1A1A);
    unsigned int edge = active ? 0x00FFD700 : 0x00FFFFFF;
    fill_rect(x, y, w, 1, edge);
    fill_rect(x, y + h - 1, w, 1, edge);
    fill_rect(x, y, 1, h, edge);
    fill_rect(x + w - 1, y, 1, h, edge);
}

static void explorer_draw_wrapped_text(const char* text, int x, int y, int w, int h) {
    int max_cols = (w - 8) / 9;
    int max_lines = (h - 8) / 18;
    int line = 0;
    int col = 0;
    char linebuf[32];
    int idx = 0;

    for (int i = 0; text[i] && line < max_lines; i++) {
        char c = text[i];
        if (c == '\n' || col >= max_cols) {
            linebuf[col] = '\0';
            draw_string_scaled(linebuf, x + 4, y + 4 + line * 18, 0x00FFFFFF, 1);
            line++;
            col = 0;
            idx = 0;
            if (c == '\n') continue;
        }
        linebuf[idx++] = c;
        col++;
        if (col >= max_cols) {
            linebuf[col] = '\0';
            draw_string_scaled(linebuf, x + 4, y + 4 + line * 18, 0x00FFFFFF, 1);
            line++;
            col = 0;
            idx = 0;
        }
    }
    if (col > 0 && line < max_lines) {
        linebuf[col] = '\0';
        draw_string_scaled(linebuf, x + 4, y + 4 + line * 18, 0x00FFFFFF, 1);
    }
}

static int explorer_select_file_row(int row) {
    int row_index = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem[i].used) {
            if (row_index == row) return i;
            row_index++;
        }
    }
    return -1;
}

void explorer_draw(BootInfo* binfo) {
    fill_rect(EXPLORER_X, EXPLORER_Y, EXPLORER_W, EXPLORER_HEIGHT, 0x001A1A1A);
    fill_rect(EXPLORER_X, EXPLORER_Y, EXPLORER_W, 24, 0x002E3A55);
    fill_rect(EXPLORER_X, EXPLORER_Y, 1, EXPLORER_HEIGHT, 0x00FFFFFF);
    fill_rect(EXPLORER_X, EXPLORER_Y + EXPLORER_HEIGHT - 1, EXPLORER_W, 1, 0x00FFFFFF);
    fill_rect(EXPLORER_X + EXPLORER_W - 1, EXPLORER_Y, 1, EXPLORER_HEIGHT, 0x00FFFFFF);
    draw_string_scaled("Explorer", EXPLORER_X + 10, EXPLORER_Y + 4, 0x00FFFFFF, 1);

    // Кнопка закриття (червона)
    fill_rect(EXPLORER_X + EXPLORER_W - 25, EXPLORER_Y, 25, 24, 0x00AA0000);
    draw_string_scaled("X", EXPLORER_X + EXPLORER_W - 17, EXPLORER_Y + 4, 0x00FFFFFF, 1);

    draw_string_scaled("Name:", NAME_X, NAME_Y - 18, 0x00FFFFFF, 1);
    draw_string_scaled("Content:", CONTENT_X, CONTENT_Y - 18, 0x00FFFFFF, 1);
    draw_string_scaled("Status:", EXPLORER_X + 10, EXPLORER_Y + EXPLORER_HEIGHT - 30, 0x00FFFFFF, 1);

    explorer_draw_text_box(LIST_X, LIST_Y, LIST_W, LIST_H, 0);
    explorer_draw_text_box(NAME_X, NAME_Y, NAME_W, NAME_H, explorer_input_mode == 1);
    explorer_draw_text_box(CONTENT_X, CONTENT_Y, CONTENT_W, CONTENT_H, explorer_input_mode == 2);

    draw_string_scaled(explorer_filename, NAME_X + 4, NAME_Y + 4, 0x00FFFFFF, 1);
    explorer_draw_wrapped_text(explorer_content, CONTENT_X, CONTENT_Y, CONTENT_W, CONTENT_H);

    // Малюємо статус безпосередньо
    draw_string_scaled(explorer_status, EXPLORER_X + 75, EXPLORER_Y + EXPLORER_HEIGHT - 30, 0x00FFD700, 1);

    explorer_draw_button(BTN_X, BTN_Y, "New");
    explorer_draw_button(BTN_X + BTN_W + 10, BTN_Y, "Open");
    explorer_draw_button(BTN_X, BTN_Y + BTN_H + 10, "Save");
    explorer_draw_button(BTN_X + BTN_W + 10, BTN_Y + BTN_H + 10, "Delete");

    int row_index = 0;
    int y = LIST_Y + 4;
    for (int i = 0; i < MAX_FILES; i++) {
        if (filesystem[i].used) {
            unsigned int bg = (i == explorer_selected) ? 0x00575757 : 0x00000000;
            if (bg) fill_rect(LIST_X + 2, y - 2, LIST_W - 4, ROW_H, bg);
            draw_string_scaled(filesystem[i].name, LIST_X + 4, y, 0x00FFFFFF, 1);
            char sizebuf[16];
            explorer_int_to_str(filesystem[i].size, sizebuf);
            int sl = explorer_strlen(sizebuf);
            sizebuf[sl++] = ' ';
            sizebuf[sl++] = 'B';
            sizebuf[sl] = '\0';
            draw_string_scaled(sizebuf, LIST_X + LIST_W - 40, y, 0x00FFD700, 1);
            y += ROW_H;
            row_index++;
        }
    }

    if (row_index == 0) {
        draw_string_scaled("(no files)", LIST_X + 4, LIST_Y + 4, 0x00808080, 1);
    }
}

void explorer_init() {
    explorer_open = 0;
    explorer_selected = -1;
    explorer_input_mode = 0;
    explorer_filename[0] = '\0';
    explorer_content[0] = '\0';
    explorer_status[0] = '\0';
}

void explorer_open_window() {
    explorer_open = 1;
    explorer_selected = -1;
    explorer_input_mode = 0;
    kernel_trigger_desktop_redraw();
    explorer_set_status("Explorer opened");
    // explorer_draw буде викликано в циклі ядра
}

int explorer_is_active() {
    return explorer_open;
}

static void explorer_select_file(int index) {
    if (index < 0 || index >= MAX_FILES) return;
    explorer_selected = index;
    explorer_strncpy(explorer_filename, filesystem[index].name, MAX_FILENAME);
    explorer_strncpy(explorer_content, filesystem[index].data, MAX_FILE_SIZE);
    explorer_input_mode = 2;
    explorer_set_status("Selected file");
}

static void explorer_save_current() {
    int idx = xfs_find(explorer_filename);
    if (explorer_filename[0] == '\0') {
        explorer_set_status("Filename required");
        return;
    }
    if (idx < 0) {
        idx = xfs_create(explorer_filename);
        if (idx < 0) {
            explorer_set_status("Disk full");
            return;
        }
    }
    xfs_write(explorer_filename, explorer_content);
    xfs_sync();
    explorer_selected = idx;
    explorer_set_status("Saved file");
}

static void explorer_new_file() {
    explorer_selected = -1;
    explorer_filename[0] = '\0';
    explorer_content[0] = '\0';
    explorer_input_mode = 1;
    explorer_set_status("New file");
}

void explorer_on_click(int x, int y) {
    if (!explorer_open) return;

    // Перевірка чи клік у межах вікна провідника
    if (x < EXPLORER_X || x > EXPLORER_X + EXPLORER_W || 
        y < EXPLORER_Y || y > EXPLORER_Y + EXPLORER_HEIGHT) {
        return;
    }

    // Логіка закриття вікна
    if (x >= EXPLORER_X + EXPLORER_W - 25 && x <= EXPLORER_X + EXPLORER_W &&
        y >= EXPLORER_Y && y <= EXPLORER_Y + 24) {
        explorer_open = 0;
        kernel_trigger_desktop_redraw();
        return;
    }

    if (x >= LIST_X && x < LIST_X + LIST_W && y >= LIST_Y && y < LIST_Y + LIST_H) {
        int row = (y - LIST_Y) / ROW_H;
        int index = explorer_select_file_row(row);
        if (index >= 0) {
            explorer_select_file(index);
            return;
        }
    }

    if (x >= NAME_X && x < NAME_X + NAME_W && y >= NAME_Y && y < NAME_Y + NAME_H) {
        explorer_input_mode = 1;
        explorer_set_status("Filename editing");
        return;
    }

    if (x >= CONTENT_X && x < CONTENT_X + CONTENT_W && y >= CONTENT_Y && y < CONTENT_Y + CONTENT_H) {
        explorer_input_mode = 2;
        explorer_set_status("Content editing");
        return;
    }

    if (x >= BTN_X && x < BTN_X + BTN_W && y >= BTN_Y && y < BTN_Y + BTN_H) {
        explorer_new_file();
        return;
    }

    if (x >= BTN_X + BTN_W + 10 && x < BTN_X + 2 * BTN_W + 10 && y >= BTN_Y && y < BTN_Y + BTN_H) {
        if (explorer_selected >= 0) {
            explorer_select_file(explorer_selected);
            explorer_set_status("Opened file");
        } else {
            explorer_set_status("No file selected");
        }
        return;
    }

    if (x >= BTN_X && x < BTN_X + BTN_W && y >= BTN_Y + BTN_H + 10 && y < BTN_Y + 2 * BTN_H + 10) {
        explorer_save_current();
        return;
    }

    if (x >= BTN_X + BTN_W + 10 && x < BTN_X + 2 * BTN_W + 10 && y >= BTN_Y + BTN_H + 10 && y < BTN_Y + 2 * BTN_H + 10) {
        if (explorer_selected >= 0) {
            xfs_remove(filesystem[explorer_selected].name);
            explorer_selected = -1;
            explorer_filename[0] = '\0';
            explorer_content[0] = '\0';
            explorer_input_mode = 0;
            explorer_set_status("Deleted file");
        } else {
            explorer_set_status("No file selected");
        }
        return;
    }
}

void explorer_on_key(char key) {
    if (!explorer_open || explorer_input_mode == 0) return;

    char* target = explorer_input_mode == 1 ? explorer_filename : explorer_content;
    int len = explorer_strlen(target);
    if (key == 8) {
        if (len > 0) target[len - 1] = '\0';
    } else if (key >= 32 && key < 127) {
        int max = (explorer_input_mode == 1) ? MAX_FILENAME : MAX_FILE_SIZE;
        if (len < max - 1) {
            target[len] = key;
            target[len + 1] = '\0';
        }
    }
}
