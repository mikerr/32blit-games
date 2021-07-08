#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

int player,offset=4;
std::string status;

Surface *backdrop,*carsprites;

typedef std::vector<Vec2> shape;
shape innerarea, outerarea, waypoints;

typedef struct car {
	Rect sprite;
	Vec2 pos;
	Vec2 dir;
	float angle;
	float speed;
	int waypt;
	int distance;
	int rank;
} car;

car cars[4];

Rect cone = Rect ( 0,16,16,40);
Vec2 FinishLine = Vec2(75,205);

void set_tracklimits() {
// polygon inside track - sandy area
// 
	innerarea = { Vec2(50,180), Vec2(64,190), Vec2(250,190), Vec2(258,188), Vec2(264,178), 
		      Vec2(264,72), Vec2(259,65), Vec2(250,61), Vec2(191,62), Vec2(158,36), 
		      Vec2(113,62), Vec2(62,62), Vec2(51,72), Vec2(64,85), Vec2(182,85), 
		      Vec2(211,112), Vec2(209,149), Vec2(171,171), Vec2(61,171), Vec2(50,180), 
		      Vec2(50,180), Vec2(50,180), Vec2(50,180)};

// polygon surrounding track - sandy area
	outerarea = { Vec2(61,222), Vec2(257,222), Vec2(280,210), Vec2(290,188), Vec2(290,66),
		      Vec2(281,46), Vec2(257,31), Vec2(200,31), Vec2(174,8), Vec2(157,5),
		      Vec2(133,12), Vec2(106,31), Vec2(48,34), Vec2(19,62), Vec2(22,88),
		      Vec2(52,114), Vec2(169,116), Vec2(181,128), Vec2(169,139), Vec2(51,139),
		      Vec2(23,164), Vec2(23,200), Vec2(35,214), Vec2(61,222)};
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

    set_tracklimits();
    set_waypoints();

    // car starting positions on grid
    int spritenum = offset;
    //int speed = 0;
    for (auto &car: cars) {
    	car.pos = FinishLine;
    	car.sprite = Rect(spritenum*8,0,8,16);
	car.speed = 0.55f + ( rand() % 5) * 0.1f;
	spritenum++;
    }
}

void render(uint32_t time) {
static uint32_t start = time;
static int lastlap,bestlap,clock;

    // Draw track
    screen.stretch_blit(backdrop,Rect(0,0,320,240),Rect(0,0,screen.bounds.w,screen.bounds.h));

    // Draw cars
    for (auto &car: cars){
    	blit_rotate_sprite(carsprites,car.sprite,car.angle, car.pos);
	// show curent rank at top of screen
    	blit_rotate_sprite(carsprites,car.sprite,0, Vec2(car.rank * 20,10));
    	if (cars[player].speed < 0.2f) screen.text(std::to_string(car.rank), minimal_font, car.pos );
	}

    // Draw score
    screen.pen = Pen(255,255,255);
    clock = (time - start) / 100;
    status = "1st 2nd 3rd 4th";
    screen.text(status, minimal_font, Vec2(13,0));
    status = "Position: " + std::to_string(cars[0].rank);
    screen.text(status, minimal_font, Vec2(screen.bounds.w-60,0));
    status = "Lap : 1 / 3";
    screen.text(status, minimal_font, Vec2(screen.bounds.w-60,10));
    status = "Lap time: " + std::to_string(clock);
    screen.text(status, minimal_font, Vec2(0, screen.bounds.h - 10));
    status = "Last: " + std::to_string(lastlap);
    screen.text(status, minimal_font, Vec2(screen.bounds.w / 2, screen.bounds.h - 10));
    status = "Best: " + std::to_string(bestlap);
    screen.text(status, minimal_font, Vec2(screen.bounds.w - 50, screen.bounds.h - 10));

    // Manage laps
    if (near (cars[player].pos,FinishLine,20)) {
	    if (clock > 50) lastlap = clock;
	    if (lastlap < bestlap || bestlap == 0) bestlap = lastlap;
	    start = time;
    	    }
}
	
void update(uint32_t time) {

    if (pressed(Button::DPAD_LEFT)  || joystick.x < -0.2f) cars[player].angle -= 0.05f;
    if (pressed(Button::DPAD_RIGHT) || joystick.x > 0.2f)  cars[player].angle += 0.05f;

    if (pressed(Button::A) && cars[player].speed < 2 ) cars[player].speed += 0.05f;

    // slow car if off track 
    if (point_inside_shape(cars[player].pos,innerarea) || !point_inside_shape(cars[player].pos,outerarea)) 
		    cars[player].speed = 0.2;

    // Move all cars (including player's car)
    bool aicar = false;
    for (auto &car: cars){
	    float angle = car.angle;
            car.dir = Vec2( cos(angle), sin(angle));
            car.pos += car.dir * car.speed;

	    int dist = 50;
	    if (aicar){
		    // AI follows waypoints
		    // point car at checkpoint
		    float angle = ang_to_point( car.pos, waypoints[car.waypt]);
	            car.angle = angle;
		    dist = 10;
		    }

	    if (near(car.pos,waypoints[car.waypt],dist)) {
			    //move target to next checkpoint if near current one
			    car.waypt++;
			    if (car.waypt == (int) waypoints.size()) car.waypt = 1;
			    car.distance++;
	    	    	    }
    	   //Get car ranking by distance travelled
	   int rank=1;
    	   for (auto &othercar: cars){
	    	if (car.distance < othercar.distance) rank++;
	   }
	   car.rank = rank;
	   aicar = true;
    }
    // player car gradually slows down 
    cars[player].speed *= 0.98f;
}
