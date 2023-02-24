#include "SimpleChess.h"

Board::Board() : inCheck(false) {
	/* -- Initialize board to chess starting position -- */
	unsigned piecerows = 0; unsigned pawnrows = 1; pieceColor color = pieceColor::WHITE;
	for (unsigned c = 0; c < 2; ++c) {
		for (unsigned i = 0; i < 8; ++i) {
			board[pawnrows][i] = new Piece(pieceName::PAWN, color);
			switch (i) {
					case 0: case 7: board[piecerows][i] = new Piece(pieceName::ROOK, color); break;
					case 2: case 5: board[piecerows][i] = new Piece(pieceName::BISHOP, color); break;
					case 1: case 6: board[piecerows][i] = new Piece(pieceName::KNIGHT, color); break;
					case 3: board[piecerows][i] = new Piece(pieceName::QUEEN, color); break;
					case 4: board[piecerows][i] = new Piece(pieceName::KING, color); break;
			}
		}
		piecerows = 7; pawnrows = 6; color = pieceColor::BLACK;
	}
	for (int i=2; i<6; ++i) {
		for (int j=0; j<8; ++j) {
			board[i][j] = new Piece();
		}
	}
}

Board::Board(const Board &b) : inCheck(b.inCheck) {
	for (unsigned i=0; i < 8; ++i) {
		for (unsigned j=0; j < 8; ++j) {
			board[i][j] = new Piece(b.board[i][j]);
		}
	}
}

bool Board::equals(const Game &g) const {
	for (unsigned i=0; i < 8; ++i) {
		for (unsigned j=0; j < 8; ++j) {
			Piece* p = g.getPiece(i, j);
			if (board[i][j]->name != p->name
				|| board[i][j]->color != p->color)
				return false;
		}
	}
	return true;
}

Game::Game() {
	turn = 0;
	previous = NULL;
	fiftyMoveRule = 0;
	state = gameState::PLAYING;
	calculateAllPossibleMoves(pieceColor::WHITE);
}

Game::Game(const Game &g) {
	turn = g.turn;
	previous = g.previous;
	fiftyMoveRule = g.fiftyMoveRule;
	state = g.state;
	position = Board(g.position);
}

Piece* Game::getPiece(const unsigned &x, const unsigned &y) const {
	return (x < 0 || x > 7 || y < 0 || y > 7) ? NULL : position.board[x][y];
}

pieceColor Game::currentPlayer() const {
	return (turn % 2 == 0) ? pieceColor::WHITE : pieceColor::BLACK;
}

pieceColor Game::nextPlayer() const {
	return (turn % 2 == 0) ? pieceColor::BLACK : pieceColor::WHITE;
}

void Game::move(const Move &m) {
	pieceColor player = currentPlayer();
	assert(state == gameState::PLAYING);
	assert(getPiece(m.from_x, m.from_y)->color = player);

	Game* oldposition = new Game(*this);

	movePieceTo(m);
	if (m.promotePiece != pieceName::EMPTY)
		getPiece(m.to_x, m.to_y)->name = m.promotePiece;
	else if (m.castles) {
		if (m.from_y < m.to_y) {	// Queenside Castling
			if (player == pieceColor::WHITE) movePieceTo(Move(7, 7, 7, 4));
			else movePieceTo(Move(0, 7, 0, 4));
		}
		else {						// Kingside Castling
			if (player == pieceColor::WHITE) movePieceTo(Move(7, 0, 7, 2));
			else movePieceTo(Move(0, 0, 0, 2));
		}
	}
	else if (m.enpassant) {
		unsigned direction;
		(player == pieceColor::WHITE) ? direction = -1 : direction = 1;
		position.board[m.to_x + direction][m.to_y] = new Piece();
	}

	++turn;
	position.inCheck = isInCheck();
	calculateAllPossibleMoves(currentPlayer());
	checkIfEndPosition();

	previous = oldposition;
}

void Game::undo() {
	if (previous != NULL) {
		state = previous->state;
		turn = previous->turn;
		fiftyMoveRule = previous->fiftyMoveRule;
		position = Board(previous->position);
		previous = previous->previous;
		calculateAllPossibleMoves(currentPlayer());
	}
}

void Game::movePieceTo(const Move m) {
	Piece* from = getPiece(m.from_x, m.from_y);
	Piece* to = getPiece(m.to_x, m.to_y);

	if (from->name != pieceName::PAWN && to->name != pieceName::EMPTY) ++fiftyMoveRule;

	*to = *from;
	to->moved = true;
	position.board[m.from_x][m.from_y] = new Piece();
}

bool Game::isInCheck() const {
	pieceColor c = nextPlayer();
	for (unsigned i=0; i < 8; ++i) {
		for (unsigned j=0; j < 8; ++j) {
			Piece* attacker = getPiece(i, j);
			if (attacker->color == c) {
				attacker->legalMoves = getPossibleMoves(i, j);
				for (const auto &m : attacker->legalMoves) {
					Piece* attackedSquare = getPiece(m.to_x, m.to_y);
					if (attackedSquare->name == pieceName::KING && attackedSquare->color != c)
						return true;
				}
			}
		}
	}
	return false;
}

void Game::checkIfEndPosition() {
	unsigned bishops, knights;
	unsigned total = 64;
	bool movesLeft = false;
	for (int i=0; i < 8; ++i) {
		for (int j=0; j < 8; ++j) {
			Piece* p = getPiece(i, j);
			switch (p->name) {
				case pieceName::BISHOP: ++bishops; break;
				case pieceName::KNIGHT: ++knights; break;
				case pieceName::EMPTY: --total; break;
				default: break;
			}
			if (!movesLeft && p->color == currentPlayer() && !p->legalMoves.empty()) {
				movesLeft = true;
			}
		}
	}
	if (!movesLeft) {
		if (position.inCheck) {
			switch(currentPlayer()) {
				case pieceColor::WHITE: state = gameState::WON_WHITE; break;
				case pieceColor::BLACK: state = gameState::WON_BLACK; break;
				default: break;
			}
		}
		else state = gameState::DRAW;
	}
	else {
		// fifty move rule
		if (fiftyMoveRule > 50) state = gameState::DRAW;

		// insufficient material
		else if (total == 2
				|| (total == 3 && bishops == 1)
				|| (total == 3 && knights == 1))
			state = gameState::DRAW;

		// threefold repetition
		else {
			unsigned repeats = 0;
			Game* previous_position = previous;
			while (previous_position != NULL && repeats < 3) {
				if (position.equals(*previous_position)) ++repeats;
				previous_position = previous_position->previous;
			}
			if (repeats >= 3) state = gameState::DRAW;
		}
	}
}

void Game::calculateAllPossibleMoves(const pieceColor &player) const {
	for (unsigned i=0; i < 8; ++i) {
		for (unsigned j=0; j < 8; ++j) {
			Piece* p = getPiece(i, j);
			if (p->color == player) p->legalMoves = getLegalMoves(i, j);
		}
	}
}

bool Game::preventsCheck(const Move &m) const {
	Game f = Game(*this);
	f.movePieceTo(m);
	return (!f.isInCheck());
}

std::vector<Move> Game::getLegalMoves(const unsigned &x, const unsigned &y) const {
	std::vector<Move> moves = getPossibleMoves(x, y);
	for (auto m = moves.begin(); m != moves.end(); ) {
		if (!preventsCheck(*m)) moves.erase(m);
		else ++m;
	}
	return moves;
}

std::vector<Move> Game::getPossibleMoves(const unsigned &x, const unsigned &y) const {
	std::vector<Move> legalMoves;
	Piece* p = getPiece(x, y);

	// adds move to legalMoves, but checks if the move is legal for a general piece
	auto addMove = [&] (const Move &m) -> bool {
		Piece* from = getPiece(m.from_x, m.from_y);
		Piece* to = getPiece(m.to_x, m.to_y);
		if (from != NULL && to != NULL				// Piece and destination must be a piece on the board.
				&& from->color != to->color 		// Not taking a piece with thesame color.
				&& m.from_x >= 0 && m.from_x < 8	// Checking out of bounds.
				&& m.from_y >= 0 && m.from_y < 8
				&& m.to_x >= 0 && m.to_x < 8
				&& m.to_y >= 0 && m.to_y < 8) {
			legalMoves.push_back(m);
			return true;
		}
		return false;
	};

	auto isOpponent = [&] (const unsigned &dx, const unsigned &dy) -> bool {
		Piece* opponent = getPiece(dx, dy);
		if (opponent == NULL) return false;
		return (opponent->name != pieceName::EMPTY
				&& opponent->color != p->color);
	};

	switch(p->name) {
		case pieceName::PAWN: {
			unsigned direction;
			(p->color == pieceColor::WHITE) ? direction = 1 : direction = -1;

			/* -- one step -- */
			if (getPiece(x + direction, y)->name == pieceName::EMPTY) {
				if (x + direction == 0 || x + direction == 7) {	// Promoting
					addMove(Move(x, y, x + direction, y, false, false, pieceName::QUEEN));
					addMove(Move(x, y, x + direction, y, false, false, pieceName::ROOK));
					addMove(Move(x, y, x + direction, y, false, false, pieceName::BISHOP));
					addMove(Move(x, y, x + direction, y, false, false, pieceName::KNIGHT));
				}
				else addMove(Move(x, y, x + direction, y));
			}

			/* -- two steps -- */
			if (!p->moved
					&& getPiece(x + direction, y)->name == pieceName::EMPTY
					&& getPiece(x + (2 * direction), y)->name == pieceName::EMPTY) {
				addMove(Move(x, y, x + (2 * direction), y));
			}

			/* -- taking -- */
			for (auto leftOrRight : { -1, 1 }) {
				if (isOpponent(x + direction, y + leftOrRight)) {
					if (x + direction == 0 || x + direction == 7) {	// Promoting
						addMove(Move(x, y, x + direction, y + leftOrRight, false, false, pieceName::QUEEN));
						addMove(Move(x, y, x + direction, y + leftOrRight, false, false, pieceName::ROOK));
						addMove(Move(x, y, x + direction, y + leftOrRight, false, false, pieceName::BISHOP));
						addMove(Move(x, y, x + direction, y + leftOrRight, false, false, pieceName::KNIGHT));
					}
					else addMove(Move(x, y, x + direction, y + leftOrRight));
				}
			}

			/* -- en passant -- */
			bool isEnPassantSquare;
			(direction > 0) ? isEnPassantSquare = (x == 4) : isEnPassantSquare = (x == 3);
			if (isEnPassantSquare) {
				for (auto leftOrRight : { -1, 1 })
					if (isOpponent(x, y + leftOrRight)
							&& getPiece(x, y + leftOrRight)->name == pieceName::PAWN
							&& getPiece(x + direction, y + leftOrRight)->name == pieceName::EMPTY)
						addMove(Move(x, y, x + direction, y + leftOrRight, false, true));
			}
		} break;
		case pieceName::QUEEN:
		case pieceName::ROOK: {
			for (int n=1; n < 9 && addMove(Move(x, y, x + 0, y + n)) && !isOpponent(x + 0, y + n); ++n);	// right
			for (int n=1; n < 9 && addMove(Move(x, y, x + 0, y - n)) && !isOpponent(x + 0, y - n); ++n);	// left
			for (int n=1; n < 9 && addMove(Move(x, y, x + n, y + 0)) && !isOpponent(x + n, y + 0); ++n);	// up
			for (int n=1; n < 9 && addMove(Move(x, y, x - n, y + 0)) && !isOpponent(x - n, y + 0); ++n);	// down
			if (p->name != pieceName::QUEEN) break;
		}
		case pieceName::BISHOP: {
			for (int n=1; n < 9 && addMove(Move(x, y, x + n, y + n)) && !isOpponent(x + n, y + n); ++n);	// up-right
			for (int n=1; n < 9 && addMove(Move(x, y, x + n, y - n)) && !isOpponent(x + n, y - n); ++n);	// up-left
			for (int n=1; n < 9 && addMove(Move(x, y, x - n, y + n)) && !isOpponent(x - n, y + n); ++n);	// down-right
			for (int n=1; n < 9 && addMove(Move(x, y, x - n, y - n)) && !isOpponent(x - n, y - n); ++n);	// down-left
		} break;
		case pieceName::KNIGHT: {
			for (auto dx : { -1, 1 }) {
				for (auto dy : { -1, 1 }) {
					addMove(Move(x, y, x + (dx * 2), y + (dy * 1)));
					addMove(Move(x, y, x + (dx * 1), y + (dy * 2)));
				}
			}
		} break;
		case pieceName::KING: {
			for (auto dx : { -1, 0, 1 }) {
				for (auto dy : { -1, 0, 1}) {
					addMove(Move(x, y, x+dx, y+dy));
				}
			}
			if (!p->moved) {
				/* -- queenside castling -- */
				Piece* rook = (p->color == pieceColor::WHITE) ? getPiece(7, 7) : getPiece(0, 7);
				if (!rook->moved
						&& getPiece(x, 4)->name == pieceName::EMPTY
						&& getPiece(x, 5)->name == pieceName::EMPTY
						&& getPiece(x, 6)->name == pieceName::EMPTY)
					addMove(Move(x, y, 5, y, true));
				/* -- kingside castling -- */
				rook = (p->color == pieceColor::WHITE) ? getPiece(7, 0) : getPiece(0, 0);
				if (!rook->moved
						&& getPiece(x, 2)->name == pieceName::EMPTY
						&& getPiece(x, 1)->name == pieceName::EMPTY)
					addMove(Move(x, y, 1, y, true));
			}
		} break;
		default: break;
	}
	return legalMoves;
}