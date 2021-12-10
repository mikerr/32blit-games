#include "32blit.hpp"

using namespace blit;

int speed;
float dustx,count;

Vec2 center,neck,hip;

Pen black = Pen(0,0,0);
Pen white = Pen(255,255,255);

float deg2rad( float deg){
	return ( (deg / 180.0f) * 3.142);
}

void drawjoint (Vec2 start, Vec2 end) {
    start += center;
    end += center;
    screen.line (start,end);
    if (pressed(Y)) {
	    screen.circle(start,5);
	    screen.circle(end,5);
    }
}

void drawleg(int phase) {
float sinc;
Vec2 knee,ankle;

    sinc = sin(count + phase) * 30;
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
float sinc,cosc;
Vec2 shoulder,elbow,wrist;

    shoulder = neck + Vec2(0,10);

    drawjoint (shoulder,neck);

    sinc = deg2rad(sin(count + phase) * 60);

    elbow = Vec2 (sin (sinc), cos (sinc)) * 30;
    elbow += shoulder;
    drawjoint (shoulder,elbow);

    sinc = sin(count + phase) * 60;
    sinc = deg2rad(sinc + 50);

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

void drawground ( void) {

    for(int dust=0; dust < screen.bounds.w + 200; dust += 40) {
            screen.pen = Pen(225,225,225);
	    screen.pixel (Vec2(dustx + dust,220));
            screen.pen = Pen(200,200,200);
	    screen.pixel (Vec2(dustx/2 + dust,200));
            screen.pen = Pen(160,160,160);
	    screen.pixel (Vec2(dustx/4 + dust,180));
    }
}
void init() {
    set_screen_mode(ScreenMode::hires);
    center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);
    speed = 100;
}

void render(uint32_t time) {
float left,right;
    right = 3.142;

    screen.pen = black;
    screen.clear();

    drawground();

    screen.pen = white;

    drawleg(left);
    drawleg(right);

    drawtorso();

    drawarm(left);
    drawarm(right);
}

void update(uint32_t time) {
    count += speed / 2000.0f;

    dustx -= speed / 50.0f;
    if (dustx < -160) dustx = 0;

    if (pressed(DPAD_LEFT) && speed > 20) speed--;
    if (pressed(DPAD_RIGHT) && speed < 900) speed++;
}
