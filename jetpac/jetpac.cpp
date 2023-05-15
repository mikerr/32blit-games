#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

#define LEFT 0
#define RIGHT 1
#define RND(a) (rand() % a )

Surface *backdrop;

int screenbottom,dir,fire;
int fuelled,fuelgrabbed,gem;
int lives,delay,rocketsmade;
int playerdied, takeoff = -10;

Point player,fuelpos,gempos;
Vec2 speed;

#define NUMALIENS 4
Vec2 alienpos[NUMALIENS], aliendir[NUMALIENS];
Pen aliencolor[NUMALIENS];

struct alien { Vec2 pos; Vec2 dir; Pen color; };
alien aliens[NUMALIENS];

//costumes
Rect explode[] = { Rect(8,0,4,3), Rect(8,3,4,2), Rect(8,5,4,2) };
Rect meteor [] = { Rect(0,6,2,2), Rect(2,6,2,2) };
Rect jetmanwalk [] = { Rect(0,0,2,3), Rect(2,0,2,3), Rect(4,0,2,3), Rect(6,0,2,3) };
Rect jetmanfly []  = { Rect(0,3,2,3), Rect(2,3,2,3) };
Rect gems [] = { Rect(30,0,2,2), Rect(30,2,2,2), Rect(30,7,2,2), Rect(30,4,2,2), Rect(30,10,2,2)};
Rect fuel = Rect(0,8,3,2);

Rect rocketparts[3] = { Rect(5,11,2,2), Rect(5,8,2,3), Rect(5,6,2,3) };
Point rocketpos[3] ;
int rocketgrabbed[3];

// platform format: x y length
Vec3 platforms[] = { Vec3 (30,93,100), Vec3 (135,122,190), Vec3 (221,64,295), Vec3 (0,233,320)}; 

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
  aliencolor[n] = colors[1 + RND(6)] ;
  alienpos[n] = Vec2(screen.bounds.w, RND(screen.bounds.h));
  aliendir[n] = Vec2(-1, 0.1f + (RND(5) / 5.0f ));

  if ( RND(2) ) { // opposite side of screen
  	alienpos[n].x = 0;
  	aliendir[n].x *= -1;
  	}
}

void colorsprites(Pen color) { screen.sprites->palette[1] = color; } 

bool indropzone ( int x ) { 
	int zone = screen.bounds.w * 0.62;
	return (x > zone && x < zone + 10); 
}

void splat ( int n ) {
  channels[2].trigger_attack();
  screen.sprite(explode[0],alienpos[n]);
  newalien(n);
}
void beep () { channels[6].trigger_attack();}

void reset_game() {

  lives = 3; 
  rocketsmade = 1;
  player = Point(screen.bounds.w / 2, screen.bounds.h - 50);
  fuelpos = Point(RND(screen.bounds.w),10);
  gempos = Point(RND(screen.bounds.w),10);
  gem = RND(5);

  for (int n=0;n<NUMALIENS;n++) { newalien(n); }

  rocketpos[2] = Point (platforms[0].x + 20 ,20);
  rocketpos[1] = Point (platforms[1].x + 20 ,20);
  for (int n=0;n<3;n++) { rocketgrabbed[n] = 0; }

}
void init() {
  set_screen_mode(ScreenMode::hires);
  screenbottom = screen.bounds.h - 35;

  backdrop = Surface::load(level);
  screen.sprites = Surface::load(jetpac);

  //make black transparent
  screen.sprites->palette[0] = Pen(0,0,0,0); 


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

  //scale to different screen sizes
  for (Vec3 &plat : platforms) {
	  plat.x = plat.x  / (320.0f / screen.bounds.w);
	  plat.y = plat.y  / (240.0f / screen.bounds.h);
	  plat.z = plat.z  / (320.0f / screen.bounds.w);
  	  }
  reset_game();
}


void gameloop() {
  Rect costume;
  playerdied = 0;
  player += speed;
  speed.x *= 0.3f;
  speed.y *= 0.8f;
  speed.y += 1.5f; // gravity

  // wraparound edge of screen
  if (player.x > screen.bounds.w) player.x = 0; 
  if (player.x < 0 ) player.x = screen.bounds.w; 

  // bounce down from top of screen
  if (player.y < 15) { 
	player.y = 18; 
	speed.y = 3; }

  for (Vec3 plat : platforms) 
	playerhitplatform(plat);

  if ( speed.y == 0 ) costume = jetmanwalk [(player.x * 4) % 3]; 
  else costume = jetmanfly [RND(2)]; 

  if (takeoff == -10) {
      colorsprites(white);
      if (dir == LEFT)  screen.sprite(costume,player);
      if (dir == RIGHT) screen.sprite(costume,player,SpriteTransform::HORIZONTAL);
      if (fire == 1) laser(); 
  }
 
  // Rocket
  Point pos = Point( screen.bounds.w * 0.65, screenbottom - takeoff); // - takeoff);
  // assembled rocketparts
  for (int i=0; i < rocketsmade ; i++) {
        colorsprites(white);
        if (fuelled > i) colorsprites(magenta);
        if ((fuelled >= 3) && RND(2)) colorsprites(white);
        screen.sprite(rocketparts[i],pos);
        rocketpos[i] = pos;
        pos.y -= 24;
        }
  // handle grabbing & dropping rocketparts
  for (int i=rocketsmade; i < 3 ; i++) {
        colorsprites(white);
        screen.sprite(rocketparts[i],rocketpos[i]);
        }

  // Gems
  colorsprites(colors[gem+3]);
  if (gem == 0) colorsprites(colors[RND(6)]); // sparkle diamond
  if (gem == 2) colorsprites(colors[RND(2)]); // flash isotope 
  screen.sprite(gems[gem],gempos);
  if (!hitplatform(gempos)) gempos.y++;
  if (collide(gempos,player)) { 
  	beep();
	gempos = Point(RND(screen.bounds.w),10); 
	gem = RND(5); 
	lives++;
	}
  // Fuel pod
  if (rocketsmade == 3) {
    colorsprites(magenta);
    screen.sprite(fuel,fuelpos);
  }
  // Aliens
  for (int n=0; n < NUMALIENS; n++) {
  	costume = meteor[RND(2)];
  	colorsprites(aliencolor[n]);
	Point pos = alienpos[n];
  	if (pos.x < 0 || pos.x > screen.bounds.w )  { newalien(n);}

  	if (hitplatform(pos)) { splat(n); }
  	if (fire && hitlaser(pos)) { splat(n); }
  	if (collide (pos, player)) {
		splat(n); 
		playerdied = 1;
		}
  	if (aliendir[n].x < 0) screen.sprite(costume,pos,SpriteTransform::HORIZONTAL); 
  	else screen.sprite(costume,pos); 
  }

  // player
  if (playerdied) {
                if (fuelgrabbed) { 
			fuelgrabbed = 0;
                	fuelpos.y -= 30;}
  		for (int i=rocketsmade; i < 3 ; i++) { 
			if (rocketgrabbed[i]) {
				rocketgrabbed[i] = 0;
				rocketpos[i].y -= 40;}
			}
  		screen.sprite(explode[0],player);
                player += Vec2( RND(10) - 10, -10);
		delay = 10;
		lives--;
		}
  screen.pen = white;
  screen.text(std::to_string(lives),minimal_font,Vec2(100,10));
  if (lives < 0) reset_game();
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
  	if (speed.y == 0) delay--;
	}
  //aliens
if (time % 4 <2) {
  for (int n=0; n < NUMALIENS; n++) {
  	alienpos[n] += aliendir[n];
  }
  //fuelpods
  if (rocketsmade == 3) {
    if (!hitplatform(fuelpos)) fuelpos.y++;
    if (indropzone(fuelpos.x)) {
	fuelgrabbed = 0; 
  	if (fuelpos.y > screenbottom) {
  		beep();
		fuelled++;
		fuelpos = Point(RND(screen.bounds.w),10);
		}
	}
    if (fuelgrabbed) fuelpos = player + Point(0,20);  
    else {
	    if (collide(fuelpos,player)) {
  		    beep();
		    fuelgrabbed = 1; 
    	    }
         }
    }
  //rocket
  for (int i=rocketsmade; i < 3 ; i++) {
        colorsprites(white);
        if ((i == rocketsmade) && collide(rocketpos[i],player) && !rocketgrabbed[i]) {
  		beep();
		rocketgrabbed[i] = 1;
	}
        if (rocketgrabbed[i]) {
                rocketpos[i] = Point(player.x,player.y + 10);
                if (indropzone(rocketpos[i].x)) {
                        rocketpos[i].y +=20;
                        rocketgrabbed[i] = 0;}
                }
        if (!hitplatform(rocketpos[i] + Point(0,10)) ) {
                rocketpos[i].y++;
                // stack rocketpart on top of last one
                if (indropzone(rocketpos[i].x) && (rocketpos[i].y > rocketpos[i-1].y - 16)) { 
  			beep();
			rocketsmade++;
			}
                }
  }
  if ((fuelled >= 3) && collide (rocketpos[0],player)) takeoff++;
  if (takeoff > -10) {
	  takeoff++;
  	  if (takeoff > screen.bounds.h) {
              fuelled = 0; 
	      takeoff = -10;
	      int rx = rocketparts[0].x; // rocketsprites at 5,7,9,11
	      if (rx == 11) rx = 5; else rx += 2;
	      rocketparts[0].x = rocketparts[1].x = rocketparts[2].x = rx;
  	      reset_game();
          }
  }
}
}
