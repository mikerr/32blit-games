#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

typedef struct shape {
    std::vector<Vec3> points ;
} shape;

shape cube,texcube;

int low_res, wireframe;
float zoom,spin;
Vec2 joypos,pos,dir,center;
Vec3 rot;

Surface *texture;

int zbuffer[320][240];

Vec3 rotate3d (Vec3 point3d,Vec3 rot) {
static Point point;

    // rotate 3d point about X
    point = Point(point3d.y,point3d.z);
    point *= Mat3::rotation(rot.x);

    point3d.y = point.x;
    point3d.z = point.y;

    // rotate 3d point about Y
    point = Point(point3d.z,point3d.x);
    point *= Mat3::rotation(rot.y);

    point3d.z = point.x;
    point3d.x = point.y;

    // rotate 3d point about Z
    point = Point(point3d.x,point3d.y);
    point *= Mat3::rotation(rot.z);

    point3d.x = point.x;
    point3d.y = point.y;

    return (point3d);
}


Point to2d (Vec3 point3d) {
    Point point;

    // project to screen
    int z = point3d.z - 500;
    point.x = point3d.x * 500 / z;
    point.y = point3d.y  * 500 / z;

    return (point);
    }

void draw_shape(shape shape,Vec2 pos,float size){
    Vec3 p0 = shape.points[0];
    p0 = rotate3d(p0,rot);

    Point lastpos = to2d(p0) * size;

    for (auto &p: shape.points) {
        p = rotate3d(p,rot);
        Point point = to2d(p) * size;
        screen.line(lastpos + pos , point + pos);
        lastpos = point;
    }
}

bool check_zbuffer(Point point,int z) {

   if (point.x < 0 || point.y < 0) return(false);
   if (point.x >= screen.bounds.w || point.y >= screen.bounds.h) return(false);

   int zbuf = zbuffer[point.x][point.y];
   if (zbuf == 0) zbuffer[point.x][point.y] = z;
   if (zbuf != 0 && zbuf >= z) return(false);

   return (true);
}

void texmap (Vec3 p, int x, int y){
           p = rotate3d(p,rot);
           Point point = to2d(p) + pos;
	   if (check_zbuffer(point,p.z))
           	screen.blit(texture,Rect(x,y,1,1),point);
}

void draw_texture(shape shape,Vec2 pos,float size){
    for (int x=0;x < 64; x++)
        for (int y=0;y < 64; y++) {
           Vec3 p = Vec3(x,y,32);
	   texmap(p,x,y);
	}
    for (int x=0;x < 64; x++)
        for (int y=0;y < 64; y++) {
           Vec3 p = Vec3(x,y,-32);
	   texmap(p,x,y);
	}

    for (int x=0;x < 64; x++)
        for (int y=0;y < 64; y++) {
           Vec3 p = Vec3(0,x,y-32);
	   texmap(p,x,y);
	}
    for (int x=0;x < 64; x++)
        for (int y=0;y < 64; y++) {
           Vec3 p = Vec3(64,x,y-32);
	   texmap(p,x,y);
	}

    for (int x=0;x < 64; x++)
        for (int y=0;y < 64; y++) {
           Vec3 p = Vec3(x,0,y-32);
	   texmap(p,x,y);
	}
    for (int x=0;x < 64; x++)
        for (int y=0;y < 64; y++) {
           Vec3 p = Vec3(x,64,y-32);
	   texmap(p,x,y);
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
    zoom = 1;

    pos = center;
    dir = Vec2 (1,1);

    texture = Surface::load(crateimg);
}

void render(uint32_t time) {

    uint32_t ms_start = now();
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

    if (wireframe)
        draw_shape (cube, pos, 2);
    else 
    	draw_texture (texcube, pos, 1);

    rot += Vec3(spin,spin,0);

    // clear zbuffer
    for (int i = 0;i < 320; i++) 
	    for (int j =0;j<240;j++)
		    zbuffer[i][j] = 0;

     // draw FPS meter
    uint32_t ms_end = now();
    uint32_t ms = ms_end - ms_start;
    std::string status = std::to_string(1000 / ms) + " fps    " + std::to_string(ms) + " ms";
    screen.text(status, minimal_font, Point(0, screen.bounds.h - 10));
}

void update(uint32_t time) {

    Vec2 move = blit::joystick;
    if (move.y > 0) zoom += 0.01;
    if (move.y < 0) zoom -= 0.01;
    pos.x += move.x;

    if (pressed(Button::DPAD_LEFT))  { spin -= 0.001;}
    if (pressed(Button::DPAD_RIGHT)) { spin += 0.001;}

    if (pressed(Button::X))  {
        low_res = 1 - low_res;
        if (low_res) set_screen_mode(ScreenMode::hires); 
	else set_screen_mode(ScreenMode::lores); 
        center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);
        pos = center;
        }
    if (pressed(Button::Y))  { wireframe = 1 - wireframe; }
}
