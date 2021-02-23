#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

std::string status;
Surface *tileimg,*depthimg,*fullscreenimg;

bool moving = false;
bool dots = true;

int tilesize = 50;
int tilenum;

Pen colours[] = { Pen(0,0,0),Pen(255,0,0),Pen(0,255,0),Pen(0,0,255),Pen(255,255,255) };
 
void fill_with_tile (Surface *target,Surface *tile,int tilesize,Rect tilerect){
	for (int x=0; x<target->bounds.w; x += tilesize)
	   for (int y=0; y<target->bounds.h; y += tilesize)
		   target->stretch_blit(tile,tilerect,Rect(x,y,tilesize,tilesize));
}

int getdepth(int x,int y) {
			auto p = depthimg->ptr(Vec2(x,y));
			int depth = 3 *  ( p[0] != 0 );
			return ( depth );
}

void sirds() {

#define round(X) (int)((X) + 0.5)

int dpi = 44;
int e = round(2.5 * dpi); // eye separation = 2.5 inches
float mu = 0.08; // depth of field

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
	} // end for x
	int pix[screenwidth];
	for (int i=0;i<screenwidth;i++)
		pix[i]=i;

	for (int x=screenwidth-1;x>0;x--) {
		if (dots) {
			// random dots
		    if (same[x] == x) pix[x] = rand ()  % 5;
		    else pix[x] = pix[same[x]];

		    screen.pen = colours[pix[x]];
		    screen.pixel(Vec2(x,y));
		} else {
			//tiled background
		    if (same[x] == x) pix[x] = x;
		    else pix[x] = pix[same[x]];

		    screen.blit(fullscreenimg,Rect(pix[x],y,1,1),Vec2(x,y));
		}
	   }
    } // end for y
}

Surface *make_surface (int width,int height, Surface *paletteimg) {
    // make a surface and copy palette from another surface
    uint8_t *data = new uint8_t[width * height];

    Surface *newsurface = new Surface(data, PixelFormat::P, Size(width,height));
    newsurface->palette = new Pen[256];
    wmemcpy((wchar_t *)newsurface->palette, (wchar_t *)paletteimg->palette, 256 * sizeof(Pen));

    return (newsurface);
}

void init() {
    set_screen_mode(ScreenMode::hires);

    depthimg = Surface::load(depthpic);
    tileimg = Surface::load(tilepic);

    fullscreenimg = make_surface(screen.bounds.w,screen.bounds.h,tileimg);
 
}

void render(uint32_t time) {

    int x = (tilenum % 2);
    int y = (tilenum >= 2);
    Rect tilerect = Rect(64 * x, 64 * y,64,64);
    fill_with_tile(fullscreenimg,tileimg,tilesize,tilerect);

    // Clear screen
    screen.pen = Pen (0,0,0);
    screen.clear();

    // use same seed if static
    if (!moving) srand(0);
    sirds();

    // Reveal depth image
    if (pressed(MENU)) {
    	screen.pen = Pen (64,64,64);
    	for (int x=0;x<screen.bounds.w;x++)  
	    for (int y=0;y<screen.bounds.h;y++)  
		    if (getdepth(x,y)) screen.pixel(Vec2(x,y));
    }
}
	
void update(uint32_t time) {
	
	tilesize += joystick.x * 2;
	tilesize = std::max(25,tilesize);

	//dep += joystick.y / 5;

	if (pressed(DPAD_RIGHT)) { 
		static uint32_t last;
		if (now() - last > 100) { 
			tilenum++;  
			if (tilenum > 3) tilenum = 0;
		}
	}
	if (pressed(X)) { 
		static uint32_t last;
		if (now() - last > 100) { 
			dots = !dots; 
		}
	}
	if (pressed(Y)) {
		static uint32_t last;
		if (now() - last > 100) { 
			moving = !moving;
		}
	}
}
