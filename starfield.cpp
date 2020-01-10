#include "tanks.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;

typedef struct star {
    vec2 velocity;
    vec2 pos;
} star;

std::vector<star> stars;

void center_star( star &s) {
    vec2 center = vec2 (fb.bounds.w / 2, fb.bounds.h / 2);
    s.pos = center;
    s.velocity = vec2 (rand() % 100 - 50, rand() % 100 - 50) ;
}

void draw_cross(vec2 pos) {
    int x = pos.x;
    int y = pos.y;
    fb.line(vec2 (x - 10, y - 10), vec2 (x + 10, y + 10));
    fb.line(vec2 (x + 10, y - 10), vec2 (x - 10, y + 10));
}

void draw_laser(vec2 pos) {
    int x = pos.x;
    int y = pos.y;
    fb.pen(rgba(rand() % 255,rand() % 255,rand() % 255));
    fb.line(vec2 (0, fb.bounds.h), vec2 (x,y));
    fb.line(vec2 (fb.bounds.w, fb.bounds.h), vec2 (x,y));
}

int low_res, multicolor,hyperspace,fire,speed;

void init() {
    set_screen_mode(screen_mode::hires);

    for (int i=0; i < 200; i++ ) {
        star s;
        center_star(s);
        stars.push_back(s);
        }
    low_res = hyperspace = fire = 0;
    speed = 50;
}

void render(uint32_t time) {
vec2 center,move,cross;

    if (!hyperspace) {
        fb.pen(rgba(0, 0, 0));
        fb.clear();
        }

    fb.pen(rgba(255,255,255));

    for (auto &s : stars) {

        s.pos.x  += s.velocity.x / speed;
        s.pos.y  += s.velocity.y / speed;

        s.velocity.x *= 1.1f;
        s.velocity.y *= 1.1f;

        if ((s.pos.x < 0)  || (s.pos.y < 0) || (s.pos.x > fb.bounds.w)  ||  (s.pos.y > fb.bounds.h)) { center_star(s); }

        fb.pixel(s.pos);
        }
        
    move = blit::joystick;
    if ( move.x || move.y ) {
        center = vec2 (fb.bounds.w / 2, fb.bounds.h / 2);
        move.x =  move.x * center.x ;
        move.y =  move.y * center.y ;
        cross = center + move;
        draw_cross(cross);
        }

    vibration = 0.0f;
    if (fire) {
        vibration = 0.1f;
        draw_cross(cross);
        draw_laser(cross);
        }
}

void update(uint32_t time) {
        if (pressed(button::DPAD_RIGHT) && speed > 20 )  { speed--; }
        if (pressed(button::DPAD_LEFT)  && speed < 500 ) { speed++; }
        if (pressed(button::B) || pressed(button::JOYSTICK)) { fire = 1; } else { fire = 0; }
        if (pressed(button::X) || pressed(button::DPAD_UP)) { hyperspace = 1; } else { hyperspace = 0; }
        if (pressed(button::DPAD_DOWN))  {
                low_res = 1 - low_res;
                if (low_res) {
                        set_screen_mode(screen_mode::hires);
                        } else {
                        set_screen_mode(screen_mode::lores);
                        }
                }
}
