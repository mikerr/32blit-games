#include "32blit.hpp"
#include "assets.hpp"
#include "audio/mp3-stream.hpp"

using namespace blit;

#define RIGHT 1
#define LEFT -1
#define UP -1
#define DOWN 1

Surface *backdrop,*sprites,*characters;
Surface *levels[2];
MP3Stream stream;

bool grounded,jumping; // grounded, jumping or falling
int framecount,level,lives = 3;
float o2;

Point player,speed,monsterpos,playerstart = Point(50,130);
//costumes
Rect willywalk[] = { Rect(0,0,8,16), Rect(18,0,8,16), Rect(36,0,8,16), Rect(52,0,11,16) };
Rect monsters[] = { Rect(128,0,12,16), Rect(0,16,12,16)};

std::vector<Vec3> platforms,collapsing;
std::vector<Point> spikes,gempos;
Vec3 conveyor;
Pen background;
bool collectedgems[5];
int collapsed[300];

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

bool playerhitplatform (Vec3 platform){ return (player.x > platform.x && player.x < platform.z && abs(platform.y - 16 - player.y) < 2) ; }

void player_die(){
 lives--;
 jumping = 0;
 player = playerstart;
}

void setup_level(int level) {
 o2 = 200;
 player = playerstart;
 for (auto& c : collapsed) c = 0;
 for (auto& c : collectedgems) c = false;

 if (level == 0) { // platform format: start.x start.y endpoint.x
 	platforms = { Vec3(39,160,280), Vec3(70,145,190), Vec3(190,135,280), Vec3(260,120,280), Vec3(95,112,255), Vec3(39,110,70), Vec3(39,95,60), Vec3(39,80,280), Vec3(165,103,190) }; 
 	conveyor = platforms[4];
 	collapsing = { Vec3(215,135,260), Vec3(140,80,175), Vec3(180,80,215)};
 	spikes = { Point(120,40), Point(160,40), Point(215,65), Point(250,65), Point(200,100), Point(130,130)};
 	gempos = { Point(102,40), Point(157,50), Point(260,40), Point(270,90), Point(225,70)};
	monsterpos = Point(155,95);
	background = black;
 }
 if (level == 1) {
 	platforms = { Vec3(39,160,280), Vec3(55,128,85), Vec3(95,144,125), Vec3(142,135,173), Vec3(183,120,210), Vec3(103,113,157), Vec3(40,95,85), Vec3(40,80,188), Vec3(198,64,230), Vec3(198,89,255), Vec3(236,105,255), Vec3(236,112,255), Vec3(236,120,255), Vec3(236,128,255), Vec3(236,136,255) }; 
 	conveyor = platforms[1];
 	collapsing = { platforms[2], platforms[4],platforms[6],platforms[8],Vec3(236,88,255) };
 	spikes = { };
 	gempos = { Point(88,50), Point(220,50), Point(55,110), Point(180,135), Point(237,95)};
	monsterpos = Point(155,65);
	background = Pen (0,0,200);
 }

}
void gameloop(){
static int score,dir,monsterdir = LEFT;

  framecount++;
  screen.pen = black;
  screen.clear();
  // copy background
  backdrop = levels[level];
  screen.blit(backdrop,Rect (0,0,256,192),Point(30,40));
  // score
  screen.pen = yellow;
  screen.line(Point(30,39),Point(285,39));
  screen.text("00000" + std::to_string(score),minimal_font,Point(120,193));
  screen.text("00000" + std::to_string(score),minimal_font,Point(240,193));
  // air supply bar
  o2 = o2 - 0.05f;
  if (o2 < 0 ) lives = 0;
  screen.pen = white;
  screen.line(Point(60,180),Point(60+o2,180));
  // debug - show platform bounds
  if (pressed(Button::MENU)) for (Vec3 plat : platforms) screen.line(Point(plat.x,plat.y), Point(plat.z,plat.y));
  // Monster walk
  if (framecount % 2) monsterpos.x += monsterdir;
  if (monsterpos.x > 155) monsterdir = LEFT;
  if (monsterpos.x < 90)  monsterdir = RIGHT;
  Rect costume = monsters[level];
  costume.x += 18 * ((framecount / 6 ) % 3);
  bool flip = monsterdir < 0;
  screen.blit(characters,costume,monsterpos,flip);

  if (collide(player,monsterpos) || collide(player,monsterpos + Point(0,8))) 
	  player_die();
  // Spikes
  for (auto spike : spikes) 
	  if (collide(player,spike)) player_die();
  // Gems
  int gemsleft=0;
  for (int i=0;i<5;i++) {
	       if (!collectedgems[i]) {
		       gemsleft++;
  	       	       colorsprites(sprites,colors[rand() % 6]); // sparkle 
		       screen.blit(sprites,Rect(0,level*8,8,8),gempos[i]);
	       	       if (collide(player,gempos[i])) {
		       		collectedgems[i] = true;
		       		score += 100;
				}
	       }
	}
  // Dancing willies !
  costume = willywalk [(framecount / 30 )% 4]; 
  for (int i=0; i<lives; i++){
  	  colorsprites(characters,colors[i+1]);
	  screen.blit(characters,costume,Point(35 + i*16,205));
  }
  // level complete - flashing exit
  if (!gemsleft && (framecount % 2)) {
	  screen.rectangle(Rect(262,143,16,16));
  	  if (collide(player,Point(262,143))) {
	  	level = !level;
	  	setup_level(level);
	  }
  }
  // Draw Collapsed platforms
  screen.pen = background;
  for (Vec3 plat : collapsing)  
      for (int x=plat.x;x<plat.z;x++ )
	         for (int y=0;y < collapsed[x];y++)
		     screen.pixel(Point(x,plat.y + y));
  // Willy walking
  int animframe = (player.x >> 2)&3;
  costume = willywalk [animframe]; 
  if (speed.x != 0) dir = speed.x < 0;
  colorsprites(characters,white);
  screen.blit(characters,costume,player,dir);

  // lost all lives
  if (!lives) {
	  score = 0;
  	  setup_level(level);
  }
}

void init() {
  set_screen_mode(ScreenMode::hires);
  levels[0] = Surface::load(level1);
  levels[1] = Surface::load(level2);
  sprites = Surface::load(manic_sprites);
  characters = Surface::load(character_sprites);

  File::add_buffer_file("music.mp3", music, music_length);
  stream.load("music.mp3", false);
  stream.play(0);

  setup_level(level);
}

void bootscene(){ // monty python boot - death scene
static int y;
Rect leg = Rect(8,0,16,3), boot = Rect(8,3,16,13), plinth = Rect(8,16,16,16);
	screen.pen = colors[rand() % 6];
	screen.rectangle(Rect(0,0,320,175));
  	colorsprites(sprites,white);
	screen.blit(characters,willywalk[0],Point(153,145));
	for (int i=0;i<y;i+=3) screen.blit(sprites,leg,Point(150,i));
	screen.blit(sprites,boot,Point(150,y));
	screen.blit(sprites,plinth,Point(150,160));
	y += 2;
	if (y > 145) {
		y = 0;
		lives = 3;
	}
}
void render(uint32_t time) {
	if (lives > 0) gameloop();
	else bootscene();
}

void update(uint32_t time) {
static int jumpheight = 0;

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

 // keep on screen
 player.x = std::clamp((int)player.x,40,265);
 // fall onto platforms
 for (Vec3 plat : platforms) 
	if (playerhitplatform(plat)) {
    		// if on top, land
    		if (speed.y == DOWN) { 
			player.y = plat.y - 16; 
			speed.y = jumping = 0;
        		grounded = 1;
		}
    	} 
 //special platforms
 if (playerhitplatform(conveyor) && grounded) speed.x = LEFT;

 for (Vec3 plat : collapsing)  
      if (playerhitplatform(plat) && grounded)
	      if (framecount % 2) {
              	if (collapsed[player.x] < 8) 
			 for (int i=0; i<8; i++) collapsed[player.x + i]++;
	      	else player.y += 8;
	      }
 // slow down player movement
 if (time % 3 > 0) player += speed;
 //play music
 stream.update();
}
