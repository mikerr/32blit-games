#include "vector3d.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;

typedef struct shape {
    std::vector<vec3> points ;
} shape;

shape cube;

int low_res;
float zoom,spin;
vec2 joypos,pos,dir,center;
vec3 rot;

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

void draw_shape(shape shape,vec2 pos,float size){
    vec3 p0 = shape.points[0];
    p0 = rotate3d(p0,rot);

    vec2 lastpos = to2d(p0) * size;

    for (auto &p: shape.points) {
        p = rotate3d(p,rot);
        vec2 point = to2d(p) * size;
        fb.line(lastpos + pos , point + pos);
        lastpos = point;
    }
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
    
    low_res = 0;
    spin = 0.02;
    zoom = 3;

    pos = center;
    dir = vec2 (1,2);
}

void render(uint32_t time) {

    fb.pen(rgba(0, 0, 0));
    fb.clear();

    fb.pen(rgba(255,255,255));

    vec2 edge = center * 2;
    int cubesize = zoom * 10;
    if ((pos.x < cubesize ) || (pos.x > edge.x - cubesize )) { dir.x = -dir.x; spin = spin; }
    if ((pos.y < cubesize ) || (pos.y > edge.y - cubesize )) { dir.y = -dir.y; spin = spin; }

    pos = pos + dir;
    draw_shape (cube, pos, zoom);

    rot += vec3(spin,spin,0);
}

void update(uint32_t time) {

    vec2 move = blit::joystick;
    if (move.y > 0) zoom += 0.1;
    if (move.y < 0) zoom -= 0.1;
    pos.x += move.x;

    if (pressed(button::DPAD_LEFT))  { spin -= 0.001;}
    if (pressed(button::DPAD_RIGHT)) { spin += 0.001;}

    if (pressed(button::X))  {
        low_res = 1 - low_res;
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

