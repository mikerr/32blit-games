#include "32blit.hpp"
#include "jetpac.hpp"

using namespace blit;

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

#define MIRROR 1

int screenbottom,dir,fire;
int fuelled,fuelgrabbed;

float gravity;
Point player,alienpos,fuelpos;
Vec2 speed,o,zoom;
SpriteSheet *level;

Rect explode[] = { Rect(8,0,4,3), Rect(8,3,4,2), Rect(8,5,4,2) };
Rect meteor[] = { Rect(0,6,2,2), Rect(2,6,2,2) };
Rect rocketparts[] = { Rect(5,11,2,2), Rect(5,9,2,3), Rect(5,6,2,3) };
Rect jetmanwalk [] = { Rect(0,0,2,3), Rect(2,0,2,3), Rect(4,0,2,3), Rect(6,0,2,3) };
Rect jetmanfly []  = { Rect(0,3,2,3), Rect(2,3,2,3) };
Rect fuel = Rect(0,8,3,2);

Vec3 platforms[] = { Vec3 (30,93,100), Vec3 (139,122,190), Vec3 (221,64,295)}; 

Pen white = Pen(255,255,255);
Pen magenta = Pen(255,0,255);

void init() {
  set_screen_mode(ScreenMode::hires);
  level = SpriteSheet::load(level1);
  screen.sprites = SpriteSheet::load(jetpac);
  zoom = Vec2(1,1);
  gravity = 1.5;
  screenbottom = screen.bounds.h - 35;
  player = Point(150,screenbottom);
  fuelpos = Point(rand() % 25,10);
}

void laser () {
Point offset;
  screen.pen = Pen(rand() % 255, rand() % 255, rand() % 255);
  for (int x=10; x < 100; x++) {
        if ( dir == LEFT)  offset = Vec2(5-x, 10); 
        if ( dir == RIGHT) offset = Vec2(10+x,10); 
        if ( (rand() % 2) < 1 ) screen.pixel(player + offset);
        }
}
int collide (Point a, Point b) {
   if (abs(a.x - b.x) < 15  && abs(a.y - b.y) < 15 ) return 1;
   return 0;
}
void playerhitplatform (Vec3 platform){

  if ((player.x > platform.x) && (player.x < platform.z)) {
    int ypos = platform.y;

    // check if hitting underside, bounce down
    if ((speed.y < 0) && (abs(player.y - ypos)) < 3) { 
	player.y = ypos + 3; speed.y = 6;}

    // check if on top, land
    ypos -= 25;
    if ((speed.y > 0) && (abs(player.y - ypos)) < 5) { 
	player.y = ypos - 1; speed.y = 0;}
  }
}
int hitplatforms(Point pos){
  for (Vec3 plat : platforms) 
  	if ((pos.x > plat.x) && (pos.x < plat.z) && (plat.y - pos.y) < 20) return 1;
  return 0;
}
void colorsprites(Pen color) {
  screen.sprites->palette[1] = color;
}
void render(uint32_t time) {
  static int takeoff = -10;
  Rect costume;

  // background
  screen.stretch_blit(level,Rect(0,0,256,192),Rect(0,0,screen.bounds.w,screen.bounds.h));

  player += speed;
  speed.x = speed.x * 0.3;
  speed.y = speed.y * 0.8;
  speed.y += gravity;

  if (player.x > screen.bounds.w) { player.x = 0; }
  if (player.x < 0 ) { player.x = screen.bounds.w; }
  if (player.y < 15) { speed.y = 3; player.y = 18; }
  if (player.y > screenbottom) { speed.y = 0; }

  for (Vec3 plat : platforms) 
	playerhitplatform(plat);

  if (speed.y == 0){
  	int animframe = player.x % 3;
	costume = jetmanwalk[animframe];
	} else { 
  	int animframe = rand() % 2;
	costume = jetmanfly[animframe];
        }

  colorsprites(white);
  if (dir == LEFT)  screen.sprite(costume,player);
  if (dir == RIGHT) screen.sprite(costume,player,o,zoom,MIRROR);
  if (fire == 1) { laser(); } 
 
  // Rocket
  Point pos = Point(205,screenbottom - takeoff);
  for (int i=0; i < 3 ; i++) {
	colorsprites(white);
	if (fuelled > i) colorsprites(magenta);
  	screen.sprite(rocketparts[i],pos);
	pos += Vec2(0,-16);
	}
  if (fuelled > 3) takeoff++; 
  if (takeoff > screen.bounds.h) { fuelled = 0; takeoff = -10; }

  // Fuel pod
  colorsprites(magenta);
  screen.sprite(fuel,fuelpos,o,zoom);
  if (fuelpos.y < screenbottom + 8 && !hitplatforms(fuelpos)) fuelpos.y++;
  if (fuelpos.x > 200 && fuelpos.x < 210) {
	fuelgrabbed = 0; 
  	if (fuelpos.y > screenbottom) {
		fuelled++;
		fuelpos = Point(rand() % 250,10);
		}
	}
  if (collide(fuelpos,player)) fuelgrabbed = 1; 
  if (fuelgrabbed) fuelpos = player + Point(0,15);  

  // Aliens
  alienpos += Vec2 (-1,0.2);
  if (alienpos.x < 0 || alienpos.y > screenbottom) alienpos = Point(300,rand() % 200);
  costume = meteor[int(alienpos.x) % 2];
  colorsprites(Pen(rand() % 255,255,255));
  screen.sprite(costume,alienpos,o,zoom,MIRROR);
  if (collide (alienpos, player)) 
	{
	screen.sprite(explode[0],alienpos);
  	player = Point(150,screenbottom);
  	alienpos = Point(300,rand() % 200);
	}
}

void update(uint32_t time) {
 fire = 0;
 Vec2 move = blit::joystick;

 if (pressed(Button::DPAD_LEFT) || move.x < 0) {
	speed.x--;
	dir = LEFT;
	}
 if (pressed(Button::DPAD_RIGHT) || move.x > 0) {
	speed.x++;
	dir = RIGHT;
	}
 if (pressed(Button::DPAD_UP) || move.y < 0) speed.y--;
 if (pressed(Button::B) || pressed(Button::DPAD_DOWN)) fire = 1;
}
