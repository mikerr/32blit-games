#include "32blit.hpp"
#include "assets.hpp"
#include "audio/mp3-stream.hpp"
#include <cstring>
using namespace blit;

#define RIGHT 1
#define LEFT -1
#define UP -1
#define DOWN 1

Surface *background,*sprites,*characters;
//MP3Stream stream;
int PICO,OFFSET;

bool grounded, jumping; // grounded, jumping or falling
int framecount, level, score, lives;
float o2;

Point player,speed;
//costumes
Rect gem, willywalk[] = { Rect(0,0,8,16), Rect(18,0,8,16), Rect(36,0,8,16), Rect(52,0,11,16) };

std::vector<Vec3> platforms,collapsing,monsters;
std::vector<Point> spikes,gems;
Vec3 conveyor;
int conveyordir;
Pen backcolor;
bool collectedgems[5];
int collapsed[320];

typedef struct sprite {
	Point pos;
	int dir;
	Rect costume;
} sprite;
sprite monster[5];

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

void free_surface (Surface *surface) {
	if (surface) {
		delete[] surface->data;
		delete[] surface->palette;
		delete surface;
	}
}
void colorsprites(Surface *sprites,Pen color) { sprites->palette[1] = color; } 

int collide (Point a, Point b) { return (abs(a.x - b.x) < 8  && abs(a.y - b.y) < 8 ); } 

bool playerhitplatform (Vec3 platform){ return (player.x > platform.x && player.x < platform.z && abs(platform.y - 16 - player.y) < 2) ; }

void setup_level(int level) {
 const uint8_t  *levels[] = { titlescreen,level1, level2, level3, level4, level5, level6, level7, level8, level9, level10, level11, level12, level13, level14, level15, level16, level17, level18, level19 };
 Rect monstertypes[] = { Rect(128,0,12,16), Rect(128,0,12,16), Rect(0,16,12,16), Rect(128,16,12,16), Rect(0,32,12,16), Rect(0,16,12,14), Rect(0,48,12,16), Rect(128,48,12,16), Rect(128,16,12,16), Rect(128,64,12,16), Rect(0,80,12,14), Rect(128,48,12,16), Rect(128,80,12,16), Rect(0,96,12,16), Rect(128,96,12,16), Rect(0,112,12,14), Rect(0,48,12,16), Rect(0,16,12,16), Rect(128,16,12,16), Rect(0,32,12,16), Rect(0,16,12,14) };

 o2 = 200;
 speed.x = 0;
 conveyordir = LEFT;
 player = Point(40 - OFFSET,140);
 for (auto& c : collapsed) c = 0;
 for (auto& c : collectedgems) c = false;

 gem = Rect(0,0,8,8); 
 backcolor = black;
 switch (level) {
	case 0:
 	platforms = {Vec3(262,130,265)};
 	player = Point(263 - OFFSET,110);
	break;

	case 1:
 	platforms = { Vec3(39,160,280), Vec3(70,145,190), Vec3(190,135,280), Vec3(260,120,280), Vec3(95,112,255), Vec3(39,110,70), Vec3(39,95,60), Vec3(39,80,280), Vec3(165,103,190) }; 
 	conveyor = platforms[4];
 	collapsing = { Vec3(215,135,260), Vec3(140,80,175), Vec3(180,80,215)};
 	spikes = { Point(120,40), Point(160,40), Point(215,65), Point(250,65), Point(200,100), Point(130,130)};
 	gems = { Point(102,40), Point(157,50), Point(260,40), Point(270,90), Point(225,70)};
	monsters = {Vec3(95,95,155)};
	break;

 	case 2: 
 	platforms = { Vec3(39,160,280), Vec3(55,128,85), Vec3(95,144,125), Vec3(142,135,173), Vec3(183,120,210), Vec3(103,113,157), Vec3(40,95,85), Vec3(40,80,188), Vec3(198,64,230), Vec3(198,89,255), Vec3(236,105,255), Vec3(236,112,255), Vec3(236,120,255), Vec3(236,128,255), Vec3(236,136,255) }; 
 	conveyor = platforms[1];
 	collapsing = { platforms[2], platforms[4],platforms[6],platforms[8],Vec3(236,88,255) };
 	spikes = { };
 	gems = { Point(88,50), Point(220,50), Point(55,110), Point(180,135), Point(237,95)};
	monsters = {Vec3(121,144,255), Vec3(40,65,180)};
	backcolor = Pen(0,0,200);
	break;

	case 3:
 	platforms = { Vec3(39,160,280), Vec3(66,135,115), Vec3(143,125,180), Vec3(194,145,320), Vec3(74,110,120), Vec3(227,118,320), Vec3(40,95,83), Vec3(39,80,320), Vec3(240,96,320)};
 	conveyor = platforms[4];
 	collapsing = {Vec3(60,80,320)};
 	spikes = { Point(40,120),Point(175,55),Point(117,40), Point(252,40)};
 	gems = { Point(195,90), Point(265,90), Point(75,40), Point(150,40), Point(212,40)};
	monsters = {Vec3(45,144,185), Vec3(40,65,170), Vec3(175,65,258)};
	break;
 
	case 4:
 	platforms = { Vec3(39,160,280), Vec3(72,135,96), Vec3(170,143,195), Vec3(250,134,320), Vec3(126,125,149), Vec3(206,125,229), Vec3(39,120,70), Vec3(165,112,199), Vec3(260,120,320), Vec3(83,104,97), Vec3(235,104,265), Vec3(39,97,65), Vec3(39,80,50), Vec3(80,80,100), Vec3(165,80,180), Vec3(125,87,140), Vec3(198,87,230), Vec3(180,65,230), Vec3(240,71,280)};
 	conveyor = platforms[6];
	conveyordir = RIGHT;
 	collapsing = {platforms[11]};
	monsters = {Vec3(45,145,155), Vec3(96,144,195)};
 	gems = { Point(42,40), Point(127,50), Point(270,90), Point(230,50)};
 	spikes = { Point(215,125)};
 	player = Point(280 - OFFSET,125);
	break;
  
	case 5:
 	platforms = { Vec3(39,160,280),Vec3(50,145,83), Vec3(206,145,224), Vec3(250,135,280), Vec3(82,120,236), Vec3(210,104,250), Vec3(255,90,280), Vec3(196,80,243)};
 	conveyor = platforms[1];
 	gems = { Point(52,110), Point(135,126), Point(150,90), Point(166,90), Point(270,95) };
 	spikes = { Point(53,135), Point(166,135), Point(198,60)};
	monsters = {Vec3(85,145,200), Vec3(220,144,265), Vec3(86,105,120), Vec3(140,105,200)};
	break;

	default:
 	platforms = { Vec3(39,160,280)};
 }

  int i=0;
  for (auto m: monsters) {
	  monster[i].pos = Point(m.z,m.y);
	  monster[i].dir = LEFT;
  	  monster[i].costume = monstertypes[level];
	  i++;
  }
  if (background) free_surface (background);
  background = Surface::load(levels[level]);

  // PICO / smaller screen support /
  for (Vec3 &plat : platforms) { plat.x -= OFFSET; plat.z -= OFFSET; }
  for (Vec3 &plat : collapsing) { plat.x -= OFFSET; plat.z -= OFFSET; }
  for (Point &p : gems) p.x = p.x - OFFSET;
  for (Point &p : spikes) p.x = p.x - OFFSET;
}

void player_die(){
 lives--;
 jumping = 0;
 setup_level(level);
}

void gameloop(){
static int playerdir;
int i,gemsleft;

  framecount++;
  screen.pen = black;
  screen.clear();

  // copy background
  screen.blit(background,Rect (0,0,256,192),Point(30-OFFSET,40));
  screen.rectangle(Rect(0,185,320,240));

  // score
  screen.pen = yellow;
  screen.line(Point(30-OFFSET,39),Point(285,39));
  screen.text("High Score 00000" + std::to_string(score),minimal_font,Point(40-OFFSET,193));
  screen.text("Score 00000" + std::to_string(score),minimal_font,Point(200-OFFSET,193));

  // air supply bar
  screen.pen = red;
  screen.rectangle(Rect(30-OFFSET,176,80,10));
  screen.pen = green;
  screen.rectangle(Rect(110-OFFSET,176,175-OFFSET,10));
  screen.pen = white;
  screen.text("AIR",minimal_font,Point(40-OFFSET,178));
  screen.rectangle(Rect(60-OFFSET,180,o2,2));

  // Monsters
  i=0;
  for (auto m : monsters) {
  	if (framecount % 2) monster[i].pos.x += monster[i].dir;
  	if (monster[i].pos.x > m.z - OFFSET) monster[i].dir = LEFT;
  	if (monster[i].pos.x < m.x - OFFSET) monster[i].dir = RIGHT;

  	// Monster walk
  	Rect costume = monster[i].costume;
	costume.x += 18 * ((framecount / 6 ) % 3);
  	bool flip = monster[i].dir < 0;
  	screen.blit(characters,costume,monster[i].pos,flip);
	i++;
	}
  // Gems
  gemsleft=0;
  i=0;
  for (auto gempos : gems) {
	       if (!collectedgems[i]) {
		       gemsleft++;
  	       	       colorsprites(sprites,colors[rand() % 6]); // sparkle 
		       screen.blit(sprites,gem,gempos);
	       	       if (collide(player,gempos)) {
		       		collectedgems[i] = true;
		       		score += 100;
				}
	       }
	       i++;
  }
  // level complete - flashing exit
  if (!gemsleft && (framecount % 2)) {
	  if (level !=0) screen.rectangle(Rect(262-OFFSET,143,16,16));
  	  if (collide(player,Point(262-OFFSET,143))) {
	  	level++;
		if (level == 19) level  = 0;
	  	setup_level(level);
	  }
  }
  // Draw Collapsed platforms
  screen.pen = backcolor;
  for (Vec3 plat : collapsing)  
      for (int x=plat.x;x<plat.z;x++ )
	         for (int y=0;y < collapsed[x];y++)
		     screen.pixel(Point(x,plat.y + y));
  // Willy walking
  int animframe = (player.x >> 2)&3;
  Rect costume = willywalk [animframe]; 
  if (speed.x != 0) playerdir = speed.x < 0;
  colorsprites(characters,white);
  screen.blit(characters,costume,player,playerdir);

  // Dancing willies !
  costume = willywalk [(framecount / 30 )% 4]; 
  for (int i=0; i<lives; i++){
  	  colorsprites(characters,colors[i+1]);
	  screen.blit(characters,costume,Point(40-OFFSET + i*16,205));
  }
 // DEBUG platform placement
 if (pressed(Button::Y)) {
         screen.pen = white;
	 for (auto p : platforms) screen.line(Point(p.x,p.y), Point(p.z,p.y));
	 screen.text(std::to_string(player.x + OFFSET) + "," + std::to_string(player.y),minimal_font,Point( 200,220));
 	}
}

void init() {
  set_screen_mode(ScreenMode::hires);
  if (screen.bounds.w <320) PICO = OFFSET = 38;

  sprites = Surface::load(manic_sprites);
  characters = Surface::load(character_sprites);

  lives = 3;
  level = 0;
  setup_level(level);
  if (!PICO) {
  	//File::add_buffer_file("music.mp3", music, music_length);
  	//stream.load("music.mp3", false);
  	//stream.play(0);
  }
}

void bootscene(){ // monty python boot - death scene
static int y;
Rect leg = Rect(8,0,16,3), boot = Rect(8,3,16,13), plinth = Rect(8,16,16,16);
        int xcenter = screen.bounds.w / 2;
	screen.pen = colors[rand() % 6];
	screen.rectangle(Rect(0,0,screen.bounds.w,175));
  	colorsprites(characters,white);
	screen.blit(characters,willywalk[0],Point(xcenter+5,145));
	for (int i=0;i<y;i+=3) screen.blit(sprites,leg,Point(xcenter,i));
  	colorsprites(sprites,white);
	screen.blit(sprites,boot,Point(xcenter,y++));
	screen.blit(sprites,plinth,Point(xcenter,160));
	if (y++ > 145) {
		y = 0;
		lives = 3;
	}
}
void render(uint32_t time) {
	if (lives > 0) gameloop();
	else bootscene();

	if (level == 0) { //piano
		static int key,keycolor;
		if (framecount % 20 == 0) {
			keycolor = 1+ rand() % 5;
			key = rand() % 100;
		}
		screen.pen = colors[keycolor];
		screen.rectangle(Rect(key+120,160,8,8));
	}
}

void update(uint32_t time) {
static int jumpheight = 0;

 o2 = o2 - 0.01f;
 if (o2 < 0 ) lives = 0;

 if (grounded) speed.x = 0;
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

 // CHEAT mode - finish room by collecting all gems
 if (pressed(Button::B)) for (auto& c : collectedgems) c = true;

 // keep on screen
 player.x = std::clamp((int)player.x,40-OFFSET,265-OFFSET);
 // fall onto platforms
 for (Vec3 plat : platforms)  {
	if (playerhitplatform(plat)) {
    		// if on top, land
    		if (speed.y == DOWN) { 
			player.y = plat.y - 16; 
			speed.y = jumping = 0;
        		grounded = 1;
		}
    	} 
 }

 //platforms
 if (playerhitplatform(conveyor) && grounded) speed.x = conveyordir;

 for (Vec3 plat : collapsing)  
      if (playerhitplatform(plat) && grounded)
	      if (framecount % 2) {
              	if (collapsed[player.x] < 8) 
			 for (int i=0; i<8; i++) collapsed[player.x + i]++;
	      	else player.y += 8;
	      }
  // hazards - spikes & monsters
 int i=0;
 for (auto m : monsters) 
	  if (collide(player,monster[i++].pos)) player_die();
 for (auto spike : spikes) 
	  if (collide(player,spike)) player_die();

 // slow down player movement
 if (time % 3 > 0) player += speed;

 //play music
 //if (!PICO) stream.update();
 
  // lost all lives
 if (!lives) {
	  score = 0;
  	  setup_level(level);
  }
}
