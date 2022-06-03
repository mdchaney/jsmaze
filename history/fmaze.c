/*
  M Darrin Chaney
*/

#include <stdio.h>
#include <math.h>
#include <time.h>

#define MAXX 200
#define MAXY 100

readonly int deltax[4]={0,1,0,-1};
readonly int deltay[4]={-1,0,1,0};

unsigned int maze[MAXX+1][MAXY+1];

/*
char pieces[16]={32,120,113,109,120,120,108,116,
                 113,106,113,118,107,117,119,110};
*/

char pieces[16]={32,32,32,109,32,120,108,116,
                 32,106,113,118,107,117,119,110};

int maxx=21,maxy=21,w=1,s=0;

int cell_width,cell_height;

int srandom();
int random();

main(argc,argv)
int argc;
char *argv[];
{
  int i,j,k,x,y;
  char line[202];
  int ran,gran=0;

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
      case 'w':
        w=atoi(argv[++i]);
        break;
      case 's':
         s=atoi(argv[++i]);
         break;
      default:
        fprintf(stderr,"Unknown flag: %s\n",argv[i]);
        break;
      }
    }
    else
    {
      fprintf(stderr,"Unknown stuff: %s\n",argv[i]);
    }
  }

  printf("\033[H\033[2J");

  printf("\033P1p");
  printf("S(I0E) W(I1R) P[0,0] \nF(V (B) [799,0][799,479][0,479] (E));");
  printf("W(I0R);");

  cell_width= 800/maxx;
  cell_height= 240/maxy;

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

  printf("\033\\");
}

make_maze(x,y,level,dir)
int x,y,level,dir;
{
  int i,j,k,t1,t2;
  int direction;

  printf("(%d,%d): %d   %d  %d\n",x,y,maze[x][y],dir,level);

  /* clear out the cell that we just landed in... */

  i=x-1;  j=y-1;

  switch(dir)
  {
    case 0:
      printf("P[%d,%d] F( V (B) [%d,][,%d][%d,] (E));\n",
              (i*cell_width)+1,((int)(((float)j + 1.5)*cell_height))*2,
              ((i+1)*cell_width)-1,((int)(((float)j + 0.5)*cell_height))*2,
              (i*cell_width)+1);
      break;

    case 2:
      printf("P[%d,%d] F( V (B) [%d,][,%d][%d,] (E));\n",
              (i*cell_width)+1,((int)(((float)j + 0.5)*cell_height))*2,
              ((i+1)*cell_width)-1,((int)(((float)j - 0.5)*cell_height))*2,
              (i*cell_width)+1);
      break;

    case 1:
      printf("P[%d,%d] F( V (B) [,%d][%d,][,%d] (E));\n",
              (int)(((float)i + 0.5)*cell_width),((j*cell_height)+1)*2,
              (((j+1)*cell_height)-1)*2,(int)(((float)i - 0.5)*cell_width),
              ((j*cell_height)+1)*2);
      break;

    case 3:
      printf("P[%d,%d] F( V (B) [,%d][%d,][,%d] (E));\n",
              (int)(((float)i + 0.5)*cell_width),((j*cell_height)+1)*2,
              (((j+1)*cell_height)-1)*2,(int)(((float)i + 1.5)*cell_width),
              ((j*cell_height)+1)*2);
      break;
  }

  if (s!=0) sleep(s);

  for (;;)
    {
      k=0;
      do 
        {
          direction=random()&3;
          i=x+deltax[direction];
          j=y+deltay[direction];
        } while ((k++<4) && (maze[i][j]!=15));
      
      if (maze[i][j]==15)
        {
          maze[x][y] -= (1 << direction);
          maze[i][j] -= (1 << (direction ^ 2));
          make_maze(i,j,level+1,direction);
        }
      else
        {
          for (direction=0 ; (direction<4) && 
               (maze[i=(x+deltax[direction])][j=(y+deltay[direction])]!=15) ;
               direction++);

          if (direction==4)
            {
              /* printf("Returning from level: %d\n",level); */
              return;
            }
          else
            {
              maze[x][y] -= (1 << direction);
              maze[i][j] -= (1 << (direction ^ 2));
              make_maze(i,j,level+1,direction);
            }
        }
    }
}

fatal_error(str)
char *str;
{
  fprintf(stderr,"Error: %s\n",str);
  exit(0);
}
