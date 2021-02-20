#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

std::string status;
Surface *tileimg,*depthimg,*fullscreenimg;

int dpi = 44;
float mu = 0.19 ; // depth of field

void fill_with_tile (Surface *tile){
	int tilewidth = tile->bounds.w ;
	int tileheight = tile->bounds.h ;
	for (int x=0; x<screen.bounds.w; x += tilewidth)
	   for (int y=0; y<screen.bounds.h; y += tileheight)
		   screen.stretch_blit(tile,Rect(0,0,tilewidth,tileheight),Rect(x,y,tilewidth,tileheight));
}

int getdepth(int x,int y) {
			auto p = depthimg->ptr(Vec2(x,y));
			return ( p[0] != 0 );
}

void sirds() {

#define round(X) (int)((X) + 0.5)

int e = round(2.5 * dpi); // eye separation = 2.5 inches

int screenwidth = screen.bounds.w;
int screenheight = screen.bounds.h;

for (int y=0;y<screenheight;y++)  {
	int left = 0;
	int right = 0;
	int same[screenwidth];
	 
	for (int x=0;x<screenwidth;x++) same[x] = x;

	// calc row
	for (int x=0;x<screenwidth;x++) {
		int depth = getdepth(x,y);
		
		int s = round (((1 - (mu * depth)) * e) / ( 2 - (mu * depth)));

		left = x - (s + (s&y&1)) / 2;
		right = left + s;

		if (0 <= left && right < screenwidth) {
			int visible;
			int t = 1;
			float zt;

			do {
			   zt = depth + 2 * (2 - mu * depth) * t / (mu * e);
			   visible = getdepth(x - t, y) < zt && getdepth( x + t,y) < zt;
			   t++;
			   }  
			while (visible && zt < 1);

			if (visible) {
				int k;
				for (k = same[left]; k != left && k != right; k = same[left]) {
					if (k < right) 
						left = k;
					else {
						left = right;
						right = k;
					     }   
				        }
				same[left] = right;
			        }
	         }
	} // for x
	int pix[screenwidth];
	for (int x=screenwidth-1;x>0;x--) {
		if (same[x] == x) pix[x] = rand ()  % 5;
		else pix[x] = pix[same[x]];
		
		if (pix[x] == 0) screen.pen = Pen(0,0,0);
		if (pix[x] == 1) screen.pen = Pen(255,0,0);
		if (pix[x] == 2) screen.pen = Pen(0,255,0);
		if (pix[x] == 3) screen.pen = Pen(0,255,255);
		if (pix[x] == 4) screen.pen = Pen(255,255,255);

		screen.pixel(Vec2(x,y));
	   }
    } // for y
}

void init() {
    set_screen_mode(ScreenMode::hires);
    depthimg = Surface::load(depthpic);
}
void render(uint32_t time) {
    // Clear screen
    screen.pen = Pen (0,0,0);
    screen.clear();

    srand(0);
    sirds();

    screen.pen = Pen(0, 0, 0);
    screen.rectangle(Rect(0, screen.bounds.h-10, screen.bounds.w, screen.bounds.h));
    screen.pen = Pen (255,255,255);
    std::string status = " DPI: " + std::to_string(dpi) + "  Depth: " + std::to_string(mu);
    screen.text(status, minimal_font, Vec2(0, screen.bounds.h - 10));
}
	
void update(uint32_t time) {
	
	if (pressed(MENU)) {
    		screen.pen = Pen (64,64,64);
    		for (int x=0;x<screen.bounds.w;x++)  
		    for (int y=0;y<screen.bounds.h;y++)  
			    if (getdepth(x,y)) screen.pixel(Vec2(x,y));
	}
	if (pressed(DPAD_DOWN)) {
		static uint32_t last;
		if (now() - last > 100) { 
			dpi--;
			last = now();
		}
	}
	if (pressed(DPAD_UP)) {
		static uint32_t last;
		if (now() - last > 100) {
			dpi++;
			last = now();
		}
	}
	if (pressed(DPAD_LEFT)) {
		static uint32_t last;
		if (now() - last > 100) { 
			mu -= 0.01;
			last = now();
		}
	}
	if (pressed(DPAD_RIGHT)) {
		static uint32_t last;
		if (now() - last > 100) { 
			mu += 0.01;
			last = now();
		}
	}
	if (pressed(X)) {
		static uint32_t last;
		static bool res;
		if (now() - last > 100) { 
			if (res) set_screen_mode(ScreenMode::lores);
			else set_screen_mode(ScreenMode::hires);
			res = !res;
		}
	}
}
