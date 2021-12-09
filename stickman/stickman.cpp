#include "32blit.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;

typedef struct shape {
    std::vector<Vec3> points ;
} shape;

shape stickman;

int rot;
float x,count;
Vec2 center;

Vec2 neck,elbow,wrist,hip,knee,ankle;

Pen black = Pen(0,0,0);
Pen white = Pen(255,255,255);

float deg2rad( float deg){
	return ( (deg / 180.0f) * 3.142);
}

void drawjoint (Vec2 start, Vec2 end) {
    start += center;
    end += center;
    screen.line (start,end);
    if (pressed(Y)) screen.circle(end,5);
}

void drawleg(int phase) {
float sinc;

    sinc = sin(count + phase) * 20;
    sinc = deg2rad(sinc +170);

    knee = Vec2 (sin (sinc), -cos (sinc)) * 50;
    knee += hip;
    drawjoint (hip,knee);

    sinc = sin (count + phase - 0.873f) * 45;
    sinc = deg2rad(sinc + 225);

    ankle = Vec2 (sin (sinc), -cos (sinc)) * 50;
    ankle += knee;
    drawjoint (knee,ankle);
}

void drawarm(int phase) {
float sinc;

Vec2 shoulder = neck + Vec2(0,15);

    drawjoint (shoulder,neck);

    sinc = (sin(count + phase) * 40);
    sinc = deg2rad(sinc - 20);

    elbow = Vec2 (sin (sinc), cos (sinc)) * 30;
    elbow += shoulder;
    drawjoint (shoulder,elbow);

    sinc = sin (count + phase + 0.873f + 1) * 15;
    sinc = deg2rad(sinc + 90);

    wrist = Vec2 (sin (sinc), cos (sinc)) * 30;
    wrist += elbow;
    drawjoint (elbow,wrist);
}

void drawtorso(void) {

    neck = Vec2(0,-70);
    neck += Vec2(sin(count)*3,cos(count)*3);
    //neck += Vec2(rot / 10.0f, rot / 20.0f);
    neck += hip;
    drawjoint (hip,neck);

    Vec2 head = center + neck - Vec2(0,20);
    screen.circle(head,15);
    screen.pen = black; 
    screen.circle(head,14);
    screen.pen = white;
}

void init() {
    set_screen_mode(ScreenMode::hires);
    center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);
    rot = 100;
}

void render(uint32_t time) {
float left,right;
    right = 3.142;

    screen.pen = black;
    screen.clear();

    screen.pen = white;
 
    drawleg(left);
    drawleg(right);

    drawtorso();

    drawarm(left);
    drawarm(right);

    for(int dust=0; dust < screen.bounds.w + 200; dust += 40) {
            screen.pen = Pen(225,225,225);
	    screen.pixel (Vec2(x + dust,220));
            screen.pen = Pen(200,200,200);
	    screen.pixel (Vec2(x/2 + dust,200));
            screen.pen = Pen(160,160,160);
	    screen.pixel (Vec2(x/4 + dust,180));
    }

}
void update(uint32_t time) {
    count += rot / 2000.0f;

    x -= rot / 50.0f;
    if (x < -160) x = 0;

    if (pressed(DPAD_LEFT) && rot > 20) rot--;
    if (pressed(DPAD_RIGHT) && rot < 900) rot++;
}
