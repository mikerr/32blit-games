#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

Surface *level;

#define RND(x) rand() % x

int dead;
Point player,ghostpos;
Vec2 dir;

Vec2 ghosts[4];
Vec2 dots[4];
Vec2 fruitpos;

//costumes
Rect pac = Rect(57,0,2,2);
Rect deadpac = Rect(61,0,2,2);
Rect ghosty = Rect(57,8,2,2);
Rect fruit = Rect(61,6,2,2);

int collide (Point a, Point b) { return ( abs(a.x - b.x) < 15  && abs(a.y - b.y) < 15 );}

void reset() {
  player = Point(150,151);
  dir = Vec2(1,0);
  int i=0;
  for (auto &ghost:ghosts) {
	i++;
  	ghost = Vec2(150+i,100);
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
    wavSize = 0;
    wavPos = 0;
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

  //play_wav(intromusic,intromusic_length);
}

void render(uint32_t time) {
static int anim,frame;
static int dotseaten;

  // copy background
  screen.stretch_blit(level,Rect(228,0,225,249),Rect(0,0,screen.bounds.w,screen.bounds.h));

  Rect costume = pac;
  if (dir.x == -1) costume.y += 2;
  if (dir.y == -1) costume.y += 4;
  if (dir.y ==  1) costume.y += 6;

  if (frame++ % 7 == 0)  anim = !anim;
  costume.x += anim * 2;

  if (dead) {
	  costume = deadpac;
  	  costume.x += 2 * ((28 - dead) / 2);
  }
  screen.sprite(costume,player);

  //ghosts
  costume = ghosty;
  for (auto ghost:ghosts) {
  	screen.sprite(costume,ghost);
	costume.y += 2;
  }
  costume = fruit;
  if (dotseaten++ % 1000 == 0) fruitpos = Vec2(rand() % 300, rand() % 200);
  if (collide(player,fruitpos)) fruitpos = Vec2(0,0);
  if (fruitpos != Vec2(0,0)) screen.sprite(costume,fruitpos);

  // timer for death anim
  if (dead > 0) dead--;
  if ((frame % 20) == 0) play_wav(wakkawakka,wakkawakka_length);
}


void update(uint32_t time) {
Vec2 move = joystick;

 if (!dead) {
 	if (pressed(Button::DPAD_LEFT)  || move.x < 0) dir = Vec2(-1,0);
 	if (pressed(Button::DPAD_RIGHT) || move.x > 0) dir = Vec2(1,0);
 	if (pressed(Button::DPAD_UP)    || move.y < 0) dir = Vec2(0,-1);
 	if (pressed(Button::DPAD_DOWN)  || move.y > 0) dir = Vec2(0,1);

 	player += dir;

 	if (player.x < 6) player.x = 6; 
 	if (player.x > 300) player.x = 300; 

 	if (player.y > 220) player.y = 220; 
 	if (player.y < 5 ) player.y = 5;
 }
 // ghosts
 for (auto &ghost:ghosts) {
  	ghost += Vec2(-1 + rand() % 3,-1 + rand() % 3);
	if (collide(player,ghost)) {
  		dead = 50;
		reset();
  		play_wav(deathmusic,deathmusic_length);
	}
 }
}
