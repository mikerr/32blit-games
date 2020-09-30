#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

Vec3 roomcolor;
Vec2 player,screenpos,origin;
int dir;

const int d_left = 0; 
const int d_up = 1; 
const int d_down = 2; 
const int d_right = 3;

bool wolf = false;

SpriteSheet *backdrop;

bool hit (Vec2 a, Vec2 b ) {
        Vec2 diff;
        diff.x = a.x - b.x;
        diff.y = a.y - b.y;
        float distance = sqrt ( diff.x * diff.x  + diff.y * diff.y);

	return ( distance < 0.8f );
}

void newroom () {
        roomcolor = Vec3(100 + rand() % 100,rand() % 255,rand() % 255);
	wolf = !wolf;
}

void init() {
  set_screen_mode(ScreenMode::hires);
  origin = Vec2(0,0);
  newroom();
  player = Vec2(8,8);
  backdrop = SpriteSheet::load(room);
  screen.sprites = SpriteSheet::load(sabreman);
}

void render(uint32_t time) {

  backdrop->palette[1] = Pen(roomcolor.x,roomcolor.y,roomcolor.z);
  screen.stretch_blit(backdrop,Rect(0,0,264,192),Rect(0,0,screen.bounds.w,screen.bounds.h));

  screenpos.x = 150 + player.x * 9 - player.y * 9 ;
  screenpos.y = 30 + player.x * 5 + player.y * 5;

  Rect sabremanback = Rect(0,0,3,4);
  Rect sabremanfront = Rect(0,4,3,4);

  if (time % 1000> 500) {}
  if (wolf) {
  	 sabremanback = Rect(0,8,3,4);
  	 sabremanfront = Rect(4,12,3,4);
	}

  if (dir == d_left)    { screen.sprite(sabremanback,screenpos); }
  if (dir == d_up)      { screen.sprite(sabremanback,screenpos,origin,1,SpriteTransform::HORIZONTAL); }
  if (dir == d_right)   { screen.sprite(sabremanfront,screenpos); }
  if (dir == d_down)    { screen.sprite(sabremanfront,screenpos,origin,1,SpriteTransform::HORIZONTAL); }
}

void update(uint32_t time) {

Vec2 move = joystick;

// joystick diagonals 
if (move.x < 0 && move.y < 0 ) dir = d_left;
if (move.x < 0 && move.y > 0 ) dir = d_down;
if (move.x > 0 && move.y < 0 ) dir = d_up;
if (move.x > 0 && move.y > 0 ) dir = d_right;

if (pressed(Button::DPAD_LEFT)) dir = d_left;
if (pressed(Button::DPAD_RIGHT)) dir = d_right;
if (pressed(Button::DPAD_UP)) dir = d_up;
if (pressed(Button::DPAD_DOWN)) dir = d_down;

switch (dir) {
	case (d_left):
        	if (player.x > 0) player.x -= 0.1;
		break;
	case (d_right):
        	if (player.x < 16) player.x += 0.1;
		break;
	case (d_up):
        	if (player.y > 0) player.y -= 0.1;
		break;
	case (d_down):
        	if (player.y < 16) player.y += 0.1;
		break;
}

// doors 
 if (hit(player, Vec2(0,8)) && dir == d_left) {
        player = Vec2(16,8);
        newroom();
        }
 if (hit(player, Vec2(16,8)) && dir == d_right) {
        player = Vec2(0,8);
        newroom();
        }
 if (hit(player, Vec2(8,0)) && dir == d_up) {
        player = Vec2(8,16);
        newroom();
        }
 if (hit(player, Vec2(8,16)) && dir == d_down) {
        player = Vec2(8,0);
        newroom();
        }
}
