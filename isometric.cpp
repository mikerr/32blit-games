#include "32blit.hpp"
#include "sabreman.hpp"

using namespace blit;

Vec2 player,screenpos,origin;
int dir;

int d_left,d_up,d_down,d_right;
Vec3 roomcolor;

int distance (Vec2 a, Vec2 b ) {
        Vec2 diff;
        diff.x = a.x - b.x;
        diff.y = a.y - b.y;
        return ( sqrt ( diff.x * diff.x  + diff.y * diff.y));
}

void newroom () {
        roomcolor = Vec3(rand() % 255,rand() % 255,rand() % 255);
}

void init() {
  set_screen_mode(ScreenMode::hires);
  d_left = 0; d_up = 1; d_down = 2; d_right = 3;
  origin = Vec2(0,0);
  newroom();
}

void render(uint32_t time) {

  screen.sprites = SpriteSheet::load(room);
  screen.sprites->palette[0] = RGBA(roomcolor.x,roomcolor.y,roomcolor.z);
  screen.stretch_blit(screen.sprites,Rect(0,0,264,192),Rect(0,0,screen.bounds.w,screen.bounds.h));

  screenpos.x = 150 + player.x * 8 - player.y * 9 ;
  screenpos.y = 30 + player.x * 4.5 + player.y * 5.5;

  screen.sprites = SpriteSheet::load(sabreman);

  Rect sabremanback = Rect(0,0,3,4);
  Rect sabremanfront = Rect(3,0,3,4);

  if (dir == d_left)    { screen.sprite(sabremanback,screenpos); }
  if (dir == d_up)      { screen.sprite(sabremanback,screenpos,origin,1,SpriteTransform::HORIZONTAL); }
  if (dir == d_right)   { screen.sprite(sabremanfront,screenpos); }
  if (dir == d_down)    { screen.sprite(sabremanfront,screenpos,origin,1,SpriteTransform::HORIZONTAL); }
}
void update(uint32_t time) {
 if (pressed(Button::DPAD_LEFT)) {
        if (player.x > 0) player.x -= 0.1;
        dir = d_left;
        }
 if (pressed(Button::DPAD_UP)) {
        if (player.y > 0) player.y -= 0.1;
        dir = d_up;
        }
 if (pressed(Button::DPAD_RIGHT)) {
        if (player.x < 16) player.x += 0.1;
        dir = d_right;
        }
 if (pressed(Button::DPAD_DOWN)) {
        if (player.y < 16) player.y += 0.1;
        dir = d_down;
        }

// doors
 if (distance(player, Vec2(0,8)) < 1 && dir == d_left) {
        player = Vec2(16,8);
        newroom();
        }
 if (distance(player, Vec2(16,8)) < 1 && dir == d_right) {
        player = Vec2(0,8);
        newroom();
        }
 if (distance(player, Vec2(8,0)) < 1 && dir == d_up) {
        player = Vec2(8,16);
        newroom();
        }
 if (distance(player, Vec2(8,16)) < 1 && dir == d_down) {
        player = Vec2(8,0);
        newroom();
        }
}
