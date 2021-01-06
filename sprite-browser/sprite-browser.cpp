#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

int x,y,highres,index;

SpriteSheet *sheets[7];
//char *names[] = {"dingbads","pirate_characters","pirate_tilemap","platformer","space_shooter_backdrop","space_shooter_ships","top_down_shooter"};
std::string names[] = {"dingbads","pirate_characters","pirate_tilemap","platformer","space_shooter_backdrop","space_shooter_ships","top_down_shooter"};

void init() {
  set_screen_mode(ScreenMode::hires);

  sheets[0] = SpriteSheet::load(dingbads);
  sheets[1] = SpriteSheet::load(pirate_characters);
  sheets[2] = SpriteSheet::load(pirate_tilemap);
  sheets[3] = SpriteSheet::load(platformer);
  sheets[4] = SpriteSheet::load(space_shooter_backdrop);
  sheets[5] = SpriteSheet::load(space_shooter_ships);
  sheets[6] = SpriteSheet::load(top_down_shooter);

  x = 0;
  y = 0;
  index = 0;
  highres = 1;
  screen.sprites = sheets[index];
}

void render(uint32_t time_ms) {
  std::string text;

  int xx = x * 8;
  int yy = y * 8;
  
  screen.pen = Pen(20, 30, 40);
  screen.clear();

  screen.alpha = 255;
  screen.mask = nullptr;

  // draw titlebox
  screen.alpha = 255;
  screen.pen = Pen(255, 255, 255);
  screen.rectangle(Rect(0, 0, screen.bounds.w, 14));

  screen.pen = Pen(0, 0, 0);
  screen.text("Sprite browser", minimal_font, Point(5, 4));

  // spritesheet name
  screen.pen = Pen(255, 255, 255);
  text = "Spritesheet: " + names[index];
  screen.text(text, minimal_font, Point(8, 16));

  // Draw full spritesheet
  screen.stretch_blit( screen.sprites, Rect(0, 0, 128, 128), Rect(8, 24, 200,200));

  // highlight current sprite in sheet
  int xpos = xx * 200 / 128;
  int ypos = yy * 200 / 128;
  screen.alpha = 100;
  screen.rectangle(Rect(xpos+8,ypos+24,16,16));

  int rightside = screen.bounds.w - 80;
  // Draw sprite at normal size
  screen.sprite(Point(x, y), Point(rightside , 24));

  // Draw sprite stretched to 16x16 pixels
  screen.stretch_blit( screen.sprites, Rect(xx, yy, 8, 8), Rect(rightside, 64, 16, 16));
  // 32x32
  screen.stretch_blit( screen.sprites, Rect(xx, yy, 8, 8), Rect(rightside,102, 32, 32));
  // 64x64
  if (highres) screen.stretch_blit( screen.sprites, Rect(xx, yy, 8, 8), Rect(rightside, 144, 64, 64));

  // Show current sprite index
  text = "screen.sprite ( " + std::to_string(x) + "," + std::to_string(y) + " )";
  screen.pen = Pen(255, 255, 255);
  screen.text(text, minimal_font, Point(8, screen.bounds.h - 8));
}

void update(uint32_t time) {
static uint32_t last_time;

if (now() - last_time > 200) {

	x += round(joystick.x);
	y += round(joystick.y);

	if (pressed(Button::DPAD_LEFT))  x--;
	if (pressed(Button::DPAD_RIGHT)) x++;
	if (pressed(Button::DPAD_UP))    y--;
	if (pressed(Button::DPAD_DOWN))  y++;

	if (pressed(Button::Y)) {
		index--;
		if (index < 0) index = 6;
  		screen.sprites = sheets[index];
		}

	if (pressed(Button::A)) {
		index++;
		if (index > 6) index = 0;
  		screen.sprites = sheets[index];
		}
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x > 15) x = 15;
	if (y > 15) y = 15;

	last_time = now();
	}

}
