
/*
* raw(?) I/O
*/
#include "io.h"

void gotoxy(POSITION pos) {
    COORD coord = { pos.column, pos.row }; // 행, 열 반대로 전달
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void set_color(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void printc(POSITION pos, char ch, int color) {
    if (color >= 0) {
        set_color(color);
    }
    gotoxy(pos);
    printf("%c", ch);
}

KEY get_key(void) {
    if (_kbhit()) {  // 입력된 키가 있는지 확인
        int byte = _getch();
        switch (byte) {
        case 'q': return k_quit;
        case 224:
            byte = _getch();
            switch (byte) {
            case 72: return k_up;
            case 75: return k_left;
            case 77: return k_right;
            case 80: return k_down;
            default: return k_undef;
            }
        default: return byte;
        }
    }
    return k_none;
}
