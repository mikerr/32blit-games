#include "vector3d.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;

typedef struct shape {
    std::vector<Vec3> points ;
} shape;

shape cube;
shape square;
shape pyramid;

int low_res;
float zoom, zd;
Vec2 joypos,pos,dir,center;
Vec3 rot,spin;

typedef struct object {
        shape model;
        Vec3 position;
        Vec3 rotation;
        Vec3 spin;
        Vec3 velocity;
} object;

object ship;
std::vector<object> objects;

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
void draw_object(shape shape,Vec2 pos,float size){
    Vec3 p0 = shape.points[0];
    p0 = rotate3d(p0,rot);

    Vec2 lastpos = to2d(p0) * size;

    for (auto &p: shape.points) {
        p = rotate3d(p,rot);
        Vec2 point = to2d(p) * size;
        screen.line(lastpos + pos, point + pos);
        lastpos = point;
    }
}

Vec3 rand3 () {
     float x = (5 - rand() % 10) / 100.0f;
     float y = (5 - rand() % 10) / 100.0f;
     float z = (5 - rand() % 10) / 100.0f;
     return Vec3(x,y,z);
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

    square.points.push_back(Vec3(-10,-10,-10));
    square.points.push_back(Vec3(-10,-10,10));
    square.points.push_back(Vec3(-10,10,10));
    square.points.push_back(Vec3(-10,10,-10));
    square.points.push_back(Vec3(-10,-10,-10));

    pyramid.points.push_back(Vec3(-10,-10,-10)); // base
    pyramid.points.push_back(Vec3(-10,-10,10));
    pyramid.points.push_back(Vec3(-10,10,10));
    pyramid.points.push_back(Vec3(-10,10,-10));
    pyramid.points.push_back(Vec3(-10,-10,-10));
    pyramid.points.push_back(Vec3(10,0,0)); // point
    pyramid.points.push_back(Vec3(-10,10,10));
    pyramid.points.push_back(Vec3(-10,10,-10));
    pyramid.points.push_back(Vec3(10,0,0));
    pyramid.points.push_back(Vec3(-10,-10,10));

    for (int x=0; x<100; x++) {
        ship.model = cube;
        ship.spin = rand3();
        objects.push_back(ship);
        }

    low_res = 0;
    zoom = 1;
    zd = 0.01;
}

void render(uint32_t time) {

    screen.pen(RGBA(0, 0, 0));
    screen.clear();

    screen.pen(RGBA(255,255,255));

    pos = Vec2(0,0);

    int x = 1;
    for (auto &obj: objects) {
        obj.position += obj.velocity;
        obj.rotation += obj.spin;

        rot = obj.rotation;
        draw_object(obj.model,pos,zoom);
        pos.x += 30 * zoom;
        x++;
        if (x % 11 == 0) {
                pos.x = 0;
                pos.y += 30 * zoom;
                }
        if (pos.y > screen.bounds.h + 30) break;
        }
}

void update(uint32_t time) {

    Vec2 move = blit::joystick;
    zoom -= move.y /10.0;

    zoom += zd;
    if (( zoom > 6 ) || ( zoom < 1 )) zd = -zd;

    if (pressed(Button::DPAD_LEFT))  spin.y -= 0.001;
    if (pressed(Button::DPAD_RIGHT)) spin.y += 0.001;
    if (pressed(Button::DPAD_UP))    zoom += 0.01;
    if (pressed(Button::DPAD_DOWN))  zoom -= 0.01;

    if (pressed(Button::Y)) pos.x--;
    if (pressed(Button::A)) pos.x++;
    if (pressed(Button::X)) pos.y--;
    if (pressed(Button::B)) pos.y++;

    if (pressed(Button::MENU))  {
        low_res = !low_res;
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




