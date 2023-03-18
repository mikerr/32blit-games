#include "32blit.hpp"
#include "assets.hpp"
using namespace blit;

#define BOARDSIZE 220
Surface *boardimage, *allpieces;
Vec2 boardpos, cursor,compcursor;
int bstyle,turn;

typedef struct pieceobj {
	Rect pic;
	Vec2 pos;
	char type;
	int selected;

} pieceobj;
std::vector <pieceobj> pieces;

typedef struct movetype { Vec2 from; Vec2 to; } movetype;
std::vector <movetype> moves;

// grid coords to screen
Vec2 grid2screen(int a, int b){
    float size = (BOARDSIZE / 8)  + 0.9f;
    Vec2 gridpos = Vec2(a,b);
    Vec2 screenpos = boardpos + (gridpos * size);

    return (screenpos);
}

// Return sprite image from chess notation e.g. N = black knight image
Rect getimage (char p) {
    Rect img = Rect(0,0,20,20);

    if (p == 'R') img = Rect(22,76,18,28);
    if (p == 'r') img = Rect(21,22,18,28);
    if (p == 'N') img = Rect(40,65,18,40);
    if (p == 'n') img = Rect(40,14,18,40);
    if (p == 'B') img = Rect(62,62,18,40);
    if (p == 'b') img = Rect(62,14,18,40);
    if (p == 'K') img = Rect(103,57,20,50);
    if (p == 'k') img = Rect(103,0,20,50);
    if (p == 'Q') img = Rect(81,67,18,40);
    if (p == 'q') img = Rect(81,9,18,40);
    if (p == 'P') img = Rect(4,75,17,28);
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
void screenrect (Point a, Point b, Point c, Point d) { screen.line(a,b); screen.line(b,c); screen.line(c,d); screen.line(d,a); }

void draw_cursor(Vec2 cursor) {
    // draw border round selected square
    
    Vec2 a = grid2screen(cursor.x,cursor.y);
    Vec2 b = grid2screen(cursor.x+1,cursor.y);
    Vec2 c = grid2screen(cursor.x+1,cursor.y+1);
    Vec2 d = grid2screen(cursor.x,cursor.y+1);
			  
    screenrect (a,b,c,d);
}

void message (std::string text,Vec2 pos) {
           screen.pen = Pen(0,0,0);
	   screen.rectangle(Rect(pos.x - 5,pos.y,text.size() * 6,10));
           screen.pen = Pen(255,255,255);
	   screen.text(text,minimal_font,pos);
}

void help_screen() {
	   screen.pen = Pen(255,255,255);
	   screen.text("Controls",minimal_font,Vec2(50,60));
	   screen.text("A: move piece",minimal_font,Vec2(50,80));
	   screen.text("B: auto move",minimal_font,Vec2(50,100));
	   screen.text("X: change board style",minimal_font,Vec2(50,120));
	   screen.text("Y: show text board",minimal_font,Vec2(50,140));
}

int vacant(Vec2 pos) {
    for (auto piece: pieces) 
	    if (piece.pos == pos) return false;
    return true;
}

int onboard(Vec2 pos) { return (pos.x >= 0 & pos.y >= 0 && pos.x < 8 && pos.y < 8); }

int takeenemy(Vec2 from, Vec2 dest) {
	int myside, destside;
	destside = -1;
	for (auto piece : pieces) {
		if (piece.pos == from) myside = isupper(piece.type);
		if (piece.pos == dest) destside = isupper(piece.type);
	}
	if (destside != -1 && destside != myside) return true;
	else return false;
}

int addmove (Vec2 from,Vec2 to) {
	Vec2 dest = from + to ;
   if (onboard(dest) && (takeenemy(from,dest) || vacant(dest))) {
	    moves.push_back({from,dest});
   	    return true;
	    }
   else return (false);
}
void pawnaddmove (Vec2 from,Vec2 to) {
	Vec2 dest = from + to ;
   if (onboard(dest) && vacant(dest))
	    moves.push_back({from,dest});
}

void knightmoves(Vec2 pos) {
	addmove (pos, Vec2(1,2));
	addmove (pos, Vec2(1,-2));
	addmove (pos, Vec2(-1,2));
	addmove (pos, Vec2(-1,-2));
	addmove (pos, Vec2(2,1));
	addmove (pos, Vec2(2,-1));
	addmove (pos, Vec2(-2,1));
	addmove (pos, Vec2(-2,-1));
}

void rookmoves(Vec2 pos) {
	for (int n=1; n < 8 && addmove(pos,Vec2( 0, n)); n++);	// right
	for (int n=1; n < 8 && addmove(pos,Vec2( 0,-n)); n++);	// left
	for (int n=1; n < 8 && addmove(pos,Vec2( n, 0)); n++);	// up
	for (int n=1; n < 8 && addmove(pos,Vec2(-n, 0)); n++);	// down
}
void bishopmoves(Vec2 pos) {
	for (int n=1; n < 8 && addmove(pos,Vec2( n,  n)); n++);	// up-right
	for (int n=1; n < 8 && addmove(pos,Vec2( n, -n)); n++);	// up-left
	for (int n=1; n < 8 && addmove(pos,Vec2( -n, n)); n++); //down-right
	for (int n=1; n < 8 && addmove(pos,Vec2( -n,-n)); n++); //down-left
}

int inCheck(char king) { 
	// get King position
	Vec2 kingpos;
	for (auto piece : pieces) 
		if (piece.type == king) kingpos = piece.pos;
	// if any valid moves would "take" the king you're in check
	for (auto move : moves) 
		if (move.to == kingpos) return true;
	return false;
}

void get_black_moves() {
    moves.clear();
    for (auto piece: pieces) {
	    if (piece.type == 'P') {
		    pawnaddmove (piece.pos, Vec2(0,1));
		    if (piece.pos.y == 1) pawnaddmove(piece.pos,Vec2(0,-2));
		    if (takeenemy(piece.pos,piece.pos+Vec2(1,1))) addmove(piece.pos,Vec2(1,1));
		    if (takeenemy(piece.pos,piece.pos+Vec2(-1,1))) addmove(piece.pos,Vec2(-1,1));
	    }
	    if (piece.type == 'R') rookmoves(piece.pos);
	    if (piece.type == 'N') knightmoves(piece.pos);
	    if (piece.type == 'B') bishopmoves(piece.pos);
	    if (piece.type == 'Q') {
		    rookmoves(piece.pos);
		    bishopmoves(piece.pos);
	    }
    }
}

void get_white_moves() {
    moves.clear();
    for (auto piece: pieces) {
	    if (piece.type == 'p') {
		    pawnaddmove (piece.pos, Vec2(0,-1));
		    if (piece.pos.y == 6) pawnaddmove(piece.pos,Vec2(0,-2));
		    if (takeenemy(piece.pos,piece.pos+Vec2(1,-1))) addmove(piece.pos,Vec2(1,-1));
		    if (takeenemy(piece.pos,piece.pos+Vec2(-1,-1))) addmove(piece.pos,Vec2(-1,-1));
	    }
	    if (piece.type == 'r') rookmoves(piece.pos);
	    if (piece.type == 'n') knightmoves(piece.pos);
	    if (piece.type == 'b') bishopmoves(piece.pos);
	    if (piece.type == 'q') {
		    rookmoves(piece.pos);
		    bishopmoves(piece.pos);
	    }
    }
}

void do_move(){
    movetype takemove,move;
    int taking=0;
    for (auto move: moves) {
	//favour taking a piece if possible
    		if (takeenemy(move.from,move.to)) {
			takemove = move;
			taking = true;
			break;
		}
    }
    if (taking) {
	    	// remove enemy piece
	    	int num = 0;
    	    	for (auto &piece: pieces) {
	    		if (piece.pos == takemove.to) break;
			num++;
    			}
	    	pieces.erase(pieces.begin()+num);
		move = takemove;
	    	}
    else  //else just pick any random valid move
    		move = moves[rand() % moves.size()];

    //move the piece
    for (auto &piece: pieces) 
	  if (piece.pos == move.from) piece.pos = move.to;
    compcursor = move.to;
    turn = !turn;
}

void reset_game() {
    pieces.clear();
    // setup board
    set_boardline ( "RNBKQBNR",0);
    set_boardline ( "PPPPPPPP",1);
    set_boardline ( "pppppppp",6);
    set_boardline ( "rnbkqbnr",7);
    
    cursor = Vec2(4,6);
}
void init() {
    set_screen_mode(ScreenMode::hires);

    boardimage = Surface::load(board1);
    allpieces = Surface::load(pieces1);

    reset_game();
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
		if (pressed(Button::Y)) {
			std::string s(1,piece.type);
			screen.text(s,minimal_font,screenpos + Vec2(10,10));
		}
		else screen.stretch_blit(allpieces, piece.pic, Rect(screenpos.x,screenpos.y,22,28));
		}

   screen.pen = Pen(255,255,255);
   draw_cursor(cursor);

   if ((inCheck('k')) || inCheck('K')) message("CHECK!",Vec2(100,100));
   if (time - start < 2000) help_screen();
}

void update(uint32_t time) {
static uint32_t lasttime,lasttime2;

     if (time - lasttime2 > 20){
	     lasttime2 = time;
	     if (turn == 1) {
		     get_black_moves();
		     int num = moves.size();
		     if (!num) reset_game(); //white wins
		     else do_move();
	     } else {
		     get_white_moves();
		     int num = moves.size();
		     if (!num) reset_game(); //black wins
		     //else do_move();
	}
	     int num = pieces.size();
	     if (num < 3) reset_game();
     }

    if (time - lasttime > 70) {
	if (pressed(Button::DPAD_LEFT)  && cursor.x > 0) cursor.x--;
	if (pressed(Button::DPAD_RIGHT) && cursor.x < 7) cursor.x++;
	if (pressed(Button::DPAD_UP)    && cursor.y > 0) cursor.y--;
	if (pressed(Button::DPAD_DOWN)  && cursor.y < 7) cursor.y++;
	lasttime = time;
	}

    // start/finish move ... pickup drop piece
    if (buttons.released & Button::A) {
		for ( auto &p : pieces) 
			//only pickup white pieces
			if (p.pos == cursor && !isupper(p.type)) p.selected = 1;
			else { //drop - move the piece
				if (p.selected ) { 
					// check its a valid move
					get_white_moves();
				        for (auto move : moves) {
						if (move.from == p.pos && move.to == cursor) {
	    						// remove enemy piece
	    						int num = 0;
    	    						for (auto piece: pieces) {
	    							if (piece.pos == cursor) break;
								num++;
    								}
							p.pos = cursor;
							p.selected = 0;
	    						if (num < pieces.size()) pieces.erase(pieces.begin()+num);
							turn = !turn;
							return;
						}
					}
				p.selected = 0;
				}
		}
    }
    //auto move
    if (buttons.released & Button::B) 
		do_move();

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
