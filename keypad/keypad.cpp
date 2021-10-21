#include "keypad.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

//include "dictionary.h"

using namespace blit;

std::string passcode;
Vec2 center,joypos;

char numbers [] = "123456789 0 ";
std::string alpha [9] = { "   ","ABC", "DEF", "GHI", "JKL", "MNO", "PQRS", "TUV", "WXYZ" };

void draw_cross(Vec2 pos) {
    screen.pen = Pen(255,0,0);
    screen.line(pos + Vec2 (-5, -5), pos + Vec2 (5, 5) );
    screen.line(pos + Vec2 (5, -5), pos + Vec2 (-5, 5) );
}

void draw_keypad() {
    int cel = screen.bounds.w / 5;
    Vec2 pos = center - Vec2(cel,cel * 1.5f);

    for (int n=0; n<9;n++) {
    	screen.pen = Pen(255,255,255);
        screen.text(std::string(1,numbers[n]), minimal_font, pos);
    	screen.pen = Pen(128,128,128);
        screen.text(alpha[n], minimal_font, pos + Vec2(0,13));
        pos.x += cel;
	if ((n+1) % 3 == 0 ) { 
		pos.x = center.x - cel;
		pos.y += cel;
		} 
	}
    pos.x = center.x;
    screen.text("0", minimal_font, pos);
}

char keypad_press () {     
    int cel = screen.bounds.w / 5;
    int letter=10;

    int x = 3 + (joypos.x - center.x - cel * 1.5f) / cel;
    int y = 3 + (joypos.y - center.y - cel * 1.5f) / cel;

    if (x >= 0 && x < 3)  { letter = x +(3 * y); }
    return (letter);
}

void init() {
    set_screen_mode(ScreenMode::lores);
    passcode = "";
}

void render(uint32_t time) {

    screen.pen = Pen(0, 0, 0);
    screen.clear();

    draw_keypad();

    screen.text(passcode + "_", minimal_font, Vec2(0,0));

    draw_cross(joypos);

    screen.pen = Pen(255,0,0);
    int num = keypad_press();
    screen.text(std::string(1,numbers[num]), minimal_font, Vec2(0,screen.bounds.h - 10));
}

void update(uint32_t time) {
static int sub,lastnum;
static uint32_t lasttime;
    center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);

    joypos.x =  joystick.x * center.x ;
    joypos.y =  joystick.y * center.y ;

    if (pressed(Button::DPAD_LEFT))  { joypos.x = -0.5 * center.x; }
    if (pressed(Button::DPAD_RIGHT)) { joypos.x = 0.5 * center.x; }
    if (pressed(Button::DPAD_UP))    { joypos.y = -0.5 * center.y; }
    if (pressed(Button::DPAD_DOWN))  { joypos.y = 0.3 * center.y; }
    joypos = joypos + center;

    int num = keypad_press();
    if (buttons.released & Button::Y) 
	    if (passcode != "") passcode.pop_back();
    if (buttons.released & Button::A) passcode += " ";
    if (buttons.released & Button::X) passcode += numbers[num];
    if (buttons.released & Button::B) {
	    if (lastnum == num) {
	    	    if ( now() - lasttime  < 1000) 
		    {
			if (passcode != "") passcode.pop_back();
		    	sub++;
		    }
	    }
	    else sub = 0;
	    if (sub > alpha[num].size() - 1) sub = 0;
	    char letter = alpha[num][sub];
	    passcode += letter;
	    lastnum = num;
	    lasttime = now();
    }
}
