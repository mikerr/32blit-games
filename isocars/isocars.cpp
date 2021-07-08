#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

int dead,jumping;
Vec2 origin;

Rect carN = Rect(0,0,64,48);
Rect carNE = Rect(256,0,64,48);
Rect carE = Rect(384,0,64,48);
Rect carSE = Rect(320,0,64,48);
Rect carS = Rect(448,0,64,48);
Rect carSW = Rect(128,0,64,48);
Rect carW = Rect(192,0,64,48);
Rect carNW = Rect(64,0,64,48);

typedef struct car {
	Vec2 pos;
	int rot; // rotation 8 dirs
	float speed;
	Vec2 vel;
	int num;
	int costume;
} car;

struct car aicars[5];
struct car player;

Rect costumes[8] = { carN, carNE, carE, carSE, carS, carSW, carW, carNW };
Vec2 dirs[8] = { Vec2(0.7,0.7),Vec2(0,1),Vec2(-0.7,0.7),Vec2(-1,0),Vec2(-0.7,-0.7),Vec2(0,-1),Vec2(0.7,-0.7),Vec2(1,0) };
Surface *tiles, *vehicles;

Rect blank = Rect(0,0,0,0),
   grass = Rect(0,0,128,94),
   road60 = Rect(128,0,128,94),
   road30 = Rect(256,0,128,94),
   crossroad = Rect(384,0,128,94),
   cornerbr = Rect(512,0,128,94),
   cornerbl = Rect(640,0,128,94),
   cornertl = Rect(768,0,128,94),
   cornertr = Rect(896,0,128,94),
   cross30e = Rect(1024,0,128,94),
   cross60e = Rect(1152,0,128,94),
   cross30w = Rect(1279,0,128,94),
   cross60w = Rect(1409,0,128,94);
   
#define MAPSIZE 9
Rect track[MAPSIZE][MAPSIZE] = {
   { cornertr, road30, road30, road30, cross30e, road30, road30, road30, cornertl } ,
   { road60, grass, grass, grass, road60, grass, grass, grass, road60 } ,
   { road60, grass, grass, grass, road60, grass, grass, grass, road60 } ,
   { road60, grass, grass, grass, road60, grass, grass, grass, road60 } ,
   { cross60w, road30, road30, road30, crossroad, road30, road30, road30, cross60e } ,
   { road60, grass, grass, grass, road60, grass, grass, grass, road60 } ,
   { road60, grass, grass, grass, road60, grass, grass, grass, road60 } ,
   { road60, grass, grass, grass, road60, grass, grass, grass, road60 } ,
   { cornerbr, road30, road30, road30, cross30w, road30, road30, road30, cornerbl } 
   };

std::vector <Vec3> buildings = { 
	Vec3(1.6, 0.6, 70), Vec3(2.6, 0.6, 70), Vec3(6, 1, 120), Vec3(7, 1, 120),
   	Vec3(1.6, 4.5, 70), Vec3(2.6, 4.5, 70), Vec3(3.6, 4.5, 70),
   	Vec3(1.6, 5.5, 70), Vec3(2.6, 5.5, 70), Vec3(3.6, 5.5, 70),
   	Vec3(1.6, 6.6, 70), Vec3(2.6, 6.6, 70), Vec3(3.6, 6.6, 70),
   	Vec3(6, 6.5, 100),  Vec3(7, 6.5, 120)
   };

bool near(Vec2 point,Vec2 target, float howfar){
	return ( (abs(point.x - target.x) < howfar ) && (abs(point.y - target.y) < howfar));
}
void init() {
    set_screen_mode(ScreenMode::hires);

    tiles = Surface::load(tilesimg);
    vehicles = Surface::load(vehiclesimg);

    player.pos = Vec2(2,0.5);
    player.rot = 3;
    player.costume = 96;
    for (auto &car : aicars) {
    	static int num = 1;
	    car.speed = 2 + (rand() % 40) / 10.0f;
            car.num = num++;
            car.rot = 3;
	    car.pos = Vec2(num,0);
    }
}

Vec2 isometric (float x, float y) {
   Vec2 grid;
   grid = Vec2(x - y, x + y);
   grid = grid * Vec2(120,70);
   grid += origin;

   return (grid);
}

void render_track() {
   for (int x=0; x < MAPSIZE; x++)
   	for (int y=0; y < MAPSIZE; y++) {
		Vec2 grid = isometric(x,y);
		Rect tile = track[x][y];
   		screen.stretch_blit(tiles,tile,Rect(grid.x,grid.y,256,188));
		}
}

void ai_cars () {
   for (auto &car : aicars) {
      car.pos -= dirs[car.rot] * (car.speed / 100);

      Rect tile = track[(int)car.pos.x][(int)car.pos.y];
      if (tile == cornerbr) car.rot = 5;
      if (tile == cornerbl) car.rot = 7;
      if (tile == cornertl) car.rot = 1;
      if (tile == cornertr) car.rot = 3;

      Vec2 grid = isometric(car.pos.x,car.pos.y);
      Rect costume = costumes[car.rot];
      costume.y += 48 * (car.num - 1);
      screen.blit(vehicles,costume,grid);
      //screen.text(std::to_string(car.num),minimal_font,grid);
      //screen.text(std::to_string(x) + " " + std::to_string(y),minimal_font,grid);

      //if (near(car.pos,player.pos,0.001)) screen.text(std::to_string(x) + " " + std::to_string(y),minimal_font,grid);
   }
}

void render_building(Vec3 pos3) {
Point t1,t2,t3,t4;
Vec2 pos = Vec2(pos3.x,pos3.y);
Vec2 h = Vec2(0,pos3.z);
float width = 0.9;
   // north wall
   screen.pen = Pen(192,192,192);
   t1 = isometric(pos.x,pos.y + width);
   t2 = isometric(pos.x + width,pos.y + width);
   t3 = t2 - h;
   t4 = t1 - h;
   screen.triangle(t1,t2,t3); screen.triangle(t3,t4,t1);

   // west wall (sharing side t2/t3 with north)
   screen.pen = Pen(128,128,128);
   t1 = isometric(pos.x + width,pos.y);
   t4 = t1 - h;
   screen.triangle(t1,t2,t3); screen.triangle(t3,t4,t1);

   // roof
   screen.pen = Pen(64,64,64);
   t1 = isometric(pos.x,pos.y) - h;
   t2 = isometric(pos.x,pos.y + width) - h;
   t3 = isometric(pos.x + width,pos.y) - h;
   t4 = isometric(pos.x + width,pos.y + width) - h;
   screen.triangle(t1,t2,t3); screen.triangle(t2,t3,t4);
}

void render(uint32_t time) {

   screen.pen = Pen(0,0,0);
   screen.clear();

   origin = Vec2 (0,0);
   origin = isometric(1-player.pos.x, 1-player.pos.y);

   render_track();
   ai_cars();

   Rect costume = costumes[player.rot];
   costume.y = player.costume;
   screen.blit(vehicles,costume,Point(120,120));

   for (auto tower :  buildings) 
   	render_building(tower);
}

void update(uint32_t time) {
   if (!dead) {
      if (time % 16 < 2) {
   	if (pressed(DPAD_RIGHT)|| (joystick.x >  0.5f)) { 
		player.rot ++; 
      		player.vel *= 0.2f;
	}
   	if (pressed(DPAD_LEFT) || (joystick.x < -0.5f)) { 
		player.rot --; 
      		player.vel *= 0.2f;
	}
   	if (pressed(Button::A) || pressed(DPAD_UP)) player.speed++;
   	if (pressed(Button::B)) { 
		player.costume += 48;
		if (player.costume > 235) player.costume = 0;
	}
      }

      player.vel += dirs[player.rot] * (player.speed / 1500);
      player.pos -= player.vel;
      player.vel *= 0.97f;
      player.speed *= 0.95f;

      if ((player.pos.x < 0) || (player.pos.x > 9 ) || (player.pos.y > 9.2f) || (player.pos.y < 0.1f)) player.vel *= -1;

      if (player.rot < 0) player.rot = 7;
      if (player.rot > 7) player.rot = 0;

   }
}
