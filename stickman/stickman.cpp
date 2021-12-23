#include "32blit.hpp"

using namespace blit;

int speed;
float dustx,count,rotate,roll;

Vec2 center,neck,hip;

struct bone { Vec2 joint1; Vec2 joint2; };

std::vector<bone> bones;

Pen black = Pen(0,0,0);
Pen white = Pen(255,255,255);

float deg2rad( float deg){ return ( (deg / 180.0f) * 3.142); }

void add_leg(int side) {
float angle;
Vec2 knee,ankle;

    angle = sin(count + side) * 30;
    angle = deg2rad(angle +170);

    knee = Vec2 (sin (angle), -cos (angle)) * 50;
    knee += hip;
    bones.push_back( {hip,knee} );

    angle = sin (count + side - 0.873f) * 45;
    angle = deg2rad(angle + 225);

    ankle = Vec2 (sin (angle), -cos (angle)) * 50;
    ankle += knee;
    bones.push_back ( {knee,ankle} );
}

void add_arm(int side) {
float angle;
Vec2 shoulder,elbow,wrist;

    shoulder = neck + Vec2(0,10);

    bones.push_back ( {shoulder,neck} );

    angle = deg2rad(sin(count + side) * 60);

    elbow = Vec2 (sin (angle), cos (angle)) * 30;
    elbow += shoulder;
    bones.push_back ( {shoulder,elbow} );

    angle = sin(count + side) * 60;
    angle = deg2rad(angle + 50);

    wrist = Vec2 (sin (angle), cos (angle)) * 30;
    wrist += elbow;
    bones.push_back ( {elbow,wrist} );
}

void add_torso(void) {

    neck = Vec2(0,-70);
    neck += Vec2(sin(count)*3,cos(count)*3);
    //neck += Vec2(rot / 10.0f, rot / 20.0f);
    neck += hip;
    bones.push_back ( {hip,neck} );

}

void draw_head(void) {
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
float left,right = 3.142;

    screen.pen = black;
    screen.clear();

    drawground();

    screen.pen = white;

    bones.clear();

    add_leg(left);
    add_leg(right);

    add_torso();

    add_arm(left);
    add_arm(right);

    for (auto bone:bones) 
	    screen.line(bone.joint1 + center, bone.joint2 + center);
    draw_head();
}

void update(uint32_t time) {
    count += speed / 2000.0f;

    dustx -= speed / 50.0f;
    if (dustx < -160) dustx = 0;

    if (pressed(Button::Y) && speed > 20) speed--;
    if (pressed(Button::A) && speed < 900) speed++;

    if (pressed(DPAD_LEFT)) rotate--;
    if (pressed(DPAD_RIGHT)) rotate++;

    if (pressed(DPAD_UP)) roll--;
    if (pressed(DPAD_DOWN)) roll++;
}
