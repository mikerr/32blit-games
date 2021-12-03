#include "32blit.hpp"
#include "assets.hpp"

using namespace blit;

#define MAX(a,b) (((a)>(b))?(a):(b))

int lowres,clicked,credits,options;
Vec2 center,mappos,cursorpos;

struct unit {
	Vec2 pos;
	Vec2 dest;
	float angle;
	int health;
	bool enemy;
};
std::vector<unit> quads, harvesters, windtraps, refineries;

Surface *backdrop,*dunesprites;

#define MAPSIZE 256

// Static wave config
static uint32_t wavSize = 0;
static uint16_t wavPos = 0;
static uint16_t wavSampleRate = 11025;
static const uint8_t *wavSample;

// Called everytime audio buffer ends
void buffCallBack(AudioChannel &channel) {

  // Copy 64 bytes to the channel audio buffer
  for (int x = 0; x < 64; x++) {
    // Note: The sample used here has an offset, so we adjust by 0x7f. 
    channel.wave_buffer[x] = (wavPos < wavSize) ? (wavSample[wavPos]  - 0x7f) << 8 : 0;

    // As the engine is 22050Hz, we can timestretch to match 
    if (wavSampleRate == 11025) {
      if (x % 2) wavPos++;
    } else {
      wavPos++;
    }
  }
  
  if (wavPos >= wavSize) {
    channel.off();        // Stop playback of this channel.
     //Clear buffer
    wavSample = nullptr;
    wavSize = 0;
    wavPos = 0;
  }
}

void play_wav(const uint8_t *wav,uint32_t wav_len ) {

wavSample = wav;
wavSize = wav_len;
channels[0].trigger_attack();
}


void blit_rotate_sprite (Surface *sprite,Rect src, float angle, Vec2 screenpos) {
//rotate a sprite to screen at any angle
Vec2 rot;
    int width = src.w;
    int height = src.h;
    float sinangle = sin(angle);
    float cosangle = cos(angle);
    for (int x=0;x<width;x++)
	    for (int y=0;y<height;y++) {
		    int x1 = x - (width / 2);
		    int y1 = y - (height / 2);
		    rot.x = x1 * sinangle + y1 * cosangle;
		    rot.y = y1 * sinangle - x1 * cosangle;
		    Vec2 pos = rot + screenpos;
        	    screen.stretch_blit(sprite,Rect(src.x + x, src.y + y,1,1),Rect(pos.x, pos.y,2,2));
	    }
}

void draw_cursor(Vec2 pos) {
    screen.pen = Pen(255,255,255);
    screen.line(pos + Vec2 (-5, -5), pos + Vec2 (5, 5) );
    screen.line(pos + Vec2 (5, -5), pos + Vec2 (-5, 5) );
}

void draw_box(int x, int y, int w, int h){
Vec2 a = Vec2(x,y);
   screen.line(a, a + Vec2(0, h));
   screen.line(a + Vec2(0,h), a + Vec2(w,h));
   screen.line(a + Vec2(w, h),a + Vec2(w,0));
   screen.line(a + Vec2(w,0),a);
}

bool near (Vec2 cursor, Vec2 object) {
    int distance = abs(mappos.x + cursor.x - object.x - 2) + abs(mappos.y + cursor.y - object.y - 2);
    return (distance < 20);
}

bool place_building ( Rect costume, std::vector<unit> &buildings) {
Vec2 grid;
	grid.x = int (cursorpos.x / 16) * 16;
	grid.y = int (cursorpos.y / 16) * 16;
	screen.blit(dunesprites,costume,grid);
	if (clicked) {
		clicked = false;
		unit new_build;
		new_build.pos = grid + mappos;
		buildings.push_back( new_build );
		return (false);
		}
    return (true);
}

void move_units( std::vector<unit> &units ) {

    for (auto &vehicle : units) {

	// move units to their destination, one step at a time
	Vec2 dir = vehicle.dest - vehicle.pos;
	//normalize
	dir.x = dir.x / MAX(1,abs(dir.x));
	dir.y = dir.y / MAX(1,abs(dir.y));

	vehicle.angle = atan2(dir.y,dir.x);
	vehicle.pos += dir;
    }
}
void init() {
    set_screen_mode(ScreenMode::hires);
    credits = 500;

    channels[0].waveforms = Waveform::WAVE;
    channels[0].wave_buffer_callback = &buffCallBack;  

    backdrop = Surface::load(map1);
    dunesprites = Surface::load(spritesheet);

    //spawn quads at random locations
    for (int i=0;i<25;i++) {
	    unit quad;
	    int size = MAPSIZE * 2;
	    quad.pos = Vec2(rand() % size, rand() % size);
	    quad.dest = Vec2(rand() % size, rand() % size);
	    quad.enemy = rand() % 2;
	    quads.push_back(quad);
    }
}
void render(uint32_t time) {
static int commanding=-1, placing_building, building_type;

static std::string status;
Rect windtrap = Rect(0,19,32,30);
Rect refinery = Rect(0,49,42,30);

Rect windtrap_pic = Rect (33,18,41,32);
Rect refinery_pic = Rect (44,49,41,32);

Rect quadsprite = Rect(0,0,16,16);
Rect trikesprite = Rect(40,0,16,16);
Rect yardsprite = Rect(72,19,32,30);

Rect harvestersprite = Rect(68,81,40,30);
Rect harvestingsprite = Rect(68,81,60,30);

    // Draw map
    screen.stretch_blit(backdrop,Rect(mappos.x/2,mappos.y/2,screen.bounds.w/2,screen.bounds.h/2),Rect(0,0,screen.bounds.w,screen.bounds.h));
    screen.pen = Pen(255,255,255);

    if ( time < 4000) {
	screen.text ("X : toggle resolution", minimal_font, Vec2(50,screen.bounds.h /2) );
	screen.text ("Y : show mini map", minimal_font, Vec2(50,20 + screen.bounds.h /2) );
	screen.text ("A : comand unit", minimal_font, Vec2(50,30 + screen.bounds.h /2) );
    }

    // move units ( quads  and harvesters
    move_units( quads); 
    // Draw quads
    int thisquad = 0;
    for (auto &quad : quads) {

	if (near(cursorpos,quad.pos)) {
		if (!quad.enemy) status = "Harkonnen Quad";
		else status = "Atredies Trike";

    		if (clicked) {
			quad.dest = quad.pos;
			commanding = thisquad;
			clicked = 0;
			status = "Yes, sir !";
			play_wav(yessir,yessir_length);
		}
	}
	// don't draw offscreen
	Vec2 screenpos = quad.pos - mappos;
	if (screenpos.x > 0 && screenpos.y > 0 && screenpos.x < screen.bounds.w && screenpos.y < screen.bounds.h) {
           Rect unitsprite;
	   if (quad.enemy == 1)  unitsprite = trikesprite;  
	   else unitsprite = quadsprite; 

	   blit_rotate_sprite(dunesprites,unitsprite,quad.angle,screenpos);
           if (thisquad == commanding) { draw_box(screenpos.x - 8,screenpos.y - 8,18,18); }
	}
	thisquad++;
    }

    // command quads
    if (clicked && commanding != -1) {
	    		clicked = 0;
			status = "Acknowledged !";
			play_wav(ack,ack_length);
			quads[commanding].dest = mappos + cursorpos;
			commanding = -1;
		}

    // construction yard
    Vec2 yardpos = Vec2(160,160);
    screen.blit(dunesprites,yardsprite,yardpos - mappos);
    if (near (cursorpos,yardpos)) {
	    status = "Construction yard";
	            screen.blit(dunesprites,windtrap_pic,Vec2(screen.bounds.w-44,0));
	            screen.text ("Windtrap", minimal_font,Vec2(screen.bounds.w-44,32));
		    if (windtraps.size() > 0) {
	            	screen.blit(dunesprites,refinery_pic,Vec2(screen.bounds.w-44,42));
	            	screen.text ("Refinery", minimal_font,Vec2(screen.bounds.w-44,72));

		    	if (options) draw_box(screen.bounds.w-46,38,45,40);
			else draw_box(screen.bounds.w-46,0,45,40);
		    }

	    if (clicked) {
		    int cost;
		    clicked = false;
		    building_type = options;
	    	    if (building_type == 0) { cost = 100; }
	    	    if (building_type == 1) { cost = 300; }

		    if (credits >= cost) {
			    credits -= cost;
			    placing_building = true;
		    }
		    else status = "Not enough credits!";
	    }
    }

    if (placing_building) {
	    if (building_type == 0) { placing_building = place_building (windtrap,windtraps); }
       	    if (building_type == 1) { placing_building = place_building (refinery,refineries); }
    	    if (pressed(Button::B)) placing_building = false;
    }

    for (auto r : refineries) {
    	screen.blit(dunesprites,refinery, r.pos - mappos);
    	if (near (cursorpos,r.pos)) { status = "Spice Refinery"; }
    }

    for (auto w : windtraps) {
    	screen.blit(dunesprites,windtrap, w.pos - mappos);
    	if (near (cursorpos,w.pos)) { status = "Windtrap generator"; }
    }
    // harvesters
    if (refineries.size() > 0  && harvesters.size() == 0) {
		    unit harvester;
		    harvester.pos = refineries[0].pos;
		    harvester.dest = Vec2(300,120);
		    harvesters.push_back(harvester);
	    }
    if (refineries.size() > 0 )  {
    	    move_units( harvesters ); 
	    unit harvester = harvesters[0];
	    if (harvesters[0].pos != harvester.dest)
	    	blit_rotate_sprite(dunesprites,harvestersprite,harvester.angle - (3.14 / 2 ),harvester.pos - mappos);
	    else {
		Rect sprite;
		if (time % 3) sprite = harvestersprite;
		else  sprite = harvestingsprite;
		screen.blit(dunesprites,sprite,harvester.pos - mappos);
	    }

    }
    draw_cursor(cursorpos);

    // status text
    screen.text ("Credits: " + std::to_string(credits), minimal_font,Vec2(0,0));
    screen.text(status, minimal_font, Vec2(0,screen.bounds.h - 10));

    // Draw mini map
    if (pressed(Button::Y))  {
	    int mmsize = screen.bounds.w / 2.5;
	    Rect bottomright = Rect(screen.bounds.w-mmsize,screen.bounds.h-mmsize,mmsize,mmsize);
	    screen.stretch_blit(backdrop,Rect(1,0,MAPSIZE,MAPSIZE),bottomright);
    }
}

void update(uint32_t time) {
static Vec2 joypos;

    center = Vec2(screen.bounds.w / 2, screen.bounds.h / 2);

    int cspeed = screen.bounds.w / 120;
    if (pressed(Button::DPAD_LEFT))  joypos.x -= cspeed;
    if (pressed(Button::DPAD_RIGHT)) joypos.x += cspeed;
    if (pressed(Button::DPAD_UP))    joypos.y -= cspeed;
    if (pressed(Button::DPAD_DOWN))  joypos.y += cspeed;

    joypos.x = std::clamp(joypos.x,-center.x,center.x);
    joypos.y = std::clamp(joypos.y,-center.y,center.y);

    cursorpos = joypos + center;

    if ( cursorpos.x < 10 ) mappos.x--;
    if ( cursorpos.y < 10 ) mappos.y--;
    if ( cursorpos.x > screen.bounds.w - 10 ) mappos.x++;
    if ( cursorpos.y > screen.bounds.h - 10 ) mappos.y++;

    mappos.x = std::clamp((int)mappos.x,0,(int)(MAPSIZE * 2 - screen.bounds.w));
    mappos.y = std::clamp((int)mappos.y,0,(int)(MAPSIZE * 2 - screen.bounds.h));

    if (buttons.released & Button::A) { clicked = 1; }
    if (buttons.released & Button::X) {
	    lowres = !lowres;
	    if (lowres) set_screen_mode(ScreenMode::lores); 
	    else set_screen_mode(ScreenMode::hires);
	    }
    if (buttons.released & Button::B) options = !options;
}
