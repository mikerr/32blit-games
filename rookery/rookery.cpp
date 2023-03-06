#include "32blit.hpp"
#include "assets.hpp"
using namespace blit;

#define BOARDSIZE 220
Surface *boardimage, *allpieces;
Vec2 boardpos, cursor;
int bstyle;
float zoom; 

typedef struct pieceobj {
	Rect pic;
	Vec2 pos;
	char type;
	int selected;

} pieceobj;
std::vector <pieceobj> pieces;

// grid coords to screen
Vec2 grid2screen(int a, int b){
    float size = (BOARDSIZE / 8)  + 0.5f;
    Vec2 gridpos = Vec2(a,b);
    Vec2 screenpos = boardpos + (gridpos * size);

    return (screenpos);
}

// Return sprite image from chess notation e.g. N = black knight image
Rect getimage (char p) {
    Rect img = Rect(0,0,20,20);

    if (p == 'R') img = Rect(21,76,18,28);
    if (p == 'r') img = Rect(21,22,18,28);
    if (p == 'N') img = Rect(40,70,18,40);
    if (p == 'n') img = Rect(40,14,18,40);
    if (p == 'B') img = Rect(62,70,18,40);
    if (p == 'b') img = Rect(62,14,18,40);
    if (p == 'K') img = Rect(103,57,20,50);
    if (p == 'k') img = Rect(103,0,20,50);
    if (p == 'Q') img = Rect(81,67,18,40);
    if (p == 'q') img = Rect(81,9,18,40);
    if (p == 'P') img = Rect(4,81,17,28);
    if (p == 'p') img = Rect(3,27,17,28);

    return(img);
}

// place pieces according to a line of notation 
void set_boardline(std::string boardline, int y) {
    pieceobj piece;
    for (int x=0;x < 8; x++){
	    char p = boardline[x];
	    piece.pos = Vec2(x,y);
	    piece.type = p;
	    piece.pic = getimage(p);
	    piece.selected = 0;
    	    pieces.push_back(piece);
    }
}

// draw unfilled rectangle
void screenrect (Point a, Point b, Point c, Point d) {
    screen.line(a,b); screen.line(b,c); screen.line(c,d); screen.line(d,a);
}

void draw_cursor(Vec2 cursor) {
    // draw border round selected square
    
    Vec2 a = grid2screen(cursor.x,cursor.y);
    Vec2 b = grid2screen(cursor.x+1,cursor.y);
    Vec2 c = grid2screen(cursor.x+1,cursor.y+1);
    Vec2 d = grid2screen(cursor.x,cursor.y+1);
			  
    screen.pen = Pen(255,255,255);
    screenrect (a,b,c,d);
}

void help_screen() {
   	   screen.pen = Pen(255,255,255);
	   screen.text("Controls",minimal_font,Vec2(10,60));
	   screen.text("A: move piece",minimal_font,Vec2(10,80));
	   screen.text("B: cancel move",minimal_font,Vec2(10,100));
	   screen.text("X: change board style",minimal_font,Vec2(10,120));
	   screen.text("Y: swap sides",minimal_font,Vec2(10,140));
}

void init() {

    set_screen_mode(ScreenMode::hires);

    boardimage = Surface::load(board1);
    allpieces = Surface::load(pieces1);

    // setup board
    set_boardline ( "RNBKQBNR",0);
    set_boardline ( "PPPPPPPP",1);
    set_boardline ( "pppppppp",6);
    set_boardline ( "rnbkqbnr",7);

    cursor = Vec2(4,6);
}

void render(uint32_t time) {
static uint32_t start;	

    screen.pen = Pen(0,0,0);
    screen.clear();

    screen.stretch_blit(boardimage,Rect(0,0,boardimage->bounds.w,boardimage->bounds.h), Rect(boardpos.x,boardpos.y,BOARDSIZE,BOARDSIZE));

    //draw pieces
    for (auto piece: pieces) {
		Vec2 screenpos,pos;
		if (piece.selected) pos = cursor;
		else pos = piece.pos;
		screenpos = grid2screen(pos.x,pos.y);
		screen.stretch_blit(allpieces, piece.pic, Rect(screenpos.x,screenpos.y,22,28));
		}

   draw_cursor(cursor);

   if (time - start < 3000) help_screen();
}

void update(uint32_t time) {
static uint32_t lasttime,lasttime2;

     if (time - lasttime2 > 150) {
	     lasttime2 = time;
     }

    if (time - lasttime > 70) {
	if (pressed(Button::DPAD_LEFT) &&  cursor.x > 0 ) cursor.x--;
	if (pressed(Button::DPAD_RIGHT) && cursor.x < 7) cursor.x++;
	if (pressed(Button::DPAD_UP) && cursor.y > 0 ) cursor.y--;
	if (pressed(Button::DPAD_DOWN) && cursor.y < 7) cursor.y++;
	lasttime = time;
	}

    // start/finish move ... pickup drop piece
    if (buttons.released & Button::A) {
		for ( auto &p : pieces) 
			if (p.pos == cursor) p.selected = 1;
			else {
				//drop - move the piece
				if (p.selected ) p.pos = cursor;
				p.selected = 0;
				}
		}
    //cancel move
    if (buttons.released & Button::B) 
		for ( auto &p : pieces) p.selected = 0;

    //flip board round
    if (buttons.released & Button::Y) {
    		for (auto &p: pieces) {
			p.pos = Vec2(7,7) - p.pos;
		}
    }
    //change board image
    if (buttons.released & Button::X) {
	    	bstyle = !bstyle;
		// free up memory
		delete[] boardimage->data;
		delete[] boardimage->palette;
		//swap the board image
		if (bstyle)  boardimage = Surface::load(board2);
		else boardimage = Surface::load(board1);
    }
}

