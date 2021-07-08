#include "starfield.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;

typedef struct star {
    Vec2 velocity;
    Vec2 pos;
} star;

std::vector<star> stars;

typedef struct shape {
    std::vector<Vec2> points;
    Vec2 pos;
    Vec2 dir;
    Vec3 color;
    float size;
} shape;

shape alien1;

void center_star( star &s) {
    Vec2 center = Vec2 (screen.bounds.w / 2, screen.bounds.h / 2);
    s.pos = center;
    s.velocity = Vec2 (rand() % 100 - 50, rand() % 100 - 50) ;
}

void draw_cross(Vec2 pos) {
    screen.pen = Pen(255,255,255);
    screen.line(pos + Vec2 (-10, -10), pos + Vec2 (10, 10) );
    screen.line(pos + Vec2 (10, -10), pos + Vec2 (-10, 10) );
}

void draw_laser(Vec2 pos) {
    screen.pen = Pen(rand() % 255,rand() % 255,rand() % 255);
    screen.line(Vec2 (0, screen.bounds.h), pos);
    screen.line(Vec2 (screen.bounds.w, screen.bounds.h), pos);
}
void draw_shape(shape shape,Vec2 pos,float size){
    screen.pen = Pen((int)shape.color.x,(int)shape.color.y,(int)shape.color.z);
    Vec2 lastpos = shape.points[0];
    for (auto &p: shape.points) {
        Vec2 point = p * size;
        screen.line(lastpos + pos , point + pos);
        lastpos = point;
    }
}
void new_alien () {
    alien1.color = Vec3 (rand() % 255,rand() % 255,rand() % 255);
    Vec2 center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);
    alien1.pos = center;
    alien1.dir = Vec2(rand() % 6 - 3 ,rand() % 6 - 3);

    alien1.size = 0.1;
}
void move_alien() {
    alien1.pos = alien1.pos + alien1.dir;
    alien1.size = alien1.size + 0.02f;
    if ( alien1.size > 2) new_alien();
    if ((alien1.pos.x < 0)  || (alien1.pos.x > screen.bounds.w) || (alien1.pos.y < 0 ) || (alien1.pos.y > screen.bounds.h))
        new_alien();
}
int low_res, multicolor,hyperspace,fire,speed;

void init() {
    set_screen_mode(ScreenMode::hires);

    alien1.points.push_back(Vec2(0,0));
    alien1.points.push_back(Vec2(10,10));
    alien1.points.push_back(Vec2(20,0));
    alien1.points.push_back(Vec2(0,0));
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
Vec2 center,move,joypos;

    if (!hyperspace) {
        screen.pen = Pen(0, 0, 0);
        screen.clear();
        }

    screen.pen = Pen(255,255,255);

    for (auto &s : stars) {

        s.pos.x  += s.velocity.x / speed;
        s.pos.y  += s.velocity.y / speed;

        s.velocity.x *= 1.1f;
        s.velocity.y *= 1.1f;

        if ((s.pos.x < 0)  || (s.pos.y < 0) || (s.pos.x > screen.bounds.w)  ||  (s.pos.y > screen.bounds.h)) { center_star(s); }

        screen.pixel(s.pos);
        }

    move_alien();
    if (!hyperspace ) draw_shape (alien1, alien1.pos,alien1.size);

    center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);
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
        if (pressed(Button::DPAD_RIGHT) && speed > 20 )  { speed--; }
        if (pressed(Button::DPAD_LEFT)  && speed < 500 ) { speed++; }
        if (pressed(Button::B) || pressed(Button::JOYSTICK)) { fire = 1; } else { fire = 0; }
        if (pressed(Button::X) || pressed(Button::DPAD_UP)) { hyperspace = 1; } else { hyperspace = 0; }
        if (pressed(Button::DPAD_DOWN))  {
                low_res = 1 - low_res;
                if (low_res) {
                        set_screen_mode(ScreenMode::hires);
                        } else {
                        set_screen_mode(ScreenMode::lores);
                        }
                }
}


