#pragma once
#pragma once
#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>
#include <conio.h>
#include <assert.h>

/* ================= system parameters =================== */
#define TICK 10// time unit(ms)

#define N_LAYER 2
#define MAP_WIDTH	60
#define MAP_HEIGHT	18
#define MAX_SYSTEM_MESSAGES 10
//���� ���� ���� ���
#define UNIT_HARVESTER_COST 50
//�ý��� �޼��� ���� ����
#define SYSTEM_MESSAGE_LENGTH 100



/* ================= ��ġ�� ���� =================== */
// �ʿ��� ��ġ�� ��Ÿ���� ����ü
typedef struct {
	int row, column;
} POSITION;

//Ŀ�� �� ���� ����
typedef struct {
	POSITION pos;
	bool is_selected;
} CURSOR_STATE;


//�ý��� �޼��� ���� ����ü
typedef struct {
	char messages[MAX_SYSTEM_MESSAGES][SYSTEM_MESSAGE_LENGTH];
	int message_count;
} SYSTEM_MESSAGE_LOG;

extern SYSTEM_MESSAGE_LOG system_message_log;


//����� ����ü
typedef struct {
	POSITION pos;
	int length;
	int growth_rate;
	int speed;
} SANDWORM;

//�ý��� ���� ���� �ϱ� ���� ����ü 
typedef struct {
	char message[SYSTEM_MESSAGE_LENGTH];
	int production_time_left;
} SYSTEM_STATE;

extern SYSTEM_STATE system_state;
//����, ��ü ��ũ��
#define TERRAIN_BASE 'B'
#define TERRAIN_HARVESTER 'H'
#define TERRAIN_SPICE '5'
#define TERRAIN_PLATE 'P'
#define TERRAIN_ROCK 'R'
#define TERRAIN_SANDWORM 'W'

//�ǹ� ����
#define BUILDING_BASE 'B'
#define BUILDING_PLATE 'P'
#define BUILDING_DORMITORY 'D'
#define BUILDING_GARAGE 'G'
#define BUILDING_BARRACKS 'R'
#define BUILDING_SHELTER 'S'
#define BUILDING_ARENA 'A'
#define BUILDING_FACTORY 'F'

//���� ����
#define UNIT_HARVESTER 'H'
#define UNIT_FREMEN 'F'
#define UNIT_SOLDIER 'S'
#define UNIT_FIGHTER 'G'
#define UNIT_HEAVY_TANK 'T'

// Ŀ�� ��ġ
typedef struct {
	POSITION previous;  // ���� ��ġ
	POSITION current;   // ���� ��ġ
} CURSOR;

// �Է� ������ Ű ����.
// �������� enum�� �����ߴµ�, ũ�� ����� ������ ���� �˻�
typedef enum {
	// k_none: �Էµ� Ű�� ����. d_stay(�� �����̴� ���)�� ����
	k_none = 0, k_up, k_right, k_left, k_down,
	k_quit,
	k_undef, // ���ǵ��� ���� Ű �Է�	
} KEY;


// DIRECTION�� KEY�� �κ�����������, �ǹ̸� ��Ȯ�ϰ� �ϱ� ���ؼ� �ٸ� Ÿ������ ����
typedef enum {
	d_stay = 0, d_up, d_right, d_left, d_down
} DIRECTION;


/* ================= ��ġ�� ����(2) =================== */
// ���Ǽ��� ���� �Լ���. KEY, POSITION, DIRECTION ����ü���� ���������� ��ȯ

// ���Ǽ� �Լ�
inline POSITION padd(POSITION p1, POSITION p2) {
	POSITION p = { p1.row + p2.row, p1.column + p2.column };
	return p;
}

// p1 - p2
inline POSITION psub(POSITION p1, POSITION p2) {
	POSITION p = { p1.row - p2.row, p1.column - p2.column };
	return p;
}

// ����Ű���� Ȯ���ϴ� �Լ�
#define is_arrow_key(k)		(k_up <= (k) && (k) <= k_down)

// ȭ��ǥ 'Ű'(KEY)�� '����'(DIRECTION)���� ��ȯ. ���� ���� �Ȱ����� Ÿ�Ը� �ٲ��ָ� ��
#define ktod(k)		(DIRECTION)(k)

// DIRECTION�� POSITION ���ͷ� ��ȯ�ϴ� �Լ�
inline POSITION dtop(DIRECTION d) {
	static POSITION direction_vector[] = { {0, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 0} };
	return direction_vector[d];
}

// p�� d �������� �̵���Ų POSITION
#define pmove(p, d)		(padd((p), dtop(d)))

/* ================= game data =================== */
typedef struct {
	int spice;		// ���� ������ �����̽�
	int spice_max;  // �����̽� �ִ� ���差
	int population; // ���� �α� ��
	int population_max;  // ���� ������ �α� ��
} RESOURCE;


// �밭 ����� ����. ��� �߰��ϸ鼭 ���� ������ ��
typedef struct {
	POSITION pos;		// ���� ��ġ(position)
	POSITION dest;		// ������(destination)
	char repr;			// ȭ�鿡 ǥ���� ����(representation)
	int move_period;	// '�� ms���� �� ĭ �����̴���'�� ����
	int next_move_time;	// ������ ������ �ð�
	int speed;
} OBJECT_SAMPLE;

#endif

