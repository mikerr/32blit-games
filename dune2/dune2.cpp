#include "32blit.hpp"
#include "assets.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;
int x,y,fire,button_up;
std::string status;
Vec2 center,move,joypos;

bool toggle = false;

SpriteSheet *backdrop;

// Static wave config
static uint32_t wavSize = 0;
static uint16_t wavPos = 0;
static uint16_t wavSampleRate = 11025;
static const uint8_t *wavSample;

// Called everytimeaudio buffer ends
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

void flashborder(){
    int w = screen.bounds.w-1;
    int h = screen.bounds.h-1;

    screen.pen = Pen(25,25,25);
    screen.line(Vec2(0,0),Vec2(0,h));
    screen.line(Vec2(0,h),Vec2(w,h));
    screen.line(Vec2(w,h),Vec2(w,0));
    screen.line(Vec2(w,0),Vec2(0,0));
}

void init() {
    set_screen_mode(ScreenMode::hires);
    fire = button_up = 0;

    channels[0].waveforms = Waveform::WAVE;
    channels[0].callback_waveBufferRefresh = &buffCallBack;  

    backdrop = SpriteSheet::load(map1);
    screen.sprites = SpriteSheet::load(quad);
}

void render(uint32_t time) {
Vec2 cursorpos;

    screen.pen = Pen(0, 0, 0);
    screen.clear();

    screen.stretch_blit(backdrop,Rect(x+1,y,screen.bounds.w,screen.bounds.h),Rect(0,0,screen.bounds.w,screen.bounds.h));
    screen.pen = Pen(255,255,255);

    cursorpos = Vec2(0,screen.bounds.h - 10);
    screen.text(status, minimal_font, cursorpos);

    draw_cursor(joypos);

    //if (time % 2000 == 0) play_wav("Enemy unit approaching",enemyunit,enemyunit_length);

    if (time % 50 == 0) status = "";

    if (button_up) {
		button_up = 0;

		if (toggle) {
			status = "Yes, sir !";
			play_wav(yessir,yessir_length);
			}
		else {
			status = "Acknowledged !";
			play_wav(ack,ack_length);
			}
		toggle = !toggle;
		}
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
    if ( joypos.x > screen.bounds.h - 10 || pressed(Button::DPAD_RIGHT)) x += SCROLLSPEED;

    if ( joypos.y < 10 || pressed(Button::DPAD_UP)) y -= SCROLLSPEED;
    if ( joypos.y > screen.bounds.h - 10|| pressed(Button::DPAD_DOWN)) y += SCROLLSPEED;

    int width  = 568 - screen.bounds.w -2; 
    int height = 568 - screen.bounds.h; 

    if ( x < 0 || x > width)  flashborder();
    if ( y < 0 || y > height) flashborder();

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
