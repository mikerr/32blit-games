#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

Vec3 roomcolor;
Vec3 player;
Vec2 origin;
int dir;

const int LEFT = 0; 
const int RIGHT = 1;
const int UP = 2; 
const int DOWN = 3; 

bool wolf = false;
int day = 0;
int anim = 0 ;
int jump = 0;
int height = 0;

SpriteSheet *backdrop,*sbsprites,*itemsprites;

typedef struct drawobject {
	SpriteSheet *spritesheet;
	Vec3 position;
	Rect costume;
	int flip;
} object;

std::vector<object> drawlist;

bool sort_y (object obj1, object obj2) { return (obj1.position.y < obj2.position.y); }

bool hit (Vec2 a, Vec2 b ) {
        Vec2 diff;
        diff.x = a.x - b.x;
        diff.y = a.y - b.y;
        float distance = sqrt ( diff.x * diff.x  + diff.y * diff.y);

	return ( distance < 0.8f );
}

void newroom () {
        roomcolor = Vec3(100 + rand() % 100,rand() % 255,rand() % 255);
}

Vec2 isometric (Vec3 grid){
  Vec2 screen;
  screen.x = 150 + grid.x * 9 - grid.y * 9 ;
  screen.y = 30 + grid.x * 5 + grid.y * 5;
  screen.y -= 2 * grid.z;
  return screen;
}

void init() {
  set_screen_mode(ScreenMode::hires);
  origin = Vec2(0,0);
  player = Vec3(8,8,0);
  backdrop = SpriteSheet::load(room);

  sbsprites = SpriteSheet::load(sabreman);
  //make black transparent
  sbsprites->palette[0] = Pen(0,0,0,0); 

  itemsprites = SpriteSheet::load(sprites);
  itemsprites->palette[0] = Pen(0,0,0,0); 
  newroom();
}

void render(uint32_t time) {

  backdrop->palette[1] = Pen(roomcolor.x,roomcolor.y,roomcolor.z);
  screen.stretch_blit(backdrop,Rect(0,0,264,192),Rect(0,0,screen.bounds.w,screen.bounds.h));

  player.z += jump;
  height = player.z;
  if (height > 10) jump = -1;
  if (height <= 0) jump = 0;

  if (time % 3 == 0 ) {
	anim += 3;
	if (anim > 15) anim = 0;
	}

  Rect sabremanback = Rect(anim,0,3,4);
  Rect sabremanfront = Rect(anim,4,3,4);

  if (wolf) {
  	 sabremanback = Rect(anim,8,3,4);
  	 sabremanfront = Rect(anim,12,3,4);
	}

  Rect sbcostume;
  int flip;
  if (dir == LEFT)    { sbcostume = sabremanback; flip = 0;}
  if (dir == RIGHT)   { sbcostume = sabremanfront; flip = 0;}
  if (dir == UP)      { sbcostume = sabremanback; flip = 1;}
  if (dir == DOWN)    { sbcostume = sabremanfront; flip = 1;}

  Rect block = Rect(36,4,4,4);

  drawlist.clear();
  drawobject obj;

  obj.spritesheet = sbsprites;
  obj.costume = sbcostume;
  obj.position = player;
  obj.flip = flip;
  drawlist.push_back(obj);

  obj.spritesheet = itemsprites;
  obj.costume = block;
  obj.position = Vec3(5,5,0);
  obj.flip = 0;
  drawlist.push_back(obj);

  obj.position = Vec3(9,5,0);
  drawlist.push_back(obj);

  obj.position = Vec3(5,9,0);
  drawlist.push_back(obj);

  //sort and draw by y axis 
  std::sort(drawlist.begin(),drawlist.end(),sort_y);
  for (auto &item: drawlist) {
  	screen.sprites = item.spritesheet;
  	screen.sprite(item.costume,isometric(item.position),origin,1,item.flip); 
	}

  screen.sprites = itemsprites;
  Rect sun = Rect(29,0,2,2);
  if (wolf) { sun = Rect(31,0,2,2);}
  screen.sprite(sun,Vec2(240 + day / 75,205)); 
  if (day++ > 2400) {
	wolf = !wolf;
	day = 0;
	}
  Rect windowframe = Rect(36,0,6,4);
  screen.sprite(windowframe,Vec2(240,200)); 
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

if ((pressed(Button::A)) && jump == 0 ) jump = 1;

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
 Vec2 playerxy =Vec2(player.x,player.y);
 if (hit(playerxy, Vec2(0,8)) && dir == LEFT) {
        player = Vec3(16,8,0);
        newroom();
        }
 if (hit(playerxy, Vec2(16,8)) && dir == RIGHT) {
        player = Vec3(0,8,0);
        newroom();
        }
 if (hit(playerxy, Vec2(8,0)) && dir == UP) {
        player = Vec3(8,16,0);
        newroom();
        }
 if (hit(playerxy, Vec2(8,16)) && dir == DOWN) {
        player = Vec3(8,0,0);
        newroom();
        }

}
