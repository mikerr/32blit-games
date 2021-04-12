#include "32blit.hpp"
#include "assets.hpp"

#include "audio/mp3-stream.hpp"

using namespace blit;

#define LEFT 0
#define RIGHT 1
#define RND(a) (rand() % a )

Surface *backdrop,*tileblocks,*characters,*keys;
MP3Stream stream;

int dir,grounded,jumping,o2;

Vec2 player;

Vec2 speed;
Vec2 playerstart = Vec2(40,150);

Vec2  monsterpos = Vec2(155,95);
int monsterdir = 1;
//costumes
Rect willywalk [] = { Rect(0,0,8,16), Rect(18,0,8,16), Rect(36,0,8,16), Rect(52,0,11,16) };
Rect monster = Rect(128,0,12,16);

// platform format: start.x start.y endpoint.x
Vec3 platforms[] = { Vec3(39,160,280), Vec3(70,145,190), Vec3(190,135,280), Vec3(260,120,280), Vec3(95,112,255), Vec3(39,110,70), Vec3(39,95,70), Vec3(39,80,280), Vec3(165,103,190) 
}; 
Vec3 conveyor = Vec3(95,112,255);

Vec2 gems[]= { Vec2(102,40),Vec2(157,50),Vec2(260,40),Vec2(270,90),Vec2(225,70)};
bool collectedgems[5];

Vec2 spikes[]= { Vec2(120,45),Vec2(160,45), Vec2(215,65), Vec2(255,65), Vec2(200,100), Vec2(130,130)};

// ZX spectrum colors
Pen black = Pen(0,0,0);
Pen blue = Pen(0,0,255);
Pen red = Pen(255,0,0);
Pen magenta = Pen(255,0,255);
Pen green = Pen(0,255,0);
Pen cyan = Pen(0,255,255);
Pen yellow = Pen(255,255,0);
Pen white = Pen(255,255,255);
Pen colors[] = { black, blue, red, magenta, green, cyan, yellow, white };

int collide (Point a, Point b) {
   if (abs(a.x - b.x) < 8  && abs(a.y - b.y) < 8 ) return 1;
   return 0;
}

void playerhitplatform (Vec3 platform, bool solid){
  if ((player.x > platform.x) && (player.x < platform.z)) {
    int ypos = platform.y - 16;

    // if hitting underside, bounce down
    if (solid) {
    	if ((speed.y < 0) && (abs(player.y - ypos)) < 5) { 
	jumping = 0;}
    }

    // if on top, land
    if ((speed.y > 0) && abs(ypos - player.y) < 2) { 
	player.y = ypos; 
	speed.y = 0;
        grounded = 1;
	jumping = 0;
    } 
  }
}

void colorsprites(Surface *sprites,Pen color) { sprites->palette[1] = color; } 

void init() {
  set_screen_mode(ScreenMode::hires);

  backdrop = Surface::load(level);
  tileblocks = Surface::load(tiles);
  characters = Surface::load(spritesheet);
  keys = Surface::load(pickups);
  screen.sprites = keys;

  File::add_buffer_file("music.mp3", music, music_length);
  stream.load("music.mp3", false);

  // Any channel can be used here, the others are free for other sounds.
  stream.play(0);

  player = playerstart;
}

void render(uint32_t time) {
static int lives = 3;
static int dance;
static float o2 = 200;
  Rect costume;

  screen.pen = black;
  screen.clear();

  // copy background
  Rect backdropsize = Rect (0,0,backdrop->bounds.w,backdrop->bounds.h);
  screen.blit(backdrop,backdropsize,Vec2(30,40));

  screen.pen = yellow;
  screen.line(Vec2(30,39),Vec2(285,39));

  // air supply bar
  screen.pen = white;
  screen.line(Vec2(60,180),Vec2(60+o2,180));
  o2 = o2 - 0.05;
  if (o2 < 0) {
	  player = playerstart;
	  lives--;
	  o2 = 200;
  }

  //for (Vec3 plat : platforms) 
  //	 screen.line(Vec2(plat.x,plat.y), Vec2(plat.z,plat.y));

  // Willy !

  int animframe = 3-(((int)player.x >> 1)&3);
  costume = willywalk [animframe]; 
  colorsprites(characters,white);
  if (dir == RIGHT) screen.blit(characters,costume,player);
  else screen.blit(characters,costume,player,SpriteTransform::HORIZONTAL);
 
  // Monster
  if ( dance % 2) monsterpos.x += monsterdir;
  if (monsterpos.x > 155) monsterdir = -1;
  if (monsterpos.x < 90) monsterdir = 1;
  costume = monster;
  costume.x +=  18 * ((dance / 6 ) % 3);
  if (monsterdir == RIGHT) screen.blit(characters,costume,monsterpos);
  else screen.blit(characters,costume,monsterpos,SpriteTransform::HORIZONTAL);

  if (collide(player,monsterpos) || collide(player,monsterpos + Vec2(0,16))) {
	  	player = playerstart;
	        lives--;
	        jumping = 0;
  }
  // Gems
  for (int i=0; i<5;i++) {
               Vec2 gempos = gems[i];
	       colorsprites(keys,colors[RND(6)]); // sparkle 
	       if (collide(player,gempos)) 
		       collectedgems[i] = true;
	       if (!collectedgems[i])
		       screen.sprite(Rect(0,0,1,1),gempos);
	}
  // Spikes
  for (auto spike : spikes) {
  	if (collide(player,spike)) {
	  	player = playerstart;
	        lives--;
	        jumping = 0;
	}
  }
  // Dancing willies !
  dance++;
  costume = willywalk [(dance / 30 )% 4]; 
  colorsprites(characters,blue);
  for (int i=0;i<lives;i++)
	  screen.blit(characters,costume,Vec2(35+ i*16,205));
}

void update(uint32_t time) {
static int jumpheight = 0;

Vec2 move = joystick;
 if (grounded == 1) speed.x = 0;
 if (pressed(Button::DPAD_LEFT) || move.x < 0) {
	speed.x = -1;
	dir = LEFT;
 }
 if (pressed(Button::DPAD_RIGHT) || move.x > 0) {
	speed.x = 1;
	dir = RIGHT;
 }

 if ((pressed(Button::DPAD_UP) || pressed(Button::A)) && grounded) {
	grounded = 0;
	jumping = 1;
	jumpheight = player.y - 20;
 }
 if (jumping && player.y < jumpheight) jumping = 0;
 if (grounded == 1) jumping = 0;
 if (jumping && !grounded) 
	speed.y = -1;
 else 
 	speed.y = 1; // gravity


 // stay on screen
 if (player.x > 265) player.x = 265;
 if (player.x < 40 ) player.x = 40;
 if (player.y > 145 ) player.y = 145;
 if (player.y < 0 ) player.y = 0;

 // fall onto platforms
 for (Vec3 plat : platforms) 
	playerhitplatform(plat,false);
 //special platforms
 
 Vec3 platform = conveyor;
 if ((player.x > platform.x) && (player.x < platform.z)) {
	 if (grounded && abs(platform.y - 16 - player.y) < 5) speed.x = -0.5;
 }
 // slow down player movement
 if (time % 3 > 0) player += speed;

 stream.update();
}
