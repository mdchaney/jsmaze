#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <getopt.h>

int deltax[4]={0,1,0,-1};
int deltay[4]={-1,0,1,0};

int opposite_direction[4] = {2,3,0,1};
int walls[4] = {1,2,4,8};
int wall_removal_mask[4] = {0b1110, 0b1101, 0b1011, 0b0111};

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

char *unicode_light_pieces[16] = { " ", "\u2575", "\u2576", "\u2514",
										"\u2577", "\u2502", "\u250C", "\u251C",
										"\u2574", "\u2518", "\u2500", "\u2534",
										"\u2510", "\u2524", "\u252C", "\u253C" };

void show_unicode_maze(int **maze, int xsize, int ysize, bool double_wide) {
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

void show_raw_maze(int **maze, int xsize, int ysize) {
	int x, y;
	for (y=0 ; y < ysize ; y++) {
		for (x=0 ; x < xsize ; x++) {
			printf("%x", maze[x][y]);
		}
		printf("\n");
	}
}

void show_raw_maze2(int **maze, int xsize, int ysize) {
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

// float cutoff1 = 0.3333333 + seed * 0.6666667;
int get_cutoff1(float seed) {
	if (seed >= 1.0) {
		return RAND_MAX;
	} else {
		return RAND_MAX / 3 + (((int)(seed * RAND_MAX) / 3) << 1);
	}
}

// float cutoff2 = 1.0 - (1.0 - cutoff1) * (.5-seed*.5);
int get_cutoff2(int cutoff1, float seed) {
	return RAND_MAX - ((int)((RAND_MAX - cutoff1) * (1.0 - seed)) >> 1);
}

// Seed is a float between 0 and 1.  At 0, the distribution should be
// evenly split between 0, 1, and 2.  At 1.0, the entire distribution
// should be at 0, with 1 and 2 having nothing.  Between 0 and 1 the
// distribution becomes more weighted toward 0 as the seed approaches 1.
int get_straightness(float seed) {
	int cutoff1 = get_cutoff1(seed);
	int rnd = rand();
	if (rnd < cutoff1) {
		return 0;
	} else {
		int cutoff2 = get_cutoff2(cutoff1, seed);
		if (rnd < cutoff2) {
			return 1;
		} else {
			return 2;
		}
	}
}

void test_get_straightness() {
	float seed;;
	int counts[3];
	int seed_seed, i, straightness;
	printf("seed  %10s  %10s  %5d  %5d  %5d\n", "cutoff1", "cutoff2", 0, 1, 2);
	for (seed_seed=0 ; seed_seed <= 10 ; seed_seed++) {
		seed = (float)seed_seed / 10.0;
		counts[0] = 0; counts[1] = 0; counts[2] = 0;
		for (i=0 ; i < 50000; i++) {
			straightness = get_straightness(seed);
			counts[straightness]++;
		}
		printf(" %1.1f  %10d  %10d  %5d  %5d  %5d\n", seed,
						get_cutoff1(seed), get_cutoff2(get_cutoff1(seed), seed),
						counts[0], counts[1], counts[2]);
	}
	printf("\n\nRAND_MAX: %d\n", RAND_MAX);
	printf("Size of int: %lu\n", sizeof(int));
}

int** init_maze(int xsize, int ysize) {
	// Each maze cell requires only 4 bits, but the storage is
	// negligible and making each number the word size will
	// speed everything up.
	int **maze;
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

void remove_wall(int *cell, int direction) {
	*cell &= wall_removal_mask[direction];
}

void remove_entering_wall(int *cell, int direction) {
	remove_wall(cell, opposite_direction[direction]);
}

bool unvisited(int *cell) {
	return (*cell == 15) ? true : false;
}

bool make_maze(int **maze, int x, int y, float straight_preference, float right_preference, int depth, int direction) {
	if (unvisited(&maze[x][y])) {
		// remove the wall we entered from
		remove_entering_wall(&maze[x][y], direction);

		int lr = (rand() > RAND_MAX >> 1) ? 0 : 1;
		int straightness;

		if (straight_preference < 0) {
			straightness = get_straightness(-straight_preference);
		} else if (straight_preference > 0) {
			straightness = 2 - get_straightness(straight_preference);
		} else {
			straightness = rand() % 3;
		}

		int *directions = perms[direction][straightness][lr];

		for (int k=0 ; k<3 ; k++) {
			int new_direction = directions[k];
			if (make_maze(maze, x + deltax[new_direction], y + deltay[new_direction], straight_preference, right_preference, depth + 1, new_direction)) {
				remove_wall(&maze[x][y], new_direction);
			}
		}
		return true;
	} else {
		return false;
	}
}

void usage() {
	fprintf(stderr, "Usage: unicode-maze [--xsize 20] [--ysize 20] [--straightness 0] [--leftright 50]\n");
	exit(1);
}

// ends are 1,0 and xsize-2,ysize-1
void open_ends(int **maze, int xsize, int ysize) {
	remove_wall(&maze[1][0], 2);
	remove_wall(&maze[1][1], 0);
	remove_wall(&maze[xsize-2][ysize-2], 2);
	remove_wall(&maze[xsize-2][ysize-1], 0);
}

void show_test(int test_number) {
	int **maze;
	switch (test_number) {
		case 1:
			test_get_straightness();
			break;
		case 2:
			maze = init_maze(20, 20);
			show_raw_maze2(maze, 20, 20);
			break;
		case 3:
			maze = init_maze(20, 20);
			show_raw_maze(maze, 20, 20);
			break;
		case 4:
			show_unicode_pieces();
			break;
		default:
			printf("What are you trying to accomplish?\n");
	}
	exit(0);
}

int main(int argc, char **argv) {
	int xsize = 20, ysize = 20;
	int straightness = 50;
	int lr_preference = 50;
	static int singlewide = 0;
	int seed = -1;

	int testing = 0;

	int option_index = 0, c = 0;

	static struct option long_options[] = {
		{ "xsize", required_argument, 0, 'x' },
		{ "ysize", required_argument, 0, 'y' },
		{ "straightness", required_argument, 0, 's' },
		{ "leftright", required_argument, 0, 'l' },
		{ "seed", required_argument, 0, 'r' },
		{ "singlewide", no_argument, &singlewide, 1 },
		{ "test", required_argument, 0, 't' },
		{ NULL, 0, NULL, 0 }
	};

	while ((c = getopt_long(argc, argv, "", long_options, &option_index)) != -1) {
		switch(c) {
			case 'x':
				if (sscanf(optarg, "%d", &xsize) != 1) {
					usage();
				}
				break;
			case 'y':
				if (sscanf(optarg, "%d", &ysize) != 1) {
					usage();
				}
				break;
			case 's':
				if (sscanf(optarg, "%d", &straightness) != 1) {
					usage();
				}
				break;
			case 'l':
				if (sscanf(optarg, "%d", &lr_preference) != 1) {
					usage();
				}
				break;
			case 'r':
				if (sscanf(optarg, "%d", &seed) != 1) {
					usage();
				}
				break;
			case 't':
				if (sscanf(optarg, "%d", &testing) != 1) {
					usage();
				}
				break;
			case 0:
				// singlewide option, nothing to do here
				break;
			default:
				usage();
		}
	}

	// Move beyond the parsed options
	argc -= option_index;
	argv += option_index;

	if (testing > 0) {
		show_test(testing);
	} else {

		if (straightness < 0) {
			straightness = 0;
		} else if (straightness > 100) {
			straightness = 100;
		}

		float straight_preference = (float)straightness / 50.0 - 1.0;

		if (lr_preference < 0) {
			lr_preference = 0;
		} else if (lr_preference > 100) {
			lr_preference = 100;
		}

		float right_preference = (float)lr_preference / 100.0;

		if (seed > -1) {
			srand(seed);
		} else {
			srand(time(0));
		}

		int **maze = init_maze(xsize, ysize);
		make_maze(maze, 1, 1, straight_preference, right_preference, 0, 2);
		open_ends(maze, xsize, ysize);
		show_unicode_maze(maze, xsize, ysize, !singlewide);

		return 0;
	}
}
