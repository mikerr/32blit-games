#include "starfield.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;

typedef struct star {
    vec2 velocity;
    vec2 pos;
} star;

std::vector<star> stars;

typedef struct shape {
    std::vector<vec2> points;
    vec2 pos;
    vec2 dir;
    vec3 color;
    float size;
} shape;

shape alien1;

void center_star( star &s) {
    vec2 center = vec2 (fb.bounds.w / 2, fb.bounds.h / 2);
    s.pos = center;
    s.velocity = vec2 (rand() % 100 - 50, rand() % 100 - 50) ;
}

void draw_cross(vec2 pos) {
    fb.pen(rgba(255,255,255));
    fb.line(pos + vec2 (-10, -10), pos + vec2 (10, 10) );
    fb.line(pos + vec2 (10, -10), pos + vec2 (-10, 10) );
}

void draw_laser(vec2 pos) {
    fb.pen(rgba(rand() % 255,rand() % 255,rand() % 255));
    fb.line(vec2 (0, fb.bounds.h), pos);
    fb.line(vec2 (fb.bounds.w, fb.bounds.h), pos);
}
void draw_shape(shape shape,vec2 pos,float size){
    fb.pen(rgba((int)shape.color.x,(int)shape.color.y,(int)shape.color.z));
    vec2 lastpos = shape.points[0];
    for (auto &p: shape.points) {
        vec2 point = p * size;
        fb.line(lastpos + pos , point + pos);
        lastpos = point;
    }
}
void new_alien () {
    alien1.color = vec3 (rand() % 255,rand() % 255,rand() % 255);
    vec2 center = vec2(fb.bounds.w / 2, fb.bounds.h / 2);
    alien1.pos = center;
    alien1.dir = vec2(rand() % 6 - 3 ,rand() % 6 - 3);

    alien1.size = 0.1;
}
void move_alien() {
    alien1.pos = alien1.pos + alien1.dir;
    alien1.size = alien1.size + 0.02;
    if ( alien1.size > 2) new_alien();
    if ((alien1.pos.x < 0)  || (alien1.pos.x > fb.bounds.w) || (alien1.pos.y < 0 ) || (alien1.pos.y > fb.bounds.h))
        new_alien();
}
int low_res, multicolor,hyperspace,fire,speed;

void init() {
    set_screen_mode(screen_mode::hires);

    alien1.points.push_back(vec2(0,0));
    alien1.points.push_back(vec2(10,10));
    alien1.points.push_back(vec2(20,0));
    alien1.points.push_back(vec2(0,0));
    new_alien();
    for (int i=0; i < 200; i++ ) {
        star s;
        center_star(s);
        stars.push_back(s);
        }
    low_res = hyperspace = fire = 0;
    speed = 50;
}

void render(uint32_t time) {
vec2 center,move,joypos;

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

    move_alien();
    if (!hyperspace ) draw_shape (alien1, alien1.pos,alien1.size);

    center = vec2(fb.bounds.w / 2, fb.bounds.h / 2);
    move = blit::joystick;

    joypos.x =  move.x * center.x ;
    joypos.y =  move.y * center.y ;
    joypos = joypos + center;

    if ( move.x || move.y ) { draw_cross(joypos); }
    
    vibration = 0.0f;
    if (fire) {
                draw_cross(joypos);
                draw_laser(joypos);
                if (abs(alien1.pos.x - joypos.x) < 10  && abs(alien1.pos.y - joypos.y) < 10 ) {
                        vibration = 0.1f;
                        new_alien();
                        }
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

