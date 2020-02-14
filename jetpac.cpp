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
Vec2 player,alienpos,fuelpos;
Vec2 speed,o,zoom;
SpriteSheet *level;

void init() {
  set_screen_mode(ScreenMode::hires);
  level = SpriteSheet::load(level1);
  screen.sprites = SpriteSheet::load(jetpac);
  zoom = Vec2(1,1);
  gravity = 1.5;
  screenbottom = screen.bounds.h - 35;
  player = Vec2(150,screenbottom);
  fuelpos = Vec2(rand() % 25,10);
}

void laser () {
Vec2 offset;
  screen.pen = Pen(rand() % 255, rand() % 255, rand() % 255);
  for (int x=10; x < 100; x++) {
        if ( dir == LEFT)  offset = Vec2(5-x, 10); 
        if ( dir == RIGHT) offset = Vec2(10+x,10); 
        if ( (rand() % 2) < 1 ) screen.pixel(player + offset);
        }
}
int collide (Vec2 a, Vec2 b) {
   if (abs(a.x - b.x) < 15  && abs(a.y - b.y) < 15 ) return 1;
   return 0;
}
void platform (int xleft,int ypos,int xright){
  if ((player.x > xleft) && (player.x < xright)) {

    // check if hitting underside, bounce down
    if ((speed.y < 0) && (abs(player.y - ypos)) < 3) { 
	player.y = ypos + 3; speed.y = 6;}

    // check if on top, land
    ypos -= 25;
    if ((speed.y > 0) && (abs(player.y - ypos)) < 5) { 
	player.y = ypos - 1; speed.y = 0;}
  }
}
int checkplatforms(Vec2 pos){
  Vec3 plat = Vec3 (30,93,100); 
  if ((pos.x > plat.x) && (pos.x < plat.z) && (plat.y - pos.y) < 20) return 1;
  plat = Vec3 (139,122,190); 
  if ((pos.x > plat.x) && (pos.x < plat.z) && (plat.y - pos.y) < 20) return 1;
  plat = Vec3 (221,64,295); 
  if ((pos.x > plat.x) && (pos.x < plat.z) && (plat.y - pos.y) < 20) return 1;
  return 0;
}
void render(uint32_t time) {
  Rect costume;
  Rect explode[] = { Rect(8,0,4,3), Rect(8,3,4,2), Rect(8,5,4,2) };
  Rect meteor[] = { Rect(0,6,2,2), Rect(2,6,2,2) };

  screen.stretch_blit(level,Rect(0,0,256,192),Rect(0,0,screen.bounds.w,screen.bounds.h));

  player += speed;
  speed.x = speed.x * 0.3;
  speed.y = speed.y * 0.8;
  speed.y += gravity;

  if (player.x > screen.bounds.w) { player.x = 0; }
  if (player.x < 0 ) { player.x = screen.bounds.w; }
  if (player.y < 15) { speed.y = 3; player.y = 18; }
  if (player.y > screenbottom) { speed.y = 0; }

  platform(30,93,100);
  platform(139,122,190);
  platform(221,64,295);

  if (speed.y == 0){
  	int animframe = 2 * (int(player.x *3) % 4);
	costume = Rect(animframe,0,2,3); // WALK
	} else { 
  	int animframe = 2 * (rand() % 2);
	costume = Rect(animframe,3,2,3); //FLY
        }
  screen.sprites->palette[1] = Pen(255,255,255);
  if (dir == LEFT)  screen.sprite(costume,player);
  if (dir == RIGHT) screen.sprite(costume,player,o,zoom,MIRROR);
  if (fire == 1) { laser(); } 
 
  screen.sprites->palette[1] = Pen(255,255,255);
  Rect rocket[] = { Rect(5,6,2,3), Rect(5,9,2,3), Rect(5,11,2,2) };
  if (fuelled > 2) screen.sprites->palette[1] = Pen(255,0,255);
  screen.sprite(rocket[0],Vec2(205,screenbottom-26));
  if (fuelled > 1) screen.sprites->palette[1] = Pen(255,0,255);
  screen.sprite(rocket[1],Vec2(205,screenbottom-10));
  if (fuelled > 0) screen.sprites->palette[1] = Pen(255,0,255);
  screen.sprite(rocket[2],Vec2(205,screenbottom+10));

  Rect fuel = Rect(0,8,3,2);
  screen.sprites->palette[1] = Pen(255,0,255);
  screen.sprite(fuel,fuelpos,o,zoom);
  if (fuelpos.y < screenbottom + 8 && !checkplatforms(fuelpos)) fuelpos.y++;
  if (fuelpos.x > 200 && fuelpos.x < 210) {
	fuelgrabbed = 0; 
  	if (fuelpos.y > screenbottom) {
		fuelled++;
		fuelpos = Vec2(rand() % 250,10);
		}
	}
  if (collide(fuelpos,player)) fuelgrabbed = 1; 
  if (fuelgrabbed) fuelpos = player + Vec2(0,20);  

  alienpos += Vec2 (-1,0.2);
  if (alienpos.x < 0 || alienpos.y > screenbottom) alienpos = Vec2(300,rand() % 200);
  costume = meteor[alienpos.x % 2];
  screen.sprites->palette[1] = Pen(rand() % 255,255,255);
  screen.sprite(costume,alienpos,o,zoom,MIRROR);
  if (collide (alienpos, player)) 
	{
	screen.sprite(explode[0],alienpos);
  	player = Vec2(150,screenbottom);
  	alienpos = Vec2(300,rand() % 200);
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
