#include "32blit.hpp"
using namespace blit;

std::string status;
int changed;
double scale = 1./128;
double cx = -.6, cy = 0;
int color_rotate, invert;
int saturation = 1;

Pen hsv_to_rgb(int hue)
{
	int r,g,b;
        int min=0;
	int max=255;
	if (min == max) max = min + 1;
	if (invert) hue = max - (hue - min);
	if (!saturation) {
		r = g = b = 255 * (max - hue) / (max - min);
		return Pen(r,g,b);
	}
	double h = fmod(color_rotate + 1e-4 + 4.0 * (hue - min) / (max - min), 6);
	double c = 255 * saturation;
	double X = c * (1 - fabs(fmod(h, 2) - 1));
 
	r = g = b = 0;
 
	switch((int)h) {
	case 0: r = c; g = X;return Pen(r,g,b); 
	case 1:	r = X; g = c;return Pen(r,g,b);
	case 2: g = c; b = X;return Pen(r,g,b);
	case 3: g = X; b = c;return Pen(r,g,b);
	case 4: r = X; b = c;return Pen(r,g,b);
	default:r = c; b = X;return Pen(r,g,b);
	}
}

void mandel(int skip) {
	int width = screen.bounds.w;
	int height = screen.bounds.h;

	int iter, max_iter = 256;
	double x, y, zx, zy, zx2, zy2;
	for (int i = 0; i < height; i += skip) {
		y = (i - height/2) * scale + cy;
		for (int j = 0; j  < width; j += skip) {
			x = (j - width/2) * scale + cx;
			zx = hypot(x - (double).25, y);
			if (x < zx - 2 * zx * zx + (double).25) continue;
			if ((x + 1)*(x + 1) + y * y < 1/16) continue;

			zx = zy = zx2 = zy2 = 0;
			for (iter = 0; iter < max_iter && zx2 + zy2 < 4; iter++) {
				zy = 2 * zx * zy + y;
				zx = zx2 - zy2 + x;
				zx2 = zx * zx;
				zy2 = zy * zy;
			}
			if (iter < max_iter) {
			     screen.pen = hsv_to_rgb(iter);
			     if (skip == 1) {
				     screen.pixel(Point(j,i));
				     if (changed) break;
			             } 
			     else screen.rectangle(Rect(j,i,skip,skip));
			}
		}
	}
}

void init() { set_screen_mode(ScreenMode::hires); }

void render(uint32_t time) {
static int stopped = 0;

    screen.pen = Pen(0, 0, 0);
    if (changed) { 
	    // do a quick render if scrolling / zooming around ( ~ 1 frame)
	    changed = 0;
    	    screen.clear();
    	    mandel(3);
	    stopped = 0;
    } else { 
            if (!stopped) {
	    // do a longer, more detailed render when stopped ( ~ 1 second)
	    stopped = 1;
    	    screen.clear();
	    mandel(1);
	    }
    }
    screen.pen = Pen(255,255,255);
    screen.text(status,minimal_font,Point(0,screen.bounds.h - 10));
}

void update(uint32_t time) {
	if (joystick.x < -0.1 || joystick.x > 0.1 || (abs(joystick.y) > 0.1)) {
		cx += scale * (double)joystick.x;
		cy += scale * (double)joystick.y;
		changed = 1;
	}
        if (pressed(Button::DPAD_LEFT))  cx -= scale; 
        if (pressed(Button::DPAD_RIGHT)) cx += scale;
        if (pressed(Button::DPAD_UP))    cy -= scale;
        if (pressed(Button::DPAD_DOWN))  cy += scale;

        if (pressed(Button::A)) scale /= (double)1.01;
        if (pressed(Button::B)) scale *= (double)1.01; 

	// toggle buttons should react on button release
        if (buttons.released & Button::MENU) color_rotate = !color_rotate;
        if (buttons.released & Button::Y) invert = !invert; 
        if (buttons.released & Button::X) saturation = !saturation; 
	// ANY button pressed
        if (buttons) changed = 1;
}


