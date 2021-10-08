#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

typedef struct shape {
    std::vector<Vec3> points;
    std::vector<Vec3> faces;
} shape;

shape cube;

typedef std::vector<Point> pointlist;

bool low_res,showall,demo=1;
int filled, tex,maxtex,pixelsize;
float texWidth;
float zoom,spin;
Vec2 pos,dir,center;
Vec3 rot;

Surface *texture[5];

Pen black = {0,0,0};
Pen blue = {0,0,255};
Pen red = {255,0,0}; 
Pen magenta = { 255,0,255};
Pen green = {0,255,0};
Pen cyan = { 0,255,255};
Pen yellow = { 255,255,0};
Pen white = {255,255,255};

Pen colours[] = { // face colours - 2 triangles per cube side
	red, red, blue, blue, green, green, yellow,yellow, magenta, magenta, cyan, cyan
};

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

pointlist getlinepoints (Point p1,Point p2) {
int linelen=0;
    int32_t dx = int32_t(abs(p2.x - p1.x));
    int32_t dy = -int32_t(abs(p2.y - p1.y));

    int32_t sx = (p1.x < p2.x) ? 1 : -1;
    int32_t sy = (p1.y < p2.y) ? 1 : -1;

    int32_t err = dx + dy;

    pointlist linepoints;
    Point p(p1);
    while (linelen++ < 999){
	// dont draw off screen
        linepoints.push_back(p);
        if ((p.x == p2.x) && (p.y == p2.y)) break;

        int32_t e2 = err * 2;
        if (e2 >= dy) { err += dy; p.x += sx; }
        if (e2 <= dx) { err += dx; p.y += sy; }
    }
    return linepoints;
}

void texline ( Point p1,Point p2,int y) {
    float x = 0;
    pointlist allthepoints = getlinepoints (p1,p2);
    // scale non-repeating texture to face width
    float step = texWidth / allthepoints.size(); 
    for (auto p:allthepoints) {
        screen.blit(texture[tex],Rect(x,y,pixelsize,pixelsize),p);
	x += step;
    }
}
void textri (Point t1,Point t2, Point t3) {
		int y = 0;
	        pointlist allthepoints = getlinepoints (t2,t3);
		for (auto p:allthepoints){
			texline(p,t1,y);
			if (++y > texWidth) y = 0;
		}
}
void texquad(Point t1,Point t2,Point t3,Point t4) {
		   float y = 0;
	           pointlist sideApoints = getlinepoints (t2,t1);
	           pointlist sideBpoints = getlinepoints (t3,t4);
		   int sblen = sideBpoints.size();
		   float step = texWidth / sblen;
		   int sb = 0;
		   for (auto p:sideApoints){
			Point p1 = sideBpoints[sb];
			texline(p,p1,y);
			if (++sb >= sblen) break;
			y += step;
		   }
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
	int angle = normal.dot(camera);
	if ( angle > 0 ) {
            Point t1 = pos + to2d(p1) * size;
            Point t2 = pos + to2d(p2) * size;
            Point t3 = pos + to2d(p3) * size;

	    if (filled == 0) {
	       // vector lines
               screen.pen = white;
               screen.line(t1,t2);
               screen.line(t2,t3);
               screen.line(t3,t1);
	       } 
	    if (filled == 1) {
	       // filled with solid colour
               screen.pen = colours[i];
               screen.triangle(t1,t2,t3);
	       }
	    if (filled == 2) {
	       // shaded by angle from light at front/top
	       Vec3 light = {0,0,10};
	       int brightness = normal.dot(light) / 25;
               screen.pen = Pen(brightness,brightness,brightness);
               screen.triangle(t1,t2,t3);
	       }
	    if (filled == 3) {
		// texture filled
	       static Point oldt2;

	       if (i % 2) {
		       if (showall) {
		       		tex = i/4;
		       		if (tex > maxtex) tex = maxtex;
		       }
    	       	       texWidth = texture[tex]->bounds.w;
		       texquad(t2,oldt2,t1,t3);
	       }
	       else oldt2 = t2;
	       }
	    }
	i++;
    }
}


void draw_fps(uint32_t ms_start,uint32_t ms_end){
     // draw FPS meter
    uint32_t ms = ms_end - ms_start;
    screen.pen = white;
    std::string status;
    status = std::to_string(1000 / (ms >  0 ? ms : 1)) + " fps";
    screen.text(status, minimal_font, Point(0, screen.bounds.h - 10));
    status = std::to_string(ms) + " ms";
    screen.text(status, minimal_font, Point(100, screen.bounds.h - 10));
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

    spin = 0.01f;
    zoom = 5;
    pixelsize = 2;

    pos = center;
    dir = Vec2 (1,1);

    texture[tex] = Surface::load(bricksimg);
    //texture[++tex] = Surface::load(stonesimg);
    texture[++tex] = Surface::load(baboonimg);
    texture[++tex] = Surface::load(crateimg);
    //texture[++tex] = Surface::load(lenaimg);
    maxtex =2;
}

void render(uint32_t time) {

    uint32_t start = now();
    screen.pen = black;
    screen.clear();

    pos += dir;
    rot += Vec3(spin,spin,spin);

    draw_shape (cube, pos, zoom);

    draw_fps(start,now());
}

void update(uint32_t time) {
static int frames;
static int delay = 200;

    pos += joystick;

    if (buttons.released) demo = 0;
    if (pressed(DPAD_UP)) zoom += 0.1f;
    if (pressed(DPAD_DOWN)) zoom -= 0.1f;
    if (pressed(DPAD_LEFT))  { spin -= 0.001f;}
    if (pressed(DPAD_RIGHT)) { spin += 0.001f;}

    if (buttons.released & Button::X)  {
        low_res = !low_res;
        if (low_res) set_screen_mode(ScreenMode::hires); 
	else set_screen_mode(ScreenMode::lores); 
        center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);
        pos = center;
        }
    if (buttons.released & Button::Y)  { filled++; if (filled >= 4) filled = 0;}
    if (buttons.released & Button::B)  { showall = 0; tex++; if (tex > maxtex) tex = 0;}
    if (buttons.released & Button::A)  { showall = !showall;} 

    if (buttons.released & Button::JOYSTICK)  { pixelsize = 3 - pixelsize;} 

    Vec2 edge = center * 2;
    int cubesize = zoom * 10;
    if ((pos.x < cubesize ) || (pos.x > edge.x - cubesize ))  dir.x *= -1;
    if ((pos.y < cubesize ) || (pos.y > edge.y - cubesize ))  dir.y *= -1;

    if (demo && frames % delay == 0)  { 
	    delay *= 2;
	    filled++; 
	    if (filled >= 4) {
		    filled = 0;
		    delay = 200;
	    }
    }
    frames++;

}
