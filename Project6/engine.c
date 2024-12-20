//5번까지 되는대로 했습니다
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "io.h"
#include "display.h"

#define ATTACK_RANGE 1
#define ATTACK_INTERVAL 1000 
#define TICK 10

typedef enum {
	UNIT_WAITING,   
	UNIT_MOVING,     
	UNIT_PATROLLING  
} UnitState;

typedef struct {
	char name[20];
	int hp;
	UnitState state;
	POSITION position;
	POSITION target;
	POSITION patrol_start; 
	POSITION patrol_end;   
	int timer;
	int attack_power;
} Soldier;

#define MAX_SOLDIERS 10
Soldier soldiers[MAX_SOLDIERS];
int num_soldiers = 0;
Soldier enemies[MAX_SOLDIERS];

void init_soldiers();
bool add_soldier(const char* name, int hp, POSITION position);
void update_soldiers();
void move_command(Soldier* soldier, POSITION target);
void patrol_command(Soldier* soldier, POSITION start, POSITION end);

void init_soldiers() {
	num_soldiers = 0;
	for (int i = 0; i < MAX_SOLDIERS; i++) {
		soldiers[i].name[0] = '\0';
		soldiers[i].hp = 0;
		soldiers[i].state = UNIT_WAITING;
		soldiers[i].position = (POSITION){ -1, -1 };
		soldiers[i].target = (POSITION){ -1, -1 };
		soldiers[i].patrol_start = (POSITION){ -1, -1 };
		soldiers[i].patrol_end = (POSITION){ -1, -1 };
		soldiers[i].attack_power = 10; 
		soldiers[i].timer = 0;
	}
}

bool add_soldier(const char* name, int hp, POSITION position) {
	if (num_soldiers >= MAX_SOLDIERS) return false;

	Soldier* soldier = &soldiers[num_soldiers++];
	snprintf(soldier->name, sizeof(soldier->name), "%s", name);
	soldier->hp = hp;
	soldier->state = UNIT_WAITING;
	soldier->position = position;
	soldier->target = position;
	return true;
}

void move_command(Soldier* soldier, POSITION target) {
	soldier->state = UNIT_MOVING;
	soldier->target = target;
	soldier->timer = 500; 
}

void patrol_command(Soldier* soldier, POSITION start, POSITION end) {
	soldier->state = UNIT_PATROLLING;
	soldier->patrol_start = start;
	soldier->patrol_end = end;
	soldier->target = end; // Start by moving to end
	soldier->timer = 500;
}

void update_soldiers() {
	for (int i = 0; i < num_soldiers; i++) {
		Soldier* soldier = &soldiers[i];
		switch (soldier->state) {
		case UNIT_WAITING:
			break; // Do nothing
		case UNIT_MOVING:
			if (soldier->timer <= 0) {
				if (soldier->position.row == soldier->target.row &&
					soldier->position.column == soldier->target.column) {
					soldier->state = UNIT_WAITING;
				}
				else {
					if (soldier->position.row < soldier->target.row)
						soldier->position.row++;
					else if (soldier->position.row > soldier->target.row)
						soldier->position.row--;

					if (soldier->position.column < soldier->target.column)
						soldier->position.column++;
					else if (soldier->position.column > soldier->target.column)
						soldier->position.column--;

					soldier->timer = 500;
				}
			}
			else {
				soldier->timer -= TICK;
			}
			break;
		case UNIT_PATROLLING:
			if (soldier->timer <= 0) {
				if (soldier->position.row == soldier->target.row &&
					soldier->position.column == soldier->target.column) {
					soldier->target = (soldier->target.row == soldier->patrol_start.row &&
						soldier->target.column == soldier->patrol_start.column)
						? soldier->patrol_end
						: soldier->patrol_start;
				}
				else {
					if (soldier->position.row < soldier->target.row)
						soldier->position.row++;
					else if (soldier->position.row > soldier->target.row)
						soldier->position.row--;

					if (soldier->position.column < soldier->target.column)
						soldier->position.column++;
					else if (soldier->position.column > soldier->target.column)
						soldier->position.column--;

					soldier->timer = 500;
				}
			}
			else {
				soldier->timer -= TICK;
			}
			break;
		}
	}
}

void soldier_combat(Soldier* soldier, Soldier* enemies, int num_enemies) {
	for (int i = 0; i < num_enemies; i++) {
		Soldier* enemy = &enemies[i];
		int distance = abs(soldier->position.row - enemy->position.row) +
			abs(soldier->position.column - enemy->position.column);
		if (distance <= ATTACK_RANGE && enemy->hp > 0) {
			printf("%s is attacking %s\n", soldier->name, enemy->name);
			enemy->hp -= soldier->attack_power;
			if (enemy->hp <= 0) {
				printf("%s has been defeated!\n", enemy->name);
				enemy->position = (POSITION){ -1, -1 }; 
			}
		}
	}
}




typedef enum {
	HARVESTER_WAITING,
	HARVESTER_MOVING,
	HARVESTER_HARVESTING,
	HARVESTER_RETURNING
} HarvesterState;

typedef struct {
	char name[20];             
	int hp;                   
	HarvesterState state;       
	POSITION position;         
	POSITION target;           
	int spice_carried;         
	int timer;  
	int attack_power;
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

void init_harvesters() {
	num_harvesters = 0;
	for (int i = 0; i < MAX_HARVESTERS; i++) {
		harvesters[i].name[0] = '\0';
		harvesters[i].hp = 0;
		harvesters[i].state = HARVESTER_WAITING;
		harvesters[i].position = (POSITION){ -1, -1 };
		harvesters[i].target = (POSITION){ -1, -1 };
		harvesters[i].spice_carried = 0;
		harvesters[i].attack_power = 5;
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

void update_harvesters() {
	for (int i = 0; i < num_harvesters; i++) {
		Harvester* harvester = &harvesters[i];
		switch (harvester->state) {
		case HARVESTER_WAITING:
			break;
		case HARVESTER_MOVING:
			if (harvester->timer <= 0) {
				if (harvester->position.row == harvester->target.row &&
					harvester->position.column == harvester->target.column) {
					if (map[0][harvester->target.row][harvester->target.column] == TERRAIN_SPICE) {
						harvester->state = HARVESTER_HARVESTING;
						harvester->timer = 3000;
					}
					else {
						harvester->state = HARVESTER_WAITING;
					}
				}
				else {
					if (harvester->position.row < harvester->target.row)
						harvester->position.row++;
					else if (harvester->position.row > harvester->target.row)
						harvester->position.row--;

					if (harvester->position.column < harvester->target.column)
						harvester->position.column++;
					else if (harvester->position.column > harvester->target.column)
						harvester->position.column--;
					harvester->timer = 500;
				}
			}
			else {
				harvester->timer -= TICK;
			}
			break;
		case HARVESTER_HARVESTING:
			if (harvester->timer <= 0) {
				harvester->spice_carried += 5;
				if (harvester->spice_carried >= 20) {
					harvester->state = HARVESTER_RETURNING;
					harvester->target = (POSITION){ 1, 1 };
				}
				else {
					harvester->timer = 3000;
				}
			}
			else {
				harvester->timer -= TICK;
			}
			break;
		case HARVESTER_RETURNING:
			harvester_return(harvester);
			break;
		}
	}
}

void harvester_command(Harvester* harvester, POSITION target, HarvesterState command) {
	harvester->state = command;
	harvester->target = target;
	if (command == HARVESTER_MOVING) {
		harvester->timer = 500;  
	}
	else if (command == HARVESTER_HARVESTING) {
		harvester->timer = 3000;  
	}
}

void harvester_harvest(Harvester* harvester) {
	if (harvester->state != HARVESTER_HARVESTING) return;

	int spice_to_collect = rand() % 3 + 2;
	harvester->spice_carried += spice_to_collect;
	printf("Harvester '%s' collected %d spice. Total: %d\n",
		harvester->name, spice_to_collect, harvester->spice_carried);

	harvester->state = HARVESTER_RETURNING;
	harvester->target = (POSITION){ 1, 1 };
}

void harvester_return(Harvester* harvester) {
	if (harvester->state != HARVESTER_RETURNING) return; 

	
	if (harvester->position.row == harvester->target.row &&
		harvester->position.column == harvester->target.column) {
		int spice_to_store = harvester->spice_carried;
		if (resource.spice + spice_to_store > resource.spice_max) {
			spice_to_store = resource.spice_max - resource.spice; 
		}
		resource.spice += spice_to_store;
		harvester->spice_carried -= spice_to_store;
		printf("Harvester '%s' delivered %d spice. Base total: %d\n",
			harvester->name, spice_to_store, resource.spice);
		harvester->state = HARVESTER_WAITING;
		harvester->target = harvester->position; 
	}
	else {
		if (harvester->position.row < harvester->target.row) {
			harvester->position.row++;
		}
		else if (harvester->position.row > harvester->target.row) {
			harvester->position.row--;
		}
		if (harvester->position.column < harvester->target.column) {
			harvester->position.column++;
		}
		else if (harvester->position.column > harvester->target.column) {
			harvester->position.column--;
		}
		printf("Harvester '%s' is returning to base. Current position: (%d, %d)\n",
			harvester->name, harvester->position.row, harvester->position.column);
	}
}

void harvester_combat(Harvester* harvester, Soldier* enemies, int num_enemies) {
	for (int i = 0; i < num_enemies; i++) {
		Soldier* enemy = &enemies[i];
		int distance = abs(harvester->position.row - enemy->position.row) +
			abs(harvester->position.column - enemy->position.column);
		if (distance <= ATTACK_RANGE && enemy->hp > 0) {
			printf("%s is defending against %s\n", harvester->name, enemy->name);
			enemy->hp -= harvester->attack_power;
			if (enemy->hp <= 0) {
				printf("%s has been defeated by %s!\n", enemy->name, harvester->name);
				enemy->position = (POSITION){ -1, -1 }; // 제거
			}
		}
	}
}

void update_combat(Soldier* soldiers, int num_soldiers, Harvester* harvesters, int num_harvesters, Soldier* enemies, int num_enemies) {
	for (int i = 0; i < num_soldiers; i++) {
		if (soldiers[i].hp > 0) {
			soldier_combat(&soldiers[i], enemies, num_enemies);
		}
	}
	for (int i = 0; i < num_harvesters; i++) {
		if (harvesters[i].hp > 0) {
			harvester_combat(&harvesters[i], enemies, num_enemies);
		}
	}
}

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
	init_unit_list(5);  // 최대 유닛 수 5로 설정

	init_harvesters();
	init_soldiers();
	add_soldier("Soldier1", 100, (POSITION) { 3, 3 });
    add_harvester("Harvester1", 100, (POSITION){5, 5});


	init();
	intro();
	display(resource, map, cursor);
	int clear_screen_interval = 1000; // 1초마다 화면을 지우는 간격
	int last_clear_time = 0;


	move_command(&soldiers[0], (POSITION) { 7, 7 });
	harvester_command(&harvesters[0], (POSITION) { 6, 6 }, HARVESTER_MOVING);
	while (1) {
		// loop 돌 때마다(즉, TICK==10ms마다) 키 입력 확인
		KEY key = get_key();

		if (key != k_none) {
			printf("key pressed: %d\n", key);
			display(resource, map, cursor);
		}
		else {
			Sleep(TICK);
			continue;
		}
		// 키 입력이 있으면 처리
		if (is_arrow_key(key)) {
			cursor_move(ktod(key));
		}
		else {
			// 방향키 외의 입력
			switch (key) {
			case ' ':
				if (map[0][cursor.current.row][cursor.current.column] != ' ') {
					selected_position = cursor.current;
					printf("지형 또는 오브젝트 선택됨: (%d, %d)\n", selected_position.row, selected_position.column);
					display_status_window(selected_position);
				}
				break;
			case 'B': // 건설 모드 단축키 설정
				build_mode_toggle();
				break;

			case 'X':
				cancel_building(); 
				break;
			case 27:
				selected_position.row = -1;
				selected_position.column = -1;
				display_status_window(selected_position);
				printf("선택 취소됨\n");
				break;
			case 'H':
				printf("Harvester를 생산합니다.\n");
				produce_unit_or_building('H');
				display_system_message();
				break;
			case 'P': 
				if (build_mode) {
					if (resource.spice >= 30) { // 30 spice가 필요하다고 예를 들음
						printf("Plate를 건설합니다.\n"); 
						resource.spice -= 30; 
						build_timer = 3000;  
						start_building('P');
					}
					else {
						printf("자원이 부족하여 건설할 수 없습니다.\n");
						add_system_message("Not enough spice ");
					}
				}
				else {
					printf("건설 모드가 활성화되지 않았습니다.\n");
				}
				break;
			case 'F':
				printf("Fremen을 생산합니다.\n");
				break;
			case 'S':
				printf("Soldier를 생산합니다.\n");
				break;
			case 'T':
				printf("Heavy Tank를 생산합니다.\n");
				break;
			case 'G':
				printf("Fighter를 생산합니다.\n");
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

		// 샘플 오브젝트 동작
		sample_obj_move();

		sandworm_move();


		// 화면 출력
		display(resource, map, cursor);
		Sleep(TICK);
		sys_clock += TICK;


	}
}



void init_unit_list(int supply_max) {
	unit_list = (UnitList*)malloc(sizeof(UnitList));
	if (!unit_list) {
		printf("유닛 리스트 초기화 실패.\n");
		exit(1);
	}
	unit_list->head = NULL;
	unit_list->count = 0;
	unit_list->supply_max = supply_max;
}

bool add_unit(char type, const char* name, int hp) {
	if (unit_list->count >= unit_list->supply_max) {
		printf("공급 제한 초과: 유닛 추가 실패.\n");
		return false;
	}

	Unit* new_unit = (Unit*)malloc(sizeof(Unit));
	if (!new_unit) {
		printf("유닛 메모리 할당 실패.\n");
		return false;
	}

	new_unit->type = type;
	strncpy(new_unit->name, name, sizeof(new_unit->name) - 1);
	new_unit->name[sizeof(new_unit->name) - 1] = '\0';
	new_unit->hp = hp;
	new_unit->next = unit_list->head;
	unit_list->head = new_unit;
	unit_list->count++;

	printf("유닛 %s 추가됨 (%d/%d).\n", name, unit_list->count, unit_list->supply_max);
	return true;
}

bool remove_unit(const char* name) {
	if (!unit_list->head) {
		printf("유닛 목록이 비어있습니다.\n");
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
			printf("유닛 %s 삭제됨 (%d/%d).\n", name, unit_list->count, unit_list->supply_max);
			return true;
		}
		prev = current;
		current = current->next;
	}

	printf("유닛 %s 찾을 수 없음.\n", name);
	return false;
}

void print_unit_list() {
	if (!unit_list || !unit_list->head) {
		printf("유닛 목록이 비어있습니다.\n");
		return;
	}

	printf("유닛 목록 (%d/%d):\n", unit_list->count, unit_list->supply_max);
	Unit* current = unit_list->head;
	while (current) {
		printf("- 이름: %s, 타입: %c, 체력: %d\n", current->name, current->type, current->hp);
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
	printf("유닛 리스트 메모리 해제 완료.\n");
}


void build_mode_toggle(void) {
	build_mode = !build_mode;
	building_to_place = 'P';
	if (build_mode) {
		add_system_message("건설 모드 활성화");
	}
	else {
		add_system_message("건설 모드 비활성화");
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
	// layer 0(map[0])에 지형 생성
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

	// layer 1(map[1])은 비워 두기(-1로 채움)
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

// (가능하다면) 지정한 방향으로 커서 이동
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
	// 현재 위치와 목적지를 비교해서 이동 방향 결정	
	POSITION diff = psub(obj.dest, obj.pos);
	DIRECTION dir;

	// 목적지 도착. 지금은 단순히 원래 자리로 왕복
	if (diff.row == 0 && diff.column == 0) {
		if (obj.dest.row == 1 && obj.dest.column == 1) {
			// topleft --> bottomright로 목적지 설정
			POSITION new_dest = { MAP_HEIGHT - 2, MAP_WIDTH - 2 };
			obj.dest = new_dest;
		}
		else {
			// bottomright --> topleft로 목적지 설정
			POSITION new_dest = { 1, 1 };
			obj.dest = new_dest;
		}
		return obj.pos;
	}

	// 가로축, 세로축 거리를 비교해서 더 먼 쪽 축으로 이동
	if (abs(diff.row) >= abs(diff.column)) {
		dir = (diff.row >= 0) ? d_down : d_up;
	}
	else {
		dir = (diff.column >= 0) ? d_right : d_left;
	}

	// validation check
	// next_pos가 맵을 벗어나지 않고, (지금은 없지만)장애물에 부딪히지 않으면 다음 위치로 이동
	// 지금은 충돌 시 아무것도 안 하는데, 나중에는 장애물을 피해가거나 적과 전투를 하거나... 등등
	POSITION next_pos = pmove(obj.pos, dir);
	if (1 <= next_pos.row && next_pos.row <= MAP_HEIGHT - 2 && \
		1 <= next_pos.column && next_pos.column <= MAP_WIDTH - 2 && \
		map[1][next_pos.row][next_pos.column] < 0) {

		return next_pos;
	}
	else {
		return obj.pos;  // 제자리
	}
}

void sample_obj_move(void) {
	if (sys_clock <= obj.next_move_time) {
		// 아직 시간이 안 됐음
		return;
	}

	// 오브젝트(건물, 유닛 등)은 layer1(map[1])에 저장
	map[1][obj.pos.row][obj.pos.column] = -1;
	obj.pos = sample_obj_next_position();
	map[1][obj.pos.row][obj.pos.column] = obj.repr;

	obj.next_move_time = sys_clock + obj.speed;
}
//샌드웜 이동 - 가장 가까운 유닛 탐색 -> 해당 방향으로 이동, 잡아먹으면 커지고 가끔 매장지 생성
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
				printf("스파이스 매장지가 생성되었습니다.\n");
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
				add_system_message("Harvester 생산 시작!");
			}
			else if (type == 'S' && resource.spice >= 75) {
				resource.spice -= 75;
				add_system_message("Soldier 생산 시작!");
			}
			else {
				add_system_message("자원이 부족하거나 잘못된 명령입니다.");
			}
			break;
		case 'R':
			if (type == 'F' && resource.spice >= 100) {
				resource.spice -= 100;
				add_system_message("Fremen 생산 시작!");
			}
			else {
				add_system_message("자원이 부족하거나 잘못된 명령입니다.");
			}
			break;
		case 'F':
			if (type == 'T' && resource.spice >= 150) {
				resource.spice -= 150;
				add_system_message("Heavy Tank 생산 시작!");
			}
			else if (type == 'G' && resource.spice >= 125) {
				resource.spice -= 125;
				add_system_message("Fighter 생산 시작!");
			}
			else {
				add_system_message("자원이 부족하거나 잘못된 명령입니다.");
			}
			break;
		default:
			add_system_message("해당 건물에서는 이 명령을 수행할 수 없습니다.");
			break;
		}
	}
	else {
		add_system_message("먼저 건물을 선택하십시오.");
	}
}
//유닛 생산
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