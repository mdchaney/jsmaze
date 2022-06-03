/*
  M Darrin Chaney
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define MAXX 500
#define MAXY 500

char deltax[4]={0,1,0,-1};
char deltay[4]={-1,0,1,0};

unsigned char maze[MAXX+1][MAXY+1];

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
 *      0 - left first
 *      1 - right first
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

char perms[4][3][2][3]={{{{0,1,3},{0,3,1}},{{1,0,3},{3,0,1}},{{1,3,0},{3,1,0}}},
                        {{{1,2,0},{1,0,2}},{{2,1,0},{0,1,2}},{{2,0,1},{0,2,1}}},
                        {{{2,3,1},{2,1,3}},{{3,2,1},{1,2,3}},{{3,1,2},{1,3,2}}},
                        {{{3,0,2},{3,2,0}},{{0,3,2},{2,3,0}},{{0,2,3},{2,0,3}}}};

int maxx=21,maxy=21;

int max_level=0,max_level_x,max_level_y;

double right_preference=.5;  /* 1 = left first, 0=right first */
double straight_preference = 0;

char render_style='t';

char pieces[16]={32,32,32,109,32,120,108,116,
                 32,106,113,118,107,117,119,110};

float sxd,syd;
char nl=0;

void make_maze();
void fatal_error();
void show_star_maze();
void show_tek_maze();
void show_vt_maze();
void write_tek_coordinate();
int sx();
int sy();

/*int srandom();
int random();
*/
int main(argc,argv)
	int argc;
	char *argv[];
{
	int i,j,k,x,y;
	int ran,gran=0,dir;

	for (i=1 ; i<argc ; i++)
	{
		if (*argv[i]=='-')
		{
			switch (*(argv[i]+1))
			{
				case 'n':
					nl=1;
					break;
				case 'x':
					maxx=atoi(argv[++i])+1;
					break;
				case 'y':
					maxy=atoi(argv[++i])+1;
					break;
				case 'l':
					right_preference=atof(argv[++i]);
					break;
				case 's':
					straight_preference=atof(argv[++i]);
					break;
				case 'r':
					ran=atoi(argv[++i]);
					gran=1;
					break;
				case 'd':
					render_style=*argv[++i];
					if (render_style != 'v' && render_style!='t' && render_style!='s') {
						fprintf(stderr,"Unknown display: %s\n",argv[i]);
					}
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

	printf("right: %f\nstraight%f\n",right_preference,straight_preference);

	for (x=0 ; x<maxx+1 ; x++)
		for (y=0 ; y<maxy+1 ; y++)
			if ((x==0) || (x==maxx) || (y==0) || (y==maxy))
				maze[x][y]=0;
			else
				maze[x][y]=15;

	make_maze(1,1,0,2);

	for (y=0 ; y<maxy+1 ; y++)
	{
		maze[0][y]=2;
		maze[maxx][y]=8;
	}

	for (x=1 ; x<maxx ; x++)
	{
		maze[x][0]=4;
		maze[x][maxy]=1;
	}

	maze[0][0]=maze[maxx][0]=maze[maxx][maxy]=maze[0][maxy]=0;

	maze[1][0]=0;
	maze[1][1]-=1;

	if (max_level_x == maxx)                    
	{                                           
		maze[max_level_x+1][max_level_y]=0;      
		maze[max_level_x][max_level_y] -= 2;     
	}                                           
	else                                        
	{                                           
		if (max_level_x == 1)                    
		{                                        
			maze[0][max_level_y]=0;               
			maze[1][max_level_y] -= 8;            
		}                                        
		else                                     
		{                                        
			if (max_level_y == 1)                  
			{                                     
				maze[max_level_x][0]=0;            
				maze[max_level_x][1] -=  1;         
			}                                     
			else                                  
			{                                     
				maze[max_level_x][max_level_y+1]=0;
				maze[max_level_x][max_level_y] -= 4;         
			}                                     
		}                                        
	}                                           

	if (render_style=='v') {
		show_vt_maze();
	} else if (render_style=='t') {
		show_tek_maze();
	} else if (render_style=='s') {
		show_star_maze();
	}
	//sleep(20);
}

void make_maze(x,y,level,dir)
	int x,y,level,dir;
{
	char i,j,k,new_dir;
	char left_or_right,straight_level, perm_num;
	left_or_right=(((random()%1000)/1000.0)>right_preference)?1:0;
	if (straight_preference==0) {
		straight_level=random()%3;
	} else if (straight_preference<0) /* -1 = very straight */ {
		straight_level = ((double)((random() % 1000)/1000.0)*(1+straight_preference))*3;
	} else /* (straight_preference>0) */ /* 1 = very curvy */ {
		straight_level = (1-((double)(random() % 1000)/1000.0)*(1-straight_preference))*3;
	}
	if (straight_level<0) straight_level=0;
	else if (straight_level>2) straight_level=2;

	if ((level>max_level) && ((x==1)||(y==1)||(x==maxx-1)||(y==maxy-1))) {
		max_level=level; max_level_x=x; max_level_y=y;
	}

	for (k=0 ; k<3 ; k++) {
		new_dir=perms[dir][straight_level][left_or_right][k];

		i=x+deltax[new_dir];
		j=y+deltay[new_dir];

		if (maze[i][j]==15) {
			maze[x][y] -= (1 << new_dir);
			maze[i][j] -= (1 << (new_dir ^ 2));
			make_maze(i,j,level+1,new_dir);
		}
	}
}

void fatal_error(str)
	char *str;
{
	fprintf(stderr,"Error: %s\n",str);
	exit(0);
}

void show_star_maze() {

	char line1[MAXX*2+5],line2[MAXX*2+5],line3[MAXX*2+5];
	int x,y;
	char fillchar;

	memset(line3,' ',maxx*2+5);
	line3[maxx*2+1]='\0';
	for (y=0 ; y<maxy+1 ; y++)
	{
		strcpy(line1,line3);
		memset(line3,' ',maxx*2+5);
		line3[maxx*2+1]='\0';
		memset(line2,' ',maxx*2+5);
		line2[maxx*2+1]='\0';
		for (x=0 ; x<maxx+1 ; x++)
		{
			if (x==max_level_x && y==max_level_y) fillchar='A'; else fillchar='*';
			if (maze[x][y] & 1)
			{
				line1[x*2]='*';
				if (max_level_y==1 || max_level_y==maxy) line1[x*2+1]=fillchar; else line1[x*2+1]='*';
				line1[x*2+2]='*';
			}
			if (maze[x][y] & 8)
			{
				line1[x*2]='*';
				if (max_level_x==1 || max_level_x==maxx) line2[x*2]=fillchar; else line2[x*2]='*';
				line3[x*2]='*';
			}
		}
		printf("%s\n%s\n",line1,line2);
	}
}

void show_vt_maze() {
	int i,x,y;
	char line[200];
	for (y=0 ; y<maxy ; y++)
	{
		for (i=0,x=0 ; x<maxx ; x++,i++)
		{
			line[i]=0;
			if (maze[x][y] & 2) line[i] += 1;
			if (maze[x][y] & 4) line[i] += 8;
			if (maze[x+1][y+1] & 1) line[i] += 2;
			if (maze[x+1][y+1] & 8) line[i] += 4;
			line[i]=pieces[line[i]];
		}
		line[i]=0;
		printf("\033(0%s\033(B\n",&line[0]);
	}
}

void show_tek_maze() {
	int x,y;

	sxd = (3996.0/(float)(maxx+1));
	syd = (3022.0/(float)(maxy+1));

	printf("\033\014");  /* Enter Tek graphics mode */

	if (nl!=0) printf("\n");

	for (x=1 ; x<maxx+1 ; x++)
	{
		for (y=1 ; y<=maxy;)
		{
			if ((maze[x][y] & 8) == 0)
			{
				for (y++ ; (maze[x][y] & 8) == 0 && y<=maxy; y++);
			}
			else
			{
				printf("\035");  /* start a line */
				//            fprintf(stderr,"Line from (%u,%u) to ",x,y);
				write_tek_coordinate(sx(x),sy(y));
				for (y++ ; (maze[x][y] & 8) ; y++);
				//            fprintf(stderr,"(%u,%u)\n",x,y);
				write_tek_coordinate(sx(x),sy(y));
				if (nl!=0) printf("\n");
			}
		}
	}

	for (y=1 ; y<maxy+1 ; y++)
	{
		for (x=1 ; x<=maxx;)
		{
			if ((maze[x][y] & 1) == 0)
			{
				for (x++ ; (maze[x][y] & 1) == 0 && x<=maxx; x++);
			}
			else
			{
				printf("\035");  /* start a line */
				//            fprintf(stderr,"Line from (%u,%u) to ",x,y);
				write_tek_coordinate(sx(x),sy(y));
				for (x++ ; (maze[x][y] & 1) ; x++);
				//            fprintf(stderr,"(%u,%u)\n",x,y);
				write_tek_coordinate(sx(x),sy(y));
				if (nl!=0) printf("\n");
			}
		}
	}

	printf("\015");
}

void write_tek_coordinate(x,y)
	int x,y;
{
	char out[6];
	out[5]=0;

	out[0] = 0x20 + ((y & 0xf80) >> 7);
	out[1] = 0x60 + ((y & 3) << 2) + (x & 3);
	out[2] = 0x60 + ((y & 0x7c) >> 2);
	out[3] = 0x20 + ((x & 0xf80) >> 7);
	out[4] = 0x40 + ((x & 0x7c) >> 2);

	/* printf("(%d,%d)",x,y); */
	printf("%s",out);
}

int sx(x)
	int x;
{
	return (int)(50 + ((float)x * sxd));
}

int sy(y)
	int y;
{
	return (int)(50 + ((float)y * syd));
}

