#pragma once

#include <stdlib.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <thread>
#include <math.h>

#define _OUTFILE
#define OUTFILE_NAME "out.txt"


static const uint32_t FPS_MAX = 120;
static const uint32_t INPUT_C = 4;
static const uint32_t ENTITY_MAX = 40;

typedef struct keystate_t {
	char key;
	bool down;
	bool pressed;
	bool released;
	keystate_t(char k) : key(k), down(false), pressed(false), released(false) {}
};

typedef struct entity_t {
	float x_pos;
	float y_pos;
	int id;
	entity_t(float x, float y, int i) : x_pos(x), y_pos(y), id(i) {}
};

typedef std::pair<std::string, keystate_t> keybind_t;

const int SCREEN_WIDTH		= 120; // width of console window
const int SCREEN_HEIGHT		= 30;  // height of console window
const int FLOOR				= 4;   // floor height in game
const int CEIL				= 2;   // ceiling height in game

const int FUNNEL_X			= 6; // spacing from left side of screen
const int INPUT_SPACING		= 2; // space between input tiles
const int INPUT_WIDTH		= 4; // width of input tiles
const int INPUT_HEIGHT		= 3; // height of input tiles
const int INPUT_HIT_WINDOW	= 2; // size of hitbox of input tiles

const float ENTITY_SPEED = 0.15; // speed of falling entities

const float SPAWN_TIME = 40;
const float HIT_TIME = 10;

const int SCREEN_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT;

std::ofstream out_file;
wchar_t* screen = new wchar_t[SCREEN_WIDTH * SCREEN_HEIGHT + 1];
std::vector<entity_t> ents;

size_t track_frame = 0;
size_t frame = 0;
size_t next_frame = 0;
size_t dropped_total = 0;
size_t dropped_hit = 0;
size_t hit_frame = 0;
bool on_hit = false;


std::vector<keybind_t> keybinds = {
	{"W", keystate_t('W')},
	{"A", keystate_t('A')},
	{"S", keystate_t('S')},
	{"D", keystate_t('D')},
	{"R", keystate_t('R')},
	{"F", keystate_t('F')},
};

const char inputs[INPUT_C] = {
	'A',
	'W',
	'S',
	'D',
};

void init();

bool update();

void backdrop();

void write_string(std::string msg, unsigned int x, unsigned int y);

void update_keystates();


bool get_key_down(std::string key_action);
bool get_key_pressed(std::string key_action);
bool get_key_released(std::string key_action);
bool get_key_down_char(char key_char);

void out_print(std::string str);
void out_println(std::string str);