#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

Surface *level;

int dead,dotseaten;
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

//costumes
Rect pac = Rect(57,0,2,2);
Rect deadpac = Rect(61,0,2,2);
Rect ghosty = Rect(57,8,2,2);
Rect fruit = Rect(61,6,2,2);

int collide (Point a) { 
	Point b = player;
	return ( abs(a.x - b.x) < 1  && abs(a.y - b.y) < 1 );
}

void reset() {
  player = Vec2(6,8);
  dir = Vec2(0,1);
  for (auto &ghost:ghosts) {
  	ghost = Vec2(10,10);
  }
}
// Static wave config
static uint32_t wavSize = 0;
static uint16_t wavPos = 0;
static uint16_t wavSampleRate = 11025;
static const uint8_t *wavSample;

// Called everytime audio buffer ends
void buffCallBack(AudioChannel &channel) {

  // Copy 64 bytes to the channel audio buffer
  for (int x = 0; x < 64; x++) {
    // Note: The sample used here has an offset, so we adjust by 0x7f. 
    channel.wave_buffer[x] = (wavPos < wavSize) ? (wavSample[wavPos]  - 0x7f) << 8 : 0;

    // As the engine is 22050Hz, we can timestretch to match 
    if (wavSampleRate == 11025) {
      if (x % 2) wavPos++;
    } else {
      wavPos++;
    }
  }
  
  if (wavPos >= wavSize) {
    channel.off();        // Stop playback of this channel.
     //Clear buffer
    wavSample = nullptr;
    wavSize = wavPos = 0;
  }
}

void play_wav(const uint8_t *wav,uint32_t wav_len ) {
wavSample = wav;
wavSize = wav_len;
channels[0].trigger_attack();
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
  play_wav(intromusic,intromusic_length);
}

void render(uint32_t time) {
static int anim,frame;
Rect costume;

  // copy background
  screen.pen = Pen(0,0,0);
  screen.clear();
  screen.stretch_blit(level,Rect(228,0,225,249),Rect(0,0,screen.bounds.w,screen.bounds.h-7));

  // map
  for (int x = 0; x < 19; x ++)
  	for (int y = 0; y < 22; y++) {
		 Vec2 pos = (Vec2(x*xscale,y*yscale));
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
  			if (frame % 5 > 3) 
				screen.circle(pos,4);
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
  	if (frame++ % 7 == 0)  anim = !anim;
  	costume.x += anim * 2;
  } else {
	  //death animation
  	costume = deadpac;
  	costume.x += 2 * ((28 - dead) / 2);
  }
  Vec2 playerpos;
  playerpos.x = player.x * xscale - 8;
  playerpos.y = player.y * yscale - 8;
  screen.sprite(costume,playerpos);

  //ghosts
  costume = ghosty;
  for (auto ghost:ghosts) {
  	Vec2 ghostpos;
  	ghostpos.x = ghost.x * xscale - 8;
  	ghostpos.y = ghost.y * yscale - 8;
  	screen.sprite(costume,ghostpos);
	costume.y += 2;
  }
  costume = fruit;
  //if (dotseaten % 100 == 0) fruitpos = Vec2(rand() % 300, rand() % 200);
  if (collide(fruitpos)) fruitpos = Vec2(0,0);
  if (fruitpos != Vec2(0,0)) screen.sprite(costume,fruitpos);

  // timer for death anim
  if (dead > 0) dead--;
  //if ((frame % 20) == 0) play_wav(wakkawakka,wakkawakka_length);
  std::string status = std::to_string(dotseaten);
  screen.pen = Pen(255,255,255);
  screen.text(status,minimal_font,Vec2(10,220));
}

int ongridline(float x) {
	int ix = x + 0.5;
	return (abs(ix - x) <0.2);
}
void update(uint32_t time) {
Vec2 move = joystick;

 if (!dead) {
 	if (pressed(Button::DPAD_LEFT)  || move.x < 0) 
		if (ongridline(player.x)) {
			dir = Vec2(-1,0);
		}
 	if (pressed(Button::DPAD_RIGHT) || move.x > 0) 
		if (ongridline(player.x)) {
			dir = Vec2(1,0);
		}
 	if (pressed(Button::DPAD_UP) || move.y < 0) 
		if (ongridline(player.y)) {
			dir = Vec2(0,-1);
		}
 	if (pressed(Button::DPAD_DOWN) || move.y > 0) 
		if (ongridline(player.y)) {
			dir = Vec2(0,1);
		}

 	player += dir / 15.0;

	int x = player.x + 0.5;
	int y = player.y + 0.5;
	if (map[y][x] == WALL) {
 		player -= dir / 15.0;
 	}

	//wrap around left/right
	if (x > 18) player.x = 0;
	if (x < 0) player.x = 18;

	if (map[y][x] == BISCUIT) {
		map[y][x] = EMPTY;
		dotseaten++;
	}
	if (map[y][x] == PILL) {
		map[y][x] = EMPTY;
	}
 }
 	
 // ghosts
 for (auto &ghost:ghosts) {
  	Vec2 ghostdir = Vec2(-1 + rand() % 3,-1 + rand() % 3);
  	ghost += ghostdir / 20.0;
	if (collide(ghost)) {
  		dead = 50;
  		play_wav(deathmusic,deathmusic_length);
		reset();
	}
 }
}
