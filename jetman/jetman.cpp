#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

#define LEFT 0
#define RIGHT 1
#define RND(a) (rand() % a )

Surface *backdrop;

int delay,dir,fire,playerdied;
int ground = 200;

float missiletime,flytime;

Vec2 screenpos;

uint8_t terrain[1000];

#define NUMALIENS 20
struct sprite_t { Rect sprite; Vec2 pos; Vec2 speed; Pen color; int attached; int driving; int dir; int explode;};
sprite_t alien[NUMALIENS], truck, trailer, missile, alienbase, basecannon,basecannon2, bomb, fuelpod, bricks, player;

//sprite costumes
Rect costume;
Rect explode[] = { Rect(8,0,4,3), Rect(8,3,4,2), Rect(8,5,4,2) };
Rect meteor [] = { Rect(0,13,3,2), Rect(0,13,3,2) };
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
        if (RND(200) > x) screen.pixel(player.pos + offset);
        }  
}

int collide (Point a, Point b) {
   if (abs(a.x - b.x) < 15  && abs(a.y - b.y) < 15 ) return 1;
   return 0;
}

int hitplatform(Point pos){
  for (Vec3 plat : platforms) 
  	if ((pos.x > plat.x) && (pos.x < plat.z) && abs(plat.y - pos.y) < 20) return 1;
  return 0;
}

int hitlaser(Point pos){
Vec3 laser;
   if (dir == LEFT) laser = Vec3 (player.pos.x - 200, player.pos.y, player.pos.x);
   if (dir == RIGHT) laser = Vec3 (player.pos.x, player.pos.y, player.pos.x + 200);
   if ((pos.x > laser.x) && (pos.x < laser.z) && abs(laser.y - pos.y) < 2) return 1;
   return 0;
}

void newalien (int n) {
  alien[n].color = colors[1 + RND(6)] ;
  alien[n].pos = Vec2(RND(1000),50 + RND(100));
  alien[n].speed = Vec2(-1, 1 );

  if ( RND(2) ) { // opposite side of screen
  	alien[n].pos.x = 0;
  	alien[n].speed.x *= -1;
  	}
  alien[n].explode = 0;
}

void colorsprites(Pen color) { screen.sprites->palette[1] = color; } 

void splat ( int n ) {
  channels[2].trigger_attack();
  //explode for a few frames
  if (alien[n].explode == 0) alien[n].explode = 10;
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

  backdrop = Surface::load(level);
  screen.sprites = Surface::load(jetman);

  //make black transparent
  screen.sprites->palette[0] = Pen(0,0,0,0); 

  fuelpod.sprite = Rect(0,8,2,2);
  bricks.sprite = Rect(14,9,2,2);
  bomb.sprite = Rect(0,10,2,2);
  truck.sprite = Rect(7,10,6,3);
  trailer.sprite = Rect(7,13,6,3);
  basecannon.sprite = Rect(2,8,2,3);
  basecannon2.sprite = Rect(2,8,2,3);
  alienbase.sprite = Rect(12,3,4,6);

  player.pos = Vec2(screen.bounds.w / 2, screen.bounds.h - 50);
  fuelpod.pos = Vec2(100,ground);
  bricks.pos = Vec2(200,ground);
  bomb.pos = Vec2(300,ground);
  truck.pos = Vec2(50,ground);
  trailer.pos = Vec2(500,ground);
  missile.pos = Vec2(1500,100);
  basecannon.pos = Vec2(600,ground);
  basecannon2.pos = Vec2(750,ground);
  alienbase.pos = Vec2(670,ground - 25);

  missiletime = flytime = 170;
  for (int n=0;n<NUMALIENS;n++) { newalien(n); }

  // make landscape terrain
  for (int x=0;x<1000;x++) 
	terrain[x] = 15 + RND(4);
}

void render(uint32_t time) {
  // copy background
  screen.stretch_blit(backdrop,Rect(0,0,256,192),Rect(0,0,screen.bounds.w,screen.bounds.h));

  //terrain
  screen.pen = magenta;
  for (int x=0;x<320;x++) {
	  int offset = (int)screenpos.x % 320;
	  Vec2 groundlevel = Vec2(x,ground + 40);
	  Vec2 rockheight = groundlevel - Vec2(0,terrain[320 + x + offset]);
	  screen.line(groundlevel,rockheight);
  }
  // guages
  screen.pen = black;
  screen.rectangle(Rect(90 + flytime,20, 170-flytime, 11));
  screen.rectangle(Rect(90 + missiletime,38, 170-missiletime, 11));

  // direction indicator to truck location
  screen.pen = yellow;
  if (truck.pos.x < screenpos.x) screen.circle(Vec2(30,38),3);
  if (truck.pos.x > screenpos.x) screen.circle(Vec2(55,38),3);

  // direction indicator to base location
  screen.pen = yellow;
  if (alienbase.pos.x < screenpos.x) screen.circle(Vec2(270,38),3);
  if (alienbase.pos.x > screenpos.x) screen.circle(Vec2(295,38),3);

  // walk on ground
  if (!truck.driving && player.pos.y == ground) {
  	  int px =  (int) screenpos.x  % 3;
	  costume = jetmanwalk [px];
	  player.speed.x *= 0.6;
  }
  else costume = jetmanfly [RND(2)]; 

  // draw player 
  colorsprites(white);
  if (!truck.driving) screen.sprite(costume,player.pos,dir);

  // lasers
  if (fire == 1 && !truck.driving) laser(); 

  sprite_t *objects[] = { &fuelpod, &bomb, &bricks, &truck, &trailer, &alienbase, &basecannon, &basecannon2, &missile };
  for ( auto obj : objects) 
  	screen.sprite(obj->sprite,obj->pos - screenpos, obj->dir);
  
  // Aliens
  for (int n=0; n < NUMALIENS; n++) {
  	//colorsprites(alien[n].color);
	int dir = alien[n].speed.x < 0;
	costume = meteor[RND(2)];
	if (alien[n].explode > 0) costume = explode[RND(3)];
  	screen.sprite(costume,alien[n].pos - screenpos,dir);
  }
}

void read_buttons() {
Vec2 move = joystick;

 if (pressed(Button::DPAD_LEFT) || move.x < 0) {
	player.speed.x--;
	dir = LEFT; }
 if (pressed(Button::DPAD_RIGHT) || move.x > 0) {
	player.speed.x++;
	dir = RIGHT; }
 if (pressed(Button::DPAD_UP) || pressed(Button::A) || move.y < 0) 
	 player.speed.y = -1.5f;
 if (pressed(Button::B) || pressed(Button::Y)) fire = 1;
 else fire = 0;
}

void update(uint32_t time) {

  playerdied = 0;
  screenpos.x += player.speed.x / 5;
  player.pos.y += player.speed.y;
  player.speed *= 0.95f;
  player.speed.y += 0.5f; // gravity

  if (player.pos.y >  ground)  // slow down on ground
	  if (abs(player.speed.x) > 6)  player.speed.x *= 0.9f;

  read_buttons();
  if (missiletime > 0) missiletime -= 0.05f;

  // flying uses energy, recovers in truck
  if (player.pos.y < ground) flytime -= 0.2f;
  if (flytime < 0) player.speed.y = 8;

  if (player.pos.y < 50) player.pos.y = 50;
  if (player.pos.y > ground ) player.pos.y = ground; 
  
  // enter / exit truck
  if (collide(Vec2(truck.pos.x - screenpos.x,ground),player.pos)) {
	  truck.driving = 1;
	  player.pos.y = ground;
	  flytime = 170;
  }
  // jump to exit truck
  if (truck.driving) {
	  truck.pos.x = screenpos.x + 140;
	  truck.dir = dir;
  	  if (player.pos.y < ground - 10) 
	  	truck.driving = trailer.attached = 0;
  }
  //attach trailer just by driving truck on to it
  if (!trailer.attached && truck.driving && collide(player.pos, Vec2(trailer.pos.x - screenpos.x - 50,ground))) 
	  trailer.attached = 1;
  if (trailer.attached) {
	  	trailer.dir = dir;
  		if (dir == LEFT) trailer.pos.x = screenpos.x + 185;
  		if (dir == RIGHT) trailer.pos.x = screenpos.x + 95;
  }
  //aliens
  for (int n=0; n < NUMALIENS; n++) {
  	alien[n].pos += alien[n].speed;
	Point pos = alien[n].pos;
	if (pos.x > 1000 || pos.x < -1000) alien[n].speed.x *= -1;
	if (pos.y < 45 || pos.y > ground) alien[n].speed.y *= -1;
  	if (fire && hitlaser(pos)) { splat(n); }
	//immune from aliens when driving truck
	if (truck.driving) continue;
  	if (collide (pos, player.pos)) {
		splat(n); 
		//playerdied = 1;
		}
	if (alien[n].explode > 0) alien[n].explode--;
	if (alien[n].explode == 1) newalien(n);
  }
  if (playerdied) { 
	  delay = 100;
	  flytime = 0;
  }
  if (delay) delay--;
  if (delay == 1) flytime = 170;

   // missile - homes in on truck
   if (missiletime < 0) {
   	if (missile.pos.x > truck.pos.x) missile.speed.x = -2;
   	else missile.speed.x = 0;
   	if (missile.pos.x - truck.pos.x < 100) missile.speed.y += 0.2;
   	missile.pos += missile.speed;
   }

   // drop a bomb on missle to stop it
   if (!bomb.attached && collide(missile.pos + Vec2(30,0),bomb.pos)) missile.pos.x = 0;

   // pickup & drop objects
    sprite_t *objects[] = { &fuelpod, &bomb, &bricks };
    for ( auto obj : objects) {
    	if (obj->pos.y < ground + 10) obj->pos.y++; // gravity
    	if (player.pos.y != ground && !truck.driving) 
		if (collide(Vec2(obj->pos.x - screenpos.x ,obj->pos.y),player.pos)) 
			obj->attached = 1; 
    	if (obj->attached) obj->pos = player.pos + Vec2(screenpos.x,20);  
    	if (obj->attached && player.pos.y == ground) {
		obj->attached = 0;
		obj->pos.y -= 10;
	}
    	if (pressed(Button::B)) obj->attached = 0; 
	}
}
