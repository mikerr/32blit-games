#include "32blit.hpp"
#include "assets.hpp"
#include "graphics/color.hpp"
#include "types/vec2.hpp"

using namespace blit;
int lowres;

std::string status;

Surface *backdrop,*carsprite;

typedef struct shape {
	 std::vector<Vec2> points ;
} shape;

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

float maxspeed;
int player;

void settracklimits() {
// polygon inside track - sandy area
	innerarea.points.push_back(Vec2(50,180));
	innerarea.points.push_back(Vec2(64,190));
	innerarea.points.push_back(Vec2(250,190));
	innerarea.points.push_back(Vec2(258,188));
	innerarea.points.push_back(Vec2(264,178));
	innerarea.points.push_back(Vec2(264,72));
	innerarea.points.push_back(Vec2(259,65));
	innerarea.points.push_back(Vec2(250,61));
	innerarea.points.push_back(Vec2(191,62));
	innerarea.points.push_back(Vec2(158,36));
	innerarea.points.push_back(Vec2(113,62));
	innerarea.points.push_back(Vec2(62,62));
	innerarea.points.push_back(Vec2(51,72));
	innerarea.points.push_back(Vec2(64,85));
	innerarea.points.push_back(Vec2(182,85));
	innerarea.points.push_back(Vec2(211,112));
	innerarea.points.push_back(Vec2(209,149));
	innerarea.points.push_back(Vec2(171,171));
	innerarea.points.push_back(Vec2(61,171));
	innerarea.points.push_back(Vec2(50,180));
	innerarea.points.push_back(Vec2(50,180));
	innerarea.points.push_back(Vec2(50,180));
	innerarea.points.push_back(Vec2(50,180));

// polygon surrounding track - sandy area
	outerarea.points.push_back(Vec2(61,222));
	outerarea.points.push_back(Vec2(257,222));
	outerarea.points.push_back(Vec2(280,210));
	outerarea.points.push_back(Vec2(290,188));
	outerarea.points.push_back(Vec2(290,66));
	outerarea.points.push_back(Vec2(281,46));
	outerarea.points.push_back(Vec2(257,31));
	outerarea.points.push_back(Vec2(200,31));
	outerarea.points.push_back(Vec2(174,8));
	outerarea.points.push_back(Vec2(157,5));
	outerarea.points.push_back(Vec2(133,12));
	outerarea.points.push_back(Vec2(106,31));
	outerarea.points.push_back(Vec2(48,34));
	outerarea.points.push_back(Vec2(19,62));
	outerarea.points.push_back(Vec2(22,88));
	outerarea.points.push_back(Vec2(52,114));
	outerarea.points.push_back(Vec2(169,116));
	outerarea.points.push_back(Vec2(181,128));
	outerarea.points.push_back(Vec2(169,139));
	outerarea.points.push_back(Vec2(51,139));
	outerarea.points.push_back(Vec2(23,164));
	outerarea.points.push_back(Vec2(23,200));
	outerarea.points.push_back(Vec2(35,214));
	outerarea.points.push_back(Vec2(61,222));
}

void set_waypoints(){
int i=1;
	for (auto &inner: innerarea.points) {
		Vec2 outer = outerarea.points[i++];
		Vec2 dest;
		// get point half way between inner and outer track - i.e. middle of track
		dest.x = inner.x + ( (outer.x - inner.x ) / 2);
		dest.y = inner.y + ( (outer.y - inner.y ) / 2);
		waypoints.points.push_back(dest);
	}
}

float ang_to_point ( Vec2 point, Vec2 dest ) {
// return angle of vector between points

	Vec2 diff = dest - point;
	float angle = atan2( diff.y , diff.x );
	return (angle);
}

bool point_inside_shape ( Vec2 point, shape shape ) {
// http://erich.realtimerendering.com/ptinpoly/ for an explanation !

  int i, j, nvert = shape.points.size();
  bool c = false;

  for(i = 0, j = nvert - 1; i < nvert; j = i++) {
    if( ( (shape.points[i].y >= point.y ) != (shape.points[j].y >= point.y) ) &&
        (point.x <= (shape.points[j].x - shape.points[i].x) * (point.y - shape.points[i].y) / (shape.points[j].y - shape.points[i].y) + shape.points[i].x)
      )
      c = !c;
  }

  return c;
}

bool near (Vec2 point,Vec2 target){
	int hit = 0;
	if ( (abs(point.x - target.x) < 10 ) && (abs(point.y - target.y) < 10)) hit = 1;
	return (hit);
}

void init() {
    set_screen_mode(ScreenMode::hires);

    backdrop = Surface::load(trackimg);
    carsprite = Surface::load(carimg);

    settracklimits();
    set_waypoints();

    for (int i=0;i<MAXCARS;i++) {
    	cars[i].pos = Vec2( screen.bounds.w / 5, i * 5 + screen.bounds.h - 40 );
    	cars[i].speed = maxspeed = 2;
	cars[i].waypt = 1;
	}

}

void render(uint32_t time) {
int size;
static uint32_t start = time;
static int lastlap,bestlap,clock;

    Pen colours[] = {Pen(0,255,0),Pen(0,255,255),Pen(0,0,255),Pen(255,255,0)};

    // Draw track
    screen.stretch_blit(backdrop,Rect(0,0,320,240),Rect(0,0,screen.bounds.w,screen.bounds.h));

    // scale car if res changed
    size = screen.bounds.w / 64;

    // Draw car
    //screen.stretch_blit(carsprite,Rect(0,0,64,45),Rect(pos.x, pos.y, size, size / 1.5));

    for (int i=0;i<MAXCARS;i++) {
    	screen.pen = colours[i];
    	screen.line(cars[i].pos, cars[i].pos + (cars[i].dir * size));
    	screen.line(cars[i].pos, cars[i].pos - (cars[i].dir * size));
	}

    clock = (time - start) / 100;
    status = "Lap time: " + std::to_string(clock);
    screen.pen = Pen(255,255,255);
    screen.text(status, minimal_font, Vec2(0,screen.bounds.h - 10));

    status = "Last: " + std::to_string(lastlap);
    screen.text(status, minimal_font, Vec2(screen.bounds.w / 2, screen.bounds.h - 10));

    status = "Best: " + std::to_string(bestlap);
    screen.text(status, minimal_font, Vec2(screen.bounds.w - 50, screen.bounds.h - 10));

    if (near (cars[player].pos,Vec2(75,205)) ) {
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
		    cars[i].speed = i /2.5; 
		    int w = cars[i].waypt;
		    cars[i].angle = ang_to_point( cars[i].pos, waypoints.points[w]);
		    if (near(cars[i].pos,waypoints.points[w])) {
			    w++;
			    if (w == waypoints.points.size()) w = 1;
			    cars[i].waypt = w;
		    }
	    }


            if (point_inside_shape(cars[i].pos,innerarea) || !point_inside_shape(cars[i].pos,outerarea)) {
		cars[i].speed = 0.2;
	    	if (i != player) { cars[i].angle -= 0.05; }
	        }
	    }
    cars[player].speed *= 0.98;
}
