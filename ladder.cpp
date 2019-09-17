/* remake of classic Ladder for CP/M */
#include "gamelibrary.cpp"
#include "fcryptlibrary.cpp"
#include <ctype.h>
#include <string.h>

#define INITIAL_LIVES 5
#define MAXENEMIES 255
#define SLEEPTIME 75
#define DIFFICULTY 85
#define FLOOR '='
#define LADDER 'H'
#define WOMB 'V'
#define SIGN '|'
#define GIFT'&'
#define TRAP '^'
#define MONEY '$'
#define ASTERISK '*'
#define DOT '.'

void design_scene();
void set_up_actors();
int check_coordinates_player(int x, int y);
int check_coordinates_enemy(int id, int x, int y);
void show_message(int flag, int duration, char *s);
void lose_life(int id);
void reset_enemy(int id);
void show_bar();
void award_bonus_time();
void sort_highscores();
void highscore_maintenance(int flag);
long int retrieve_score(char *s);
int copy_file(char *source, char *dest);

constream win1;

struct points {
 int x;
 int y; } ;

struct player_prototype {
 struct points pt; 
 struct points direction;
 int life;
 char bkgrnd;
 unsigned int jumped :1;
 int character; } ;
 
struct player_prototype player;
struct player_prototype enemies[MAXENEMIES];
struct points wombs[3];
char screen[80][21];
int scene;
int bonus_time;
int lives;
int game_flag=-1;
int score;
char characters[9];
int level=1;
int jump_counter=0;
char *highscore_data[5];
int gifts[3][2];
FILE *f; 

int main()
{
  int i,n;
  int bonus_switch[11][2];
  char c;
  char *delay_messages[]={"\n\nYou eat quiche!", "\n\nCome on, we don't have all day!"};
  char name[15];
  char highscore[15];
  int bonus_counter=0;
  int extra_sleep=0;
  strcpy(characters, "gpodoboq-");
  int jump_sequence[5]={1, 1, -1, -1};
  randomizer(9);

  f=fopen("ladder.dat", "r");
  if (!f) {
   printf("Ladder not configured\n\n\n");
   printf("Unable to link to LADCONF.COM\n");
   printf("Ladder configuration program missing\n");
  exit(-1); }
  fclose(f);	 
  
while (1) {
 game_flag=-1;
 score=0;
 extra_sleep=0;
 for (i=0;i<3;i++)
  gifts[i][1]=1;
 lives=INITIAL_LIVES;
 win1.clrscr();
 showCursor(false);
 win1 << setattr(WHITE);
 fcrypt(1, "ladder.dat", "ladder.~~~");
 f=fopen("ladder.~~~", "r");
 /* print out startup screen */ 
 c=0;
 win1 << setxy (1,1);
 while (c!='~') {
  c=fgetc(f);
  if (c!='~')
   win1 << c;  }
 
  fclose(f);
  highscore_maintenance(0);
  for (i=0;i<5;i++) 
   win1 << setxy (43,17+i) << highscore_data[i]; 
  
 while (game_flag==-1) {
   c=0;
   win1 << setxy(58,12);
   win1 << setattr(WHITE) << level;
   /* handle keyboard input */
   if (kbhit()) {
    c=getch();
   c=tolower(c); }
   switch (c) {
    case 'p':
     --level;
     game_flag=0;
	 c=0;
    case 'l': 
     ++level; 
 	if (level>5) 
	 level=1;
    break;
    case 'i':
	 fcrypt(1, "ladder.dat", "ladder.~~~");
     f=fopen("ladder.~~~", "r");
	 while (c!='~')
	  c=fgetc(f);
     win1.clrscr();
 	 win1<<setxy(1,1);
	 game_flag=-2;
	 c=0;
    while (c!='~') {
     c=fgetc(f);
     if (c!='~')
    win1 << c;  }
    fclose(f);
	remove("ladder.~~~");
    while (c!=13)
     c=getch(); 
    break;
    case 'e': 
	 win1 << setxy(25,23) << "Exiting4";
     win1 << setxy(1,24);
     showCursor(true);
	 exit(0);
   break; }
  
  if (check_rarity(35000)) {
   i=rand() % 2;
  show_message(1, 10, delay_messages[i]); } 
  
 }
  
  /* initialize bonus counter switches */
  for (i=1;i<11;i++)
   for (n=0;n<2;n++) 
	bonus_switch[i][n]=0;
   for (i=1;i<11;i++)
	bonus_switch[i][0]+=10000*i;
  
   while (c!=ESC && game_flag>-1) {
   
    if (!game_flag) {   
     win1.clrscr();
     showCursor(false);
	 win1 << setattr(WHITE);
	 switch (level) {
      case 1:
	   scene=1;
	   bonus_time=3500;
	  break;
	  case 2:
	   scene=2;
	   bonus_time=4500;
	  break;
	  case 3:
	   scene=1;
	   bonus_time=3300;
	  break;
	  case 4:
	   scene=2;
	   bonus_time=4300;
	  break;
	  case 5:
	   scene=3;
	   bonus_time=3300;
	  break;
	  case 6:
	   level=1;
	   scene=1;
	   bonus_time=3500;
	 break; }
	 design_scene();
	 set_up_actors();
    /* show new screen */
    for (i=1;i<80;i++) 
     for (n=1;n<21;n++) 
  	  if (screen[i][n])
	   win1 << setxy(i,n) << screen[i][n];
     game_flag=1; 
	 show_message(0, 15, "Get ready!"); 
	win1 << setattr(WHITE); }
   
    /* bring new enemies */
    if (check_rarity(DIFFICULTY-bonus_time/100)) {
     n=rand() % 3 + 1; 
     for (i=0;i<MAXENEMIES;i++) {
 	 if (!enemies[i].life) {
	  enemies[i].life=1;
	 --n; }
 	if (!n)
   break; } }
  
    /* erase actors and restore background */
    win1 << setxy(player.pt.x, player.pt.y) << player.bkgrnd;
    for (i=0;i<MAXENEMIES;i++)
 	 if (enemies[i].life)
	  win1 << setxy(enemies[i].pt.x, enemies[i].pt.y) << enemies[i].bkgrnd;
	  
     /* keyboard controls */
     if(kbhit()) 
	  c=getch();
     switch (c) {
      case '4':
	   c=LEFT;
	  break;
	  case LEFT:
	   player.direction.x=-1;
	   c=0; 
	  break;
	  case '6':
	   c=RIGHT;
	  break;
	  case RIGHT:
	   player.direction.x=1;
	   c=0;
	  break;
	  case '8':
	   c=UP;
	  break;
	  case UP:
	   player.direction.y=-1;
	   c=0;
	  break;
	  case '2':
	   c=DOWN;
	  break;
	  case DOWN:
	   player.direction.y=1;
	   c=0;
	  break;
	  case SPACE:
	   if (!jump_counter && screen[player.pt.x][player.pt.y+1]) {
	    player.direction.y=0;
	    i=1;
	    /* while (!screen[player.pt.x][player.pt.y-i] && i<3)
	 	 ++i;
	    player.pt.y-=i; */
	   jump_counter=4; }
	   c=0;
	  break;
	  case '+':
	   extra_sleep+=5;
	   c=0;
	  break;
	  case '-':
	   extra_sleep-=5;
	   if (SLEEPTIME-extra_sleep<5)
		extra_sleep+=5;
	   c-0;
	  break;
	  case 'p':
       win1 << setxy(player.pt.x, player.pt.y) << (char) player.character;
       for (i=0;i<MAXENEMIES;i++)
 	    if (enemies[i].life) 
	     win1 << setxy(enemies[i].pt.x, enemies[i].pt.y) << (char) enemies[i].character;
	   win1 << setxy (1,22) << "Press RETURN to continue :";
        while (c!=13)
         c=getch(); 
	   win1 << setxy (1,22);
	   win1 << setattr(BLACK) << "Press RETURN to continue :";
	   win1 << setattr(WHITE);
	   win1 << setxy(player.pt.x, player.pt.y) << (char) player.bkgrnd;
       for (i=0;i<MAXENEMIES;i++)
 	    if (enemies[i].life) 
	     win1 << setxy(enemies[i].pt.x, enemies[i].pt.y) << (char) enemies[i].character;
	    c=0;
	 break; }
	
	/* switch character for player */
	switch (player.direction.x) {
	 case 0:
	  player.character=characters[0];
	 break;
	 case 1:
	  player.character=characters[1];
	 break;
	 case -1:
	  player.character=characters[7];
	break; }
	
	/* necessary movements for actors, player first */  
	/* is player falling down ? stop x direction */
	for (i=1;i<5;i++) 
	 if (screen[player.pt.x][player.pt.y+i] || jump_counter || screen[player.pt.x][player.pt.y]==LADDER)
	  break; 
    if (i>3)
	 player.direction.x=0;
    /* has player reached a dot ? jump at random */
	if (screen[player.pt.x+player.direction.x][player.pt.y]==DOT) {
	 n=rand() % 2;
     if (n)	
	c=SPACE; }		 
	/* move to x axis */
	if (check_coordinates_player(player.pt.x+player.direction.x, player.pt.y))
	 player.pt.x+=player.direction.x; 
    /* move to y axis, stop y direction if no floor below */
    if (screen[player.pt.x][player.pt.y]!=LADDER && !(screen[player.pt.x][player.pt.y+1]) && !jump_counter) {
	 player.pt.y++;
	player.direction.y=0; } 
	/* is asterisk, gift or money below player ? he must descend */
	if (screen[player.pt.x][player.pt.y+1]==ASTERISK || screen[player.pt.x][player.pt.y+1]==GIFT || screen[player.pt.x][player.pt.y+1]==MONEY) 
	 player.pt.y++;
	/* has player stepped on a single piece of ladder */
	if (screen[player.pt.x][player.pt.y+1]==LADDER && !(screen[player.pt.x][player.pt.y+2]) && !(screen[player.pt.x][player.pt.y-1])) {
	 player.pt.y++;
	player.direction.x=0; }
	/* player on a ladder with void under and direction y=1 */
	if (screen[player.pt.x][player.pt.y]==LADDER && !screen[player.pt.x][player.pt.y+1] && player.direction.y==1)
	 player.pt.y++;
    /* if player is on ladder, stop x direction */
    if (screen[player.pt.x][player.pt.y]==LADDER && screen[player.pt.x-player.direction.x][player.pt.y+1]!=FLOOR)
	 player.direction.x=0; 
    /* if player descends ladder, and ladder below, stop x direction */
    if (screen[player.pt.x][player.pt.y+1]==LADDER && player.direction.y==1) {
	 player.pt.y++;	
	player.direction.x=0; }
	/* if player ascends ladder, and ladder above, stop x direction and y direction when no more ladder */
    if (screen[player.pt.x][player.pt.y]==LADDER && player.direction.y==-1) {
	 if (screen[player.pt.x][player.pt.y-1]==LADDER || screen[player.pt.x][player.pt.y-1]==MONEY) {
	 player.pt.y--;
     player.direction.x=0; }
	 if (screen[player.pt.x][player.pt.y+player.direction.y]!=LADDER && screen[player.pt.x][player.pt.y+player.direction.y]!=MONEY)
	player.direction.y=0; }
    /* reduce jump counter if necessary and give extra x movement */
     if (jump_counter && screen[player.pt.x][player.pt.y]==LADDER) {
	  jump_counter=0;
     player.direction.x=0; }
	 if (jump_counter) {
	 if (!screen[player.pt.x][player.pt.y+jump_sequence[jump_counter-1]])  
	  player.pt.y+=jump_sequence[jump_counter-1];
	 if (check_coordinates_player(player.pt.x+player.direction.x, player.pt.y) && jump_counter>1)
	  player.pt.x+=player.direction.x; 
	--jump_counter; }

    /* now for the enemies */
    for (i=0;i<MAXENEMIES;i++) {
	 if (enemies[i].life) {
	  /* move to x axis if no obstacle */
	  if (check_coordinates_enemy(i, enemies[i].pt.x+enemies[i].direction.x, enemies[i].pt.y))
	   enemies[i].pt.x+=enemies[i].direction.x;
      /* move to y axis, enter asterisks and traps */
	  if (!(screen[enemies[i].pt.x][enemies[i].pt.y+1]) || screen[enemies[i].pt.x][enemies[i].pt.y+1]==ASTERISK || screen[enemies[i].pt.x][enemies[i].pt.y+1]==TRAP) {
	   enemies[i].direction.x=0; 
	  enemies[i].pt.y++; }
	  /* if sign is below, move right or left and down */
	  if (screen[enemies[i].pt.x][enemies[i].pt.y+1]==SIGN) {
	   n=rand() % 2;
	  enemies[i].direction.x=!n ? -1 : 1; }
	  /* if enemy is on floor with no direction x, give him one */
	  if (screen[enemies[i].pt.x][enemies[i].pt.y+1]==FLOOR && !enemies[i].direction.x) {
	   enemies[i].direction.y=0;
	   n=rand() % 2;
  	  enemies[i].direction.x= !n ? -1 : 1; } 
	  /* enemy found ladder, change x direction if random check succeeds */
	  if (screen[enemies[i].pt.x][enemies[i].pt.y]==LADDER) {
	   n=rand() % 2;
       if (n)
	  enemies[i].direction.x*=-1; }
     /* enemy found ladder, descend if random check succeeds */
	  if (screen[enemies[i].pt.x][enemies[i].pt.y+1]==LADDER) {
	   n=rand() % 2;
       if (n) {
        enemies[i].direction.x=0;
	  enemies[i].direction.y=1; } }		
	  if (enemies[i].direction.y)
	 enemies[i].pt.y++; } }
 
    /* show actors and copy background */
    player.bkgrnd=screen[player.pt.x][player.pt.y];
    win1 << setxy(player.pt.x, player.pt.y) << (char) player.character;
    for (i=0;i<MAXENEMIES;i++)
 	 if (enemies[i].life) {
	  enemies[i].bkgrnd=screen[enemies[i].pt.x][enemies[i].pt.y];
	win1 << setxy(enemies[i].pt.x, enemies[i].pt.y) << (char) enemies[i].character; }
   
    show_bar();
   
   /* check for collisions and jumps over enemies */
   for (i=0;i<MAXENEMIES;i++) {
    if (enemies[i].life) {	  
     if (player.pt.x==enemies[i].pt.x && player.pt.y==enemies[i].pt.y) 
	  lose_life(i);
    if (player.pt.x+player.direction.x==enemies[i].pt.x && player.pt.y==enemies[i].pt.y) 
	 lose_life(i);
    if (player.pt.x==enemies[i].pt.x+enemies[i].direction.x && player.pt.y==enemies[i].pt.y) 
	 lose_life(i);
    if (player.pt.x==enemies[i].pt.x && player.pt.y+player.direction.y==enemies[i].pt.y) 
	 lose_life(i);
    if (player.pt.x+player.direction.x==enemies[i].pt.x && player.pt.y==enemies[i].pt.y+enemies[i].direction.y) 
	 lose_life(i);
    if (player.pt.x+player.direction.x==enemies[i].pt.x && player.pt.y+player.direction.y==enemies[i].pt.y) 
     lose_life(i);
    if (screen[player.pt.x][player.pt.y]==TRAP || (screen[player.pt.x][player.pt.y+1]==TRAP)) {
	 enemies[254].pt.x=player.pt.x;
	 enemies[254].pt.y=player.pt.y;
	lose_life(254); }
    if (!jump_counter)
	 enemies[i].jumped=0;
	 for (n=0;n<3;n++)
      if (player.pt.x+(n*player.direction.x)==enemies[i].pt.x && enemies[i].pt.y-player.pt.y<3 && enemies[i].pt.y-player.pt.y>0 && !enemies[i].jumped) {
	   score+=200;
	   enemies[i].jumped=1; } 
    if (screen[enemies[i].pt.x][enemies[i].pt.y]==ASTERISK)
   reset_enemy(i);} }
  
    /* check for lives, goal, bonuses etc */
    if (screen[player.pt.x][player.pt.y]==MONEY) {
     show_message(1, 15, "Hooka!");
     award_bonus_time();
     ++level;
	 for (i=0;i<3;i++)
      gifts[i][1]=1;
    game_flag=0; }	 
    if (lives<1)
     c=ESC;
    ++bonus_counter;
    if (bonus_counter==25) {
     bonus_counter=0;
    if (bonus_time>0)
     bonus_time-=100; }
    if (!bonus_time && game_flag) {
     enemies[254].pt.x=player.pt.x;
     enemies[254].pt.y=player.pt.y;   
	 show_bar();
    lose_life(254); }
    for (i=1;i<11;i++) {
     if (score>=bonus_switch[i][0] && !bonus_switch[i][1]) {
      ++lives;
    bonus_switch[i][1]=1; } }
    if (screen[player.pt.x][player.pt.y]==GIFT){
     score+=2900;
     player.bkgrnd=0;
	 for (i=0;i<3;i++)
	  if (player.pt.y==gifts[i][0])
	   gifts[i][1]=0;
    screen[player.pt.x][player.pt.y]=0; }
  
    /* prepare for loop */
   Sleep(SLEEPTIME-level+extra_sleep); }
	
	/* exit routine */ 
    level=1;
 
    /* let's check for new highscore */
	if (score>retrieve_score(highscore_data[4])) {
	 win1.clrscr();
	 fcrypt(1, "ladder.dat", "ladder.~~~");
     f=fopen("ladder.~~~", "r");
	 for (i=0;i<2;i++) {
	  while (c!='~')
	   c=fgetc(f); 
     c=0; } 
     c=fgetc(f); 
	 while (c!='~') {
	  c=fgetc(f);
	  if (c!='~')
	 win1 << c; }
     fclose(f);
	 remove("ladder.~~~");
	 scanf("%s", name);
	 itoa(score, highscore, 10);
	 strcat(name, " ");
	 strcat(name, highscore);
	 strcpy(highscore_data[4], name);
	 highscore_maintenance(1); } 
	
    game_flag=-2;
}

  return 0;
}

/* design screen array according to levels */
void design_scene()
{
  int i,n;
  char *label;
  
  /* erase old screen */
  for (i=1;i<80;i++)
   for (n=1;n<21;n++)
    screen[i][n]=0;
  
   switch (scene) {

    case 1:
	 strcpy(label,"Easy Street");
	 screen[40][1]=WOMB;
	 for (i=8;i<68;i++) 
	  screen[i][4]=FLOOR;
	 for (i=1;i<80;i++) 
	  screen[i][8]=FLOOR; 
     for (i=47;i<50;i++) 
	  screen[i][8]=0;
	 screen[69][9]=SIGN;
	 screen[77][9]=SIGN;
	 for (i=0;i<strlen(label);i++)
	  screen[68+i][10]=label[i];
     for (i=8;i<63;i++) 
	  screen[i][12]=FLOOR;
	 for (i=38;i<40;i++) 
	  screen[i][12]=0;	 
	 for (i=1;i<73;i++) 
	  screen[i][16]=FLOOR; 
     screen[25][16]=0;
	 screen[48][16]=0;
	 for (i=1;i<80;i++) 
	  screen[i][20]=FLOOR; 
     for (i=1;i<20;i++) {
	  screen[17][i]=LADDER;
	 screen[58][i]=LADDER; }
	 screen[17][1]=0;
	 screen[17][2]=0;
	 gifts[0][0]=9;
	 if (gifts[0][1])
	  screen[17][gifts[0][0]]=GIFT;
	 screen[17][10]=0;
	 screen[17][17]=0;
	 screen[17][18]=0;
	 screen[17][19]=0;
	 screen[17][20]=FLOOR;
	 screen[17][16]=FLOOR;
	 screen[58][1]=MONEY;
	 screen[58][4]=FLOOR;
	 screen[58][5]=0;
	 screen[58][6]=0;
	 screen[58][12]=FLOOR;
	 screen[58][13]=0;
	 screen[58][14]=0;
	 screen[28][7]=LADDER;
	 screen[28][8]=LADDER;
	 screen[28][9]=LADDER;
	 screen[28][12]=LADDER;
	 screen[1][19]=ASTERISK;
	 screen[79][19]=ASTERISK;
	break;
	case 2:
	 strcpy(label,"Long Island");
	 screen[75][1]=MONEY;
	 gifts[0][0]=2;
	 if (gifts[0][1])
	  screen[69][2]=GIFT;
	 screen[69][3]=WOMB;
	 screen[70][3]=SIGN;
	 screen[13][3]=SIGN;
	 for (i=1;i<78;i++) {
	  screen[i][4]=FLOOR;
	  screen[i][8]=FLOOR;
	  screen[i][12]=FLOOR;
	  screen[i][16]=FLOOR;
	 screen[i][20]=FLOOR; }
	 screen[29][4]=0;
	 screen[55][4]=0;
	 screen[78][20]=FLOOR;
	 screen[79][20]=FLOOR;
	 gifts[1][0]=7;
	 if (gifts[1][1])
	  screen[26][7]=GIFT;
	 screen[27][8]=0;
	 screen[28][7]=SIGN;
	 screen[34][8]=0;
	 screen[35][8]=0;
	 screen[54][7]=DOT;
	 screen[55][8]=0;
	 screen[56][7]=DOT;
	 screen[27][12]=0;
	 screen[28][12]=0;
	 screen[29][12]=0;
	 screen[35][10]=SIGN;
	 screen[35][11]=SIGN;
	 screen[36][12]=0;
	 screen[37][12]=0;
	 screen[53][11]=DOT;
	 screen[54][12]=0;
	 screen[55][12]=0;
	 screen[56][11]=DOT;
	 screen[28][14]=SIGN;
	 screen[28][15]=SIGN;
	 screen[26][16]=0;
	 screen[27][16]=0;
	 screen[36][16]=0;
	 screen[37][16]=0;
	 screen[38][16]=0;
	 screen[39][16]=0;
	 screen[53][15]=DOT;
	 screen[54][16]=0; 
	 screen[55][16]=0;
	 screen[56][16]=0;
	 screen[57][15]=DOT;
	 for (i=1;i<15;i++)
	  screen[i][18]=FLOOR;
     screen[14][19]=SIGN;
	 for (i=0;i<strlen(label);i++) 
	  screen[2+i][19]=label[i];
     screen[28][19]=ASTERISK;
	 screen[37][18]=SIGN;
	 screen[37][19]=SIGN;
	 screen[55][19]=ASTERISK;
	 for (i=2;i<17;i++) {
	  screen[5][i]=LADDER;
	 screen[75][i]=LADDER; }
	 screen[5][2]=0;
	 screen[5][8]=FLOOR;
	 screen[5][9]=0;
	 screen[5][10]=0;
	 screen[5][16]=FLOOR;
	 screen[75][4]=FLOOR;
	 screen[75][5]=0;
	 screen[75][6]=0;
	 screen[75][12]=FLOOR;
	 screen[75][13]=0;
	 screen[75][14]=0;
	 screen[75][17]=LADDER;
	 screen[75][18]=LADDER;
	 screen[75][19]=LADDER;
	break;
	case 3:
	 strcpy(label,"Ghost Town");
	 screen[29][1]=WOMB;
	 screen[45][1]=WOMB;
	 screen[57][1]=WOMB;
	 screen[73][1]=MONEY;
	 for (i=0;i<3;i++) {
	  screen[72+i][2]=MONEY;
	 screen[72+i][3]=MONEY; }
	 screen[71][3]=MONEY;
	 screen[75][3]=MONEY;
	 screen[79][9]=MONEY; 
	 for (i=1;i<15;i++)
	  screen[i][4]=FLOOR;
     for (i=63;i<80;i++)
	  screen[i][4]=FLOOR;
     gifts[0][0]=7;
	 if (gifts[0][1])
      screen[42][7]=GIFT;
     for (i=6;i<70;i++)
	  screen[i][8]=FLOOR;
     screen[20][8]=0;
	 screen[21][8]=0;
	 screen[22][8]=0;
	 screen[27][8]=0;
	 screen[28][8]=0;
	 screen[29][8]=0;
	 screen[30][8]=0;
	 screen[31][8]=0;
	 screen[33][8]=0;
	 screen[34][8]=0;
	 screen[35][8]=0;
	 screen[36][8]=0;
	 screen[43][8]=0;
	 screen[44][8]=0;
	 screen[45][8]=0;
	 screen[46][8]=0;
     screen[48][8]=0;
	 screen[49][8]=0;
	 screen[50][8]=0;
	 screen[55][8]=0;
	 screen[56][8]=0;
	 screen[57][8]=0;
	 screen[58][8]=0;
	 screen[20][9]=TRAP;
	 screen[21][9]=TRAP;
	 screen[22][9]=TRAP;
	 screen[27][9]=TRAP;
	 screen[28][9]=TRAP;
	 screen[29][9]=TRAP;
	 screen[30][9]=TRAP;
	 screen[31][9]=TRAP;
	 screen[33][9]=TRAP;
	 screen[34][9]=TRAP;
	 screen[35][9]=TRAP;
	 screen[36][9]=TRAP;
	 screen[43][9]=TRAP;
	 screen[44][9]=TRAP;
	 screen[45][9]=TRAP;
     screen[46][9]=TRAP;
     screen[48][9]=TRAP;
	 screen[49][9]=TRAP;
	 screen[50][9]=TRAP;
	 screen[55][9]=TRAP;
	 screen[56][9]=TRAP;
	 screen[57][9]=TRAP;
	 for (i=11;i<76;i++)
	  screen[i][12]=FLOOR;
     screen[11][11]=SIGN;
	 gifts[1][0]=11;
	 if (gifts[1][1])
	  screen[63][11]=GIFT;
	 screen[71][10]=SIGN;
	 screen[71][11]=SIGN;
	 screen[64][12]=0;
	 for (i=15;i<80;i++)
	  screen[i][16]=FLOOR;
     gifts[2][0]=13;
	 if (gifts[2][1])
     screen[14][13]=GIFT;
     screen[15][15]=SIGN;
	 screen[33][18]=TRAP;
	 screen[32][19]=TRAP;
	 screen[33][19]=TRAP;
	 screen[34][19]=TRAP;
	 screen[1][19]=ASTERISK;
	 screen[79][19]=ASTERISK;
	 for (i=1;i<80;i++)
	  screen[i][20]=FLOOR;
     for (i=0;i<strlen(label);i++)
	  screen[5][9+i]=label[i];
     for (i=0;i<5;i++) {
	  screen[11][3+i]=LADDER;
	  screen[64][3+i]=LADDER;
	  screen[79][3+i]=LADDER;
	  screen[33][11+i]=LADDER;
	 screen[71][15+i]=LADDER; }
	 screen[79][8]=LADDER;
	 screen[64][8]=LADDER;
	  screen[33][16]=LADDER;
	  screen[71][19]=LADDER;
	 screen[11][8]=FLOOR;
   break; }
}

/* set up actor structures */
void set_up_actors()
{
  int i,n;
  int r;
  
   switch (scene) {
	case 1:
	 player.character=characters[0];
	 player.pt.x=2;
	 player.pt.y=19;
	 player.life=1;
	 player.bkgrnd=screen[player.pt.x][player.pt.y];
	 player.direction.x=0;
	 player.direction.y=0;
	 wombs[0].x=40;
	 wombs[0].y=2;
	 for (i=0;i<MAXENEMIES;i++) {
	  enemies[i].character='o';
	 reset_enemy(i); }
	 break;
	 case 2:
	  player.character=characters[0];
	  player.pt.x=15;
	  player.pt.y=19;
	  player.life=1;
	  player.bkgrnd=screen[player.pt.x][player.pt.y];
	  player.direction.x=0;
	  player.direction.y=0;
	  wombs[0].x=14;
	  wombs[0].y=3;
	  wombs[1].x=68;
	  wombs[1].y=3;
	  for (i=0;i<MAXENEMIES;i++) {
	   enemies[i].character='o'; 
	  reset_enemy(i); }
	 break;
	 case 3:
	  player.character=characters[0];
	  player.pt.x=5;
	  player.pt.y=3;
	  player.life=1;
	  player.bkgrnd=screen[player.pt.x][player.pt.y];
	  player.direction.x=0;
	  player.direction.y=0;
	  wombs[0].x=29;
	  wombs[0].y=2;
	  wombs[1].x=45;
	  wombs[1].y=2;
	  wombs[2].x=57;
	  wombs[2].y=2;
	  for (i=0;i<MAXENEMIES;i++) {
	   enemies[i].character='o'; 
	  reset_enemy(i); }
   break; }
}
	  
/* return true if player can move left or right */
int check_coordinates_player(int x, int y) 
{
  int i;	
	
    if (x<1 || x>79) {
     player.direction.x=0;
   return 0; }
   
    if (screen[x][y]==WOMB || screen[x][y]==SIGN) {
     player.direction.x=0;
    return 0; }
	
	if (screen[x][y]==DOT ) {
	 i=rand() % 2;
	 if (!i)
	player.direction.x*=-1; }
	
 return 1;
} 

/* return true if enemy can move left or right */
int check_coordinates_enemy(int id, int x, int y)
{
  int i;	
	
    if (x<1 || x>79) {
     enemies[id].direction.x*=-1;
   return 0; }
   
    if (screen[x][y]==WOMB || screen[x][y]==SIGN) {
     enemies[id].direction.x*=-1;
    return 0; }
	
	if (screen[x][y]==DOT ) {
	 i=rand() % 2;
	 if (!i)
	enemies[id].direction.x*=-1; }
  
 return 1;
} 

/* show a message at the bottom of the screen */
void show_message(int flag, int duration, char *s)
{
   int i;
   
   for (i=0;i<duration;i++) {
    if (kbhit())
	 break;
	win1 << setxy(1,22);
     win1 << setattr(WHITE) << s;
	Sleep(SLEEPTIME);
	 win1 << setxy(1,22);
	if (flag) 
     win1 << setattr(BLACK) << s;
    Sleep(SLEEPTIME); } 
   win1 << setxy(1,22);
   win1 << setattr(BLACK) << s;
}

/* routine for life loss */
void lose_life(int id)
{
  int i,n;
  
	--lives;
	win1 << setxy(enemies[id].pt.x, enemies[id].pt.y) << (char) screen[enemies[id].pt.x][enemies[id].pt.y];
	n=0;
	while (characters[n]!='-') {
	  ++n;
	  if (n>8)
	   n=0;
	win1 << setxy(player.pt.x, player.pt.y) << (char) characters[n]; }
	++n;
	for (i=0;i<5;i++) {
     while (characters[n]!='-') {
	  ++n;
	  if (n>8)
	   n=0;
	  win1 << setxy(player.pt.x, player.pt.y) << (char) characters[n];
    Sleep(SLEEPTIME); } }
	player.pt.x=0;
	jump_counter=0;
   game_flag=0; 
}

/* reset enemy */
void reset_enemy(int id)
{
  int r;
  
   win1 << setxy (enemies[id].pt.x, enemies[id].pt.y) << enemies[id].bkgrnd;
   r=rand() % scene;  
   enemies[id].life=0;
   enemies[id].pt.x=wombs[r].x;
   enemies[id].pt.y=wombs[r].y;
   enemies[id].jumped=0;
   enemies[id].bkgrnd=screen[enemies[id].pt.x][enemies[id].pt.y];
   enemies[id].direction.y=0;
   r=rand() % 2;
   if (scene!=3)
    enemies[id].direction.x= !r ? -1 : 1; 
   else
	enemies[id].direction.x=0;
}

/* show informational bar */
void show_bar()
{
   win1 << setxy(1,21);
   printf("Lads    %d     Level    %d     Score      %.3d                Bonus time    %.4d", lives, level, score, bonus_time);
}
   
/* reduce bonus time and add to score */
void award_bonus_time()
{
   while (bonus_time) {
    bonus_time-=100;
    score+=100;
	show_bar();
   Sleep(SLEEPTIME); }
}   

/* read&write highscores in .dat file */
void highscore_maintenance(int flag) /* 0 read, 1 write */
{
  int i,n;
  char c;
  size_t size;
  void *str;
  char s[30];
  FILE *f2;

   switch(flag) {
	case 0:
     f=fopen("ladder.~~~", "r");
     for (i=0;i<3;i++) {
	  while (c!='~')
	   c=fgetc(f); 
     c=0; }
     n=0;
     for (i=0;i<5;i++) {
	  win1 << setxy(43,17+i);
	  while (c!='~') {
	   c=fgetc(f);
	   if (c!='~' && c!='\n') 
 	   s[n++]=c; } 
	  s[n]='\0';
	  size=sizeof(s);
	  str=malloc(size);
	  memcpy(str, s, size);
	  highscore_data[i]= (char *) str;
	  n=0;
     c=0;	} 
     fclose(f);
	 remove("ladder.~~~");
	break;
	case 1:
	 sort_highscores();
	 fcrypt(1, "ladder.dat", "ladder.~~~");
     f=fopen("ladder.~~~", "r");
	 f2=fopen("ladder.tmp", "w");
     for (i=0;i<3;i++) {
	  while (c!='~') {
	   c=fgetc(f);   
	  fputc(c,f2); }
     c=0; }
	 fclose(f);
	 for (i=0;i<5;i++) {
	  fprintf(f2, highscore_data[i]);
	 fputc('~', f2); }
	 fclose(f2);
	 /* copy_file("ladder.tmp", "ladder.dat"); */
	 fcrypt(0, "ladder.tmp", "ladder.dat");
	 remove("ladder.~~~");
	 remove("ladder.tmp\n");
   break; }
   
}

/* retrieve score from highscore_data array */
long int retrieve_score(char *s)
{
  int n,l;	
  char num[10];
  
  l=0;
  n=0;
    while (isalpha(s[n++]));
	while (s[n])
	 num[l++]=s[n++];
     num[l]='\0'; 
	 
    l=atoi(num);
 
  return l;
}

/* sort highscores array */
void sort_highscores()
{
 int i,n;
 long int temp;
 long int highscores[5];
 char s[30];

    for (i=0;i<5;i++)
     highscores[i]=retrieve_score(highscore_data[i]);	
	  
     for (i=0;i<5;i++) {
      for (n=0;n<4;n++)  {
       if (highscores[n+1]>highscores[n]) {
        temp=highscores[n];
        highscores[n]=highscores[n+1];
	    highscores[n+1]=temp; 	
		strcpy(s, highscore_data[n]);
		strcpy(highscore_data[n], highscore_data[n+1]); 
	 strcpy(highscore_data[n+1], s); } } } 
}

/* copy source to destination file */
int copy_file(char *source, char *dest)
{
  FILE *f;
  FILE *w;
  char c;
  char buf[BUFSIZ];
  size_t size;
  
   f=fopen(source, "rb");
   w=fopen(dest, "wb");
   if (!f || !w)
	return -1;
   
    while (size = fread(buf, 1, BUFSIZ, f)) 
     fwrite(buf, 1, size, w);
	
	fclose(f);
	fclose(w);
	
  return 0;
}

 