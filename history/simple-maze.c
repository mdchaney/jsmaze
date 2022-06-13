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

int perms[4][6][3]={{{0,1,3},{0,3,1},{1,0,3},{3,0,1},{1,3,0},{3,1,0}},
                    {{1,2,0},{1,0,2},{2,1,0},{0,1,2},{2,0,1},{0,2,1}},
                    {{2,3,1},{2,1,3},{3,2,1},{1,2,3},{3,1,2},{1,3,2}},
                    {{3,0,2},{3,2,0},{0,3,2},{2,3,0},{0,2,3},{2,0,3}}};

int** init_maze(int xsize, int ysize) {
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

bool make_maze(int **maze, int x, int y, int depth, int direction) {
	if (unvisited(&maze[x][y])) {
		remove_entering_wall(&maze[x][y], direction);

		int *directions = perms[direction][random() % 6];

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

// ends are 1,0 and xsize-2,ysize-1
void open_ends(int **maze, int xsize, int ysize) {
	remove_wall(&maze[1][0], 2);
	remove_wall(&maze[1][1], 0);
	remove_wall(&maze[xsize-2][ysize-2], 2);
	remove_wall(&maze[xsize-2][ysize-1], 0);
}

void show_star_maze(int **maze, int xsize, int ysize) {

   char *line1, *line2, *line3;
   int x, y;

	line1 = malloc(sizeof(char) * xsize * 2 + 5);
	line2 = malloc(sizeof(char) * xsize * 2 + 5);
	line3 = malloc(sizeof(char) * xsize * 2 + 5);

   memset(line3,' ',xsize*2+5);
   line3[xsize*2+1]='\0';

   for (y=0 ; y<ysize ; y++)
   {
      strcpy(line1,line3);
      memset(line3,' ',xsize*2+5);
      line3[xsize*2+1]='\0';
      memset(line2,' ',xsize*2+5);
      line2[xsize*2+1]='\0';
      for (x=0 ; x<xsize ; x++)
      {
         if (maze[x][y] & 1)
         {
            line1[x*2]='*';
            line1[x*2+1]='*';
            line1[x*2+2]='*';
         }
         if (maze[x][y] & 8)
         {
            line1[x*2]='*';
            line2[x*2]='*';
            line3[x*2]='*';
         }
      }
      printf("%s\n%s\n",line1,line2);
   }
}

void usage() {
   fprintf(stderr, "Usage: simple-maze [--xsize 20] [--ysize 20] [--seed x]\n");
   exit(1);
}

int main(int argc, char **argv) {
	int xsize = 20, ysize = 20;
	int seed = -1;
	static int set = 0;

	int option_index = 0, c = 0;

	static struct option long_options[] = {
		{ "xsize", required_argument, 0, 'x' },
		{ "ysize", required_argument, 0, 'y' },
		{ "seed", required_argument, 0, 'r' },
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
			case 'r':
				if (sscanf(optarg, "%d", &seed) != 1) {
					usage();
				}
				break;
			case 0:
				break;
			default:
				usage();
		}
	}

	// Move beyond the parsed options
	argc -= option_index;
	argv += option_index;

	if (seed > -1) {
		srand(seed);
	} else {
		srand(time(0));
	}

	int **maze = init_maze(xsize, ysize);
	make_maze(maze, 1, 1, 0, 2);
	open_ends(maze, xsize, ysize);
	show_star_maze(maze, xsize, ysize);

	return 0;
}
