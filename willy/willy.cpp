#include "32blit.hpp"
#include "assets.hpp"
#include "audio/mp3-stream.hpp"

using namespace blit;

#define RIGHT 1
#define LEFT -1
#define UP -1
#define DOWN 1

Surface *backdrop,*tileblocks,*pythonboot,*characters,*keys;
Surface *levels[2];
MP3Stream stream;

bool grounded,jumping; // grounded, jumping or falling
int framecount,lives = 3;

Vec2 player,speed,playerstart = Vec2(40,120);
Point monsterpos = Point(155,95);
//costumes
Rect willywalk [] = { Rect(0,0,8,16), Rect(18,0,8,16), Rect(36,0,8,16), Rect(52,0,11,16) };
Rect monsters[] = { Rect(128,0,12,16), Rect(0,16,12,16)};

// platform format: start.x start.y endpoint.x
Vec3 platforms[] = { Vec3(39,160,280), Vec3(70,145,190), Vec3(190,135,280), Vec3(260,120,280), Vec3(95,112,255), Vec3(39,110,70), Vec3(39,95,60), Vec3(39,80,280), Vec3(165,103,190) 
}; 
Vec3 conveyor = Vec3(95,112,255);
Vec3 collapsing [] = { Vec3(215,135,260), Vec3(140,80,175), Vec3(180,80,215)};
int collapsed[300];

Point spikes[]= { Point(120,40), Point(160,40), Point(215,65), Point(250,65), Point(200,100), Point(130,130)};
Point gempos[]= { Point(102,40), Point(157,50), Point(260,40), Point(270,90), Point(225,70)};
bool collectedgems[5];

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

void colorsprites(Surface *sprites,Pen color) { sprites->palette[1] = color; } 

int collide (Point a, Point b) { return (abs(a.x - b.x) < 8  && abs(a.y - b.y) < 8 ); } 

void playerhitplatform (Vec3 platform, bool solid){
  if ((player.x > platform.x) && (player.x < platform.z)) {
    int ypos = platform.y - 16;

    // if hitting underside, bounce down
    if (solid && (speed.y == UP) && abs(player.y - ypos) < 5) jumping = 0;

    // if on top, land
    if ((speed.y == DOWN) && abs(ypos - player.y) < 2) { 
	player.y = ypos; 
	speed.y = jumping = 0;
        grounded = 1;
    } 
  }
}

void player_die(){
 lives--;
 jumping = 0;
 player = playerstart;
}

void gameloop(){
static int score,level,dir,monsterdir = 1,timer;
static float o2 = 200;
Rect costume;

  framecount++;

  screen.pen = black;
  screen.clear();
  // copy background
  backdrop = levels[level];
  Rect backdropsize = Rect (0,0,backdrop->bounds.w,backdrop->bounds.h);
  screen.blit(backdrop,backdropsize,Point(30,40));

  screen.pen = yellow;
  screen.line(Point(30,39),Point(285,39));
  screen.text("00000" + std::to_string(score),minimal_font,Point(120,193));
  screen.text("00000" + std::to_string(score),minimal_font,Point(240,193));

  // air supply bar
  screen.pen = white;
  screen.line(Point(60,180),Point(60+o2,180));
  o2 = o2 - 0.05;
  if (o2 < 0 || !lives) {
	  o2 = 200;
	  score = 0;
	  player_die();
	  for (int i=0;i<300;i++) collapsed[i]=0;
	  for (auto& collected : collectedgems)
		  collected = false;
  }
  if (pressed(Button::MENU)) // debug
  	for (Vec3 plat : platforms) 
  	 	screen.line(Point(plat.x,plat.y), Point(plat.z,plat.y));
 
  // Monster
  if (framecount % 2) monsterpos.x += monsterdir;
  if (monsterpos.x > 155) monsterdir = LEFT;
  if (monsterpos.x < 90)  monsterdir = RIGHT;
  costume = monsters[level];
  costume.x += 18 * ((framecount / 6 ) % 3);
  bool flip = monsterdir < 0;
  screen.blit(characters,costume,monsterpos,flip);

  if (collide(player,monsterpos) || collide(player,monsterpos + Point(0,8))) 
	  player_die();
  // Gems
  int gemsleft=0;
  for (int i=0;i<5;i++) {
	       if (!collectedgems[i]) {
  	       	       colorsprites(keys,colors[rand() % 6]); // sparkle 
		       screen.blit(keys,Rect(0,0,8,8),gempos[i]);
	       		if (collide(player,gempos[i])) {
		       		collectedgems[i] = true;
		       		score += 100;
				}
			gemsleft++;
	       }
	}
  // Spikes
  for (auto spike : spikes)  
  	if (collide(player,spike)) 
	  	player_die();
  
  // Dancing willies !
  costume = willywalk [(framecount / 30 )% 4]; 
  for (int i=0; i<lives; i++){
  	  colorsprites(characters,colors[i+1]);
	  screen.blit(characters,costume,Point(35 + i*16,205));
  }
  // level complete
  if (!gemsleft && (framecount % 2)) {
	  screen.rectangle(Rect(262,143,16,16));
  	  if (collide(player,Point(262,143))) {
	  	for (int i=0;i<300;i++) collapsed[i]=0;
	  	level = !level;
	  	player = playerstart;
	  }
  }
  // Collapsing platforms
  for (Vec3 plat : collapsing)  {
      if ((player.x > plat.x) && (player.x < plat.z)) {
	 if (grounded && abs(plat.y - 16 - player.y) < 5) 
              if (framecount % 2)
		 if (collapsed[(int)player.x] < 8) 
			 for (int i=0; i<8; i++) 
				 collapsed[(int)player.x + i]++;
		 else player.y += 8;
      }
      screen.pen = black;
      for (int x=plat.x;x<plat.z;x++ ){
	      int ycollapse = collapsed[x];
	      if (ycollapse>0)
	         for (int y=plat.y; y < plat.y + ycollapse; y++ )
		     screen.pixel(Point(x,y));
      }
  }
  // Willy
  int animframe = 3-(((int)player.x >> 1)&3);
  costume = willywalk [animframe]; 
  colorsprites(characters,white);
  if (speed.x != 0) dir = speed.x < 0;
  screen.blit(characters,costume,player,dir);
}

void init() {
  set_screen_mode(ScreenMode::hires);

  levels[0] = Surface::load(level1);
  levels[1] = Surface::load(level2);
  tileblocks = Surface::load(tiles);
  pythonboot = Surface::load(bootpic);
  characters = Surface::load(spritesheet);
  keys = Surface::load(pickups);

  keys->palette[0] = Pen(0,0,0,0); // make black transparent
  characters->palette[0] = Pen(0,0,0,0);

  File::add_buffer_file("music.mp3", music, music_length);
  stream.load("music.mp3", false);
  stream.play(0);

  player = playerstart;
}

void bootscene(){
Rect boot = Rect(0,0,16,16);
Rect plinth = Rect(0,16,16,16);
	// monty python boot
	static int framecount,y;
	framecount++;
	screen.pen = colors[rand() % 6];
	screen.rectangle(Rect(0,0,320,175));
  	pythonboot->palette[0] = screen.pen;
	screen.blit(characters,willywalk[0],Point(153,145));
	for (int i=0;i<y;i+=3) screen.blit(pythonboot,boot,Point(150,i));
	screen.blit(pythonboot,plinth,Point(150,160));
        if (y < 145) y++;
	if (framecount > 160) {
		framecount = y = 0;
		lives = 3;
	}
}
void render(uint32_t time) {
	if (lives > 0) gameloop();
	else bootscene();
}

void update(uint32_t time) {
static int jumpheight = 0,collapse =0;

 if (grounded == 1) speed.x = 0;
 if (pressed(Button::DPAD_LEFT)  || joystick.x < 0) speed.x = LEFT;
 if (pressed(Button::DPAD_RIGHT) || joystick.x > 0) speed.x = RIGHT;
 if ((pressed(Button::DPAD_UP) || pressed(Button::A)) && grounded) {
	grounded = 0;
	jumping = 1;
	jumpheight = player.y - 20;
 }
 if (jumping && player.y < jumpheight) jumping = 0; // reached top of jump

 if (jumping && !grounded) speed.y = UP;
 else speed.y = DOWN; // gravity

 // keep on screen
 player.x = std::max((int)player.x,40);
 player.x = std::min((int)player.x,265);
 // fall onto platforms
 for (Vec3 plat : platforms) 
	playerhitplatform(plat,false);
 //special platforms
 Vec3 platform = conveyor;
 if ((player.x > platform.x) && (player.x < platform.z)) {
	 if (grounded && abs(platform.y - 16 - player.y) < 5) speed.x = LEFT;
 }
 // slow down player movement
 if (time % 3 > 0) player += speed;
 //play music
 stream.update();
}
