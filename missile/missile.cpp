#include "32blit.hpp"
// missile command
//
using namespace blit;

typedef struct missile {
    Vec2 start;
    Vec2 pos;
    Vec2 velocity;
    bool trail;
} missile;

std::vector<missile> missiles;

typedef struct shape {
    std::vector<Vec2> points;
    bool hit;
} shape;

shape cities[3];
    
bool fire;
int ground,score;;
Vec2 center,firepos;

void draw_cross(Vec2 pos) {
    int s = 5;
    screen.pen = Pen(255,0,0);
    screen.line(pos + Vec2 (-s, -s), pos + Vec2 (s, s) );
    screen.line(pos + Vec2 (s, -s), pos + Vec2 (-s, s) );
}

void draw_laser(Vec2 pos) {
    screen.pen = Pen(rand() % 255,rand() % 255,rand() % 255);
    screen.line(Vec2 (0, screen.bounds.h), pos);
    screen.line(Vec2 (screen.bounds.w, screen.bounds.h), pos);
}

bool offscreen (Vec2 pos) {
    if (pos.x > screen.bounds.w || pos.x < 0 ) return(true);
    if (pos.y > screen.bounds.h || pos.y < 0 ) return(true);
    return (false);
}

float rnd (int max) {
    float random = rand() % (max * 10);
    random = random / 10;
    return (random);
}

bool hit ( Vec2 p1,Vec2 p2) {
	Vec2 d = p1 - p2;
	int distance = abs(d.x) + abs(d.y);
	if (distance < 10) return true;
	return false;
}

void draw_shape(shape shape){
    Vec2 lastpos = shape.points[0];
    for (auto &p: shape.points) {
        screen.line(lastpos , p);
        lastpos = p;
    }
}

bool point_in_shape ( Vec2 point, std::vector<Vec2> shape ){
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

void init() {
    set_screen_mode(ScreenMode::lores);
    center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);

    for (int i=0; i < 10; i++ ) {
        missile m;
        missiles.push_back(m);
        }
    firepos = center;
    ground = screen.bounds.h - 1;
    int city = ground - 10;

    cities[0].points.push_back(Vec2(0,ground));
    cities[0].points.push_back(Vec2(10,ground));
    cities[0].points.push_back(Vec2(20,city));
    cities[0].points.push_back(Vec2(30,city));
    cities[0].points.push_back(Vec2(40,ground));
    cities[0].points.push_back(Vec2(50,ground));

    cities[1].points.push_back(Vec2(50,ground));
    cities[1].points.push_back(Vec2(60,ground));
    cities[1].points.push_back(Vec2(70,city));
    cities[1].points.push_back(Vec2(80,city));
    cities[1].points.push_back(Vec2(90,ground));
    cities[1].points.push_back(Vec2(100,ground));

    cities[2].points.push_back(Vec2(100,ground));
    cities[2].points.push_back(Vec2(110,ground));
    cities[2].points.push_back(Vec2(120,city));
    cities[2].points.push_back(Vec2(130,city));
    cities[2].points.push_back(Vec2(140,ground));
    cities[2].points.push_back(Vec2(150,ground));
}

void render(uint32_t time) {
static int heat,health;

    vibration = 0.0f;
    screen.pen = Pen(0, 0, 0);
    screen.clear();

    health = 3;
    for (auto &city : cities) {
        screen.pen = Pen(0,255,0);
        if (city.hit) { 
		screen.pen = Pen(255,0,0); 
		health--;
	}
	draw_shape(city);
        }

    for (auto &m : missiles) {
        m.pos  += m.velocity;
        screen.pen = Pen(255,255,255);
        if (m.trail) screen.line(m.start,m.pos);
	else screen.pixel(m.pos);

	if (offscreen(m.pos) || m.velocity.y == 0 ) {
		m.start = Vec2(rand () % screen.bounds.w, 0);
		m.pos = m.start;
		m.velocity = Vec2(1.0 - rnd(2),rnd(1));
		if (rnd(100) < 90) m.trail = true;
		else m.trail = false;
		}
	for (auto &city : cities) {
		if (point_in_shape(m.pos,city.points)) {
			//m.velocity.y = 0;
			//vibration = 0.1f;
    			screen.text("City destroyed !",minimal_font,Vec2(0,screen.bounds.h - 30));
                	screen.pen = Pen(255,0,0);
			draw_shape(city);
			city.hit = true;
			for (auto &p : city.points) 
				p.y = ground;
	        	}
		}
        }

    draw_cross(firepos);

    if (fire && heat < 120) {
                draw_laser(firepos);
		heat+= 5;
    		for (auto &m : missiles) {
			if (hit (m.pos , firepos))
				m.velocity.y = 0;
			}
                }

    //draw laser temperature line
    screen.pen = Pen(255,0,0);
    screen.line(Vec2(screen.bounds.w-1,screen.bounds.h),Vec2(screen.bounds.w-1,screen.bounds.h - heat));
    if (!fire && heat > 0) heat = heat - 2;

    if (health) score++;
    else screen.text("Cities destroyed - all is lost !",minimal_font,Vec2(0,screen.bounds.h - 30));
    screen.text(std::to_string(score / 10),minimal_font,Vec2(0,screen.bounds.h - 10));
}

void update(uint32_t time) {
    
	if (joystick.x || joystick.y) firepos = (joystick * center) + center;

        if (pressed(Button::DPAD_LEFT))  firepos.x--; 
        if (pressed(Button::DPAD_RIGHT)) firepos.x++;
        if (pressed(Button::DPAD_UP))    firepos.y--; 
        if (pressed(Button::DPAD_DOWN))  firepos.y++;

	fire = false;
        if (pressed(Button::A)) fire = true; 
}


