#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

int lowres,pic,dir;
float angle,speed,zoom;
bool blur,demo,pan;

Surface *pictures[2];

void blit_rotate_sprite (Surface *sprite,Rect src, float angle, bool blur, Vec2 screenpos) {
//rotate a sprite to screen at any angle
Vec2 rot;
    int blocks = 1;
    if (blur) blocks = 2;
    int width = src.w;
    int height = src.h;

    float sinangle = sin(angle);
    float cosangle = cos(angle);
    for (int x=0;x<width;x++)
	    for (int y=0;y<height;y++) {
		    int x1 = x - (width / 2);
		    int y1 = y - (height / 2);
		    rot.x = x1 * sinangle + y1 * cosangle;
		    rot.y = y1 * sinangle - x1 * cosangle;
		
		    Vec2 pos = (rot * zoom ) + screenpos;
        	    screen.stretch_blit(sprite,Rect(src.x + x, src.y + y,1,1),Rect(pos.x, pos.y,blocks,blocks));
	    }
}
void init() {
    set_screen_mode(ScreenMode::hires);
    pictures[0] = Surface::load(baboonimg);
    pictures[1] = Surface::load(lenaimg);

    blur = true;
    speed = 0.01;
    zoom = 1;
    dir = 1;
    demo = true;
}

void render(uint32_t time) {
std::string status;

    Vec2 center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);

    if (pan) center.x += speed * 100;
    else angle += speed;

    uint32_t ms_start = now();

    // Clear screen
    screen.pen = Pen (0,0,0);
    screen.clear();

    if (demo) {
	    zoom += 0.02f * dir;
	    if (zoom > 3.0f || zoom < 0.1f) dir *= -1 ;
	    if (zoom < 0.1f) pic = 1 - pic;
            if (time % 3000 < 5) blur = !blur;
    }

    // Draw bitmap
    blit_rotate_sprite(pictures[pic],Rect(0,0,256,256),angle,blur,center);
    
    uint32_t ms_end = now();

    screen.pen = Pen(255,255,255);
    status = "Zoom: " + std::to_string(zoom);
    screen.text(status, minimal_font, Vec2(screen.bounds.w - 50, screen.bounds.h - 20));
    status = "Blur: " + std::to_string(blur);
    screen.text(status, minimal_font, Vec2(screen.bounds.w - 50, screen.bounds.h - 10));

    // draw FPS meter
    uint32_t ms = ms_end - ms_start;
    status = std::to_string(1000 / ms) + " fps    " + std::to_string(ms) + " ms";
    screen.text(status, minimal_font, Vec2(0, screen.bounds.h - 10));
}
	
void update(uint32_t time) {

    if (pressed(Button::DPAD_LEFT)  || joystick.x < -0.2f) speed -= 0.005f;
    if (pressed(Button::DPAD_RIGHT) || joystick.x > 0.2f)  speed += 0.005f;

    if (abs(speed) > 0.1) speed = speed / 2;
    if (pressed(Button::DPAD_UP)  || joystick.y < -0.2f) zoom += 0.01f;
    if (pressed(Button::DPAD_DOWN) || joystick.y > 0.2f) zoom -= 0.01f;

    if (pressed(Button::A)) speed = 0;
    if (buttons.released) demo = false;
    if (buttons.released & Button::B) {
	    blur = !blur;
    }
    if (buttons.released & Button::Y) {
		    pan = !pan;
		    speed = 0;
    }

    if (pressed(Button::MENU)) {
    	screen.text("H E L P   M E N U", minimal_font, Vec2(50, 0));
    	screen.text("Up/down    - Zoom in/out", minimal_font, Vec2(0 , 30));
    	screen.text("Left/Right - Rotate", minimal_font, Vec2(0 , 40));
    	screen.text("X: Toggle resolution", minimal_font, Vec2(50 , 60));
    	screen.text("Y: Pan mode", minimal_font, Vec2(10 , 70));
    	screen.text("A: Stop", minimal_font, Vec2(90 , 70));
    	screen.text("B: Blur on/off", minimal_font, Vec2(50 , 80));
    }
    if (buttons.released & Button::X) {
	    	lowres = !lowres;
	    	if (lowres) set_screen_mode(ScreenMode::lores);
	    	else set_screen_mode(ScreenMode::hires);
	    }
}
