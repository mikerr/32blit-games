#include "32blit.hpp"
#include "assets.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;

int low_res,fire,button_up;
std::string status;
Vec2 center,move,joypos;

bool toggle = false;

SpriteSheet *backdrop;

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
  
  // For this example, clear the values
  if (wavPos >= wavSize) {
    channels[0].off();        // Stop playback of this channel.
    //Clear buffer
    wavSample = nullptr;
    wavSize = 0;
    wavPos = 0;
  }
}


void draw_cursor(Vec2 pos) {
    screen.pen = Pen(255,255,255);
    screen.line(pos + Vec2 (-5, -5), pos + Vec2 (5, 5) );
    screen.line(pos + Vec2 (5, -5), pos + Vec2 (-5, 5) );
}

void play_wav(char *message, const uint8_t *wav,uint32_t wav_len ) {

screen.pen = Pen(255,0,0);
screen.text(message, minimal_font, Vec2(0,0));

wavSample = wav;
wavSize = wav_len;
channels[0].trigger_attack();
}

void init() {
    set_screen_mode(ScreenMode::lores);
    low_res = fire = button_up = 0;

    channels[0].waveforms = Waveform::WAVE;
    channels[0].callback_waveBufferRefresh = &buffCallBack;  // Set callback address

    backdrop = SpriteSheet::load(map1);
    screen.sprites = SpriteSheet::load(quad);
}

void render(uint32_t time) {
Vec2 cursorpos;

static int x=0;
static int y=0;

    screen.pen = Pen(0, 0, 0);
    screen.clear();

    screen.stretch_blit(backdrop,Rect(x,y,56,56),Rect(0,0,screen.bounds.h,screen.bounds.h));

    if ( joypos.x < 5 ) x--;
    if ( joypos.x > screen.bounds.h - 5) x++;

    if ( joypos.y < 5 ) y--;
    if ( joypos.y > screen.bounds.h - 5) y++;

    if ( x < 0 ) x = 0;
    if ( y < 0 ) y = 0;
    if ( x > 200 ) x = 200;
    if ( y > 200 ) y = 200;

    screen.pen = Pen(255,255,255);

    if (!low_res) status = "Lo-res - down to toggle";
	    else status = "Hi-res - down to toggle";
    cursorpos = Vec2(0,screen.bounds.h - 10);
    screen.text(status, minimal_font, cursorpos);

    draw_cursor(joypos);

    //if (time % 2000 == 0) play_wav("Enemy unit approaching",enemyunit,enemyunit_length);

    if (button_up) {
		button_up = 0;

		if (toggle) play_wav("Yes sir!",yessir,yessir_length);
		else 	play_wav("Acknowledged",ack,ack_length);
		toggle = !toggle;
		}
}

void update(uint32_t time) {
	if (pressed(Button::B)) { 
		fire = 1; 
		}  else { 
		if (fire == 1) { // button released
			button_up = 1; 
			fire = 0;
			}
		}
	if (pressed(Button::DPAD_DOWN))  { 
		low_res = 1 - low_res;
		if (low_res) { 
			set_screen_mode(ScreenMode::hires); 
			} else { 
			set_screen_mode(ScreenMode::lores);
			}
                }

    center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);
    move = blit::joystick;

    joypos.x =  move.x * center.x ;
    joypos.y =  move.y * center.y ;
    joypos = joypos + center;
}
