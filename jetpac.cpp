#include "32blit.hpp"
#include "jetpac.hpp"

using namespace blit;

#define LEFT 0
#define RIGHT 1

int screenbottom,dir,fire;
int fuelled,fuelgrabbed;

float gravity;
Point player,fuelpos;
Vec2 speed;
SpriteSheet *level;

#define NUMALIENS 4
Vec2 alienpos[NUMALIENS],aliendir[NUMALIENS];

Rect explode[] = { Rect(8,0,4,3), Rect(8,3,4,2), Rect(8,5,4,2) };
Rect meteor [] = { Rect(0,6,2,2), Rect(2,6,2,2) };
Rect rocketparts[] = { Rect(5,11,2,2), Rect(5,9,2,3), Rect(5,6,2,3) };
Rect jetmanwalk [] = { Rect(0,0,2,3), Rect(2,0,2,3), Rect(4,0,2,3), Rect(6,0,2,3) };
Rect jetmanfly []  = { Rect(0,3,2,3), Rect(2,3,2,3) };
Rect fuel = Rect(0,8,3,2);

Vec3 platforms[] = { Vec3 (30,93,100), Vec3 (139,122,190), Vec3 (221,64,295)}; 

Pen white = Pen(255,255,255);
Pen magenta = Pen(255,0,255);


void laser () {
Point offset;
  screen.pen = Pen(rand() % 255, rand() % 255, rand() % 255);
  for (int x=10; x < 100; x++) {
        if ( dir == LEFT)  offset = Vec2(5-x, 10); 
        if ( dir == RIGHT) offset = Vec2(10+x,10); 
        if ( (rand() % 3) < 1 ) screen.pixel(player + offset);
        }
}

int collide (Point a, Point b) {
   if (abs(a.x - b.x) < 15  && abs(a.y - b.y) < 15 ) return 1;
   return 0;
}

void playerhitplatform (Vec3 platform){
  if ((player.x > platform.x) && (player.x < platform.z)) {
    int ypos = platform.y;

    // if hitting underside, bounce down
    if ((speed.y < 0) && (abs(player.y - ypos)) < 3) { 
	player.y = ypos + 3; speed.y = 6;}

    // if on top, land
    ypos -= 25;
    if ((speed.y > 0) && (abs(player.y - ypos)) < 5) { 
	player.y = ypos - 1; speed.y = 0;}
  }
}

int hitplatform(Point pos){
  for (Vec3 plat : platforms) 
  	if ((pos.x > plat.x) && (pos.x < plat.z) && abs(plat.y - pos.y) < 20) return 1;
  return 0;
}

int hitlaser(Point pos){
Vec3 laser;

   if (dir == LEFT) laser = Vec3 (player.x - 100, player.y, player.x);
   if (dir == RIGHT) laser = Vec3 (player.x, player.y, player.x + 100);
   if ((pos.x > laser.x) && (pos.x < laser.z) && abs(laser.y - pos.y) < 2) return 1;
   return 0;
}

void newalien (int n) {
  alienpos[n] = Vec2(300, rand() % 200);
  aliendir[n] = Vec2(-1, 0.1 + ((rand() % 20) / 20.0f ));

  if ( rand() % 2 ) {
  alienpos[n].x = 0;
  aliendir[n].x = 1 + (rand() % 10 / 10.0f);
  }
}

void colorsprites(Pen color) { screen.sprites->palette[1] = color; }

void init() {
  set_screen_mode(ScreenMode::hires);
  level = SpriteSheet::load(level1);
  screen.sprites = SpriteSheet::load(jetpac);
  gravity = 1.5;
  screenbottom = screen.bounds.h - 35;
  player = Point(150,screenbottom);
  fuelpos = Point(rand() % 25,10);
  for (int n=0;n<NUMALIENS;n++) {
	newalien(n);
  }
}

void render(uint32_t time) {
  static int playerdied, takeoff = -10;
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
	costume = jetmanwalk [player.x % 3];
	} else { 
	costume = jetmanfly [rand() % 2];
        }
  colorsprites(white);
  if (dir == LEFT)  screen.sprite(costume,player);
  if (dir == RIGHT) screen.sprite(costume,player,SpriteTransform::HORIZONTAL);
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
  screen.sprite(fuel,fuelpos);
  if (fuelpos.y < screenbottom + 8 && !hitplatform(fuelpos)) fuelpos.y++;
  if (fuelpos.x > 200 && fuelpos.x < 210) {
	fuelgrabbed = 0; 
  	if (fuelpos.y > screenbottom) {
		fuelled++;
		fuelpos = Point(rand() % 250,10);
		}
	}
  if (collide(fuelpos,player)) fuelgrabbed = 1; 
  if (fuelgrabbed) fuelpos = player + Point(0,20);  

  // Aliens
  costume = meteor[rand() % 2];
  colorsprites(Pen(255, 0, 0));

  for (int n=0; n < NUMALIENS; n++) {
  	alienpos[n] += aliendir[n];
  	if (alienpos[n].x < 0 || alienpos[n].x > 350 )  { newalien(n);}
  	if (alienpos[n].y > screenbottom )  { newalien(n);}

  	if (hitplatform(alienpos[n])) { newalien(n);}
  	if (fire && hitlaser(alienpos[n])) { newalien(n); }
  	if (collide (alienpos[n], player)) {
		playerdied = 1;
		newalien(n);
		}
  	if (aliendir[n].x < 0) screen.sprite(costume,alienpos[n],SpriteTransform::HORIZONTAL); 
  	if (aliendir[n].x > 0) screen.sprite(costume,alienpos[n]);
  }
  if (playerdied) {
  		player = Point(150,screenbottom);
                if (fuelgrabbed) { 
			fuelgrabbed = 0;
                	fuelpos.y -= 20;}
		playerdied = 0;
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
 if (pressed(Button::DPAD_UP) || pressed(Button::X) || move.y < 0) speed.y--;
 if (pressed(Button::B) || pressed(Button::A)) fire = 1;
}
