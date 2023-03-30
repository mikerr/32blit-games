#include "32blit.hpp"
#include "assets.hpp"
using namespace blit;

#define BOARDSIZE 220
Surface *boardimage, *allpieces;
Point boardpos, cursor;
int bstyle,turn;
char lasttakenpiece;
std::string status;

typedef struct piece_t {
	Rect pic;
	Point pos;
	char type;
	int selected;

} piece_t;

std::vector <piece_t> pieces;

typedef struct move_t { Point from; Point to; char piecetaken;} move_t;
std::vector <move_t> moves, movehistory;
move_t lastmove;


// grid coords to screen
Point grid2screen(int a, int b){
    float size = (BOARDSIZE / 8)  + 0.9f;
    Point gridpos = Point(a,b);
    Point screenpos = boardpos + (gridpos * size);

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
    piece_t piece;
    for (int x=0;x < 8; x++){
	    char p = boardline[x];
	    piece.pos = Point(x,y);
	    piece.type = p;
	    piece.pic = getimage(p);
	    piece.selected = 0;
    	    pieces.push_back(piece);
    }
}

// draw unfilled rectangle
void screenrect (Point a, Point b, Point c, Point d) { screen.line(a,b); screen.line(b,c); screen.line(c,d); screen.line(d,a); }

void draw_cursor(Point cursor) {
    // draw border round selected square
    
    Point a = grid2screen(cursor.x,cursor.y);
    Point b = grid2screen(cursor.x+1,cursor.y);
    Point c = grid2screen(cursor.x+1,cursor.y+1);
    Point d = grid2screen(cursor.x,cursor.y+1);
			  
    screenrect (a,b,c,d);
}

void show_text (std::string text,Point pos) {
           screen.pen = Pen(0,0,0);
	   screen.rectangle(Rect(pos.x - 5,pos.y,text.size() * 6,10));
           screen.pen = Pen(255,255,255);
	   screen.text(text,minimal_font,pos);
}

void help_screen() {
	   screen.pen = Pen(255,255,255);
	   screen.text("Controls",minimal_font,Point(50,60));
	   screen.text("A: move piece",minimal_font,Point(50,80));
	   screen.text("B: undo last move",minimal_font,Point(50,100));
	   screen.text("X: change board style",minimal_font,Point(50,120));
	   screen.text("Y: show text board",minimal_font,Point(50,140));
}

// Chess functions

bool vacant(Point pos) {
    for (auto piece: pieces) 
	    if (piece.pos == pos) return false;
    return true;
}

bool onboard(Point pos) { return (pos.x >= 0 && pos.y >= 0 && pos.x < 8 && pos.y < 8); }

bool takeenemy(Point from, Point dest) {
	// check if move is taking an enemy piece
	int myside = 0;
	int destside = -1;
	for (auto piece : pieces) {
		if (piece.pos == from) myside = isupper(piece.type);
		if (piece.pos == dest) destside = isupper(piece.type);
	}
	if (destside != -1 && destside != myside) return true;
	else return false;
}

void pawnaddmove (Point from,Point to) {
Point dest = from + to ;
   if (onboard(dest) && vacant(dest))
	    moves.push_back({from,dest});
}

bool addmove (Point from,Point to) {
Point dest = from + to ;
   if (onboard(dest) && vacant(dest)) {
	    moves.push_back({from,dest});
   	    return true;
	    }
   if (takeenemy(from,dest)){
	    moves.push_back({from,dest});
   	    return false;
   }
   return false;
}

void kingmoves(Point pos) {
	addmove (pos, Point(1,0));
	addmove (pos, Point(1,1));
	addmove (pos, Point(1,-1));
	addmove (pos, Point(0,1));
	addmove (pos, Point(-1,1));
	addmove (pos, Point(-1,0));
	addmove (pos, Point(-1,-1));
	addmove (pos, Point(0,-1));
}
void knightmoves(Point pos) {
	addmove (pos, Point(1,2));
	addmove (pos, Point(1,-2));
	addmove (pos, Point(-1,2));
	addmove (pos, Point(-1,-2));
	addmove (pos, Point(2,1));
	addmove (pos, Point(2,-1));
	addmove (pos, Point(-2,1));
	addmove (pos, Point(-2,-1));
}

void rookmoves(Point pos) {
	for (int n=1; n < 8 && addmove(pos,Point( 0, n)); n++);	// right
	for (int n=1; n < 8 && addmove(pos,Point( 0,-n)); n++);	// left
	for (int n=1; n < 8 && addmove(pos,Point( n, 0)); n++);	// up
	for (int n=1; n < 8 && addmove(pos,Point(-n, 0)); n++);	// down
}
void bishopmoves(Point pos) {
	for (int n=1; n < 8 && addmove(pos,Point( n,  n)); n++);	// up-right
	for (int n=1; n < 8 && addmove(pos,Point( n, -n)); n++);	// up-left
	for (int n=1; n < 8 && addmove(pos,Point( -n, n)); n++); //down-right
	for (int n=1; n < 8 && addmove(pos,Point( -n,-n)); n++); //down-left
}

bool valid_move(Point from, Point to) {
    for (auto move : moves) 
		if (move.from == from && move.to == to)  return true;
    return false;
}
void remove_piece(Point p){
	// find & remove enemy piece
	 int num = 0;
    	 for (auto &piece: pieces) {
	    	if (piece.pos == p) {
			lasttakenpiece = piece.type;
			break;
		}
		num++;
		}
	pieces.erase(pieces.begin()+num);
}

bool inCheck(char king) { 
	// get King position
	Point kingpos;
	for (auto piece : pieces) 
		if (piece.type == king) kingpos = piece.pos;
	// if any valid moves would take the king, then you're in check
	for (auto move : moves) 
		if (move.to == kingpos) return true;
	return false;
}

void get_black_moves() {
    moves.clear();
    for (auto piece: pieces) {
	    if (piece.type == 'P') {
		    pawnaddmove (piece.pos, Point(0,1));
		    if (piece.pos.y == 1 && vacant(piece.pos + Point(0,1))) pawnaddmove(piece.pos,Point(0,2));
		    if (takeenemy(piece.pos,piece.pos+Point(1,1))) addmove(piece.pos,Point(1,1));
		    if (takeenemy(piece.pos,piece.pos+Point(-1,1))) addmove(piece.pos,Point(-1,1));
	    }
	    if (piece.type == 'K') kingmoves(piece.pos);
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
		    pawnaddmove (piece.pos, Point(0,-1));
		    if (piece.pos.y == 6) pawnaddmove(piece.pos,Point(0,-2));
		    if (takeenemy(piece.pos,piece.pos+Point(1,-1))) addmove(piece.pos,Point(1,-1));
		    if (takeenemy(piece.pos,piece.pos+Point(-1,-1))) addmove(piece.pos,Point(-1,-1));
	    }
	    if (piece.type == 'k') kingmoves(piece.pos);
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
    move_t move;
    lasttakenpiece = 0;
    for (auto m: moves)  {
	//favour taking a piece if possible
    		if (takeenemy(m.from,m.to) && (rand() % 10 > 2)) { 
			move = m; 
			remove_piece(m.to);
			break; 
		} else 
    			//or just pick any random valid move
    			move = moves[rand() % moves.size()];
    }
    //move the piece
    for (auto &piece: pieces) 
	  if (piece.pos == move.from) piece.pos = move.to;
    move.piecetaken = lasttakenpiece;
    lastmove = move;
}

void undo_last_move() {
	        // undo move
	        if (!movehistory.size()) return;
		lastmove = movehistory.back(); 
		for (auto &p : pieces) 
			if (p.pos == lastmove.to) p.pos = lastmove.from;
		// add taken piece back to board
		if (lastmove.piecetaken != 0) {
			piece_t p;
			p.type = lastmove.piecetaken;
			p.pic = getimage(lastmove.piecetaken);
			p.pos = lastmove.to;
			p.selected = 0;
			pieces.push_back(p);
		}
		movehistory.pop_back(); 
}
void reset_game() {
    pieces.clear();
    // setup board
    set_boardline ( "RNBKQBNR",0);
    set_boardline ( "PPPPPPPP",1);
    set_boardline ( "pppppppp",6);
    set_boardline ( "rnbqkbnr",7);
    
    cursor = Point(4,6);
}

char *move_notation(move_t move) {
	char notation[4];
	sprintf((char *)&notation,"%c%d%c%d",'a' + move.from.x, 8 - move.from.y,'a'+ move.to.x, 8 - move.to.y);
	return (notation);
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
		Point screenpos,pos;
		if (piece.selected) pos = cursor;
		else pos = piece.pos;
		screenpos = grid2screen(pos.x,pos.y);
		screen.stretch_blit(allpieces, piece.pic, Rect(screenpos.x,screenpos.y,22,28));
		}

   screen.pen = Pen(255,255,255);

   //draw labels
   for (int i=0; i < 8;i++) {
	   show_text(std::string(1,'a' + i),grid2screen(i,8));
	   show_text(std::to_string(i+1),grid2screen(0,7-i));
   }

   draw_cursor(cursor);

   screen.pen = Pen(200,200,0);
   draw_cursor(lastmove.from);
   draw_cursor(lastmove.to);

   screen.pen = Pen(255,255,255);
   show_text(status,Point(100,100));
   if (time - start < 2000) help_screen();
}

void update(uint32_t time) {
static uint32_t lasttime;
int WHITE = 0;
int BLACK = 1;

    if (turn == BLACK) {
			bool incheck;
			int count = 0;
	    		//am I in check ?
			do { 
				get_black_moves();
				do_move(); 
				movehistory.push_back(lastmove);

				get_white_moves();
    			        incheck = inCheck('K');
    			        if (incheck) undo_last_move();
				if (++count > 500) {
					status = "checkmate";
					break;
				}
    			} while (incheck);
			turn = WHITE;
    			}
    if (time - lasttime > 100) {
	if ((pressed(Button::DPAD_LEFT)  || joystick.x < 0) && cursor.x > 0) cursor.x--;
	if ((pressed(Button::DPAD_RIGHT) || joystick.x > 0) && cursor.x < 7) cursor.x++;
	if ((pressed(Button::DPAD_UP)    || joystick.y < 0) && cursor.y > 0) cursor.y--;
	if ((pressed(Button::DPAD_DOWN)  || joystick.y > 0) && cursor.y < 7) cursor.y++;
	lasttime = time;
	}
    // start/finish move ... pickup drop piece
    if (buttons.released & Button::A) {
		for ( auto &p : pieces)  {
			if (p.pos == cursor && p.selected) {
				//if start and end are same, drop the piece
				p.selected = 0;
				break;
			}
			//only pickup white pieces
			if (p.pos == cursor && !isupper(p.type)) p.selected = 1;
			else { //drop - move the piece
				if (p.selected ) { 
					move_t move = {p.pos,cursor,0};
					// check its a valid move
					get_white_moves();
				        if (valid_move(move.from,move.to)) {
							//check for take 
    							int taking = takeenemy(move.from,move.to);
							
							// do move
							p.pos = move.to;
							p.selected = 0;
	    						if (taking) { 
								remove_piece(move.to);
								move.piecetaken = lasttakenpiece;
							}
							movehistory.push_back(move);
							turn = BLACK;
							return;
					}
				p.selected = 0;
				}
			}
		}
    }
    if (buttons.released & Button::B) undo_last_move(); 
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
