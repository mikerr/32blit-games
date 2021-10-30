#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

int lowres,pic,dir;
float angle,speed,zoom;
bool demo,pan;

Surface *pictures[2];
Surface *bricks;


typedef std::vector<Point> pointlist;

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

void texline (Surface *texture, Point p1,Point p2,int y) {
    float x = 0;
    pointlist allthepoints = getlinepoints (p1,p2);
    // scale non-repeating texture to face width
    float step = (float) texture->bounds.w / allthepoints.size(); 
    for (auto p:allthepoints) {
        if (p.x > 0 && p.y > 0 && p.x < screen.bounds.w && p.y < screen.bounds.h )
		screen.blit(texture,Rect(x,y,2,2),p);
	x += step;
    }
}
void texquad(Surface *texture,Point t1,Point t2,Point t3,Point t4) {
		   float y = 0;
	           pointlist sideApoints = getlinepoints (t2,t1);
	           pointlist sideBpoints = getlinepoints (t3,t4);
		   int sblen = sideBpoints.size();
		   float step = (float) texture->bounds.w / sblen;
		   int sb = 0;
		   for (auto p:sideApoints){
			Point p1 = sideBpoints[sb];
			texline(texture,p,p1,y);
			if (++sb >= sblen) break;
			y += step;
		   }
}

void blit_rotate_sprite (Surface *sprite,Rect src, float angle, int blur, Vec2 screenpos) {

Vec2 rot;
    int width = src.w;
    int height = src.h;

    float sinangle = sin(angle);
    float cosangle = cos(angle);
    pointlist corners;
    for (int x=0;x<width;x+=width-1)
	    for (int y=0;y<height;y+=height-1) {
		    int x1 = x - (width / 2);
		    int y1 = y - (height / 2);
		    rot.x = x1 * sinangle + y1 * cosangle;
		    rot.y = y1 * sinangle - x1 * cosangle;
		
		    Vec2 pos = (rot * zoom ) + screenpos;
		    corners.push_back(pos);
    }
    texquad(sprite, corners[0],corners[1],corners[3],corners[2]);
}
void blit_rotate_sprite_old (Surface *sprite,Rect src, float angle, int blur, Vec2 screenpos) {
//rotate a sprite to screen at any angle
// simple code, but heavy on floating point
Vec2 rot;
    int width = src.w;
    int height = src.h;

    float sinangle = sin(angle);
    float cosangle = cos(angle);
    for (int x=0;x<width;x++)
	    for (int y=0;y<height;y++) {
		    int x1 = x - (width / 2);
		    int y1 = y - (height / 2);
		    rot.x = x1 * sinangle + y1 * cosangle;
		    rot.y = y1 * sinangle - x1 * cosangle;
		
		    Vec2 pos = (rot * zoom ) + screenpos;
        	    screen.stretch_blit(sprite,Rect(src.x + x, src.y + y,1,1),Rect(pos.x, pos.y,blur,blur));
	    }
}
void tilescreen(Surface *tile,Point pos){
    int width = tile->bounds.w; 
    int height = tile->bounds.h; 

    pos.x = pos.x % width;
    pos.y = pos.y % height;
    for (int x = pos.x - width;x<screen.bounds.w;x+= width)
        for (int y= pos.y - height;y<screen.bounds.h;y+= height)
             screen.blit(tile,Rect(0,0,width,height),Point(x,y));
}
void init() {
    set_screen_mode(ScreenMode::hires);
    pictures[0] = Surface::load(baboonimg);

    if (screen.bounds.w >= 320) {
	  //on 32blit-sonsole, not reduced RAM picosyation 
    	pictures[1] = Surface::load(lenaimg);
    	bricks = Surface::load(bricksimg);
    } else {
	    //PICO
    	    set_screen_mode(ScreenMode::lores);
    }

    speed = 0.01;
    zoom = 1;
    dir = 1;
    demo = true;
}

void render(uint32_t time) {
std::string status;
static int x,y=0;
    Vec2 center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);

    if (pan) center.x += speed * 100;
    else angle += speed;

    uint32_t ms_start = now();

    // Clear screen
    screen.pen = Pen (0,0,0);
    screen.clear();

    if (speed >0) x += 3;
    else x -= 3;

    if (screen.bounds.w >= 320) // on 32blit-console
    	tilescreen(bricks,Point(x,y));

    // Draw bitmap
    int blur = 1 + zoom;
    blit_rotate_sprite(pictures[pic],Rect(0,0,256,256),angle,blur,center);

    // draw FPS meter
    uint32_t ms = now() - ms_start;
    if (ms == 0) ms = 1;
    status = std::to_string(1000 / ms) + " fps    " + std::to_string(ms) + " ms";
    screen.pen = Pen(255,255,255);
    screen.text(status, minimal_font, Vec2(0, screen.bounds.h - 10));
}
	
void update(uint32_t time) {

    if (pressed(Button::DPAD_LEFT)  || joystick.x < -0.2f) speed -= 0.005f;
    if (pressed(Button::DPAD_RIGHT) || joystick.x > 0.2f)  speed += 0.005f;

    if (abs(speed) > 0.1) speed = speed / 2;
    if (pressed(Button::DPAD_UP)  || joystick.y < -0.2f) zoom += 0.01f;
    if (pressed(Button::DPAD_DOWN) || joystick.y > 0.2f) zoom -= 0.01f;

    if (pressed(Button::A)) speed = 0;
    if (buttons.released & Button::Y) {
		    pan = !pan;
		    speed = 0;
    }

    if (buttons.released & Button::X) {
	    	lowres = !lowres;
	    	if (lowres) set_screen_mode(ScreenMode::lores);
	    	else set_screen_mode(ScreenMode::hires);
	    }

    if (buttons.released) demo = false;
    if (demo) {
	    zoom += 0.005f * dir;
	    if (zoom > 1.0f || zoom < 0.1f) dir *= -1 ;
    	    if (screen.bounds.w >= 320) {
	   	 if (zoom < 0.1f) pic = 1 - pic;
	    }
    }
}
