#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

char deltax[4]={0,1,0,-1};
char deltay[4]={-1,0,1,0};

int opposite_direction[4] = {2,3,0,1};
char walls[4] = {1,2,4,8};
char wall_removal_mask[4] = {0b1110, 0b1101, 0b1011, 0b0111};

/* perms array has these indices:
 *   1. direction of entering square
 *   2. a perm #
 *   3. a direction #
 *   4. sequence #
 *
 *   Note that the perm# is broken down as follows:
 *      0 - go straight first
 *      1 - go straight second
 *      2 - go straight last
 *
 *   Direction # is:
 *      0 - right first
 *      1 - left first
 *
 *   Therefore:
 *
 *   	  0,0 - straight,right,left
 *   	  0,1 - straight,left,right
 *   	  1,0 - right,straight,left
 *   	  1,1 - left,straight,right
 *   	  2,0 - right,left,straight
 *   	  2,1 - left,right,straight
 */

int perms[4][3][2][3]={{{{0,1,3},{0,3,1}},{{1,0,3},{3,0,1}},{{1,3,0},{3,1,0}}},
                        {{{1,2,0},{1,0,2}},{{2,1,0},{0,1,2}},{{2,0,1},{0,2,1}}},
                        {{{2,3,1},{2,1,3}},{{3,2,1},{1,2,3}},{{3,1,2},{1,3,2}}},
                        {{{3,0,2},{3,2,0}},{{0,3,2},{2,3,0}},{{0,2,3},{2,0,3}}}};

int max_level = 0;
int max_level_x = -1;
int max_level_y = -1;

double right_preference=.5;  /* 1 = left first, 0 = right first */
double straight_preference = -.5;

char *unicode_light_pieces[16] = { " ", "\u2575", "\u2576", "\u2514",
										"\u2577", "\u2502", "\u250C", "\u251C",
										"\u2574", "\u2518", "\u2500", "\u2534",
										"\u2510", "\u2524", "\u252C", "\u253C" };

void show_unicode_maze(char **maze, int xsize, int ysize, bool double_wide) {
	int x, y, char_offset, char_offset2;
	for (y=0 ; y < ysize-1 ; y++) {
		for (x=0 ; x < xsize-1 ; x++) {
			char_offset = 0;
			if (maze[x][y] & 2) char_offset |= 1;
			if (maze[x][y] & 4) char_offset |= 8;
			if (maze[x+1][y+1] & 1) char_offset |= 2;
			if (maze[x+1][y+1] & 8) char_offset |= 4;
			printf("%s", unicode_light_pieces[char_offset]);
			if (double_wide) {
				char_offset2 = 0;
				if (char_offset & 2) char_offset2 |= 10;
				printf("%s", unicode_light_pieces[char_offset2]);
			}
		}
		printf("\n");
	}
}

void show_raw_maze(char **maze, int xsize, int ysize) {
	int x, y;
	for (y=0 ; y < ysize ; y++) {
		for (x=0 ; x < xsize ; x++) {
			printf("%x", maze[x][y]);
		}
		printf("\n");
	}
}

void show_raw_maze2(char **maze, int xsize, int ysize) {
	int x, y, char_offset;
	for (y=0 ; y < ysize-1 ; y++) {
		for (x=0 ; x < xsize-1 ; x++) {
			char_offset = 0;
			if (maze[x][y] & 2) char_offset |= 1;
			if (maze[x][y] & 4) char_offset |= 8;
			if (maze[x+1][y+1] & 1) char_offset |= 2;
			if (maze[x+1][y+1] & 8) char_offset |= 4;
			printf("%x", char_offset);
		}
		printf("\n");
	}
}

void show_unicode_pieces() {
	int i;
	for (i=0 ; i < 16 ; i++) {
		printf("%2d  %x  %s\n", i, i, unicode_light_pieces[i]);
	}
}

int get_straightness(float seed) {
	// float cutoff1 = 0.3333333 + seed * 0.6666667;
	int cutoff1 = RAND_MAX / 3 + 2 * (int)(seed * RAND_MAX) / 3;
	int rnd = rand();
	if (rnd < cutoff1) {
		return 0;
	} else {
		// float cutoff2 = 1.0 - (1.0 - cutoff1) * (.5-seed*.5);
		int cutoff2 = RAND_MAX - (RAND_MAX - cutoff1) * ((RAND_MAX >> 1) - ((int)(seed * RAND_MAX) >> 1));
		if (rnd < cutoff2) {
			return 1;
		} else {
			return 2;
		}
	}
}

char** init_maze(int xsize, int ysize) {
	char **maze;
	int x, y;

	maze = malloc(xsize * (sizeof *maze));

	for (x=0 ; x < xsize ; x++) {
		maze[x] = malloc(ysize * (sizeof **maze));
	}

	// Start off with every interior cell unvisited.
	for (x=1 ; x < xsize-1 ; x++) {
		for (y=1 ; y < ysize-1 ; y++) {
			maze[x][y] = 15;
		}
	}

	// Corners need to be 0
	maze[0][0] = 0;
	maze[0][ysize-1] = 0;
	maze[xsize-1][0] = 0;
	maze[xsize-1][ysize-1] = 0;

	// This will surround the inner maze with "visited" cells so
	// we don't have to do bounds checking.  It sets up the correct
	// walls so that the unicode maze will display properly.
	for (y=1 ; y < ysize-1 ; y++) {
		maze[0][y] = 2;       // left side
		maze[xsize-1][y] = 8; // right side
	}

	for (x=1 ; x < xsize-1 ; x++) {
		maze[x][0] = 4;        // top
		maze[x][ysize-1] = 1;  // bottom
	}

	return maze;
}

void remove_wall(char *cell, int direction) {
	*cell &= wall_removal_mask[direction];
}

void remove_entering_wall(char *cell, int direction) {
	remove_wall(cell, opposite_direction[direction]);
}

bool unvisited(char *cell) {
	return (*cell == 15) ? true : false;
}

bool make_maze(char **maze, int x, int y, int depth, int direction) {
	if (unvisited(&maze[x][y])) {
		// remove the wall we entered from
		remove_entering_wall(&maze[x][y], direction);

		int lr = (rand() > RAND_MAX >> 1) ? 0 : 1;
		int straightness;

		if (straight_preference < 0) {
			straightness = get_straightness(straight_preference);
		} else if (straight_preference > 0) {
			straightness = 2 - get_straightness(straight_preference);
		} else {
			straightness = rand() % 3;
		}

		int *directions = perms[direction][straightness][lr];

		for (int k=0 ; k<3 ; k++) {
			int new_direction = directions[k];
			if (make_maze(maze, x + deltax[new_direction], y + deltay[new_direction], depth + 1, new_direction)) {
				remove_wall(&maze[x][y], new_direction);
			}
		}
		return true;
	} else {
		return false;
	}
}

int main(int argc, char **argv) {
	char **maze = init_maze(20, 20);
	srand(time(0));
	make_maze(maze, 1, 1, 0, 2);
	show_unicode_maze(maze, 20, 20, true);
	//show_raw_maze2(maze, 20, 20);
	//show_raw_maze(maze, 20, 20);
	//show_unicode_pieces();

	return 0;
}
