#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

Surface *level;

int dead,dotseaten,ghostkiller;
Vec2 player,fruitpos;
Vec2 dir;

Vec2 ghosts[4];

#define WALL     0
#define BISCUIT  1
#define EMPTY    2
#define BLOCK    3
#define PILL     4

int map[22][19] = {
    	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 4, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 4, 0},
	{0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0},
	{0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0},
	{0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0},
	{0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0},
	{2, 2, 2, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 2, 2, 2},
	{0, 0, 0, 0, 1, 0, 1, 0, 0, 3, 0, 0, 1, 0, 1, 0, 0, 0, 0},
	{2, 2, 2, 2, 1, 1, 1, 0, 3, 3, 3, 0, 1, 1, 1, 2, 2, 2, 2},
	{0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0},
	{2, 2, 2, 0, 1, 0, 1, 1, 1, 2, 1, 1, 1, 0, 1, 0, 2, 2, 2},
	{0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0},
	{0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0},
	{0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0},
	{0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0},
	{0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0},
	{0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

int xscale = 18;
int yscale = 11;

std::string status;

//costumes
Rect pac = Rect(57,0,2,2);
Rect deadpac = Rect(61,0,2,2);
Rect redghost = Rect(57,8,2,2);
Rect scaredghost = Rect(73,8,2,2);
Rect whiteghost = Rect(79,8,2,2);
Rect fruit = Rect(61,6,2,2);

// Static wave config
static const uint8_t *wavSample;
static uint32_t wavSize = 0;
static uint16_t wavPos = 0;

// Called everytime audio buffer ends
void buffCallBack(AudioChannel &channel) {
  for (int x = 0; x < 64; x++) {
    // Note: The sample used here has an offset, so we adjust by 0x7f. 
    channel.wave_buffer[x] = (wavPos < wavSize) ? (wavSample[wavPos]  - 0x7f) << 8 : 0;

    // As the engine is 22050Hz, timestretch to match 
    if (x % 2) wavPos++;
  }
  
  if (wavPos >= wavSize) {
    channel.off();        
    wavPos = 0;
  }
}

void play_wav(const uint8_t *wav,uint32_t wav_len ) {
  wavSample = wav;
  wavSize = wav_len;
  channels[0].trigger_attack();
}

int collide (Vec2 target) { 
  Vec2 diff = player - target; 
  int distance = diff.x * diff.x + diff.y * diff.y;
  return ( distance < 1.5 );
}

void reset() {
  player = Vec2(6,11);
  dir = Vec2(0,1);
  for (auto &ghost:ghosts) {
  	ghost = Vec2(8 + rand() % 2,8);
  }
}

void newlevel () {
  dotseaten = 0;
}

Vec2 grid(Vec2 pos) {
  Vec2 screenpos;
  screenpos.x = pos.x * xscale;
  screenpos.y = pos.y * yscale;
  return(screenpos);
}

int ongridline(float x) {
  int ix = x ;
  return (abs(ix - x) <  0.01f);
}

int blockedmove(Vec2 pos,Vec2 dir){
  Vec2 next = pos + dir  + Vec2(0.5f,0.5f);
  return (map[(int)next.y][(int)next.x] == WALL) ;
}

void init() {
  set_screen_mode(ScreenMode::hires);

  channels[0].waveforms = Waveform::WAVE;
  channels[0].wave_buffer_callback = &buffCallBack;

  level = SpriteSheet::load(pacman);
  screen.sprites = SpriteSheet::load(pacman);

  //make black transparent
  screen.sprites->palette[0] = Pen(0,0,0,0); 
  reset();
  fruitpos = Vec2(10,15);
  play_wav(intromusic,intromusic_length);
}

void render(uint32_t time) {
static int anim,frame;
Rect costume;

  // player and ghosts are 16x16 sprites
  Vec2 spritecenter = Vec2(8,8); 
  // copy background
  screen.pen = Pen(0,0,0);
  screen.clear();
  screen.stretch_blit(level,Rect(228,0,225,249),Rect(0,0,screen.bounds.w,screen.bounds.h-7));

  // map
  for (int x = 0; x < 19; x ++)
  	for (int y = 0; y < 22; y++) {
		 Vec2 pos = grid(Vec2(x,y));
		 if (map[y][x] == WALL) { 
  			screen.pen=Pen(0,0,255);
			//screen.circle(pos,4);
		 }
		 if (map[y][x] == BISCUIT) { 
  			screen.pen=Pen(255,183,174);
			screen.circle(pos,2);
		 }
		 if (map[y][x] == PILL) { 
			screen.pen=Pen(255,255,255);
  			if (frame % 5 > 3) screen.circle(pos,4);
		 	}
		 }

  //player
  if (!dead) {
	// face right direction
  	costume = pac;
  	if (dir.x == -1) costume.y += 2;
  	if (dir.y == -1) costume.y += 4;
  	if (dir.y ==  1) costume.y += 6;

	// chomp animation
  	if (frame++ % 9 == 0)  anim = !anim;
  	costume.x += anim * 2;
  } else {
	  //death animation
  	costume = deadpac;
  	costume.x += 2 * ((28 - dead) / 2);
  	// timer for death anim
  	if (dead > 0) dead--;
  }
  screen.sprite(costume,grid(player) - spritecenter);

  //ghosts
  costume = redghost;
  if (ghostkiller) {
	  if (ghostkiller < 100) costume = whiteghost;
	  else costume = scaredghost;
	  ghostkiller--;
  }
  for (auto ghost:ghosts) {
  	screen.sprite(costume,grid(ghost) + spritecenter);
	if (!ghostkiller) costume.y += 2; // change colour
  }

  costume = fruit;
  //if (dotseaten % 100 == 0) fruitpos = Vec2(rand() % 300, rand() % 200);
  if (fruitpos != Vec2(0,0)) screen.sprite(costume,grid(fruitpos));
  if (collide(fruitpos)) fruitpos = Vec2(0,0);

  if (dotseaten == 178) newlevel();
  //if ((frame % 20) == 0) play_wav(wakkawakka,wakkawakka_length);
  screen.pen = Pen(255,255,255);
  status = "SCORE: " + std::to_string(dotseaten);
  screen.text(status,minimal_font,Vec2(10,230));
  status = "TIME: " + std::to_string(frame /50);
  screen.text(status,minimal_font,Vec2(150,230));
}

void update(uint32_t time) {

 Vec2 move = joystick;
 if (!dead) {
	Vec2 newdir;
	int x = round(player.x);
	int y = round(player.y);

 	if (pressed(Button::DPAD_LEFT)  || move.x < 0) {
		newdir = Vec2(-1,0);
		if (ongridline(player.x) && !blockedmove(player,newdir)) {
			player.y = round(player.y);
			dir = newdir;
		}
	}
 	if (pressed(Button::DPAD_RIGHT) || move.x > 0) {
		newdir = Vec2(1,0);
		if (ongridline(player.x) && !blockedmove(player,newdir)) {
			player.y = round(player.y);
			dir = newdir;
		}
	}
 	if (pressed(Button::DPAD_UP) || move.y < 0) {
		newdir = Vec2(0,-1);
		if (ongridline(player.y) && !blockedmove(player,newdir)) {
			player.x = round(player.x);
			dir = newdir;
		}
	}
 	if (pressed(Button::DPAD_DOWN) || move.y > 0) {
		newdir = Vec2(0,1);
		if (ongridline(player.y) && !blockedmove(player,newdir)) {
			player.x = round(player.x);
			dir = newdir;
		}
	}

	if (!blockedmove(player,dir))
 	player += dir / 15.0;

	//wrap around left/right
	if (x > 18) player.x = 0;
	if (x < 0) player.x = 18;
	if (y > 21) player.y = 0;
	if (y < 0) player.y = 21;

	if (map[y][x] == BISCUIT) {
		dotseaten++;
		map[y][x] = EMPTY;
	}
	if (map[y][x] == PILL) {
		ghostkiller = 400; //lasts for 8 seconds (400 frames)
		map[y][x] = EMPTY;
	}
 }
 	
 // ghosts
 for (auto &ghost:ghosts) {
  	Vec2 ghostdir;
	ghostdir = (player - ghost) / 50.0f;
       	ghostdir = Vec2(-2 + rand() % 5,-2 + rand() % 5);
  	if (ghostkiller) ghostdir = -ghostdir;
	if (!blockedmove(ghost,ghostdir))
		ghost += ghostdir / 20.0;
 }
 for (auto &ghost:ghosts) {
	if (collide(ghost)) {
		if (!ghostkiller) {
  			dead = 50; // dead for 1 second (50 frames)
  			play_wav(deathmusic,deathmusic_length);
			reset();
		} else {
			ghost = Vec2(8,8);
			}
		}
 }
}
