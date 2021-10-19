#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

int x,y,sindex;
Vec2 size;

Surface *sheet=nullptr;;
std::string names[] = {"dingbads","pirate_characters","pirate_tilemap","platformer","space_shooter_backdrop","space_shooter_ships","top_down_shooter"};

Surface *load_sheet(int ssindex) {
  if (sheet) { // delete to free up memory
	  delete[] sheet->data;
	  delete[] sheet->palette;
	  delete sheet;
  }
  switch (ssindex) {
	  case 0: 
		sheet = Surface::load(dingbads);
		break;
	  case 1: 
  		sheet =  Surface::load(pirate_characters);
		break;
	  case 2: 
  		sheet =  Surface::load(pirate_tilemap);
		break;
	  case 3: 
  		sheet =  Surface::load(platformer);
		break;
	  case 4: 
  		sheet =  Surface::load(space_shooter_backdrop);
		break;
	  case 5: 
  		sheet =  Surface::load(space_shooter_ships);
		break;
	  case 6: 
  		sheet =  Surface::load(top_down_shooter);
		break;
  }
  return(sheet);
}

void init() {
  set_screen_mode(ScreenMode::hires);

  x = 0;
  y = 0;
  sindex = 0;
  size = Vec2(1,1);

  screen.sprites = load_sheet(sindex);
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
  text = "Spritesheet: " + names[sindex];
  screen.text(text, minimal_font, Point(8, 16));

  // Draw full spritesheet
  screen.stretch_blit( screen.sprites, Rect(0, 0, 128, 128), Rect(8, 24, 200,200));

  // highlight current sprite in sheet
  int xpos = xx * 200 / 128;
  int ypos = yy * 200 / 128;
  screen.alpha = 100;
  screen.rectangle(Rect(xpos+8,ypos+24,14 * size.x,14 * size.y));

  int rightside = screen.bounds.w - 80;
  if (screen.bounds.w < 320) 
	  rightside = 205;

  // Draw sprite at normal size
  screen.alpha = 255;
  //screen.sprite(Point(x, y), Point(rightside , 24));
  screen.stretch_blit( screen.sprites, Rect(xx, yy, 8 * size.x, 8 * size.y), Rect(rightside, 24, 8 * size.x, 8 * size.y));

  // Draw sprite stretched to 16x16 pixels
  screen.stretch_blit( screen.sprites, Rect(xx, yy, 8 * size.x, 8 * size.y), Rect(rightside, 64, 16 * size.x, 16 * size.y));
  // 32x32
  screen.stretch_blit( screen.sprites, Rect(xx, yy, 8 * size.x, 8 * size.y), Rect(rightside,52 + (size.y * 50), 32 * size.x, 32 * size.y));
  // 64x64
  if (size.x * size.y == 1) screen.stretch_blit( screen.sprites, Rect(xx, yy, 8, 8), Rect(rightside, 144, 64, 64));

  // Show current sprite index
  text = "screen.sprite ( " + std::to_string(x) + "," + std::to_string(y) + " )";
  screen.pen = Pen(255, 255, 255);
  screen.text(text, minimal_font, Point(8, screen.bounds.h - 8));
}

void update(uint32_t time) {
static uint32_t last_time;

if (now() - last_time > 200) {

	// move cursor around sheet
	x += round(joystick.x) * (double) size.x;
	y += round(joystick.y) * (double) size.y;

	if (pressed(Button::DPAD_LEFT))  x--;
	if (pressed(Button::DPAD_RIGHT)) x++;
	if (pressed(Button::DPAD_UP))    y--;
	if (pressed(Button::DPAD_DOWN))  y++;

	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x > 15) x = 15;
	if (y > 15) y = 15;

	// change spritesheet to previous one
	if (pressed(Button::Y)) {
		sindex--;
		if (sindex < 0) sindex = 6;
  		screen.sprites = load_sheet(sindex);
		}

	// change spritesheet to next one
	if (pressed(Button::A)) {
		sindex++;
		if (sindex > 6) sindex = 0;
  		screen.sprites = load_sheet(sindex);
		}
	// change sprite size up - 1x1 1x2 2x2
	if (pressed(Button::X)) {
		size.x = 3 - size.x;
	}

	// change sprite size down
	if (pressed(Button::B)) {
		size.y = 3 - size.y;
	}

	last_time = now();
	}

}
