#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

int lowres,pic;
float angle,speed;
bool blur;
std::string status;

Surface *pictures[2];

void blit_rotate_sprite (Surface *sprite,Rect src, float angle, bool blur, Vec2 screenpos) {
//rotate a sprite to screen at any angle
Vec2 rot;
    int blocks = 1;
    if (blur) blocks = 2;
    int width = src.w;
    int height = src.h;
    for (int x=0;x<width;x++)
	    for (int y=0;y<height;y++) {
		    int x1 = x - (width / 2);
		    int y1 = y - (height / 2);
		    rot.x = x1 * sin(angle) + y1 * cos(angle);
		    rot.y = y1 * sin(angle) - x1 * cos(angle);
		    Vec2 pos = rot + screenpos;
        	    screen.stretch_blit(sprite,Rect(src.x + x, src.y + y,1,1),Rect(pos.x, pos.y,blocks,blocks));
	    }
}
void init() {
    set_screen_mode(ScreenMode::hires);
    pictures[0] = Surface::load(baboonimg);
    pictures[1] = Surface::load(lenaimg);

    blur = true;
    speed = 0.01;
}

void render(uint32_t time) {

    Vec2 center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);

    angle += speed;

    uint32_t ms_start = now();

    // Clear screen
    screen.pen = Pen (0,0,0);
    screen.clear();

    if (time % 1000 < 5) blur = !blur;
    if (time % 2000 < 5) pic = 1 - pic;


    // Draw bitmap
    blit_rotate_sprite(pictures[pic],Rect(0,0,256,256),angle,blur,center);
    
    uint32_t ms_end = now();

    // Draw score
    screen.pen = Pen(255,255,255);
    status = "Blur: " + std::to_string(blur);
    screen.text(status, minimal_font, Vec2(screen.bounds.w - 50, screen.bounds.h - 10));


    // draw FPS meter
    uint32_t ms = ms_end - ms_start;
    status = std::to_string(1000 / ms) + " fps    " + std::to_string(ms) + " ms";
    screen.text(status, minimal_font, Vec2(0, screen.bounds.h - 10));
}
	
void update(uint32_t time) {

    if (pressed(Button::DPAD_LEFT)  || joystick.x < -0.2) speed -= 0.005;
    if (pressed(Button::DPAD_RIGHT) || joystick.x > 0.2)  speed += 0.005;

    if (abs(speed) > 0.5) speed = speed / 2;
    if (pressed(Button::DPAD_UP)  || joystick.y < -0.2) ;
    if (pressed(Button::DPAD_DOWN) || joystick.y > 0.2) ;

    if (pressed(Button::A)) speed = 0;
    if (pressed(Button::B)) blur = !blur;

    if (pressed(Button::X)) {
	    lowres = !lowres;
	    if (lowres) set_screen_mode(ScreenMode::lores);
	    else set_screen_mode(ScreenMode::hires);
	    }
}
