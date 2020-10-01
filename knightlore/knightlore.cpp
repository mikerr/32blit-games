#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

Vec3 roomcolor;
Vec2 player,screenpos,origin;
int dir;

const int LEFT = 0; 
const int RIGHT = 1;
const int UP = 2; 
const int DOWN = 3; 

bool wolf = false;
int anim = 0 ;

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
  //make black transparent
  screen.sprites->palette[0] = Pen(0,0,0,0); 
}

void render(uint32_t time) {

  backdrop->palette[1] = Pen(roomcolor.x,roomcolor.y,roomcolor.z);
  screen.stretch_blit(backdrop,Rect(0,0,264,192),Rect(0,0,screen.bounds.w,screen.bounds.h));

  screenpos.x = 150 + player.x * 9 - player.y * 9 ;
  screenpos.y = 30 + player.x * 5 + player.y * 5;

  if (time % 2 == 0) {
	if (++anim > 5) anim = 0;
	}

  int frame = anim * 3;
  Rect sabremanback = Rect(frame,0,3,4);
  Rect sabremanfront = Rect(frame,4,3,4);

  if (wolf) {
  	 sabremanback = Rect(frame,8,3,4);
  	 sabremanfront = Rect(frame,12,3,4);
	}

  if (dir == LEFT)    { screen.sprite(sabremanback,screenpos); }
  if (dir == RIGHT)   { screen.sprite(sabremanfront,screenpos); }
  if (dir == UP)      { screen.sprite(sabremanback,screenpos,origin,1,SpriteTransform::HORIZONTAL); }
  if (dir == DOWN)    { screen.sprite(sabremanfront,screenpos,origin,1,SpriteTransform::HORIZONTAL); }
}

void update(uint32_t time) {

Vec2 move = joystick;

// joystick diagonals to match isometric display
if (move.x < 0 && move.y < 0 ) dir = LEFT;
if (move.x > 0 && move.y > 0 ) dir = RIGHT;
if (move.x > 0 && move.y < 0 ) dir = UP;
if (move.x < 0 && move.y > 0 ) dir = DOWN;

if (pressed(Button::DPAD_LEFT)) dir = LEFT;
if (pressed(Button::DPAD_RIGHT)) dir = RIGHT;
if (pressed(Button::DPAD_UP)) dir = UP;
if (pressed(Button::DPAD_DOWN)) dir = DOWN;

switch (dir) {
	case (LEFT):
        	if (player.x > 0) player.x -= 0.1;
		break;
	case (RIGHT):
        	if (player.x < 16) player.x += 0.1;
		break;
	case (UP):
        	if (player.y > 0) player.y -= 0.1;
		break;
	case (DOWN):
        	if (player.y < 16) player.y += 0.1;
		break;
}

// doors 
 if (hit(player, Vec2(0,8)) && dir == LEFT) {
        player = Vec2(16,8);
        newroom();
        }
 if (hit(player, Vec2(16,8)) && dir == RIGHT) {
        player = Vec2(0,8);
        newroom();
        }
 if (hit(player, Vec2(8,0)) && dir == UP) {
        player = Vec2(8,16);
        newroom();
        }
 if (hit(player, Vec2(8,16)) && dir == DOWN) {
        player = Vec2(8,0);
        newroom();
        }
}
