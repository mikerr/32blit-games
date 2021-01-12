#include "32blit.hpp"
#include "assets.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;
int x,y,fire,lowres,button_up,maps;
std::string status;
Vec2 center,move,joypos;

struct unit {
	Vec2 pos;
	Vec2 dest;
};

unit quads[10];

Surface *backdrop,*quadsprite;

#define MAPSIZE 568

// Static wave config
static uint32_t wavSize = 0;
static uint16_t wavPos = 0;
static uint16_t wavSampleRate = 11025;
static const uint8_t *wavSample;

// Called everytime audio buffer ends
void buffCallBack(void *) {

  // Copy 64 bytes to the channel audio buffer
  for (int x = 0; x < 64; x++) {
    // Note: The sample used here has an offset, so we adjust by 0x7f. 
    channels[0].wave_buffer[x] = (wavPos < wavSize) ? wavSample[wavPos] - 0x7f : 0;

    // As the engine is 22050Hz, we can timestretch to match 
    if (wavSampleRate == 11025) {
      if (x % 2) wavPos++;
    } else {
      wavPos++;
    }
  }
  
  if (wavPos >= wavSize) {
    channels[0].off();        // Stop playback of this channel.
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

void draw_cursor(Vec2 pos) {
    screen.pen = Pen(255,255,255);
    screen.line(pos + Vec2 (-5, -5), pos + Vec2 (5, 5) );
    screen.line(pos + Vec2 (5, -5), pos + Vec2 (-5, 5) );
}

void draw_box(int x, int y, int w, int h){
    screen.pen = Pen(25,25,25);
    screen.line(Vec2(x,y),Vec2(x,h));
    screen.line(Vec2(x,h),Vec2(w,h));
    screen.line(Vec2(w,h),Vec2(w,y));
    screen.line(Vec2(w,y),Vec2(x,y));
}

void init() {
    set_screen_mode(ScreenMode::lores);
    fire = button_up = 0;

    channels[0].waveforms = Waveform::WAVE;
    channels[0].callback_waveBufferRefresh = &buffCallBack;  

    maps = 1;
    backdrop = Surface::load(map1);
    quadsprite = Surface::load(quad);

    //spawn 10 quads at random locations
    for (int i=0;i<10;i++) {
	    quads[i].pos = Vec2(rand() % MAPSIZE, rand() % MAPSIZE);
	    quads[i].dest = Vec2(rand() % MAPSIZE, rand() % MAPSIZE);
    }
}

void render(uint32_t time) {
static int commanding = 0;
int selected = 0;
int cursor = 0;

    if (pressed(Button::Y)) {
	    maps++;
	    if (maps > 3) maps = 1;
	    if (maps == 1) backdrop = Surface::load(map1);
	    if (maps == 2) backdrop = Surface::load(map2);
	    if (maps == 3) backdrop = Surface::load(map3);
    }
    // Draw map
    screen.stretch_blit(backdrop,Rect(x+1,y,screen.bounds.w,screen.bounds.h),Rect(0,0,screen.bounds.w,screen.bounds.h));

    // Draw quads
    for (int i=1;i<10;i++) {
	int size = 15;
	Vec2 unitpos = quads[i].pos;

	int distance = abs(x + joypos.x - unitpos.x - 10) + abs(y + joypos.y - unitpos.y - 10);
	if (distance < 20) {
		if (status == "") status = "Harkonnen quad unit";
		selected = i;
	}

	// move units to their destination, one step at a time
	Vec2 dir = quads[i].dest - unitpos;
	//normalize
	dir.x = dir.x / std::max(1,abs(dir.x));
	dir.y = dir.y / std::max(1,abs(dir.y));

	quads[i].pos += dir;

        if (i == selected) size = 20;

	screen.stretch_blit(quadsprite,Rect(0,0,56,52),Rect(unitpos.x - x, unitpos.y - y, size, size));
	}

    Vec2 yard = Vec2(200,166);
    int distance = abs(x + joypos.x - yard.x - 10) + abs(y + joypos.y - yard.y - 10);
    if (distance < 20) {
	    status = "Construction yard";
    }

    draw_cursor(joypos);

    if (time % 50 == 0) status = "";

    if (time % 2000 == 0) { 
	    	    status = "Warning, Enemy unit approaching";
		    //play_wav(enemyunit,enemyunit_length);
		    }

    if (button_up) {
		button_up = 0;

		if (commanding) {
			status = "Acknowledged !";
			play_wav(ack,ack_length);
			quads[commanding].dest = Vec2(x,y) + joypos;
			commanding = 0;
		}
		if (selected) {
			status = "Yes, sir !";
			play_wav(yessir,yessir_length);
			commanding = selected;
			}
    }
    screen.pen = Pen(255,255,255);
    Vec2 screenbottom = Vec2(0,screen.bounds.h - 10);
    screen.text(status, minimal_font, screenbottom);
}

void update(uint32_t time) {

    center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);

    // stretch joystick diagonals to reach corners
    float diag = 1;
    if ( (fabs((float)joystick.x) > (double)0.5f) && (fabs((float)joystick.y) > (double)0.5f) ) diag = 1.5;

    joypos.x =  joystick.x * center.x * diag;
    joypos.y =  joystick.y * center.y * diag;

    joypos = joypos + center;

    int SCROLLSPEED = 1;
    if ( joypos.x < 10 || pressed(Button::DPAD_LEFT)) x -= SCROLLSPEED;
    if ( joypos.x > screen.bounds.w - 10 || pressed(Button::DPAD_RIGHT)) x += SCROLLSPEED;

    if ( joypos.y < 10 || pressed(Button::DPAD_UP)) y -= SCROLLSPEED;
    if ( joypos.y > screen.bounds.h - 10|| pressed(Button::DPAD_DOWN)) y += SCROLLSPEED;

    int width  = MAPSIZE - screen.bounds.w; 
    int height = MAPSIZE - screen.bounds.h; 

    if ( x < 0 || x > width)  draw_box(0,0,screen.bounds.w-1,screen.bounds.h-1);
    if ( y < 0 || y > height) draw_box(0,0,screen.bounds.w-1,screen.bounds.h-1);

    x = std::min(x,width);
    y = std::min(y,height);
    x = std::max(0,x);
    y = std::max(0,y);

    if (pressed(Button::A)) fire = 1; 
    if (!pressed(Button::A) && fire == 1 ) {
		 // button released
		button_up = 1; 
		fire = 0;
		}
    if (pressed(Button::X)) {
	    if (lowres) set_screen_mode(ScreenMode::lores);
	    else set_screen_mode(ScreenMode::hires);
	    lowres = !lowres;
	    }
}
