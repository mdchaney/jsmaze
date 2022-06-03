/*
  M Darrin Chaney
*/

#include <stdio.h>
#include <math.h>
#include <time.h>

#define MAXX 500
#define MAXY 500

const int deltax[4]={0,1,0,-1};
const int deltay[4]={-1,0,1,0};

unsigned char maze[MAXX+1][MAXY+1];

const
int perms[24][4]={{0,1,2,3},{0,1,3,2},{0,2,1,3},{0,2,3,1},{0,3,1,2},{0,3,2,1},
                  {1,0,2,3},{1,0,3,2},{1,2,0,3},{1,2,3,0},{1,3,0,2},{1,3,2,0},
                  {2,0,1,3},{2,0,3,1},{2,1,0,3},{2,1,3,0},{2,3,0,1},{2,3,1,0},
                  {3,0,1,2},{3,0,2,1},{3,1,0,2},{3,1,2,0},{3,2,0,1},{3,2,1,0}};

int maxx=21,maxy=21;

int max_level=0,max_level_x,max_level_y;

int srandom();
int random();

main(argc,argv)
int argc;
char *argv[];
{
  int i,j,k,x,y;
  char line[202];
  int ran,gran=0,dir;

  for (i=1 ; i<argc ; i++)
  {
    if (*argv[i]=='-')
    {
      switch (*(argv[i]+1))
      {
      case 'x':
        maxx=atoi(argv[++i])+1;
        break;
      case 'y':
        maxy=atoi(argv[++i])+1;
        break;
      case 'r':
        ran=atoi(argv[++i]);
        gran=1;
        break;
      default:
        fprintf(stderr,"Unknown flag: %s\n",argv[i]);
        break;
      }
    }
    else
    {
      fprintf(stderr,"Unknown flag: %s\n",argv[i]);
    }
  }

  if (gran==1)
    srandom(ran);
  else
    srandom(time(0));

  for (x=0 ; x<maxx+1 ; x++)
    for (y=0 ; y<maxy+1 ; y++)
      if ((x==0) || (x==maxx) || (y==0) || (y==maxy))
        maze[x][y]=0;
      else
        maze[x][y]=15;

  make_maze(1,1,0,2);

}

make_maze(x,y,level,dir)
int x,y,level,dir;
{
  int i,j,k;
  int direction,perm_num;

  if ((level>max_level) && ((x==1) || (y==1) || (x==maxx-1) || (y==maxy-1)))
  {
    max_level=level;
    max_level_x=x;
    max_level_y=y;
  }

  perm_num = random() % 24;

  for (k=0 ; k<4 ; k++)
  {
    direction=perms[perm_num][k];

    i=x+deltax[direction];
    j=y+deltay[direction];

    if (maze[i][j]==15)
    {
      maze[x][y] -= (1 << direction);
      maze[i][j] -= (1 << (direction ^ 2));
      make_maze(i,j,level+1,direction);
    }
  }
}

fatal_error(str)
char *str;
{
  fprintf(stderr,"Error: %s\n",str);
  exit(0);
}
