#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

bool showpos;
int player,offset=4;
float maxspeed;
std::string status;

Surface *backdrop,*carsprites;

typedef std::vector<Vec2> shape;
shape innerarea, outerarea, waypoints;

typedef struct car {
	Vec2 pos;
	Vec2 dir;
	float angle;
	float speed;
	int waypt;
	int distance;
	int rank;
} car;

#define MAXCARS 4
car cars[MAXCARS];

Rect cone = Rect ( 0,16,16,40);

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
// check a given point is enclosed by a polygon
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

bool near(Vec2 point,Vec2 target, int howfar){
	return ( (abs(point.x - target.x) < howfar ) && (abs(point.y - target.y) < howfar));
}

void blit_rotate_sprite (Surface *sprite,Rect src, float angle, Vec2 screenpos) {
//rotate a sprite to screen at any angle
Vec2 rot;
    int width = src.w;
    int height = src.h;
    for (int x=0;x<width;x++)
	    for (int y=0;y<height;y++) {
		    int x1 = x - (width / 2);
		    int y1 = y - (height / 2);
		    rot.x = x1 * sin(angle) + y1 * cos(angle);
		    rot.y = y1 * sin(angle) - x1 * cos(angle);
		    Vec2 pos = rot + screenpos;
        	    screen.stretch_blit(sprite,Rect(src.x + x, src.y + y,1,1),Rect(pos.x, pos.y,2,2));
	    }
}

void init() {
    set_screen_mode(ScreenMode::hires);

    backdrop = Surface::load(trackimg);
    carsprites = Surface::load(carimg);

    carsprites->palette[0] = Pen(0,0,0,0); //transparent

    maxspeed = 2;
    set_tracklimits();
    set_waypoints();

    // car starting positions on grid
    for (int i=0;i<MAXCARS;i++) 
    	cars[i].pos = Vec2( screen.bounds.w / 3, i * 5 + screen.bounds.h - 40 );
}

void render(uint32_t time) {
static uint32_t start = time;
static int lastlap,bestlap,clock;

    // Draw track
    screen.stretch_blit(backdrop,Rect(0,0,320,240),Rect(0,0,screen.bounds.w,screen.bounds.h));

    // Draw cars
    for (int i=0;i<MAXCARS;i++) {
	Vec2 pos = cars[i].pos;
    	blit_rotate_sprite(carsprites,Rect(8*(i+offset),0,8,16),cars[i].angle, pos);
	// show curent rank
    	blit_rotate_sprite(carsprites,Rect(8*(i+offset),0,8,16),0, Vec2(cars[i].rank * 20,10));
    	if (cars[player].speed < 0.2) screen.text(std::to_string(cars[i].rank), minimal_font, pos );
	}

    // Draw score
    screen.pen = Pen(255,255,255);
    clock = (time - start) / 100;
    status = "Lap time: " + std::to_string(clock);
    screen.text(status, minimal_font, Vec2(0, screen.bounds.h - 10));
    status = "Last: " + std::to_string(lastlap);
    screen.text(status, minimal_font, Vec2(screen.bounds.w / 2, screen.bounds.h - 10));
    status = "Best: " + std::to_string(bestlap);
    screen.text(status, minimal_font, Vec2(screen.bounds.w - 50, screen.bounds.h - 10));

    // Manage laps
    Vec2 FinishLine = Vec2(75,205);
    if (near (cars[player].pos,FinishLine,20)) {
	    if (clock > 50) lastlap = clock;
	    if (lastlap < bestlap || bestlap == 0) bestlap = lastlap;
	    start = time;
    	    }
}
	
void update(uint32_t time) {

    if (pressed(Button::DPAD_LEFT)  || joystick.x < -0.2) cars[player].angle -= 0.05;
    if (pressed(Button::DPAD_RIGHT) || joystick.x > 0.2)  cars[player].angle += 0.05;

    if (pressed(Button::A) && cars[player].speed < maxspeed ) cars[player].speed += 0.05;

    showpos = false;
    if (pressed(Button::MENU)) { showpos = !showpos; }

    if (pressed(Button::Y)) { 
	    static uint32_t lasttime;
	    if (now() - lasttime > 100) {
		    offset++;
		    if (offset > 13) offset = 0;
	        }
	    lasttime = now();
    }

    // slow car if off track 
    if (point_inside_shape(cars[player].pos,innerarea) || !point_inside_shape(cars[player].pos,outerarea)) 
		    cars[player].speed = 0.2;

    // Move all cars (including player's car)
    for (int i=0;i<MAXCARS;i++) {
	    float angle = cars[i].angle;
            cars[i].dir = Vec2( cos(angle), sin(angle));
            cars[i].pos += cars[i].dir * cars[i].speed;

	    int w = cars[i].waypt;
	    int dist = 50;
	    if (i != player) { // AI follows waypoints
		    // point car at checkpoint
		    float angle = ang_to_point( cars[i].pos, waypoints[w]);
	            cars[i].angle = angle;
		    cars[i].speed = 0.6 + i * 0.1;
		    dist = 10;
		            }

	    if (near(cars[i].pos,waypoints[w],dist)) {
			    //move target to next checkpoint if near current one
			    w++;
			    if (w == (int) waypoints.size()) w = 1;
			    cars[i].waypt = w;
			    cars[i].distance++;
	    	    	    }
    	   //Get car ranking by distance travelled
	   int rank=1;
    	   for (int j=0;j<MAXCARS;j++) {
	    	if (cars[i].distance < cars[j].distance) rank++;
	   }
	   cars[i].rank = rank;
    }
    // player car gradually slows down 
    cars[player].speed *= 0.98;
}
