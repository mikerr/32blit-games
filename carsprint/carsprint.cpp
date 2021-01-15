#include "32blit.hpp"
#include "assets.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;

int lowres,player;
float maxspeed;
std::string status;

Surface *backdrop,*carsprite;

typedef std::vector<Vec2> shape;
shape innerarea, outerarea, waypoints;

typedef struct car {
	Vec2 pos;
	Vec2 dir;
	float angle;
	float speed;
	int waypt;
} car;

#define MAXCARS 4
car cars[MAXCARS];

void set_tracklimits() {
// polygon inside track - sandy area
	innerarea.push_back(Vec2(50,180));
	innerarea.push_back(Vec2(64,190));
	innerarea.push_back(Vec2(250,190));
	innerarea.push_back(Vec2(258,188));
	innerarea.push_back(Vec2(264,178));
	innerarea.push_back(Vec2(264,72));
	innerarea.push_back(Vec2(259,65));
	innerarea.push_back(Vec2(250,61));
	innerarea.push_back(Vec2(191,62));
	innerarea.push_back(Vec2(158,36));
	innerarea.push_back(Vec2(113,62));
	innerarea.push_back(Vec2(62,62));
	innerarea.push_back(Vec2(51,72));
	innerarea.push_back(Vec2(64,85));
	innerarea.push_back(Vec2(182,85));
	innerarea.push_back(Vec2(211,112));
	innerarea.push_back(Vec2(209,149));
	innerarea.push_back(Vec2(171,171));
	innerarea.push_back(Vec2(61,171));
	innerarea.push_back(Vec2(50,180));
	innerarea.push_back(Vec2(50,180));
	innerarea.push_back(Vec2(50,180));
	innerarea.push_back(Vec2(50,180));

// polygon surrounding track - sandy area
	outerarea.push_back(Vec2(61,222));
	outerarea.push_back(Vec2(257,222));
	outerarea.push_back(Vec2(280,210));
	outerarea.push_back(Vec2(290,188));
	outerarea.push_back(Vec2(290,66));
	outerarea.push_back(Vec2(281,46));
	outerarea.push_back(Vec2(257,31));
	outerarea.push_back(Vec2(200,31));
	outerarea.push_back(Vec2(174,8));
	outerarea.push_back(Vec2(157,5));
	outerarea.push_back(Vec2(133,12));
	outerarea.push_back(Vec2(106,31));
	outerarea.push_back(Vec2(48,34));
	outerarea.push_back(Vec2(19,62));
	outerarea.push_back(Vec2(22,88));
	outerarea.push_back(Vec2(52,114));
	outerarea.push_back(Vec2(169,116));
	outerarea.push_back(Vec2(181,128));
	outerarea.push_back(Vec2(169,139));
	outerarea.push_back(Vec2(51,139));
	outerarea.push_back(Vec2(23,164));
	outerarea.push_back(Vec2(23,200));
	outerarea.push_back(Vec2(35,214));
	outerarea.push_back(Vec2(61,222));
}

void set_waypoints(){
int i=1;
	for (auto &inner: innerarea) {
		Vec2 outer = outerarea[i++];
		Vec2 dest;
		// get points half way between inner and outer track 
		// i.e. middle of track to guide AI
		dest.x = inner.x + ( (outer.x - inner.x ) / 2);
		dest.y = inner.y + ( (outer.y - inner.y ) / 2);
		waypoints.push_back(dest);
	}
}

float ang_to_point ( Vec2 point, Vec2 dest ) {
// return angle of vector between points

	Vec2 diff = dest - point;
	float angle = atan2( diff.y , diff.x );
	return (angle);
}

bool point_inside_shape ( Vec2 point, shape shape ){
// http://erich.realtimerendering.com/ptinpoly/ for an explanation !

  int i, j, nvert = shape.size();
  bool c = false;

  for(i = 0, j = nvert - 1; i < nvert; j = i++) {
    if( ( (shape[i].y >= point.y ) != (shape[j].y >= point.y) ) &&
        (point.x <= (shape[j].x - shape[i].x) * (point.y - shape[i].y) / (shape[j].y - shape[i].y) + shape[i].x)
      )
      c = !c;
  }

  return c;
}

bool near(Vec2 point,Vec2 target){
	return ( (abs(point.x - target.x) < 10 ) && (abs(point.y - target.y) < 10));
}

void init() {
    set_screen_mode(ScreenMode::hires);

    backdrop = Surface::load(trackimg);
    carsprite = Surface::load(carimg);

    maxspeed = 2;
    set_tracklimits();
    set_waypoints();

    for (int i=0;i<MAXCARS;i++) 
    	cars[i].pos = Vec2( screen.bounds.w / 5, i * 5 + screen.bounds.h - 40 );

}

void render(uint32_t time) {
int size;
static uint32_t start = time;
static int lastlap,bestlap,clock;

    Pen colours[] = {Pen(0,255,0),Pen(0,255,255),Pen(255,128,0),Pen(255,255,0)};

    // Draw track
    screen.stretch_blit(backdrop,Rect(0,0,320,240),Rect(0,0,screen.bounds.w,screen.bounds.h));

    // scale car if res changed
    size = screen.bounds.w / 64;

    // Draw car
    //screen.stretch_blit(carsprite,Rect(0,0,64,45),Rect(pos.x, pos.y, size, size / 1.5));

    for (int i=0;i<MAXCARS;i++) {
    	screen.pen = colours[i];
    	Vec2 rotatedline = cars[i].dir * size;
    	screen.line(cars[i].pos - rotatedline, cars[i].pos + rotatedline);
	}

    clock = (time - start) / 100;
    status = "Lap time: " + std::to_string(clock);
    screen.pen = Pen(255,255,255);
    screen.text(status, minimal_font, Vec2(0,screen.bounds.h - 10));

    status = "Last: " + std::to_string(lastlap);
    screen.text(status, minimal_font, Vec2(screen.bounds.w / 2, screen.bounds.h - 10));

    status = "Best: " + std::to_string(bestlap);
    screen.text(status, minimal_font, Vec2(screen.bounds.w - 50, screen.bounds.h - 10));

    Vec2 FinishLine = Vec2(75,205);
    if (near (cars[player].pos,FinishLine)) {
	    if (clock > 100) lastlap = clock;
	    if (lastlap < bestlap || bestlap == 0) bestlap = lastlap;
	    start = time;
    	    }
}
	
void update(uint32_t time) {

    if (pressed(Button::DPAD_LEFT)  || joystick.x < -0.2) cars[player].angle -= 0.05;
    if (pressed(Button::DPAD_RIGHT) || joystick.x > 0.2)  cars[player].angle += 0.05; 

    if (pressed(Button::A) && cars[player].speed < maxspeed ) cars[player].speed += 0.05;

    if (pressed(Button::X)) {
	    if (lowres) set_screen_mode(ScreenMode::lores);
	    else set_screen_mode(ScreenMode::hires);
	    lowres = !lowres;
	    }

    for (int i=0;i<MAXCARS;i++) {
	    float angle = cars[i].angle;
            cars[i].dir = Vec2( cos(angle), sin(angle));
            cars[i].pos += cars[i].dir * cars[i].speed;

	    if (i != player) { // AI
		    cars[i].speed = i / 2.5; 
		    unsigned int w = cars[i].waypt;
		    cars[i].angle = ang_to_point( cars[i].pos, waypoints[w]);
		    if (near(cars[i].pos,waypoints[w])) {
			    w++;
			    if (w == waypoints.size()) w = 1;
			    cars[i].waypt = w;
		    }
	    }
            if (point_inside_shape(cars[i].pos,innerarea) || !point_inside_shape(cars[i].pos,outerarea)) 
		    cars[i].speed = 0.2;
	    }
    cars[player].speed *= 0.98;
}