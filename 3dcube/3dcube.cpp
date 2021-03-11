#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

typedef struct shape {
    std::vector<Vec3> points;
    std::vector<Vec3> faces;
} shape;

shape cube,texcube;

int low_res, wireframe;
float zoom,spin;
Vec2 joypos,pos,dir,center;
Vec3 rot;

Surface *texture;

Pen colours[] = {
	Pen(255,0,0),
	Pen(255,0,0),
	Pen(0,255,0),
	Pen(0,255,0),
	Pen(0,0,255),
	Pen(0,0,255),
	Pen(255,255,0),
	Pen(255,255,0),
	Pen(255,0,255),
	Pen(255,0,255),
	Pen(255,255,255),
	Pen(255,255,255)
};
Pen white = {255,255,255};
Pen black = {0,0,0};

Vec3 rotate3d (Vec3 point3d,Vec3 rot) {
static Vec2 point;

    // rotate 3d point about X
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

void draw_shape(shape shape,Vec2 pos,float size){
    int i=0;
    for (auto &face: shape.faces) {
        // draw each face / triangle
    	Vec3 p1 = shape.points[face.x];	    
    	Vec3 p2 = shape.points[face.y];	    
    	Vec3 p3 = shape.points[face.z];	    

        p1 = rotate3d(p1,rot);
        p2 = rotate3d(p2,rot);
        p3 = rotate3d(p3,rot);

	// calculate normal - face pointing direction.
	Vec3 v1 = p2 - p1;
	Vec3 v2 = p3 - p1;
	Vec3 normal = v1.cross(v2);

	// Only draw if facing camera (hidden surface removal)

	Vec3 camera = {0,0,10};
	if ( normal.dot(camera) > 0 ) {
            Point t1 = pos + to2d(p1) * size;
            Point t2 = pos + to2d(p2) * size;
            Point t3 = pos + to2d(p3) * size;

	    if (!wireframe) {
	       // filled
               screen.pen = colours[i++];
               screen.triangle(t1,t2,t3);
	       } else {
	       // vector
               screen.pen = white;
               screen.line(t1,t2);
               screen.line(t2,t3);
               screen.line(t3,t1);
	       }
	}
    }
}

void init() {
    set_screen_mode(ScreenMode::hires);
    center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);

    // cube points
    cube.points = {
	     { 10,  10,  10},
             {-10,  10,  10},
             {-10, -10,  10},
             { 10, -10,  10},
             { 10,  10, -10},
             {-10,  10, -10},
             {-10, -10, -10},
             { 10, -10, -10}
	       };
    // cube faces
    // (triangles)
    cube.faces = {
            {0, 1, 2},
            {0, 2, 3},
            {4, 0, 3},
            {4, 3, 7},
            {5, 4, 7},
            {5, 7, 6},
            {1, 5, 6},
            {1, 6, 2},
            {4, 5, 1},
            {4, 1, 0},
            {2, 6, 7},
            {2, 7, 3}
	    };

    low_res = 0;
    spin = 0.01f;
    zoom = 1;

    pos = center;
    dir = Vec2 (1,1);

    texture = Surface::load(crateimg);
}

void render(uint32_t time) {

    uint32_t ms_start = now();
    screen.pen = black;
    screen.clear();

    screen.pen = white;

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

    draw_shape (cube, pos, 5);

    rot += Vec3(spin,spin,0);

     // draw FPS meter
    uint32_t ms_end = now();
    uint32_t ms = ms_end - ms_start;
    std::string status = std::to_string(1000 / (ms>0 ? ms: 1)) + " fps    " + std::to_string(ms) + " ms";
    screen.text(status, minimal_font, Point(0, screen.bounds.h - 10));
}

void update(uint32_t time) {

    Vec2 move = blit::joystick;
    if (move.y > 0) zoom += 0.01f;
    if (move.y < 0) zoom -= 0.01f;
    pos.x += move.x;

    if (pressed(Button::DPAD_LEFT))  { spin -= 0.001f;}
    if (pressed(Button::DPAD_RIGHT)) { spin += 0.001f;}

    if (pressed(Button::X))  {
        low_res = 1 - low_res;
        if (low_res) set_screen_mode(ScreenMode::hires); 
	else set_screen_mode(ScreenMode::lores); 
        center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);
        pos = center;
        }
    if (pressed(Button::Y))  { wireframe = 1 - wireframe; }
}
