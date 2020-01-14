#include "keypad.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;

int low_res,fire,button_up;
std::string passcode;
vec2 center,move,joypos;

void draw_cursor(vec2 pos) {
    fb.pen(rgba(255,255,255));
    fb.line(pos + vec2 (-5, -5), pos + vec2 (5, 5) );
    fb.line(pos + vec2 (5, -5), pos + vec2 (-5, 5) );
}

void draw_keypad() {
    int cel = fb.bounds.w / 5;
    vec2 pos = center - vec2(cel,cel * 1.5f);

    for (int n=1; n<10;n++) {
        fb.text(std::to_string(n), &minimal_font[0][0], pos);
        pos.x += cel;
        if (n % 3 == 0 ) {
                pos.x = center.x - cel;
                pos.y += cel;
                }
        }
    pos.x = center.x;
    fb.text("0", &minimal_font[0][0], pos);
}

char keypad_press () {
    char letters[] = "123456789 0 ";
    int cel = fb.bounds.w / 5;
    int letter=10;

    int x = 3 + (joypos.x - center.x - cel * 1.5f) / cel;
    int y = 3 + (joypos.y - center.y - cel * 1.5f) / cel;

    if (x >= 0 && x < 3)  { letter = x +(3 * y); }
    return (letters[letter]);
}

void init() {
    set_screen_mode(screen_mode::lores);
    low_res = fire = button_up = 0;
    passcode = "";
}

void render(uint32_t time) {
vec2 cursorpos;

    fb.pen(rgba(0, 0, 0));
    fb.clear();

    fb.pen(rgba(255,255,255));

    draw_keypad();

    cursorpos = vec2(0,fb.bounds.h - 20);
    fb.text(passcode, &minimal_font[0][0], cursorpos);

    if (move.x || move.y) {
        draw_cursor(joypos);

        fb.pen(rgba(255,0,0));
        char letter = keypad_press();
        fb.text(std::string(1,letter), &minimal_font[0][0], vec2(0,0));

        if (button_up) {
                passcode += letter;
                button_up = 0;
                }
        }
}

void update(uint32_t time) {
        if (pressed(button::B)) { fire = 1; }
        if (!pressed(button::B)) {
                if (fire == 1) { button_up = 1; }
                fire = 0;
                }
        if (pressed(button::DPAD_DOWN))  {
                low_res = 1 - low_res;
                if (low_res) {
                        set_screen_mode(screen_mode::hires);
                        } else {
                        set_screen_mode(screen_mode::lores);
                        }
                }

    center = vec2(fb.bounds.w / 2, fb.bounds.h / 2);
    move = blit::joystick;

    joypos.x =  move.x * center.x ;
    joypos.y =  move.y * center.y ;
    joypos = joypos + center;
}
