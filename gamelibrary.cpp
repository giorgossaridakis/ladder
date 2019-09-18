/* small game library */
#include <windows.h>
#include <constrea.h>

void showCursor(bool showFlag);
void randomizer(long int depth);
int mod (double a, double b);
int check_rarity(int rarity);

#define UP 72
#define LEFT 75
#define RIGHT 77
#define DOWN 80
#define SPACE 32
#define PAUSE 'p'
#define ESC 27

/* show / hide cursor */
void showCursor(bool showFlag)
{   //enable/disable cursor visibility
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = showFlag; // set the cursor visibility
    SetConsoleCursorInfo(out, &cursorInfo);
}

/* randomize numbers better */
void randomizer(long int depth)
{
  time_t timer;
  int seed_number;
  int i,n,l;

  randomize();
  time(&timer);
  seed_number=mod(timer,10000);
   for (n=0;n<seed_number;n++) {
    for (i=0;i<depth;i++)
   l=(rand () % 1111); }
}

/* remaining from division */
int mod (double a, double b)
{
int tmp =a/b;

return a-(b*tmp);
}

/* return 1 if object is lucky enough to appear */
int check_rarity(int rarity)
{
  int i,n,l;
  int success[3];
  
   if (!rarity)
	return 1;
   else
	i=rand()% rarity;
    n=int(rarity/3);   
    for (l=1;l<4;l++) 
     success[l-1]=n*l;
	for (l=0;l<3;l++) {
	 if (i==success[l])
	  return 1; }
  return 0;
}