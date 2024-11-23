//5������ �Ǵ´�� �߽��ϴ�
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "io.h"
#include "display.h"

typedef enum {
	HARVESTER_WAITING,
	HARVESTER_MOVING,
	HARVESTER_HARVESTING
} HarvesterState;

typedef struct {
	char name[20];             
	int hp;                   
	HarvesterState state;       
	POSITION position;         
	POSITION target;           
	int spice_carried;         
	int timer;                  
} Harvester;

#define MAX_HARVESTERS 10
Harvester harvesters[MAX_HARVESTERS];
int num_harvesters = 0;

typedef struct Unit {
	char type;                  
	char name[20];              
	int hp;                     
	struct Unit* next;          
} Unit;

typedef struct UnitList {
	Unit* head;                
	int count;                 
	int supply_max;            
} UnitList;

UnitList* unit_list = NULL;
void init_unit_list(int supply_max);
bool add_unit(char type, const char* name, int hp);
bool remove_unit(const char* name);
void print_unit_list();
void free_unit_list();

void init_harvesters();
bool add_harvester(const char* name, int hp, POSITION position);
void update_harvesters();
void harvester_command(Harvester* harvester, POSITION target, HarvesterState command);
void harvester_harvest(Harvester* harvester);
void harvester_return(Harvester* harvester);



extern char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH];
extern RESOURCE resource;
extern POSITION selected_position;
extern SYSTEM_MESSAGE_LOG system_message_log;
extern SANDWORM sandworm;

void init(void);
void intro(void);
void outro(void);
void cursor_move(DIRECTION dir);
void sample_obj_move(void);
POSITION sample_obj_next_position(void);
void produce_unit_or_building(char type);
void sandworm_move(void);
void add_system_message(const char* message);
void build_mode_toggle(void);       
void start_building(char building_type); 
void cancel_building(void);        

SYSTEM_STATE system_state = { "", 0 };

bool build_mode = false;              
char building_to_place = ' ';        
int build_timer = 0;

/* ================= control =================== */
int sys_clock = 0;		// system-wide clock(ms)
CURSOR cursor = { { 1, 1 }, {1, 1} };
POSITION selected_position = { -1, -1 };


/* ================= game data =================== */
char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH] = { 0 };

RESOURCE resource = {
	.spice = 0,
	.spice_max = 0,
	.population = 0,
	.population_max = 0
};

OBJECT_SAMPLE obj = {
	.pos = {1, 1},
	.dest = {MAP_HEIGHT - 2, MAP_WIDTH - 2},
	.repr = 'o',
	.speed = 300,
	.next_move_time = 300
};

SANDWORM sandworm = { {10, 10}, 1, 1, 500 };

SYSTEM_MESSAGE_LOG system_message_log = { .message_count = 0 };

/* ================= main() =================== */
int main(void) {
	srand((unsigned int)time(NULL));
	init_unit_list(5);  // �ִ� ���� �� 5�� ����


	init();
	intro();
	display(resource, map, cursor);
	int clear_screen_interval = 1000; // 1�ʸ��� ȭ���� ����� ����
	int last_clear_time = 0;



	while (1) {
		// loop �� ������(��, TICK==10ms����) Ű �Է� Ȯ��
		KEY key = get_key();

		if (key != k_none) {
			printf("key pressed: %d\n", key);
			display(resource, map, cursor);
		}
		else {
			Sleep(TICK);
			continue;
		}
		// Ű �Է��� ������ ó��
		if (is_arrow_key(key)) {
			cursor_move(ktod(key));
		}
		else {
			// ����Ű ���� �Է�
			switch (key) {
			case ' ':
				if (map[0][cursor.current.row][cursor.current.column] != ' ') {
					selected_position = cursor.current;
					printf("���� �Ǵ� ������Ʈ ���õ�: (%d, %d)\n", selected_position.row, selected_position.column);
					display_status_window(selected_position);
				}
				break;
			case 'B': // �Ǽ� ��� ����Ű ����
				build_mode_toggle();
				break;

			case 'X':
				cancel_building(); 
				break;
			case 27:
				selected_position.row = -1;
				selected_position.column = -1;
				display_status_window(selected_position);
				printf("���� ��ҵ�\n");
				break;
			case 'H':
				printf("Harvester�� �����մϴ�.\n");
				produce_unit_or_building('H');
				display_system_message();
				break;
			case 'P': 
				if (build_mode) {
					if (resource.spice >= 30) { // 30 spice�� �ʿ��ϴٰ� ���� ����
						printf("Plate�� �Ǽ��մϴ�.\n"); 
						resource.spice -= 30; 
						build_timer = 3000;  
						start_building('P');
					}
					else {
						printf("�ڿ��� �����Ͽ� �Ǽ��� �� �����ϴ�.\n");
						add_system_message("Not enough spice ");
					}
				}
				else {
					printf("�Ǽ� ��尡 Ȱ��ȭ���� �ʾҽ��ϴ�.\n");
				}
				break;
			case 'F':
				printf("Fremen�� �����մϴ�.\n");
				break;
			case 'S':
				printf("Soldier�� �����մϴ�.\n");
				break;
			case 'T':
				printf("Heavy Tank�� �����մϴ�.\n");
				break;
			case 'G':
				printf("Fighter�� �����մϴ�.\n");
				break;
			case k_quit:
				outro();
				break;
			case k_none:
			case k_undef:
			default: break;
			}
		}
		if (sys_clock - last_clear_time >= clear_screen_interval) {
			system("cls");
			last_clear_time = sys_clock;
		}


		if (build_timer > 0) {
			build_timer -= TICK;
			if (build_timer <= 0) {
				if (selected_position.row >= 0 && selected_position.row < MAP_HEIGHT &&
					selected_position.column >= 0 && selected_position.column + 1 < MAP_WIDTH) {
					map[0][selected_position.row][selected_position.column] = building_to_place;
					add_system_message("Building construction completed!");
				}
				build_mode = false;
				building_to_place = ' ';
				selected_position.row = -1; 
				selected_position.column = -1;
			}
		}

		// ���� ������Ʈ ����
		sample_obj_move();

		sandworm_move();


		// ȭ�� ���
		display(resource, map, cursor);
		Sleep(TICK);
		sys_clock += TICK;


	}
}

void init_harvesters() {
	num_harvesters = 0;
	for (int i = 0; i < MAX_HARVESTERS; i++) {
		harvesters[i].name[0] = '\0';
		harvesters[i].hp = 0;
		harvesters[i].state = HARVESTER_WAITING;
		harvesters[i].position = (POSITION){ -1, -1 };
		harvesters[i].target = (POSITION){ -1, -1 };
		harvesters[i].spice_carried = 0;
		harvesters[i].timer = 0;
	}
}

bool add_harvester(const char* name, int hp, POSITION position) {
	if (num_harvesters >= MAX_HARVESTERS) return false;

	Harvester* harvester = &harvesters[num_harvesters++];
	snprintf(harvester->name, sizeof(harvester->name), "%s", name);
	harvester->hp = hp;
	harvester->state = HARVESTER_WAITING;
	harvester->position = position;
	harvester->target = position;
	harvester->spice_carried = 0;
	harvester->timer = 0;

	return true;
}

void init_unit_list(int supply_max) {
	unit_list = (UnitList*)malloc(sizeof(UnitList));
	if (!unit_list) {
		printf("���� ����Ʈ �ʱ�ȭ ����.\n");
		exit(1);
	}
	unit_list->head = NULL;
	unit_list->count = 0;
	unit_list->supply_max = supply_max;
}

bool add_unit(char type, const char* name, int hp) {
	if (unit_list->count >= unit_list->supply_max) {
		printf("���� ���� �ʰ�: ���� �߰� ����.\n");
		return false;
	}

	Unit* new_unit = (Unit*)malloc(sizeof(Unit));
	if (!new_unit) {
		printf("���� �޸� �Ҵ� ����.\n");
		return false;
	}

	new_unit->type = type;
	strncpy(new_unit->name, name, sizeof(new_unit->name) - 1);
	new_unit->name[sizeof(new_unit->name) - 1] = '\0';
	new_unit->hp = hp;
	new_unit->next = unit_list->head;
	unit_list->head = new_unit;
	unit_list->count++;

	printf("���� %s �߰��� (%d/%d).\n", name, unit_list->count, unit_list->supply_max);
	return true;
}

bool remove_unit(const char* name) {
	if (!unit_list->head) {
		printf("���� ����� ����ֽ��ϴ�.\n");
		return false;
	}

	Unit* current = unit_list->head;
	Unit* prev = NULL;

	while (current) {
		if (strcmp(current->name, name) == 0) {
			if (prev) {
				prev->next = current->next;
			}
			else {
				unit_list->head = current->next;
			}
			free(current);
			unit_list->count--;
			printf("���� %s ������ (%d/%d).\n", name, unit_list->count, unit_list->supply_max);
			return true;
		}
		prev = current;
		current = current->next;
	}

	printf("���� %s ã�� �� ����.\n", name);
	return false;
}

void print_unit_list() {
	if (!unit_list || !unit_list->head) {
		printf("���� ����� ����ֽ��ϴ�.\n");
		return;
	}

	printf("���� ��� (%d/%d):\n", unit_list->count, unit_list->supply_max);
	Unit* current = unit_list->head;
	while (current) {
		printf("- �̸�: %s, Ÿ��: %c, ü��: %d\n", current->name, current->type, current->hp);
		current = current->next;
	}
}

void free_unit_list() {
	if (!unit_list) return;

	Unit* current = unit_list->head;
	while (current) {
		Unit* to_free = current;
		current = current->next;
		free(to_free);
	}

	free(unit_list);
	unit_list = NULL;
	printf("���� ����Ʈ �޸� ���� �Ϸ�.\n");
}


void build_mode_toggle(void) {
	build_mode = !build_mode;
	building_to_place = 'P';
	if (build_mode) {
		add_system_message("�Ǽ� ��� Ȱ��ȭ");
	}
	else {
		add_system_message("�Ǽ� ��� ��Ȱ��ȭ");
	}
}

void start_building(char building_type) {
	if (selected_position.row >= 0 && selected_position.column >= 0) {
		build_timer = 3000; 
		building_to_place = building_type;
		snprintf(system_state.message, SYSTEM_MESSAGE_LENGTH, "Building %c...", building_type);
	}
}

void cancel_building(void) {
	if (build_timer > 0) { 
		build_timer = 0;
		building_to_place = '\0';
		add_system_message("Building cancelled");
	}
}

/* ================= subfunctions =================== */
void intro(void) {
	printf("DUNE 1.5\n");
	Sleep(2000);
	system("cls");
}

void outro(void) {
	printf("exiting...\n");
	exit(0);
}

void init(void) {
	// layer 0(map[0])�� ���� ����
	for (int j = 0; j < MAP_WIDTH; j++) {
		map[0][0][j] = '#';
		map[0][MAP_HEIGHT - 1][j] = '#';
	}

	for (int i = 1; i < MAP_HEIGHT - 1; i++) {
		map[0][i][0] = '#';
		map[0][i][MAP_WIDTH - 1] = '#';
		for (int j = 1; j < MAP_WIDTH - 1; j++) {
			map[0][i][j] = ' ';
		}
	}

	// layer 1(map[1])�� ��� �α�(-1�� ä��)
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			map[1][i][j] = -1;
		}
	}


	map[0][1][1] = 'B';
	map[0][2][2] = 'H';
	map[0][3][3] = '5';
	map[0][1][3] = 'P';
	map[0][2][3] = 'P';
	map[0][1][4] = 'P';
	map[0][2][4] = 'P';
	map[0][5][5] = 'R';
	map[0][7][7] = 'R';
	map[0][9][9] = 'R';
	map[0][4][4] = 'W';
	map[0][6][6] = 'W';

	// object sample
	map[1][obj.pos.row][obj.pos.column] = 'o';
}

// (�����ϴٸ�) ������ �������� Ŀ�� �̵�
void cursor_move(DIRECTION dir) {
	POSITION curr = cursor.current;
	POSITION new_pos = pmove(curr, dir);

	// validation check
	if (1 <= new_pos.row && new_pos.row <= MAP_HEIGHT - 2 && \
		1 <= new_pos.column && new_pos.column <= MAP_WIDTH - 2) {

		cursor.previous = cursor.current;
		cursor.current = new_pos;
	}
}

/* ================= sample object movement =================== */
POSITION sample_obj_next_position(void) {
	// ���� ��ġ�� �������� ���ؼ� �̵� ���� ����	
	POSITION diff = psub(obj.dest, obj.pos);
	DIRECTION dir;

	// ������ ����. ������ �ܼ��� ���� �ڸ��� �պ�
	if (diff.row == 0 && diff.column == 0) {
		if (obj.dest.row == 1 && obj.dest.column == 1) {
			// topleft --> bottomright�� ������ ����
			POSITION new_dest = { MAP_HEIGHT - 2, MAP_WIDTH - 2 };
			obj.dest = new_dest;
		}
		else {
			// bottomright --> topleft�� ������ ����
			POSITION new_dest = { 1, 1 };
			obj.dest = new_dest;
		}
		return obj.pos;
	}

	// ������, ������ �Ÿ��� ���ؼ� �� �� �� ������ �̵�
	if (abs(diff.row) >= abs(diff.column)) {
		dir = (diff.row >= 0) ? d_down : d_up;
	}
	else {
		dir = (diff.column >= 0) ? d_right : d_left;
	}

	// validation check
	// next_pos�� ���� ����� �ʰ�, (������ ������)��ֹ��� �ε����� ������ ���� ��ġ�� �̵�
	// ������ �浹 �� �ƹ��͵� �� �ϴµ�, ���߿��� ��ֹ��� ���ذ��ų� ���� ������ �ϰų�... ���
	POSITION next_pos = pmove(obj.pos, dir);
	if (1 <= next_pos.row && next_pos.row <= MAP_HEIGHT - 2 && \
		1 <= next_pos.column && next_pos.column <= MAP_WIDTH - 2 && \
		map[1][next_pos.row][next_pos.column] < 0) {

		return next_pos;
	}
	else {
		return obj.pos;  // ���ڸ�
	}
}

void sample_obj_move(void) {
	if (sys_clock <= obj.next_move_time) {
		// ���� �ð��� �� ����
		return;
	}

	// ������Ʈ(�ǹ�, ���� ��)�� layer1(map[1])�� ����
	map[1][obj.pos.row][obj.pos.column] = -1;
	obj.pos = sample_obj_next_position();
	map[1][obj.pos.row][obj.pos.column] = obj.repr;

	obj.next_move_time = sys_clock + obj.speed;
}
//����� �̵� - ���� ����� ���� Ž�� -> �ش� �������� �̵�, ��Ƹ����� Ŀ���� ���� ������ ����
void sandworm_move(void) {
	if (sys_clock % sandworm.speed != 0) {
		return;
	}
	POSITION closest_unit = { -1, -1 };
	int min_distance = MAP_HEIGHT * MAP_WIDTH;

	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			if (map[1][i][j] == 'H' || map[1][i][j] == 'F' || map[1][i][j] == 'S' || map[1][i][j] == 'T' || map[1][i][j] == 'G') {
				int distance = abs(sandworm.pos.row - i) + abs(sandworm.pos.column - j);
				if (distance < min_distance) {
					min_distance = distance;
					closest_unit.row = i;
					closest_unit.column = j;
				}
			}
		}
	}
	if (closest_unit.row != -1 && closest_unit.column != -1) {
		if (sandworm.pos.row < closest_unit.row) {
			sandworm.pos.row++;
		}
		else if (sandworm.pos.row > closest_unit.row) {
			sandworm.pos.row--;
		}

		if (sandworm.pos.column < closest_unit.column) {
			sandworm.pos.column++;
		}
		else if (sandworm.pos.column > closest_unit.column) {
			sandworm.pos.column--;
		}

		if (sandworm.pos.row == closest_unit.row && sandworm.pos.column == closest_unit.column) {
			map[1][closest_unit.row][closest_unit.column] = -1;
			sandworm.length += sandworm.growth_rate;

			if (rand() % 5 == 0) {
				map[0][closest_unit.row][closest_unit.column] = '5';
				printf("�����̽� �������� �����Ǿ����ϴ�.\n");
			}
		}
	}
}
void produce_unit_or_building(char type) {
	if (selected_position.row != -1) {
		switch (map[0][selected_position.row][selected_position.column]) {
		case 'B':
			if (type == 'H' && resource.spice >= 50) {
				resource.spice -= 50;
				add_system_message("Harvester ���� ����!");
			}
			else if (type == 'S' && resource.spice >= 75) {
				resource.spice -= 75;
				add_system_message("Soldier ���� ����!");
			}
			else {
				add_system_message("�ڿ��� �����ϰų� �߸��� ����Դϴ�.");
			}
			break;
		case 'R':
			if (type == 'F' && resource.spice >= 100) {
				resource.spice -= 100;
				add_system_message("Fremen ���� ����!");
			}
			else {
				add_system_message("�ڿ��� �����ϰų� �߸��� ����Դϴ�.");
			}
			break;
		case 'F':
			if (type == 'T' && resource.spice >= 150) {
				resource.spice -= 150;
				add_system_message("Heavy Tank ���� ����!");
			}
			else if (type == 'G' && resource.spice >= 125) {
				resource.spice -= 125;
				add_system_message("Fighter ���� ����!");
			}
			else {
				add_system_message("�ڿ��� �����ϰų� �߸��� ����Դϴ�.");
			}
			break;
		default:
			add_system_message("�ش� �ǹ������� �� ����� ������ �� �����ϴ�.");
			break;
		}
	}
	else {
		add_system_message("���� �ǹ��� �����Ͻʽÿ�.");
	}
}
//���� ����
void produce_unit(char unit_type) {
	if (selected_position.row != -1 && map[0][selected_position.row][selected_position.column] == 'B') {
		if (unit_type == 'H') {
			if (resource.spice >= UNIT_HARVESTER_COST) {
				resource.spice -= UNIT_HARVESTER_COST;
				system_state.production_time_left = 3000;
				snprintf(system_state.message, SYSTEM_MESSAGE_LENGTH, "Producing Harvester...");
			}
			else {
				snprintf(system_state.message, SYSTEM_MESSAGE_LENGTH, "Not enough spice");
			}
		}
	}
	else {
		snprintf(system_state.message, SYSTEM_MESSAGE_LENGTH, "Invalid selection");
	}
}