#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

typedef struct shape {
    std::vector<Vec3> points ;
} shape;

shape snowflakes;

SpriteSheet *snow;

int snowsize = 500;
float speed = 20;

void init() {
    set_screen_mode(ScreenMode::hires);

    for (int i=0; i < 100; i++) 
    	snowflakes.points.push_back(Vec3(rand() % screen.bounds.w, rand() % screen.bounds.h, rand() % 100));

    snow = SpriteSheet::load(snowflake);
}

void render(uint32_t time) {

    screen.pen = Pen(0,0,0);
    screen.clear();

    for (auto &p: snowflakes.points) {
	screen.stretch_blit(snow,Rect(0,0,256,256),Rect(p.x, p.y, p.z * screen.bounds.w / snowsize, p.z * screen.bounds.h / snowsize));

	p.y +=  p.z /  speed ;
	if (p.y > screen.bounds.h) { p.x = rand() % screen.bounds.w ; p.y = -50; } 

	p.x += tilt.x * p.z ;
	if (p.x > screen.bounds.w) p.x = -50;   
	if (p.x < -50 ) p.x = screen.bounds.w;   
	}
}

void update(uint32_t time) {

    //pos += joystick;

    if (pressed(Button::DPAD_UP)   && snowsize > 200)  snowsize--;
    if (pressed(Button::DPAD_DOWN) && snowsize < 1000) snowsize++;

    if (pressed(Button::DPAD_RIGHT) && speed > 4)   speed -= 0.1;
    if (pressed(Button::DPAD_LEFT)  && speed < 200) speed += 0.1;

}

