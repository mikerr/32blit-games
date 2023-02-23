#include "32blit.hpp"
#include "assets.hpp"
using namespace blit;

Surface *boardimage, *allpieces;
Vec2 boardpos = Vec2(40,10);
int x,y;

typedef struct pieceobj {
	Rect pic;
	Vec2 pos;

} piece;

std::vector <pieceobj> pieces;

Vec2 grid2screen(Vec2 boardpos, Vec2 gridpos) {
float size = 220 / 8;
    Vec2 screenpos = boardpos + (gridpos * size) + Vec2(5,-5);

    return (screenpos);
}

Rect getimage (char p) {
    Rect img = Rect(0,0,10,10);;
    if (p == 'R') img = Rect(67,42,60,130);
    if (p == 'r') img = Rect(67,210,60,130);
    if (p == 'N') img = Rect(126,42,65,130);
    if (p == 'n') img = Rect(126,210,65,130);
    if (p == 'B') img = Rect(190,42,65,130);
    if (p == 'b') img = Rect(190,210,65,120);
    if (p == 'K') img = Rect(324,42,65,120);
    if (p == 'k') img = Rect(324,210,65,120);
    if (p == 'Q') img = Rect(253,45,65,120);
    if (p == 'q') img = Rect(253,210,65,120);
    if (p == 'P') img = Rect(17,42,50,120);
    if (p == 'p') img = Rect(17,220,50,120);
    return(img);
}

void set_boardline(std::string boardline, int y) {
    pieceobj piece;
    for (int x=0;x < 8; x++){
	    char p = boardline[x];
	    piece.pos = Vec2(x,y);
	    piece.pic = getimage(p);
    	    pieces.push_back(piece);
    }
}
void init() {

    set_screen_mode(ScreenMode::hires);

    boardimage = Surface::load(board2);
    allpieces = Surface::load(pieces1);

    // setup board
    set_boardline ( "RNBKQBNR",0);
    set_boardline ( "PPPPPPPP",1);
    set_boardline ( "pppppppp",6);
    set_boardline ( "rnbkqbnr",7);
}

void render(uint32_t time) {

    screen.stretch_blit(boardimage,Rect(0,0,boardimage->bounds.w,boardimage->bounds.h), Rect(boardpos.x,boardpos.y,220,220));

    for (auto piece: pieces) {
		Vec2 screenpos = grid2screen(boardpos,piece.pos);
    		screen.stretch_blit(allpieces, piece.pic, Rect(screenpos.x,screenpos.y,20,30));
		}
    screen.pen = Pen(255,255,255);
    screen.line(grid2screen(boardpos,Vec2(x,y)),grid2screen(boardpos,Vec2(x+1,y)) );	
}

void update(uint32_t time) {
	if (pressed(Button::DPAD_LEFT)) x--;
	if (pressed(Button::DPAD_RIGHT)) x++;
	if (pressed(Button::DPAD_UP)) y--;
	if (pressed(Button::DPAD_DOWN))  y++;
}

