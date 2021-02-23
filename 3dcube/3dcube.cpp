#include "32blit.hpp"

using namespace blit;

typedef struct shape {
    std::vector<Vec3> points ;
} shape;

shape cube;

int low_res;
float zoom,spin;
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

void draw_shape(shape shape,Vec2 pos,float size){
    Vec3 p0 = shape.points[0];
    p0 = rotate3d(p0,rot);

    Vec2 lastpos = to2d(p0) * size;

    for (auto &p: shape.points) {
        p = rotate3d(p,rot);
        Vec2 point = to2d(p) * size;
        screen.line(lastpos + pos , point + pos);
        lastpos = point;
    }
}


void init() {
    set_screen_mode(ScreenMode::hires);
    center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);

    cube.points.push_back(Vec3(-10,-10,-10));
    cube.points.push_back(Vec3(-10,-10,10));
    cube.points.push_back(Vec3(-10,10,10));
    cube.points.push_back(Vec3(10,10,10));
    cube.points.push_back(Vec3(10,-10,10));
    cube.points.push_back(Vec3(10,-10,-10));
    cube.points.push_back(Vec3(-10,-10,-10));
    cube.points.push_back(Vec3(-10,10,-10));
    cube.points.push_back(Vec3(-10,10,10));
    cube.points.push_back(Vec3(-10,10,-10));
    cube.points.push_back(Vec3(10,10,-10));
    cube.points.push_back(Vec3(10,10,10));
    cube.points.push_back(Vec3(10,10,-10));
    cube.points.push_back(Vec3(10,-10,-10));
    cube.points.push_back(Vec3(10,-10,10));
    cube.points.push_back(Vec3(-10,-10,10));

    low_res = 0;
    spin = 0.02;
    zoom = 3;

    pos = center;
    dir = Vec2 (1,2);
}

void render(uint32_t time) {

    screen.pen = (Pen(0, 0, 0));
    screen.clear();

    screen.pen = (Pen(255,255,255));

    Vec2 edge = center * 2;
    int cubesize = zoom * 10;
    if ((pos.x < cubesize ) || (pos.x > edge.x - cubesize )) { 
	    dir.x = -dir.x; 
	    spin = spin; 
    }
    if ((pos.y < cubesize ) || (pos.y > edge.y - cubesize )) { 
	    dir.y = -dir.y; 
	    spin = spin; 
    }

    pos = pos + dir;
    draw_shape (cube, pos, zoom);

    rot += Vec3(spin,spin,0);
}

void update(uint32_t time) {

    Vec2 move = blit::joystick;
    if (move.y > 0) zoom += 0.1;
    if (move.y < 0) zoom -= 0.1;
    pos.x += move.x;

    if (pressed(Button::DPAD_LEFT))  { spin -= 0.001;}
    if (pressed(Button::DPAD_RIGHT)) { spin += 0.001;}

    if (pressed(Button::X))  {
        low_res = 1 - low_res;
        if (low_res) {
                set_screen_mode(ScreenMode::hires);
                center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);
                } else {
                set_screen_mode(ScreenMode::lores);
                center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);
                }
        pos = center;
        }
}

