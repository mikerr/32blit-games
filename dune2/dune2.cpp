#include "32blit.hpp"
#include "assets.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;
int x,y,fire,button_up;
std::string status;
Vec2 center,move,joypos;

struct unit {
	Vec2 pos;
	Vec2 dest;
};

unit quads[10];
unit trikes[10];
unit soldiers[10];

SpriteSheet *backdrop,*quadsprite;

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

    // As the engine is 22050Hz, we can timestretch to match by incrementing our sample every other step (every even 'x')
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
    set_screen_mode(ScreenMode::hires);
    fire = button_up = 0;

    channels[0].waveforms = Waveform::WAVE;
    channels[0].callback_waveBufferRefresh = &buffCallBack;  

    backdrop = SpriteSheet::load(map1);
    quadsprite = SpriteSheet::load(quad);

    //spawn 10 quads at random locations
    for (int x=0;x<10;x++) {
	    quads[x].pos = Vec2(rand() % 300, rand() % 200);
	    quads[x].dest = quads[x].pos;
    }
}

void render(uint32_t time) {
static int commanding = 0;
int selected = 0;
int cursor = 0;

    // Draw map
    screen.stretch_blit(backdrop,Rect(x+1,y,screen.bounds.w,screen.bounds.h),Rect(0,0,screen.bounds.w,screen.bounds.h));

    // Draw quads
    for (int i=1;i<10;i++) {
	int size = 20;
	Vec2 unitpos = quads[i].pos;

	int distance = abs(joypos.x - unitpos.x - 10) + abs(joypos.y - unitpos.y - 10);
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

        if (i == selected) size = 30;
	screen.stretch_blit(quadsprite,Rect(0,0,56,52),Rect(unitpos.x,unitpos.y,size,size));
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
			quads[commanding].dest = joypos;
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

    int SCROLLSPEED = 3;
    if ( joypos.x < 10 || pressed(Button::DPAD_LEFT)) x -= SCROLLSPEED;
    if ( joypos.x > screen.bounds.w - 10 || pressed(Button::DPAD_RIGHT)) x += SCROLLSPEED;

    if ( joypos.y < 10 || pressed(Button::DPAD_UP)) y -= SCROLLSPEED;
    if ( joypos.y > screen.bounds.h - 10|| pressed(Button::DPAD_DOWN)) y += SCROLLSPEED;

    int width  = 568 - screen.bounds.w -2; 
    int height = 568 - screen.bounds.h; 

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
}
