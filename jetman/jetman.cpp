#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

#define LEFT 0
#define RIGHT 1
#define RND(a) (rand() % a )

Surface *backdrop;

int screenbottom;
int delay;
int dir,fire,playerdied;

Point player;
Vec2 speed;

int terrain[400];

#define NUMALIENS 4
struct sprite_t { Rect sprite; Vec2 pos; Vec2 dir ; Pen color; int attached; };
sprite_t alien[NUMALIENS];
sprite_t truck,trailer,fuel;

//sprite costumes
Rect explode[] = { Rect(8,0,4,3), Rect(8,3,4,2), Rect(8,5,4,2) };
Rect meteor [] = { Rect(0,6,2,2), Rect(2,6,2,2) };
Rect jetmanwalk [] = { Rect(0,0,2,3), Rect(2,0,2,3), Rect(4,0,2,3), Rect(6,0,2,3) };
Rect jetmanfly []  = { Rect(0,3,2,3), Rect(2,3,2,3) };

 
// platform format: x y length
Vec3 platforms[] = { Vec3 (0,45,320), Vec3 (0,228,320)}; 

// ZX spectrum colors
Pen black = Pen(0,0,0);
Pen blue = Pen(0,0,255);
Pen red = Pen(255,0,0);
Pen magenta = Pen(255,0,255);
Pen green = Pen(0,255,0);
Pen cyan = Pen(0,255,255);
Pen yellow = Pen(255,255,0);
Pen white = Pen(255,255,255);
Pen colors[] = { black,blue,red,magenta,green,cyan,yellow,white};

void laser () {
Point offset;
  screen.pen = colors[RND(7)];
  for (int x=10; x < 200; x++) {
        if ( dir == LEFT)  offset = Vec2(5-x, 10); 
        if ( dir == RIGHT) offset = Vec2(10+x,10); 
        if (RND(200) > x) screen.pixel(player + offset);
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
   if (dir == LEFT) laser = Vec3 (player.x - 200, player.y, player.x);
   if (dir == RIGHT) laser = Vec3 (player.x, player.y, player.x + 200);
   if ((pos.x > laser.x) && (pos.x < laser.z) && abs(laser.y - pos.y) < 2) return 1;
   return 0;
}

void newalien (int n) {
  alien[n].color = colors[1 + RND(6)] ;
  alien[n].pos = Vec2(screen.bounds.w, RND(screen.bounds.h));
  alien[n].dir = Vec2(-1, 0.1f + (RND(5) / 5.0f ));

  if ( RND(2) ) { // opposite side of screen
  	alien[n].pos.x = 0;
  	alien[n].dir.x *= -1;
  	}
}

void colorsprites(Pen color) { screen.sprites->palette[1] = color; } 

void splat ( int n ) {
  channels[2].trigger_attack();
  screen.sprite(explode[0],alien[n].pos);
  newalien(n);
}
void beep () { channels[6].trigger_attack();}

void init_sound() {
  //explosion noise
  channels[2].waveforms   = Waveform::NOISE;
  channels[2].attack_ms   = 3;
  channels[2].decay_ms    = 3;
  channels[2].sustain     = 1;
  channels[2].release_ms  = 10;

  // short chirp - item pickup
  channels[6].waveforms = Waveform::TRIANGLE;
  channels[6].frequency = 1200;
  channels[6].attack_ms = 5;
  channels[6].decay_ms = 5;
  channels[6].sustain = 0;
  channels[6].release_ms = 5;
}

void init() {
  init_sound();
  set_screen_mode(ScreenMode::hires);
  screenbottom = screen.bounds.h - 35;

  backdrop = Surface::load(level);
  screen.sprites = Surface::load(jetman);

  //make black transparent
  screen.sprites->palette[0] = Pen(0,0,0,0); 

  fuel.sprite = Rect(0,8,3,2);
  truck.sprite = Rect(7,10,6,3);
  trailer.sprite = Rect(7,13,6,3);

  player = Point(screen.bounds.w / 2, screen.bounds.h - 50);
  fuel.pos = Vec2(RND(screen.bounds.w),80);
  truck.pos.x = 50;
  trailer.pos.x = 500;

  for (int n=0;n<NUMALIENS;n++) { newalien(n); }
  //scale to different screen sizes
  for (Vec3 &plat : platforms) {
	  plat.x = plat.x  / (320.0f / screen.bounds.w);
	  plat.y = plat.y  / (240.0f / screen.bounds.h);
	  plat.z = plat.z  / (320.0f / screen.bounds.w);
  }
  // make landscape terrain
  for (int x=0;x<400;x++) 
		terrain[x] = 15 + rand() % 4;
}


Vec2 screenpos;;
void gameloop() {
  Rect costume;
  static float time = 170;
  static float flytime = 170;
  int ground = 200;

  playerdied = 0;
  screenpos.x += speed.x / 4;
  player.y += speed.y;
  speed *= 0.9f;
  speed.y += 1.5f; // gravity

  screen.pen = magenta;
  for (int x=0;x<320;x++) {
	  int offset = 10 - int(screenpos.x) % 10;
	  screen.line(Vec2(x,ground + 40),Vec2(x,ground + 40 - terrain[320 - x + offset]));
  }
  // guages
  screen.pen = black;
  screen.rectangle(Rect(90 + flytime,20, 170-flytime, 11));
  screen.rectangle(Rect(90 + time,38, 170-time, 11));
  screen.pen = yellow;

  // direction indicator to truck location
  if (truck.pos.x < screenpos.x) screen.circle(Vec2(35,38),3);
  if (truck.pos.x > screenpos.x) screen.circle(Vec2(55,38),3);

  if (time > 0) time -= 0.05f;

  // flying uses energy, recovers on ground 
  if (ground - player.y > 1) flytime -= 0.3f;
  if (ground - player.y < 1) flytime += 0.1f;
  if (flytime > 170) flytime = 170;
  if (flytime < 0) speed.y = 8;

  for (Vec3 plat : platforms) 
	playerhitplatform(plat);

  //bounce from top of screen
  if (player.y < 45) player.y = 50;
  if (player.y > ground + 20) player.y = ground;

  // walk on ground
  int px = (int) screenpos.x;
  if ( player.y > 180 ) costume = jetmanwalk [( px * 4) % 3]; 
  else costume = jetmanfly [RND(2)]; 

  // enter / exit truck
  if (collide(Vec2(truck.pos.x - screenpos.x,ground),player)) {
	  truck.attached = 1;
	  player.y = ground;
  }
  // jump to exit truck
  if (truck.attached && pressed(Button::DPAD_UP)) {
	  truck.attached = 0;
	  trailer.attached = 0;
	  player.y -= 20;
	  truck.pos.x = screenpos.x + 140;
  }
  if (truck.attached) costume = truck.sprite;
  else screen.sprite(truck.sprite,Vec2(truck.pos.x - screenpos.x,ground));
 
  if (!trailer.attached && truck.attached && collide(player, Vec2(trailer.pos.x - screenpos.x - 50,ground)) ) {
	  trailer.attached = 1;
  } 
  if (trailer.attached && truck.attached) {
  	if (dir == LEFT) trailer.pos.x = screenpos.x + 205;
  	if (dir == RIGHT) trailer.pos.x = screenpos.x + 115;
  }
  if (trailer.attached && dir == RIGHT) screen.sprite(trailer.sprite,Vec2(trailer.pos.x - screenpos.x,ground),SpriteTransform::HORIZONTAL);
  else screen.sprite(trailer.sprite,Vec2(trailer.pos.x - screenpos.x,ground));

  // draw player or vehicle
  colorsprites(white);
  if (dir == LEFT)  screen.sprite(costume,player);
  if (dir == RIGHT) screen.sprite(costume,player,SpriteTransform::HORIZONTAL);
  if (fire == 1 && !truck.attached) laser(); 

  // Fuel pod
    colorsprites(magenta);
    screen.sprite(fuel.sprite,Vec2(fuel.pos.x - screenpos.x, fuel.pos.y));
  // Aliens
  for (int n=0; n < NUMALIENS; n++) {
  	costume = meteor[RND(2)];
  	colorsprites(alien[n].color);
	Point pos = alien[n].pos;
  	if (pos.x < 0 || pos.x > screen.bounds.w )  { newalien(n);}

  	if (hitplatform(pos)) { splat(n); }
  	if (fire && hitlaser(pos)) { splat(n); }
	//immune from aliens inside truck
  	if (collide (pos, player) && !truck.attached) {
		splat(n); 
		playerdied = 1;
		}
  	if (alien[n].dir.x < 0) screen.sprite(costume,pos,SpriteTransform::HORIZONTAL); 
  	else screen.sprite(costume,pos); 
  }

  // player
  if (playerdied) {
                if (fuel.attached) { 
			fuel.attached = 0;
                	fuel.pos.y -= 30;}
  		screen.sprite(explode[0],player);
                player += Vec2( RND(10) - 10, -10);
		delay = 50;
		}
}

void render(uint32_t time) {
  // copy background
  screen.stretch_blit(backdrop,Rect(0,0,256,192),Rect(0,0,screen.bounds.w,screen.bounds.h));
  gameloop();
}

void read_buttons() {
fire = 0;
Vec2 move = joystick;

 if (pressed(Button::DPAD_LEFT) || move.x < 0) {
	speed.x--;
	dir = LEFT; }
 if (pressed(Button::DPAD_RIGHT) || move.x > 0) {
	speed.x++;
	dir = RIGHT; }
 if (pressed(Button::DPAD_UP) || pressed(Button::A) || move.y < 0) speed.y--;
 if (pressed(Button::B) || pressed(Button::Y)) fire = 1;
}

void update(uint32_t time) {
if (!delay) {
	read_buttons();
	} else {
  	player.x += (150 - player.x) / 20;
  	delay--;
	}
  //aliens
  for (int n=0; n < NUMALIENS; n++) 
  	alien[n].pos += alien[n].dir;
  //fuelpods
    if (!hitplatform(fuel.pos)) fuel.pos.y++;
    if (fuel.attached) fuel.pos = (Vec2) player + Vec2(screenpos.x,20);  
    else if (collide(Vec2(fuel.pos.x - screenpos.x ,fuel.pos.y),player) && !truck.attached) fuel.attached = 1; 
    if (pressed(Button::B)) fuel.attached = 0; 
}
