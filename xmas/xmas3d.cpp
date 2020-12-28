#include "32blit.hpp"
#include "types/vec3.hpp"
#include "types/vec2.hpp"
#include "graphics/color.hpp"

#include "assets.hpp"
using namespace blit;

typedef struct shape {
    std::vector<Vec3> points ;
} shape;

shape tree,snowflakes;

SpriteSheet *snow;

int low_res;
int snowsize;
float zoom,spin,speed;
Vec2 joypos,pos,dir,center;
Vec3 rot;

Vec2 rotate_point(Vec2 point,float angle){
    Mat3 t = Mat3::identity();
    t *= Mat3::rotation(angle) ;
    point *= t;
    return (point);
}

Vec3 rotate3d (Vec3 point3d,Vec3 rot) {
    Vec2 point;

    // rotate 3d point about X
    point = Vec2(point3d.y,point3d.z);
    point = rotate_point (point,rot.x);

    point3d.y = point.x;
    point3d.z = point.y;

    // rotate 3d point about Y
    point = Vec2(point3d.z,point3d.x);
    point = rotate_point (point,rot.y);

    point3d.z = point.x;
    point3d.x = point.y;

    // rotate 3d point about Z
    point = Vec2(point3d.x,point3d.y);
    point = rotate_point (point,rot.z);

    point3d.x = point.x;
    point3d.y = point.y;

    return (point3d);
}
Vec2 to2d (Vec3 point3d) {
    Vec2 point;

    // project to screen
    int z = point3d.z - 100;
    point.x = point3d.x * 100 / z;
    point.y = point3d.y * 100 / z;

    return (point);
    }

int onscreen (Vec2 to ){
   int x = to.x;
   int y = to.y;

   if ((x < 0) || (y < 0) || (x > screen.bounds.w) || (y > screen.bounds.h)) return (0);
   return (1);
}

void draw_shape(shape shape,Vec2 pos,float size){
    Vec3 p0 = shape.points[0];
    p0 = rotate3d(p0,rot);

    Vec2 lastpos = to2d(p0) * size;

    for (auto &p: shape.points) {
        p = rotate3d(p,rot);
        Vec2 point = to2d(p) * size;
        
        Vec2 from = lastpos + pos;
        Vec2 to = point + pos;

        if (onscreen(from) && onscreen (to)) screen.line(from,to) ;

        lastpos = point;
    }
}


void init() {
    set_screen_mode(ScreenMode::hires);
    center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);

    int branchwidth = 10;
    int branchtop = 100;

    tree.points.push_back(Vec3(0,120,0));
    
    for (int i=0; i < 10; i++) {
    	tree.points.push_back(Vec3(branchwidth,branchtop,0));
    	tree.points.push_back(Vec3(0,branchtop,0));

    	branchtop -= 20;
    	branchwidth += 10;
	}

    low_res = 0;
    spin = 0.02;
    zoom = 1;

    snowsize = 500;
    speed = 10;

    pos = center;
    dir = Vec2 (0,0);

    for (int i=0; i < 100; i++) {
    	snowflakes.points.push_back(Vec3(rand() % screen.bounds.w,rand() % screen.bounds.h,rand() % 100));
	}


    snow = SpriteSheet::load(snowflake);
}

void render(uint32_t time) {

    screen.pen = Pen(0, 0, 0);
    screen.clear();

    screen.pen = (Pen(255,255,255));

    pos = pos + dir;
    draw_shape (tree, pos, zoom);

    rot += Vec3(0,spin,0);


    for (auto &p: snowflakes.points) {
	screen.stretch_blit(snow,Rect(0,0,256,256),Rect(p.x,p.y, p.z * screen.bounds.w / snowsize, p.z * screen.bounds.h / snowsize));

	p.y +=  p.z /  speed ;
	if (p.y > screen.bounds.h) { p.y = -50; } // set a few pixels off screen so it gradually appears
	}
}

void update(uint32_t time) {

    pos += joystick;

    if (pressed(Button::DPAD_UP) && snowsize > 200)  { snowsize -= 1;}
    if (pressed(Button::DPAD_DOWN) && snowsize < 1000)  { snowsize += 1;}

    if (pressed(Button::DPAD_RIGHT) && speed > 4)  { speed -= 0.1;}
    if (pressed(Button::DPAD_LEFT) && speed < 200) { speed += 0.1;}

}

