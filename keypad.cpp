#include "keyboard.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;


int low_res,fire,button_up;
std::string passcode;
Vec2 center,move,joypos;

void draw_cursor(Vec2 pos) {
    screen.pen = Pen(255,255,255);
    screen.line(pos + Vec2 (-5, -5), pos + Vec2 (5, 5) );
    screen.line(pos + Vec2 (5, -5), pos + Vec2 (-5, 5) );
}

void draw_keypad() {
    int cel = screen.bounds.w / 5;
    Vec2 pos = center - Vec2(cel,cel * 1.5f);

    for (int n=1; n<10;n++) {
        screen.text(std::to_string(n), minimal_font, pos);
        pos.x += cel;
	if (n % 3 == 0 ) { 
		pos.x = center.x - cel;
		pos.y += cel;
		} 
	}
    pos.x = center.x;
    screen.text("0", minimal_font, pos);
}

char keypad_press () {     
    char letters[] = "123456789 0 ";
    int cel = screen.bounds.w / 5;
    int letter=10;

    int x = 3 + (joypos.x - center.x - cel * 1.5f) / cel;
    int y = 3 + (joypos.y - center.y - cel * 1.5f) / cel;

    if (x >= 0 && x < 3)  { letter = x +(3 * y); }
    return (letters[letter]);
}

void init() {
    set_screen_mode(ScreenMode::lores);
    low_res = fire = button_up = 0;
    passcode = "";
}

void render(uint32_t time) {
Vec2 cursorpos;

    screen.pen = Pen(0, 0, 0);
    screen.clear();

    screen.pen = Pen(255,255,255);

    draw_keypad();

    cursorpos = Vec2(0,screen.bounds.h - 20);
    screen.text(passcode, minimal_font, cursorpos);

    if (move.x || move.y) {
    	draw_cursor(joypos);

    	screen.pen = Pen(255,0,0);
	char letter = keypad_press();
    	screen.text(std::string(1,letter), minimal_font, Vec2(0,0));
    
	if (button_up) {
		passcode += letter;
		button_up = 0;
		}
	}
}

void update(uint32_t time) {
	if (pressed(Button::B)) { fire = 1; } 
        if (!pressed(Button::B)) {
		if (fire == 1) { button_up = 1; }
		fire = 0;
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
