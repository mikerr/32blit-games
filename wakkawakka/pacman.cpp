#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

Surface *level;

int dead,dotseaten,ghostkiller;

typedef struct spriteobj {
	Vec2 pos;
	Vec2 dir;
} spriteobj;

spriteobj player,fruit,ghosts[4];

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
Rect redghost = Rect(57,8,2,2);
Rect scaredghost = Rect(73,8,2,2);
Rect whiteghost = Rect(79,8,2,2);
Rect cherry = Rect(61,6,2,2);

// player and ghosts are 16x16 sprites
Vec2 spritecenter = Vec2(8,8); 

// Static wave config
static const uint8_t *wavSample;
static uint32_t wavSize = 0;
static uint16_t wavPos = 0;

// Called everytime audio buffer ends
void buffCallBack(AudioChannel &channel) {
  for (int x = 0; x < 64; x++) {
    channel.wave_buffer[x] = (wavPos < wavSize) ? (wavSample[wavPos]  - 0x7f) << 8 : 0;
    // As the engine is 22050Hz, timestretch to match 
    if (x % 2) wavPos++;
  }
  if (wavPos >= wavSize) { channel.off();wavPos = 0;}
}

void play_wav(const uint8_t *wav,uint32_t wav_len ) {
  wavSample = wav;
  wavSize = wav_len;
  channels[0].trigger_attack();
}

int collide (Vec2 target) { 
  Vec2 diff = player.pos - target; 
  int distance = diff.x * diff.x + diff.y * diff.y;
  return ( distance < 1.5 );
}

void reset() {
  player.pos = Vec2(6,11);
  player.dir = Vec2(0,1);
  for (auto &ghost:ghosts) {
  	ghost.pos = Vec2(8,8);
  	ghost.dir = Vec2(1 - rand() % 3,1 - rand() % 3);
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
  Vec2 next = pos + dir + Vec2(0.5f,0.5f);
  return (map[(int)next.y][(int)next.x] == WALL) ;
}

Vec2 randomdir() {
	Vec2 dir;
	int random = (rand() % 2) ? -1 : 1 ;
	// don't do diagonals
	if (rand() % 2) dir.x = random;
	else dir.y = random;

	return (dir);
}
void init() {
  set_screen_mode(ScreenMode::hires);

  xscale = screen.bounds.w / 18;
  yscale = screen.bounds.h / 21;

  channels[0].waveforms = Waveform::WAVE;
  channels[0].wave_buffer_callback = &buffCallBack;

  level = Surface::load(levelpic);
  screen.sprites = Surface::load(pacman);

  //make black transparent
  screen.sprites->palette[0] = Pen(0,0,0,0); 
  reset();
  fruit.pos = Vec2(10,15);
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
		 Vec2 pos = grid(Vec2(x,y));
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
	// pacman faces direction of movement
  	costume = pac;
  	if (player.dir.x == -1) costume.y += 2;
  	if (player.dir.y == -1) costume.y += 4;
  	if (player.dir.y ==  1) costume.y += 6;

	// chomp animation
  	if (frame++ % 9 == 0)  anim = !anim;
  	costume.x += anim * 2;
  } else {
	  //death animation
  	costume = deadpac;
  	costume.x += 22; 
  	costume.x -= 2 * (dead / 4);
  	// timer for death anim
  	if (dead > 0) dead--;
  }
  screen.sprite(costume,grid(player.pos) - spritecenter);

  //ghosts
  costume = redghost;
  if (ghostkiller) {
	  if (ghostkiller < 100) costume = whiteghost;
	  else costume = scaredghost;
	  ghostkiller--;
  }
  for (auto ghost:ghosts) {
  	screen.sprite(costume,grid(ghost.pos) );
	if (!ghostkiller) costume.y += 2; // change colour
  }

  if (fruit.pos != Vec2(0,0)) screen.sprite(cherry,grid(fruit.pos));
  if (collide(fruit.pos)) fruit.pos = Vec2(0,0);
  if (dotseaten == 178) newlevel();

  if (wavPos == 0 ) play_wav(siren,siren_length);

  std::string status = "SCORE: " + std::to_string(dotseaten);
  screen.text(status,minimal_font,Vec2(10,230));
  status = "TIME: " + std::to_string(frame /50);
  screen.pen = Pen(255,255,255);
  screen.text(status,minimal_font,Vec2(150,230));
}

void update(uint32_t time) {

 if (!dead) {
	Vec2 newdir;
	int x = round(player.pos.x);
	int y = round(player.pos.y);

 	if (pressed(Button::DPAD_LEFT)  || joystick.x < 0) {
		newdir = Vec2(-1,0);
		if (ongridline(player.pos.x) && !blockedmove(player.pos,newdir)) {
			player.dir = newdir;
		}
	}
 	if (pressed(Button::DPAD_RIGHT) || joystick.x > 0) {
		newdir = Vec2(1,0);
		if (ongridline(player.pos.x) && !blockedmove(player.pos,newdir)) {
			player.dir = newdir;
		}
	}
 	if (pressed(Button::DPAD_UP) || joystick.y < 0) {
		newdir = Vec2(0,-1);
		if (ongridline(player.pos.y) && !blockedmove(player.pos,newdir)) {
			player.dir = newdir;
		}
	}
 	if (pressed(Button::DPAD_DOWN) || joystick.y > 0) {
		newdir = Vec2(0,1);
		if (ongridline(player.pos.y) && !blockedmove(player.pos,newdir)) {
			player.dir = newdir;
		}
	}

	if (!blockedmove(player.pos,player.dir))
 	 	player.pos += player.dir / 15.0;

	//wrap around left/right
	if (x > 17) player.pos.x = 0;
	if (x < 0) player.pos.x = 17;

	if (map[y][x] == BISCUIT) {
		dotseaten++;
  		play_wav(wakkawakka,wakkawakka_length);
		map[y][x] = EMPTY;
	}
	if (map[y][x] == PILL) {
		ghostkiller = 400; //lasts for 8 seconds (400 frames)
		map[y][x] = EMPTY;
	}
 }
 	
 // ghosts
 for (auto &ghost:ghosts) {
  	//if (ghostkiller) ghostdir = -ghostdir;
	ghost.pos += ghost.dir / 30.0f;

	if (blockedmove(ghost.pos,ghost.dir))
		ghost.dir = randomdir();
	if (collide(ghost.pos)) {
		if (!ghostkiller) {
  			dead = 50; // dead for 1 second (50 frames)
  			play_wav(deathmusic,deathmusic_length);
			reset();
			break;
		}
	}
 }
}
