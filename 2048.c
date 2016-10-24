
/*
	2048 for the bitbox 

	TODO: 
		highlights
		musique, animations (bg)
		sounds
		2 players : si augmente de 1, balance a l'autre ?

*/
#include <stdint.h>
#include <stdlib.h> // rand
#include <bitbox.h> 
#include <lib/blitter/blitter.h>

#include "levels.h"
#include "pieces.h"

// -------------------------------------------------------------------------------------------------------

#define SCREEN_W 64
#define SCREEN_H 64 

#define FRAMES_MOVE 1 // frames between each move

// board position. Board is 4 slots of 4, so is 16x16 tiles
#define BOARD_X 6
#define BOARD_Y 6

// Game over panel
#define GAMEOVER_X 2
#define GAMEOVER_Y 5

// score
#define SCORE_X 18
#define SCORE_Y 0

// high score
#define HISCORE_X 25
#define HISCORE_Y 0

// excite
#define EXCITE_X 2
#define EXCITE_Y 2

// index of tile representing number 0 in tileset
#define TILE_NB_BASE 193;

// -------------------------------------------------------------------------------------------------------

enum GameState {
	StateStartScreen, StateWaiting, StateGameOver, StateMoving
};

struct Game {
	enum GameState state; 
	int score;
	int best_score;
	int level; // best piece so far

	int tab[16]; 
	int moving_dir; // increments when moving

	object *o;
};
// ram tilemap
uint8_t tmap_screen[SCREEN_W*SCREEN_H]; 


// button, start_pos, next_col, next_line (delta to pos to go to next column, or line )
const int moves[][4] = { 
	{ gamepad_down,  8, 1, 4}, 
	{ gamepad_up,    7,-1,-4}, 
	{ gamepad_right,14,-4, 1}, 
	{ gamepad_left,  1, 4,-1}, 
};

// levels bg tilemaps

struct Game game;

// -------------------------------------------------------------------------------------------------------

int move() {
	// get all "down" one step lower - until not possible

	int moved = 0; // something has moved (ie move not finished)
	int pos=moves[game.moving_dir][1]; // starting case to check whole thing

	// shortcuts 
	const int d_col = moves[game.moving_dir][2]; // next column
	const int d_line = moves[game.moving_dir][3]; // next line

	for (int line=0;line<3;line++, pos -= 4*d_col+d_line) { 
		for (int col=0;col<4;col++, pos += d_col) {
			if (game.tab[pos]) {
				// nothing down : take down
				if (game.tab[pos+d_line] ==0 ) {
					game.tab[pos+d_line]=game.tab[pos];
					game.tab[pos]=0;
					moved=1;

				} else if (game.tab[pos+d_line] == game.tab[pos]) { 
				// combine !
					game.tab[pos+d_line] += 1;
					game.score += 1<<game.tab[pos+d_line];
					if (game.best_score<game.score) {
						game.best_score=game.score;
					}
					// son combine  
					game.tab[pos]=0;
					moved=1;

					// level up ?
					if (game.level<game.tab[pos+d_line]-1) {
						// son level up
						game.level += 1;
					}
				}
			} 
		}
	}

	// returns 1 if ended move, ie nothing moved
	return !moved;
}


void end_move()
// move finished, add a new piece and check game over
{
	int empty[16]; 
	int nb_empty=0;
	// get empty cells
	for (int i=0;i<16;i++) 
		if (game.tab[i]==0)
			empty[nb_empty++]=i;

	// game over ?
	if (nb_empty==0) { 
		game.state=StateGameOver;
	} else {
		// Add a new one  (animate?)
		int a=rand();
		game.tab[empty[a%nb_empty]]= a%4 ? 1 : 2;
		game.state=StateWaiting;
	}
}

static void drawNumber(int x, int y, long number, int length) {
    int pos=0;

    do {
    	tmap_screen[ y*SCREEN_W + x + (length - pos)] = number%10 + TILE_NB_BASE;
        number /= 10;
    } while (++pos < length);
}
/*
void tmap_blit(int x, int y, uint32_t header, const uint8_t *src)
{
	int tmapw = (header>>24);
	int tmaph = ((header>>16) & 0xff);

	for (int j=0;j<tmaph;j++)
		for (int i=0;i<tmapw;i++) 
		{
			uint8_t c = src[tmapw * j+i ];
			if (c) tmap_screen[(j+y)*SCREEN_W+i+x] = c;
		}
}
*/

void draw()
{
	// draw bg
	if (game.state==StateStartScreen) {
		tmap_blit(game.o,0,0,levels_header,&levels_tmap[levels_start][0]);
		return;
	}

	else 
		tmap_blit(game.o,0,0,levels_header,&levels_tmap[game.level][0]);

	// draw board
	for (int i=0;i<16;i++) {
		tmap_blit(game.o,BOARD_X+4*(i%4),BOARD_Y+4*(i/4),pieces_header,&pieces_tmap[game.tab[i]][0]);
	}

	// scores
	drawNumber(SCORE_X, SCORE_Y, game.score, 6);
	drawNumber(HISCORE_X, HISCORE_Y, game.best_score, 6);
	
	

	if (game.score >= game.best_score+1000) {
		game.o->x = rand()%8;
		game.o->y = rand()%8;
		tmap_blit(game.o,EXCITE_X,EXCITE_Y,levels_header,&levels_tmap[levels_excite][0]);
	}

	if (game.state==StateGameOver)
		tmap_blit(game.o,GAMEOVER_X,GAMEOVER_Y,levels_header,&levels_tmap[levels_gameover][0]);
	
}


void reset_game()
{
	game.state=StateWaiting;
	for (int i=0;i<16;i++) 
		game.tab[i]=0;
	game.level=0;
	game.score=0;
	game.moving_dir=0xffff; // None

	end_move(); // insert new piece
	// lancer son / zik
}

void game_init()
{
 	blitter_init();
	game.o = tilemap_new(&levels_tset[0],640,480,TMAP_HEADER(SCREEN_W,SCREEN_H,TSET_16, TMAP_U8),&tmap_screen[0]);  
	game.o->x=0; 
	game.o->y=0;

	game.state=StateStartScreen;
	game.best_score=0;
}

void game_frame() 
{
	int dir;

	// update game
	switch (game.state) {

		case StateStartScreen : 
		case StateGameOver :
			if (GAMEPAD_PRESSED(0,start)) 
				reset_game();
		break;

		case StateWaiting :
			for (dir=0;dir<4;dir++)
				if (gamepad_buttons[0] & moves[dir][0]) 
					break;
			if (dir<4 && dir != game.moving_dir) {
				// move game
				game.moving_dir=dir;
				game.state=StateMoving;
				// lancer son ?
			}
		break;

		case StateMoving :
			if (vga_frame % FRAMES_MOVE==0)
				if (move()) end_move();
		break;
	}

	draw();
}
