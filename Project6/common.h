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
//유닛 생산 관련 상수
#define UNIT_HARVESTER_COST 50
//시스템 메세지 길이 설정
#define SYSTEM_MESSAGE_LENGTH 100



/* ================= 위치와 방향 =================== */
// 맵에서 위치를 나타내는 구조체
typedef struct {
	int row, column;
} POSITION;

//커서 및 선택 상태
typedef struct {
	POSITION pos;
	bool is_selected;
} CURSOR_STATE;


//시스템 메세지 관리 구조체
typedef struct {
	char messages[MAX_SYSTEM_MESSAGES][SYSTEM_MESSAGE_LENGTH];
	int message_count;
} SYSTEM_MESSAGE_LOG;

extern SYSTEM_MESSAGE_LOG system_message_log;


//샌드웜 구조체
typedef struct {
	POSITION pos;
	int length;
	int growth_rate;
	int speed;
} SANDWORM;

//시스템 상태 저장 하기 위한 구조체 
typedef struct {
	char message[SYSTEM_MESSAGE_LENGTH];
	int production_time_left;
} SYSTEM_STATE;

extern SYSTEM_STATE system_state;
//지형, 객체 매크로
#define TERRAIN_BASE 'B'
#define TERRAIN_HARVESTER 'H'
#define TERRAIN_SPICE '5'
#define TERRAIN_PLATE 'P'
#define TERRAIN_ROCK 'R'
#define TERRAIN_SANDWORM 'W'

//건물 지정
#define BUILDING_BASE 'B'
#define BUILDING_PLATE 'P'
#define BUILDING_DORMITORY 'D'
#define BUILDING_GARAGE 'G'
#define BUILDING_BARRACKS 'R'
#define BUILDING_SHELTER 'S'
#define BUILDING_ARENA 'A'
#define BUILDING_FACTORY 'F'

//유닛 지정
#define UNIT_HARVESTER 'H'
#define UNIT_FREMEN 'F'
#define UNIT_SOLDIER 'S'
#define UNIT_FIGHTER 'G'
#define UNIT_HEAVY_TANK 'T'

// 커서 위치
typedef struct {
	POSITION previous;  // 직전 위치
	POSITION current;   // 현재 위치
} CURSOR;

// 입력 가능한 키 종류.
// 수업에서 enum은 생략했는데, 크게 어렵지 않으니 예제 검색
typedef enum {
	// k_none: 입력된 키가 없음. d_stay(안 움직이는 경우)에 대응
	k_none = 0, k_up, k_right, k_left, k_down,
	k_quit,
	k_undef, // 정의되지 않은 키 입력	
} KEY;


// DIRECTION은 KEY의 부분집합이지만, 의미를 명확하게 하기 위해서 다른 타입으로 정의
typedef enum {
	d_stay = 0, d_up, d_right, d_left, d_down
} DIRECTION;


/* ================= 위치와 방향(2) =================== */
// 편의성을 위한 함수들. KEY, POSITION, DIRECTION 구조체들을 유기적으로 변환

// 편의성 함수
inline POSITION padd(POSITION p1, POSITION p2) {
	POSITION p = { p1.row + p2.row, p1.column + p2.column };
	return p;
}

// p1 - p2
inline POSITION psub(POSITION p1, POSITION p2) {
	POSITION p = { p1.row - p2.row, p1.column - p2.column };
	return p;
}

// 방향키인지 확인하는 함수
#define is_arrow_key(k)		(k_up <= (k) && (k) <= k_down)

// 화살표 '키'(KEY)를 '방향'(DIRECTION)으로 변환. 정수 값은 똑같으니 타입만 바꿔주면 됨
#define ktod(k)		(DIRECTION)(k)

// DIRECTION을 POSITION 벡터로 변환하는 함수
inline POSITION dtop(DIRECTION d) {
	static POSITION direction_vector[] = { {0, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 0} };
	return direction_vector[d];
}

// p를 d 방향으로 이동시킨 POSITION
#define pmove(p, d)		(padd((p), dtop(d)))

/* ================= game data =================== */
typedef struct {
	int spice;		// 현재 보유한 스파이스
	int spice_max;  // 스파이스 최대 저장량
	int population; // 현재 인구 수
	int population_max;  // 수용 가능한 인구 수
} RESOURCE;


// 대강 만들어 봤음. 기능 추가하면서 각자 수정할 것
typedef struct {
	POSITION pos;		// 현재 위치(position)
	POSITION dest;		// 목적지(destination)
	char repr;			// 화면에 표시할 문자(representation)
	int move_period;	// '몇 ms마다 한 칸 움직이는지'를 뜻함
	int next_move_time;	// 다음에 움직일 시간
	int speed;
} OBJECT_SAMPLE;

#endif

