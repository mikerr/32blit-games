#include "vector3d.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;

typedef struct shape {
    std::vector<vec3> points ;
} shape;

shape cube;
shape square;
shape pyramid;

int low_res;
float zoom, zd;
vec2 joypos,pos,dir,center;
vec3 rot,spin;

typedef struct object {
        shape model;
        vec3 position;
        vec3 rotation;
        vec3 spin;
        vec3 velocity;
} object;

object ship;
std::vector<object> objects;

vec2 rotate_point(vec2 point,float angle){
    mat3 t = mat3::identity();
    t *= mat3::rotation(angle) ;
    point *= t;
    return (point);
}

vec3 rotate3d (vec3 point3d,vec3 rot) {
    vec2 point;

    // rotate 3d point about X
    point = vec2(point3d.y,point3d.z);
    point = rotate_point (point,rot.x);

    point3d.y = point.x;
    point3d.z = point.y;

    // rotate 3d point about Y
    point = vec2(point3d.z,point3d.x);
    point = rotate_point (point,rot.y);
    
    point3d.z = point.x;
    point3d.x = point.y;

    // rotate 3d point about Z
    point = vec2(point3d.x,point3d.y);
    point = rotate_point (point,rot.z);

    point3d.x = point.x;
    point3d.y = point.y;

    return (point3d);
}

vec2 to2d (vec3 point3d) {
    vec2 point;

    // project to screen
    int z = point3d.z - 100;
    point.x = point3d.x * 100 / z;
    point.y = point3d.y * 100 / z;

    return (point);
    }

void draw_object(shape shape,vec2 pos,float size){
    vec3 p0 = shape.points[0];
    p0 = rotate3d(p0,rot);

    vec2 lastpos = to2d(p0) * size;

    for (auto &p: shape.points) {
        p = rotate3d(p,rot);
        vec2 point = to2d(p) * size;
        fb.line(lastpos + pos, point + pos);
        lastpos = point;
    }
}

vec3 rand3 () {
     float x = (5 - rand() % 10) / 100.0f;
     float y = (5 - rand() % 10) / 100.0f;
     float z = (5 - rand() % 10) / 100.0f;
     return vec3(x,y,z);
}
void init() {
    set_screen_mode(screen_mode::hires);
    center = vec2(fb.bounds.w / 2, fb.bounds.h / 2);

    cube.points.push_back(vec3(-10,-10,-10));
    cube.points.push_back(vec3(-10,-10,10));
    cube.points.push_back(vec3(-10,10,10));
    cube.points.push_back(vec3(10,10,10));
    cube.points.push_back(vec3(10,-10,10));
    cube.points.push_back(vec3(10,-10,-10));
    cube.points.push_back(vec3(-10,-10,-10));
    cube.points.push_back(vec3(-10,10,-10));
    cube.points.push_back(vec3(-10,10,10));
    cube.points.push_back(vec3(-10,10,-10));
    cube.points.push_back(vec3(10,10,-10));
    cube.points.push_back(vec3(10,10,10));
    cube.points.push_back(vec3(10,10,-10));
    cube.points.push_back(vec3(10,-10,-10));
    cube.points.push_back(vec3(10,-10,10));
    cube.points.push_back(vec3(-10,-10,10));

    square.points.push_back(vec3(-10,-10,-10));
    square.points.push_back(vec3(-10,-10,10));
    square.points.push_back(vec3(-10,10,10));
    square.points.push_back(vec3(-10,10,-10));
    square.points.push_back(vec3(-10,-10,-10));

    pyramid.points.push_back(vec3(-10,-10,-10)); // base
    pyramid.points.push_back(vec3(-10,-10,10));
    pyramid.points.push_back(vec3(-10,10,10));
    pyramid.points.push_back(vec3(-10,10,-10));
    pyramid.points.push_back(vec3(-10,-10,-10));
    pyramid.points.push_back(vec3(10,0,0)); // point
    pyramid.points.push_back(vec3(-10,10,10));
    pyramid.points.push_back(vec3(-10,10,-10));
    pyramid.points.push_back(vec3(10,0,0));
    pyramid.points.push_back(vec3(-10,-10,10));

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

    fb.pen(rgba(0, 0, 0));
    fb.clear();

    fb.pen(rgba(255,255,255));

    pos = vec2(0,0);

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
        if (pos.y > fb.bounds.h + 30) break;
        }
}
void update(uint32_t time) {

    vec2 move = blit::joystick;
    zoom -= move.y /10.0;

    zoom += zd;
    if (( zoom > 6 ) || ( zoom < 1 )) zd = -zd;

    if (pressed(button::DPAD_LEFT))  spin.y -= 0.001;
    if (pressed(button::DPAD_RIGHT)) spin.y += 0.001;
    if (pressed(button::DPAD_UP))    zoom += 0.01;
    if (pressed(button::DPAD_DOWN))  zoom -= 0.01;

    if (pressed(button::Y)) pos.x--;
    if (pressed(button::A)) pos.x++;
    if (pressed(button::X)) pos.y--;
    if (pressed(button::B)) pos.y++;

    if (pressed(button::MENU))  {
        low_res = !low_res;
        if (low_res) {
                set_screen_mode(screen_mode::hires);
                center = vec2(fb.bounds.w / 2, fb.bounds.h / 2);
                } else {
                set_screen_mode(screen_mode::lores);
                center = vec2(fb.bounds.w / 2, fb.bounds.h / 2);
                }
        pos = center;
        }
}

