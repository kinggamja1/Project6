/*
*  display.c:
* ȭ�鿡 ���� ������ ���
* ��, Ŀ��, �ý��� �޽���, ����â, �ڿ� ���� ���
* io.c�� �ִ� �Լ����� �����
*/

#include "display.h"
#include "io.h"

extern SYSTEM_STATE system_state;
extern SYSTEM_MESSAGE_LOG system_message_log;
extern POSITION selected_position;
extern SANDWORM sandworm;
extern char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH];
// ����� ������� �»��(topleft) ��ǥ
const POSITION resource_pos = { 0, 0 };
const POSITION map_pos = { 1, 0 };

const POSITION status_window_pos = { MAP_HEIGHT + 2, 0 };
const POSITION system_message_pos = { MAP_HEIGHT + 8, 0 };
const POSITION command_window_pos = { MAP_HEIGHT + 10, 0 };


char backbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };
char frontbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };

void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]);
void display_resource(RESOURCE resource);
void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void display_cursor(CURSOR cursor);
void display_status_window(POSITION selected_position);
void display_system_message(void);
void display_commands(void);



void display(
	RESOURCE resource,
	char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH],
	CURSOR cursor) {
	system("cls");



	display_resource(resource);
	display_map(map);
	display_cursor(cursor);
	display_status_window(selected_position);
	display_system_message();
	display_commands();
}

void display_resource(RESOURCE resource) {
	set_color(COLOR_RESOURCE);
	gotoxy(resource_pos);
	printf("spice = %d/%d, population=%d/%d\n",
		resource.spice, resource.spice_max,
		resource.population, resource.population_max
	);
}

// subfunction of draw_map()
void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]) {
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			for (int k = 0; k < N_LAYER; k++) {
				if (src[k][i][j] >= 0) {
					dest[i][j] = src[k][i][j];
				}
			}
		}
	}
}

void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	project(map, backbuf);

	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			if (frontbuf[i][j] != backbuf[i][j]) {
				POSITION pos = { i, j };
				char ch = backbuf[i][j];


				if (i == sandworm.pos.row && j == sandworm.pos.column) {
					ch = 'W';
					set_color(COLOR_ORANGE);
				}
				else {
					switch (ch) {
					case 'B':
						set_color(COLOR_BLUE);
						break;
					case 'H':
						set_color(COLOR_RED);
						break;
					case '5':
						set_color(COLOR_YELLOW);
						break;
					case 'P':
						set_color(COLOR_BLACK);
						break;
					case 'R':
						set_color(COLOR_GRAY);
						break;
					case 'W':
						set_color(COLOR_ORANGE);
						break;
					default:
						set_color(COLOR_DEFAULT);
						break;
					}


				}
				printc(padd(map_pos, pos), ch, -1);
				frontbuf[i][j] = backbuf[i][j];
			}

		}
	}
}

// frontbuf[][]���� Ŀ�� ��ġ�� ���ڸ� ���� �ٲ㼭 �״�� �ٽ� ���
void display_cursor(CURSOR cursor) {
	POSITION prev = cursor.previous;
	POSITION curr = cursor.current;

	char ch = frontbuf[prev.row][prev.column];
	printc(padd(map_pos, prev), ch, COLOR_DEFAULT);

	ch = frontbuf[curr.row][curr.column];
	printc(padd(map_pos, curr), ch, COLOR_CURSOR);
}
void display_status_window(POSITION selected_position) {
	set_color(COLOR_DEFAULT);
	gotoxy(status_window_pos);
	printf("����â: ���õ� ����/���� ���� �� ����");
	if (selected_position.row == -1) {
		printf("����â: ���õ� �׸� ����");
	}
	else {
		char selected_char = map[0][selected_position.row][selected_position.column];
		switch (selected_char) {
		case 'B':
			printf("����â: ���� (Base)");
			break;
		case 'H':
			printf("����â: �Ϻ����� (Harvester)");
			break;
		case '5':
			printf("����â: �����̽� ������");
			break;
		case 'P':
			printf("����â: ���� (Plate)");
			break;
		case 'R':
			printf("����â: ���� (Rock)");
			break;
		case 'W':
			printf("����â: ����� (Sandworm)");
			break;
		default:
			printf("����â: �� ����");
			break;
		}
	}
}

void display_system_message(void) {
	set_color(COLOR_DEFAULT);
	gotoxy(system_message_pos);
	printf("�ý��� �޽���: %s", system_state.message);

	for (int i = 0; i < system_message_log.message_count; i++) {
		gotoxy((POSITION) { system_message_pos.row + i + 1, system_message_pos.column });
		printf("%s\n", system_message_log.messages[i]);
	}
}

void display_commands(void) {
	set_color(COLOR_DEFAULT);
	gotoxy(command_window_pos);
	printf("���â: \n");
	printf("H: Harvester ����, M: �̵�, P: Plate �Ǽ�\n");
	printf("F: Fremen ����, S: Soldier ����\n");
	printf("T: Heavy Tank ����, G: Fighter ����\n");
}

void add_system_message(const char* message) {
	if (system_message_log.message_count >= MAX_SYSTEM_MESSAGES) {
		for (int i = 1; i < MAX_SYSTEM_MESSAGES; i++) {
			strcpy(system_message_log.messages[i - 1], system_message_log.messages[i]);
		}
		system_message_log.message_count--;
	}

	strncpy(system_message_log.messages[system_message_log.message_count], message, SYSTEM_MESSAGE_LENGTH - 1);
	system_message_log.messages[system_message_log.message_count][SYSTEM_MESSAGE_LENGTH - 1] = '\0';
	system_message_log.message_count++;
}