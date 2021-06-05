#include "32blit.hpp"
using namespace blit;

void draw_cross(Vec2 pos) {
    int s = 5;
    screen.line(pos + Vec2 (-s, -s), pos + Vec2 (s, s) );
    screen.line(pos + Vec2 (s, -s), pos + Vec2 (-s, s) );
}

int changed = 1;
double scale = 1./128;
double cx = -.6, cy = 0;
int color_rotate = 0;
int saturation = 1;
int invert = 0;

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
void julia(int skip){
int w = screen.bounds.w;
int h = screen.bounds.h;
float zoom = 1 + scale;
int iter,maxiter = 255;
float moveX = 0;
float moveY = 0;
double cX = cx;//-0.7;
double cY = cy;//0.27015;
double zx, zy, tmp;
 
            for (int x = 0; x < w; x += skip)
                for (int y = 0; y < h; y += skip)
                {
                    zx = 1.5 * (x - w / 2) / (0.5 * zoom * w) + moveX;
                    zy = 1.0 * (y - h / 2) / (0.5 * zoom * h) + moveY;
                    iter = maxiter;
                    while (zx * zx + zy * zy < 4 && iter > 1)
                    {
                        tmp = zx * zx - zy * zy + cX;
                        zy = (double)2.0 * zx * zy + cY;
                        zx = tmp;
                        iter--;
                    }
	   	    screen.pen = hsv_to_rgb(iter);
		    screen.pixel(Point(x,y));
                }
}
void mandel_zoom(int max_iter,int skip) {
	int width = screen.bounds.w;
	int height = screen.bounds.h;

	int iter;
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

    if (changed) { 
    	    screen.pen = Pen(0, 0, 0);
    	    screen.clear();
    	    mandel_zoom(256,3);
	    //julia(1);
	    changed = 0;
	    stopped = 0;
    } else { 
            if (!stopped) {
    	    screen.pen = Pen(0, 0, 0);
    	    screen.clear();
	    mandel_zoom(256,1);
	    //julia(1);
	    stopped = 1;
	    }
    }
}

void update(uint32_t time) {
	if (joystick.x < -0.1 || joystick.x > 0.1 || (abs(joystick.y) > 0.1)) {
		cx += scale * (double)joystick.x;
		cy += scale * (double)joystick.y;
		changed = 1;
	}

        if (pressed(Button::DPAD_LEFT))  { changed = 1; cx -= scale; }
        if (pressed(Button::DPAD_RIGHT)) { changed = 1; cx += scale;}
        if (pressed(Button::DPAD_UP))    { changed = 1; cy -= scale;}
        if (pressed(Button::DPAD_DOWN))  { changed = 1; cy += scale;}

        if (pressed(Button::A)) { changed = 1; scale /= (double)1.01; }
        if (pressed(Button::B)) { changed = 1; scale *= (double)1.01; }

        if (pressed(Button::MENU)) { changed = 1; color_rotate = !color_rotate; }
        if (pressed(Button::X))  { changed = 1; saturation = !saturation; }
        if (pressed(Button::Y))  { changed = 1; invert = !invert; }
}


