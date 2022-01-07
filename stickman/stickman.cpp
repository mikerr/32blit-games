#include "32blit.hpp"

using namespace blit;

int speed;
float count, rotate, roll, spin;

Vec2 center;
Vec3 neck, hip;
Vec3 rot3d, pos;

struct bone { Vec3 joint1; Vec3 joint2; };

std::vector<bone> bones;

Pen black = Pen(0,0,0);
Pen white = Pen(255,255,255);
Pen green = Pen(0,255,0);

float deg2rad( float deg){ return ( (deg / 180.0f) * 3.142); }

Vec3 rotate3d (Vec3 point3d,Vec3 rot) {
	static Vec2 point;

	    //rotate 3d point about X
	    point = Vec2(point3d.y,point3d.z);
	    point *= Mat3::rotation(rot.x);
	    
	    point3d.y = point.x;
	    point3d.z = point.y;
	   
	    // rotate 3d point about Y
	    point = Vec2(point3d.z,point3d.x);
	    point *= Mat3::rotation(rot.y);
	    
	    point3d.z = point.x;
	    point3d.x = point.y;
	    
	    // rotate 3d point about Z
	    point = Vec2(point3d.x,point3d.y);
	    point *= Mat3::rotation(rot.z);
	    
	    point3d.x = point.x;
	    point3d.y = point.y;
	    
	    return (point3d);
}
	    
	    
Vec2 to2d (Vec3 point3d) {
	    Vec2 point;
	    
	    // project to screen
	    int z = point3d.z - 500;
	    point.x = point3d.x * 500 / z;
	    point.y = point3d.y * 500 / z;
	    
	    return (point);
}
	    
void add_leg(int side) {
float angle;
Vec3 knee,ankle;

    angle = sin(count + side) * 30;
    angle = deg2rad(angle +170);

    knee = Vec3 (sin (angle), -cos (angle),0) * 50;
    knee += hip;
    bones.push_back( {hip,knee} );

    angle = sin (count + side - 0.873f) * 45;
    angle = deg2rad(angle + 225);

    ankle = Vec3 (sin (angle), -cos (angle),0) * 50;
    ankle += knee;
    bones.push_back ( {knee,ankle} );
}

void add_arm(int side) {
float angle;
Vec3 shoulder,elbow,wrist;

    shoulder = neck + Vec3(0,10,0);

    bones.push_back ( {shoulder,neck} );

    angle = deg2rad(sin(count + side) * 60);

    elbow = Vec3 (sin (angle), cos (angle),0) * 30;
    elbow += shoulder;
    bones.push_back ( {shoulder,elbow} );

    angle = sin(count + side) * 60;
    angle = deg2rad(angle + 50);

    wrist = Vec3 (sin (angle), cos (angle),0) * 30;
    wrist += elbow;
    bones.push_back ( {elbow,wrist} );
}

void add_torso(void) {

    neck = Vec3(0,-70,0);
    neck += hip;
    bones.push_back ( {hip,neck} );

}

void draw_head( Vec2 head) {
 
    head += center;
    screen.circle(head,15);
    screen.pen = black; 
    screen.circle(head,14);
}

void draw_stickman(void) {

    for (auto bone:bones) 
	    screen.line(to2d(bone.joint1) + center, to2d(bone.joint2) + center);
}

void draw_ground(void) {
static float dustx;

    dustx -= speed / 20.0f;
    if (dustx < 0) dustx = 100;

    // make a grid of dots
    for (int x=0; x < 10; x++) 
    	    for (int y=0; y < 10 ; y++) {
	    	Vec3 dust = Vec3(x-5,0,y-5) * 50;
	        dust.x += dustx;
		
		dust = rotate3d(dust,rot3d);
		dust.y -= 100;
		screen.pixel(to2d(dust) + center);
	    }

}
void init() {
    set_screen_mode(ScreenMode::hires);
    center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);
    speed = 100;
    roll = 300;
}

void render(uint32_t time) {
float left,right = 3.142;

    screen.pen = black;
    screen.clear();

    screen.pen = green;
    draw_ground();

    screen.pen = white;

    bones.clear();

    add_leg(left);
    add_leg(right);

    add_torso();

    add_arm(left);
    add_arm(right);

    Vec3 head = neck - Vec3(0,20,0);

    for (auto &bone:bones) {
	    bone.joint1 += pos;
	    bone.joint2 += pos;
    }
    head += pos;

    //stickman
    for (auto &bone:bones) {
		bone.joint1 = rotate3d(bone.joint1, rot3d);
		bone.joint2 = rotate3d(bone.joint2, rot3d);
    }
    head = rotate3d(head,rot3d);

    draw_stickman();
    draw_head(to2d(head));
}

void update(uint32_t time) {
static int demo=true;
    count += speed / 2000.0f;

    pos.x += blit::joystick.x;
    pos.z += blit::joystick.y;

    if (pressed(Button::Y) && speed > 20) speed--;
    if (pressed(Button::A) && speed < 900) speed++;

    if (pressed(DPAD_LEFT)) rotate--;
    if (pressed(DPAD_RIGHT)) rotate++;

    if (pressed(DPAD_UP)) roll--;
    if (pressed(DPAD_DOWN)) roll++;

    if (pressed(Button::X) ) spin++;
    if (pressed(Button::B) ) spin--;

    if (buttons.released) demo = false;
    if (demo) rotate--; 

    rot3d = Vec3(roll/100,rotate/100,spin/100);
    }
