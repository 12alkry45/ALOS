#define VIDEO_MEMORY 0xB8000
#define MAX_COLS 80
#define MAX_ROWS 25
#define WHITE_ON_BLACK 0x0F
#define RED_ON_WHITE 0xF4

#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5

void clear_screen();
void kernel_print_at(char* message, int col, int row);
void kernel_print(char* message);
