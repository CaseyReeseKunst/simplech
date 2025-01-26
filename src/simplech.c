/*_____________________________________________________________________________

  ----------> name: simple checkers
  ----------> author: martin fierz
  ----------> purpose: platform independent checkers engine
  ----------> version: 1.11
  ----------> date: 8th october 98
  ----------> description: checkers.c contains a simple but fast  
  checkers engine and a simple interface to this engine. checkers.c
  contains three main parts: interface, search and move
  generation. these parts are separated in the code. if you want to
  improve on the play of checkers, you will mainly have to improve on
  the evaluation function; this version does nothing but count
  material.  even though it only counts pieces, this checkers engine
  will blow away all shareware checkers programs you will find on the
  internet, with the exception of blitz54, which is written for DOS.

  board representation: the standard checkers notation is

  (white)
  32  31  30  29
  28  27  26  25
  24  23  22  21
  20  19  18  17
  16  15  14  13
  12  11  10   9
  8   7   6   5
  4   3   2   1
  (black)

  the internal representation of the board is different, it is a
  array of int with length 46, the checkers board is numbered
  like this:

  (white)
  37  38  39  40
  32  33  34  35
  28  29  30  31
  23  24  25  26
  19  20  21  22
  14  15  16  17
  10  11  12  13
  5   6   7   8
  (black)

  let's say, you would like to teach the program that it is
  important to keep a back rank guard. you can for instance
  add the following (not very sophisticated) code for this:

  if(b[6] & (BLACK|MAN)) eval++;
  if(b[8] & (BLACK|MAN)) eval++;
  if(b[37] & (WHITE|MAN)) eval--;
  if(b[39] & (WHITE|MAN)) eval--;

  the evaluation function is seen from the point of view of the
  black player, so you increase the value v if you think the
  position is good for black.

  if you want to program a different interface, you call the
  function "checkers":

  checkers(int b[46],int color, double maxtime, char *str);

  in b[46] you store the position, each square having one of
  the values of {FREE, BLACK|MAN, BLACK|KING, WHITE|MAN, WHITE|KING,
  OCCUPIED}. OCCUPIED are those squares which do not appear in the
  above board representation, i.e. 0,1,2,3,9,18,27,36,41-45.
  color is the color to move, either BLACK or WHITE.
  maxtime is the about half the average time it will take to
  calculate a move.

  after checkers completes, you will have the new board position in
  b[46] and some information on the search in str.

  initcheckers(int b[46]) initializes board b to the starting
  position of checkers.

  have fun!

  questions, comments, suggestions to:

  Martin Fierz
  mafierz@ibm.net

  ----------> choose your platform */
#undef WIN95
#undef DOS
#undef MAC
#define UNIX

#ifdef WIN95
#include <windows.h>
unsigned int _stklen = 50000;
#endif
#ifdef DOS
#define int long
unsigned int _stklen = 40000;
#endif
#ifdef MAC

#endif

/*----------> includes */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef UNIX
#define TICKS CLOCKS_PER_SEC
#else
#define TICKS CLK_TCK
#endif

/*----------> definitions */
#define OCCUPIED 0
#define WHITE 1
#define BLACK 2
#define MAN 4
#define KING 8
#define FREE 16
#define CHANGECOLOR 3
#define MAXDEPTH 99
#define MAXMOVES 20
#define MAXTIME 0.05

/*----------> compile options  */
#undef MUTE
#undef VERBOSE
#undef STATISTICS
#undef SHOWLONGMOVES
#ifdef SHOWLONGMOVES
#define LONGMOVE 5
#endif

/*----------> structure definitions  */
struct move2
  {
    short n;
    int m[8];
  };

/*----------> function prototypes  */
/*----------> part I: interface */
void printboard (int human, int b[46], int color);
void initcheckers (int b[46]);
void timerroutine (int human, int b[46]);
void movetonotation (struct move2 move, char str[80]);
char *reduce (char str[80]);	/* pch */
int partof (char buf[240], char str[80]);	/* pch */
/*----------> part II: search */
int checkers (int b[46], int color, double maxtime, char *str);
int alphabeta (int b[46], int depth, int alpha, int beta, int color);
int firstalphabeta (int b[46], int depth, int alpha, int beta, int color, struct move2 *best);
void domove (int b[46], struct move2 move);
void undomove (int b[46], struct move2 move);
int evaluation (int b[46], int color);
/*----------> part III: move generation */
int generatemovelist (int b[46], struct move2 movelist[MAXMOVES], int color);
int generatecapturelist (int b[46], struct move2 movelist[MAXMOVES], int color);
void blackmancapture (int b[46], int *n, struct move2 movelist[MAXMOVES], int square);
void blackkingcapture (int b[46], int *n, struct move2 movelist[MAXMOVES], int square);
void whitemancapture (int b[46], int *n, struct move2 movelist[MAXMOVES], int square);
void whitekingcapture (int b[46], int *n, struct move2 movelist[MAXMOVES], int square);
int testcapture (int b[46], int color);

/*----------> globals  */
#ifdef STATISTICS
int alphabetas, generatemovelists, evaluations, generatecapturelists, testcaptures;
#endif
int value[17] =
{0, 0, 0, 0, 0, 1, 256, 0, 0, 16, 4096, 0, 0, 0, 0, 0, 0};


/*-------------- PART I: INTERFACE ------------------------------------------*/

int
main ()
/*----------> purpose: provide a simple interface to checkers.
  ----------> version: 1.0
  ----------> date: 24th october 97 */
/*----------> modified for xcheckers, pch,
  ----------> 99-02-09 */
{
  char str[80];
  char buf[240];
  int askme = 1;
  int human = BLACK;
  int program = WHITE;
  int len;
  int b[46];
  int choice;
  int n;
  struct move2 movelist[MAXMOVES];
  int i;
  double maxtime;

#ifdef WIN95
/*----------> WIN95: get a new console, set its title, input and output */
  FreeConsole ();
  AllocConsole ();
  SetConsoleTitle ("Checkers 99");
#endif

  setvbuf (stdout, NULL, _IONBF, 0);

  printf ("\nsimple checkers version 1.11pl5");
  printf ("\n8th october 98, 27th november 99");
  printf ("\nby martin fierz");

  do
    {
      printf ("\n");
      maxtime = MAXTIME;
      askme = 1;

      i = 1;
      while (i)
	{
	  printf ("\ntype b/w to play black/white: ");
	  n = tolower (getchar ());
	  if (n == 'w')
	    {
	      human = WHITE;
	      program = BLACK;
	      i = 0;
	    }
	  else if (n == 'b')
	    {
	      human = BLACK;
	      program = WHITE;
	      i = 0;
	    }
	  else if (n == 'q')
	    {
	      askme = 0;
	      i = 0;
	    }
	  /* clear buffer */
	  while (n != '\n' && n != EOF)
	    n = getchar ();
	}
      if (!askme)
	break;

      i = 1;
      while (i)
	{
	  printf ("\nset program strength (1-9): ");
	  n = getchar ();
	  if (isdigit (n))
	    {
	      n -= 48;
	      if (n < 10 && n > 0)
		{
		  n *= n;
		  maxtime *= n;
		  i = 0;
		}
	    }
	  else if (n == 'q')
	    {
	      askme = 0;
	      i = 0;
	    }
	  /* clear buffer */
	  while (n != '\n' && n != EOF)
	    n = getchar ();
	}
      if (!askme)
	break;

      initcheckers (b);

      if (program == BLACK)
	{
	  printboard (human, b, program);
	  checkers (b, program, maxtime, &str[0]);
	}

      do
	{
	  len = 0;
	  choice = 0;
	  printboard (human, b, human);
	  if (testcapture (b, human))
	    n = generatecapturelist (b, movelist, human);
	  else
	    n = generatemovelist (b, movelist, human);
	  if (n <= 0)
	    {
	      printf ("program wins\n");
	      break;
	    }
	  for (i = 0; i < n; i++)
	    {
	      if (!(i % 6) && (i))
		printf ("\n");
	      movetonotation (movelist[i], str);
	      printf ("%c: %s   ", i + 97, str);
	      len += sprintf (buf + len, "%c: %s ", i + 97, str);
	    }
	  printf ("\nyour move? (t tests, z leaves)  ");
	  scanf ("%70s", &str[0]);
	  if (strstr (str, "new"))
	    {
	      askme = 2;
	      break;
	    }
	  if (strstr (str, "quit"))
	    {
	      askme = 0;
	      break;
	    }
	  if (isalpha (str[0]))
	    choice = tolower (str[0] - 96);
	  else if ((i = partof (buf, reduce (str))) >= 0)
	    choice = (buf[i] - 96);
	  if (choice == 20)
	    {
	      printf ("\ntesting, please be patient\n");
	      timerroutine (human, b);
	      continue;
	    }
	  if (choice == 26)
	    {
	      break;
	    }
	  if (choice <= 0 || choice > n)
	    {
	      printf ("\ninvalid move!\n");
	      continue;
	    }
	  domove (b, movelist[choice - 1]);
	  printboard (human, b, program);
	  if (!checkers (b, program, maxtime, &str[0]))
	    {
	      printf ("you win!\n");
	      break;
	    }
#ifdef VERBOSE
	  printf ("%s", str);
#endif
	}
      while (1);

      while (n != '\n' && n != EOF)
	n = getchar ();

      i = 1;
      while (i == askme)
	{
	  printf ("\nq quits, p to play again: ");
	  n = tolower (getchar ());
	  if (n == 'q')
	    askme = 0;
	  else if (n == 'p' || n == 'n')
	    i = 0;
	  while (n != '\n' && n != EOF)
	    n = getchar ();
	}
    }
  while (askme);

  printf ("\nsimple checkers version 1.11pl5");
  printf ("\n8th october 98, 27th november 99");
  printf ("\nby martin fierz");
  printf ("\n");
  return (0);
}

/* enables input of moves in the format here-there (pdn), e.g.  11-15,
 * looks up the move entered in the movelist and returns its position;
 * or less than zero if not found. pch, 99-02-09 */

char *
reduce (char str[80])		/* pch */
{
  int i = strcspn (str, "-");
  char *tmp = strdup (str);

  if ((tmp = strrchr (tmp, '-')) == 0)
    return ("0-0");
  str[i] = 0;
  strcat (str, tmp);
  return (str);
}

int
partof (char buf[240], char str[80])	/* pch */
{
  int len = strlen (str);
  int pos = strlen (buf) - 7;

  while (pos >= 0)
    {
      if (!isalpha (buf[pos]))
	{
	  pos--;
	  continue;
	}
      if (!strncmp (&buf[pos + 3], str, len))
	{
	  if (isgraph (buf[pos + 3 + len]))
	    {
	      pos--;
	      continue;
	    }
	  break;
	}
      pos--;
    }
  return pos;
}

/* end move conversion  */

void
movetonotation (struct move2 move, char str[80])
{
  int j, from, to;
  char c;

  from = move.m[0] % 256;
  to = move.m[1] % 256;
  from = from - (from / 9);
  to = to - (to / 9);
  from -= 5;
  to -= 5;
  j = from % 4;
  from -= j;
  j = 3 - j;
  from += j;
  j = to % 4;
  to -= j;
  j = 3 - j;
  to += j;
  from++;
  to++;
  c = '-';
  sprintf (str, "%i%c%i", from, c, to);
}

void
initcheckers (int b[46])
/*----------> purpose: initialize the checkerboard
  ----------> version: 1.0
  ----------> date: 24th october 97 */
{
  int i;

  for (i = 0; i < 46; i++)
    b[i] = OCCUPIED;

  for (i = 5; i <= 40; i++)
    b[i] = FREE;

  for (i = 5; i <= 17; i++)
    b[i] = (BLACK | MAN);

  for (i = 28; i <= 40; i++)
    b[i] = (WHITE | MAN);

  for (i = 9; i <= 36; i += 9)
    b[i] = OCCUPIED;
}

void
timerroutine (int human, int b[46])
/*----------> purpose: time the other routines
  ----------> version: 1.0
  ----------> date: 24th october 97 */
{

#ifdef VERBOSE
  printf ("\nno timing in verbose mode!");
#else
  int i;
  double start, time;
  struct move2 movelist[20];
  double factor;
  int capture;

  start = clock ();
  for (i = 0; i < 1000000; i++)
    evaluation (b, human);
  time = (clock () - start) / TICKS;
  factor = (1.44 / time);
  printf ("\n\nevaluations: time for 1M iterations is %2.2f s, %2.2f", time, factor);

  start = clock ();
  for (i = 0; i < 1000000; i++)
    capture = testcapture (b, human);
  time = (clock () - start) / TICKS;
  factor = (2.44 / time);
  printf ("\ntestcapture: time for 1M iterations is %2.2f s, %2.2f", time, factor);

  if (capture)
    {
      start = clock ();
      for (i = 0; i < 1000000; i++)
	generatecapturelist (b, movelist, human);
      time = (clock () - start) / TICKS;
      printf ("\ncapturelist: time for 1M iterations is %2.2f s", time);
    }
  else
    {
      start = clock ();
      for (i = 0; i < 1000000; i++)
	generatemovelist (b, movelist, human);
      time = (clock () - start) / TICKS;
      factor = (2.44 / time);
      printf ("\nmovelist: time for 1M iterations is %2.2f s, %2.2f", time, factor);
    }

  start = clock ();
  for (i = 0; i < 2000000; i++)
    domove (b, movelist[0]);
  time = (clock () - start) / TICKS;
  factor = (0.79 / time);
  printf ("\ndomove: time for 2M iterations is %2.2f s, %2.2f\n\n", time, factor);
  undomove (b, movelist[0]);	/* pch */
#endif
}

void
printboard (int human, int b[46], int color)
/*----------> purpose: print the checkerboard
  ----------> version: 1.0
  ----------> date: 24th october 97 */
/*----------> modified pch, to print
  ----------> reverse board, 99-02-09 */
{
  int i, j;
  char c[18] = "     wb  WB     --";

  if (human == BLACK)
    {
      if (color == BLACK)
	printf ("\n   --------------- ");
      else
	printf ("\n.  --------------- ");
      for (i = 37; i >= 10; i -= 9)
	{
	  printf ("\n   ");
	  for (j = i; j <= i + 3; j++)
	    printf ("  %c ", c[b[j]]);
	  printf ("\n   ");
	  for (j = i - 5; j <= i - 2; j++)
	    printf ("%c   ", c[b[j]]);
	}
    }
  else
    /* human == WHITE */
    {
      if (color == WHITE)
	printf ("\n*  --------------- ");
      else
	printf ("\n-  --------------- ");
      for (i = 10; i <= 37; i += 9)
	{
	  printf ("\n   ");
	  for (j = i - 2; j >= i - 5; j--)
	    printf ("  %c ", c[b[j]]);
	  printf ("\n   ");
	  for (j = i + 3; j >= i; j--)
	    printf ("%c   ", c[b[j]]);
	}
    }
  printf ("\n\n");
}

/*-------------- PART II: SEARCH --------------------------------------------*/


int
checkers (int b[46], int color, double maxtime, char *str)
/*----------> purpose: entry point to checkers. find a move on board b for color
  ---------->          in the time specified by maxtime, write the best move in
  ---------->          board, returns information on the search in str
  ----------> returns 1 if a move is found & executed, 0, if there is no legal
  ----------> move in this position.
  ----------> version: 1.1
  ----------> date: 9th october 98 */
{
  int i, numberofmoves;
  double start;
  int eval;
  struct move2 best, movelist[MAXMOVES];
  char msg[80];

#ifdef STATISTICS
  alphabetas = 0;
  generatemovelists = 0;
  generatecapturelists = 0;
  evaluations = 0;
#endif

/*--------> check if there is only one move */
  numberofmoves = generatecapturelist (b, movelist, color);
  if (numberofmoves == 1)
    {
      domove (b, movelist[0]);
      sprintf (str, "forced capture");
      movetonotation (movelist[0], msg);	/* pch */
      printf ("program chooses %s\n", msg);	/* pch */
      return (1);
    }
  numberofmoves += generatemovelist (b, movelist, color); /* pch += */
  if (numberofmoves == 1)
    {
      domove (b, movelist[0]);
      sprintf (str, "only move");
      movetonotation (movelist[0], msg);	/* pch */
      printf ("program chooses %s\n", msg);	/* pch */
      return (1);
    }
/*--------> check if there is a move at all */
  if (numberofmoves == 0)
    return (0);

  start = clock ();
  eval = firstalphabeta (b, 1, -10000, 10000, color, &best);
  for (i = 2; (i <= MAXDEPTH) && ((clock () - start) / TICKS < maxtime); i++)
    {
      eval = firstalphabeta (b, i, -10000, 10000, color, &best);
#ifndef MUTE
      printf ("t %2.2f, d %2i, v %4i\n", (clock () - start) / TICKS, i, eval);
#ifdef STATISTICS
      printf ("  nod %i, gm %i, gc %i, eva %i",
	      alphabetas, generatemovelists, generatecapturelists,
	      evaluations);
#endif
#endif
    }
  i--;
#ifdef STATISTICS
  sprintf (str, "\n\nt %2.2f, d %2i, v %4i  nod %i, gm %i, gc %i, eva %i", (clock () - start) / TICKS, i, eval, alphabetas, generatemovelists, generatecapturelists, evaluations);
#else
  sprintf (str, "\n\ntime %2.2f, depth %i, eval %i", (clock () - start) / TICKS, i, eval);
#endif
  domove (b, best);
  movetonotation (best, msg);	/* pch */
  printf ("program chooses %s\n", msg);		/* pch */

  return (1);
}

int
firstalphabeta (int b[46], int depth, int alpha, int beta, int color, struct move2 *best)
/*----------> purpose: search the game tree and find the best move.
  ----------> version: 1.0
  ----------> date: 25th october 97 */
{
  int i;
  int value;
  int numberofmoves;
  int capture;
  struct move2 movelist[MAXMOVES];

#ifdef STATISTICS
  alphabetas++;
#endif

/*----------> test if captures are possible */
  capture = testcapture (b, color);

/*----------> recursion termination if no captures and depth=0*/
  if (depth == 0)
    {
      if (capture == 0)
	return (evaluation (b, color));
      else
	depth = 1;
    }

/*----------> generate all possible moves in the position */
  if (capture == 0)
    {
      numberofmoves = generatemovelist (b, movelist, color);
/*----------> if there are no possible moves, we lose: */
      if (numberofmoves == 0)
	{
	  if (color == BLACK)
	    return (-5000);
	  else
	    return (5000);
	}
    }
  else
    numberofmoves = generatecapturelist (b, movelist, color);

/*----------> for all moves: execute the move, search tree, undo move. */
  for (i = 0; i < numberofmoves; i++)
    {
      domove (b, movelist[i]);

      value = alphabeta (b, depth - 1, alpha, beta, (color ^ CHANGECOLOR));

      undomove (b, movelist[i]);
      if (color == BLACK)
	{
	  if (value >= beta)
	    return (value);
	  if (value > alpha)
	    {
	      alpha = value;
	      *best = movelist[i];
	    }
	}
      if (color == WHITE)
	{
	  if (value <= alpha)
	    return (value);
	  if (value < beta)
	    {
	      beta = value;
	      *best = movelist[i];
	    }
	}
    }
  if (color == BLACK)
    return (alpha);
  return (beta);
}

int
alphabeta (int b[46], int depth, int alpha, int beta, int color)
/*----------> purpose: search the game tree and find the best move.
  ----------> version: 1.0
  ----------> date: 24th october 97 */
{
  int i;
  int value;
  int capture;
  int numberofmoves;
  struct move2 movelist[MAXMOVES];

#ifdef STATISTICS
  alphabetas++;
#endif

/*----------> test if captures are possible */
  capture = testcapture (b, color);

/*----------> recursion termination if no captures and depth=0*/
  if (depth == 0)
    {
      if (capture == 0)
	return (evaluation (b, color));
      else
	depth = 1;
    }

/*----------> generate all possible moves in the position */
  if (capture == 0)
    {
      numberofmoves = generatemovelist (b, movelist, color);
/*----------> if there are no possible moves, we lose: */
      if (numberofmoves == 0)
	{
	  if (color == BLACK)
	    return (-5000);
	  else
	    return (5000);
	}
    }
  else
    numberofmoves = generatecapturelist (b, movelist, color);

/*----------> for all moves: execute the move, search tree, undo move. */
  for (i = 0; i < numberofmoves; i++)
    {
      domove (b, movelist[i]);

      value = alphabeta (b, depth - 1, alpha, beta, color ^ CHANGECOLOR);

      undomove (b, movelist[i]);

      if (color == BLACK)
	{
	  if (value >= beta)
	    return (value);
	  if (value > alpha)
	    alpha = value;
	}
      if (color == WHITE)
	{
	  if (value <= alpha)
	    return (value);
	  if (value < beta)
	    beta = value;
	}
    }
  if (color == BLACK)
    return (alpha);
  return (beta);
}

void
domove (int b[46], struct move2 move)
/*----------> purpose: execute move on board
  ----------> version: 1.1
  ----------> date: 25th october 97 */
{
  int square, after;
  int i;

  for (i = 0; i < move.n; i++)
    {
      square = (move.m[i] % 256);
      after = ((move.m[i] >> 16) % 256);
      b[square] = after;
    }
}

void
undomove (int b[46], struct move2 move)
/*----------> purpose:
  ----------> version: 1.1
  ----------> date: 25th october 97 */
{
  int square, before;
  int i;

  for (i = 0; i < move.n; i++)
    {
      square = (move.m[i] % 256);
      before = ((move.m[i] >> 8) % 256);
      b[square] = before;
    }
}

int
evaluation (int b[46], int color)
/*----------> purpose:
  ----------> version: 1.1
  ----------> date: 18th april 98 */
{
  int i, j;
  int eval;
  int v1, v2;
  int nbm, nbk, nwm, nwk;
  int nbmc = 0, nbkc = 0, nwmc = 0, nwkc = 0;
  int nbme = 0, nbke = 0, nwme = 0, nwke = 0;
  int code = 0;
  int value[17] =
  {0, 0, 0, 0, 0, 1, 256, 0, 0, 16, 4096, 0, 0, 0, 0, 0, 0};
  int edge[14] =
  {5, 6, 7, 8, 13, 14, 22, 23, 31, 32, 37, 38, 39, 40};
  int center[8] =
  {15, 16, 20, 21, 24, 25, 29, 30};

  int row[41] =
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 3, 3, 3, 3, 4, 4, 4, 4, 0, 5, 5, 5, 5, 6, 6, 6, 6, 0, 7, 7, 7, 7};

  int safeedge[4] =
  {8, 13, 32, 37};

  int tempo = 0;
  int nm, nk;

  int turn = 2;			// color to move gets +turn

  int brv = 3;			// multiplier for back rank

  int kcv = 5;			// multiplier for kings in center

  int mcv = 1;			// multiplier for men in center

  int mev = 1;			// multiplier for men on edge

  int kev = 5;			// multiplier for kings on edge

  int cramp = 5;		// multiplier for cramp

  int opening = -2;		// multipliers for tempo

  int midgame = -1;
  int endgame = 2;
  int intactdoublecorner = 3;


  int backrank;

  int stonesinsystem = 0;

#ifdef STATISTICS
  evaluations++;
#endif


  for (i = 5; i <= 40; i++)
    code += value[b[i]];

  nwm = code % 16;
  nwk = (code >> 4) % 16;
  nbm = (code >> 8) % 16;
  nbk = (code >> 12) % 16;


  v1 = 100 * nbm + 130 * nbk;
  v2 = 100 * nwm + 130 * nwk;

  eval = v1 - v2;		/* material values */
  eval += (250 * (v1 - v2)) / (v1 + v2);	/* favor exchanges if in
						   material plus */

  nm = nbm + nwm;
  nk = nbk + nwk;
/*--------- fine evaluation below -------------*/

  if (color == BLACK)
    eval += turn;
  else
    eval -= turn;
  /* (white) 37  38  39  40 32  33  34  35 28  29  30  31 23  24  25  26
     19  20  21  22 14  15  16  17 10  11  12  13 5   6   7   8 (black)
   */
  /* cramp */
  if (b[23] == (BLACK | MAN) && b[28] == (WHITE | MAN))
    eval += cramp;
  if (b[22] == (WHITE | MAN) && b[17] == (BLACK | MAN))
    eval -= cramp;

  /* back rank guard */

  code = 0;
  if (b[5] & MAN)
    code++;
  if (b[6] & MAN)
    code += 2;
  if (b[7] & MAN)
    code += 4;
  if (b[8] & MAN)
    code += 8;
  switch (code)
    {
    case 0:
      code = 0;
      break;
    case 1:
      code = -1;
      break;
    case 2:
      code = 1;
      break;
    case 3:
      code = 0;
      break;
    case 4:
      code = 1;
      break;
    case 5:
      code = 1;
      break;
    case 6:
      code = 2;
      break;
    case 7:
      code = 1;
      break;
    case 8:
      code = 1;
      break;
    case 9:
      code = 0;
      break;
    case 10:
      code = 7;
      break;
    case 11:
      code = 4;
      break;
    case 12:
      code = 2;
      break;
    case 13:
      code = 2;
      break;
    case 14:
      code = 9;
      break;
    case 15:
      code = 8;
      break;
    }
  backrank = code;


  code = 0;
  if (b[37] & MAN)
    code += 8;
  if (b[38] & MAN)
    code += 4;
  if (b[39] & MAN)
    code += 2;
  if (b[40] & MAN)
    code++;
  switch (code)
    {
    case 0:
      code = 0;
      break;
    case 1:
      code = -1;
      break;
    case 2:
      code = 1;
      break;
    case 3:
      code = 0;
      break;
    case 4:
      code = 1;
      break;
    case 5:
      code = 1;
      break;
    case 6:
      code = 2;
      break;
    case 7:
      code = 1;
      break;
    case 8:
      code = 1;
      break;
    case 9:
      code = 0;
      break;
    case 10:
      code = 7;
      break;
    case 11:
      code = 4;
      break;
    case 12:
      code = 2;
      break;
    case 13:
      code = 2;
      break;
    case 14:
      code = 9;
      break;
    case 15:
      code = 8;
      break;
    }
  backrank -= code;
  eval += brv * backrank;


  /* intact double corner */
  if (b[8] == (BLACK | MAN))
    {
      if (b[12] == (BLACK | MAN) || b[13] == (BLACK | MAN))
	eval += intactdoublecorner;
    }

  if (b[37] == (WHITE | MAN))
    {
      if (b[32] == (WHITE | MAN) || b[33] == (WHITE | MAN))
	eval -= intactdoublecorner;
    }
  /* (white) 37  38  39  40 32  33  34  35 28  29  30  31 23  24  25  26
     19  20  21  22 14  15  16  17 10  11  12  13 5   6   7   8 (black)
   */

  /* center control */
  for (i = 0; i < 8; i++)
    {
      if (b[center[i]] != FREE)
	{
	  if (b[center[i]] == (BLACK | MAN))
	    nbmc++;
	  if (b[center[i]] == (BLACK | KING))
	    nbkc++;
	  if (b[center[i]] == (WHITE | MAN))
	    nwmc++;
	  if (b[center[i]] == (WHITE | KING))
	    nwkc++;
	}
    }
  eval += (nbmc - nwmc) * mcv;
  eval += (nbkc - nwkc) * kcv;

  /* edge */
  for (i = 0; i < 14; i++)
    {
      if (b[edge[i]] != FREE)
	{
	  if (b[edge[i]] == (BLACK | MAN))
	    nbme++;
	  if (b[edge[i]] == (BLACK | KING))
	    nbke++;
	  if (b[edge[i]] == (WHITE | MAN))
	    nwme++;
	  if (b[edge[i]] == (WHITE | KING))
	    nwke++;
	}
    }
  eval -= (nbme - nwme) * mev;
  eval -= (nbke - nwke) * kev;



  /* tempo */
  for (i = 5; i < 41; i++)
    {
      if (b[i] == (BLACK | MAN))
	tempo += row[i];
      if (b[i] == (WHITE | MAN))
	tempo -= 7 - row[i];
    }

  if (nm >= 16)
    eval += opening * tempo;
  if ((nm <= 15) && (nm >= 12))
    eval += midgame * tempo;
  if (nm < 9)
    eval += endgame * tempo;



  for (i = 0; i < 4; i++)
    {
      if (nbk + nbm > nwk + nwm && nwk < 3)
	{
	  if (b[safeedge[i]] == (WHITE | KING))
	    eval -= 15;
	}
      if (nwk + nwm > nbk + nbm && nbk < 3)
	{
	  if (b[safeedge[i]] == (BLACK | KING))
	    eval += 15;
	}
    }





  /* the move */
  if (nwm + nwk - nbk - nbm == 0)
    {
      if (color == BLACK)
	{
	  for (i = 5; i <= 8; i++)
	    {
	      for (j = 0; j < 4; j++)
		{
		  if (b[i + 9 * j] != FREE)
		    stonesinsystem++;
		}
	    }
	  if (stonesinsystem % 2)
	    {
	      if (nm + nk <= 12)
		eval++;
	      if (nm + nk <= 10)
		eval++;
	      if (nm + nk <= 8)
		eval += 2;
	      if (nm + nk <= 6)
		eval += 2;
	    }
	  else
	    {
	      if (nm + nk <= 12)
		eval--;
	      if (nm + nk <= 10)
		eval--;
	      if (nm + nk <= 8)
		eval -= 2;
	      if (nm + nk <= 6)
		eval -= 2;
	    }
	}
      else
	{
	  for (i = 10; i <= 13; i++)
	    {
	      for (j = 0; j < 4; j++)
		{
		  if (b[i + 9 * j] != FREE)
		    stonesinsystem++;
		}
	    }
	  if ((stonesinsystem % 2) == 0)
	    {
	      if (nm + nk <= 12)
		eval++;
	      if (nm + nk <= 10)
		eval++;
	      if (nm + nk <= 8)
		eval += 2;
	      if (nm + nk <= 6)
		eval += 2;
	    }
	  else
	    {
	      if (nm + nk <= 12)
		eval--;
	      if (nm + nk <= 10)
		eval--;
	      if (nm + nk <= 8)
		eval -= 2;
	      if (nm + nk <= 6)
		eval -= 2;
	    }
	}
    }


  return (eval);
}



/*-------------- PART III: MOVE GENERATION ----------------------------------*/

int
generatemovelist (int b[46], struct move2 movelist[MAXMOVES], int color)
/*----------> purpose:generates all moves. no captures. returns number of moves
  ----------> version: 1.0
  ----------> date: 25th october 97 */
{
  int n = 0, m;
  int i;

#ifdef STATISTICS
  generatemovelists++;
#endif

  if (color == BLACK)
    {
      for (i = 5; i <= 40; i++)
	{
	  if ((b[i] & BLACK) != 0)
	    {
	      if ((b[i] & MAN) != 0)
		{
		  if ((b[i + 4] & FREE) != 0)
		    {
		      movelist[n].n = 2;
		      if (i >= 32)
			m = (BLACK | KING);
		      else
			m = (BLACK | MAN);
		      m = m << 8;
		      m += FREE;
		      m = m << 8;
		      m += i + 4;
		      movelist[n].m[1] = m;
		      m = FREE;
		      m = m << 8;
		      m += (BLACK | MAN);
		      m = m << 8;
		      m += i;
		      movelist[n].m[0] = m;
		      n++;
		    }
		  if ((b[i + 5] & FREE) != 0)
		    {
		      movelist[n].n = 2;
		      if (i >= 32)
			m = (BLACK | KING);
		      else
			m = (BLACK | MAN);
		      m = m << 8;
		      m += FREE;
		      m = m << 8;
		      m += i + 5;
		      movelist[n].m[1] = m;
		      m = FREE;
		      m = m << 8;
		      m += (BLACK | MAN);
		      m = m << 8;
		      m += i;
		      movelist[n].m[0] = m;
		      n++;
		    }
		}
	      if ((b[i] & KING) != 0)
		{
		  if ((b[i + 4] & FREE) != 0)
		    {
		      movelist[n].n = 2;
		      m = (BLACK | KING);
		      m = m << 8;
		      m += FREE;
		      m = m << 8;
		      m += i + 4;
		      movelist[n].m[1] = m;
		      m = FREE;
		      m = m << 8;
		      m += (BLACK | KING);
		      m = m << 8;
		      m += i;
		      movelist[n].m[0] = m;
		      n++;
		    }
		  if ((b[i + 5] & FREE) != 0)
		    {
		      movelist[n].n = 2;
		      m = (BLACK | KING);
		      m = m << 8;
		      m += FREE;
		      m = m << 8;
		      m += i + 5;
		      movelist[n].m[1] = m;
		      m = FREE;
		      m = m << 8;
		      m += (BLACK | KING);
		      m = m << 8;
		      m += i;
		      movelist[n].m[0] = m;
		      n++;
		    }
		  if ((b[i - 4] & FREE) != 0)
		    {
		      movelist[n].n = 2;
		      m = (BLACK | KING);
		      m = m << 8;
		      m += FREE;
		      m = m << 8;
		      m += i - 4;
		      movelist[n].m[1] = m;
		      m = FREE;
		      m = m << 8;
		      m += (BLACK | KING);
		      m = m << 8;
		      m += i;
		      movelist[n].m[0] = m;
		      n++;
		    }
		  if ((b[i - 5] & FREE) != 0)
		    {
		      movelist[n].n = 2;
		      m = (BLACK | KING);
		      m = m << 8;
		      m += FREE;
		      m = m << 8;
		      m += i - 5;
		      movelist[n].m[1] = m;
		      m = FREE;
		      m = m << 8;
		      m += (BLACK | KING);
		      m = m << 8;
		      m += i;
		      movelist[n].m[0] = m;
		      n++;
		    }
		}
	    }
	}
    }
  else
    /* color = WHITE */
    {
      for (i = 5; i <= 40; i++)
	{
	  if ((b[i] & WHITE) != 0)
	    {
	      if ((b[i] & MAN) != 0)
		{
		  if ((b[i - 4] & FREE) != 0)
		    {
		      movelist[n].n = 2;
		      if (i <= 13)
			m = (WHITE | KING);
		      else
			m = (WHITE | MAN);
		      m = m << 8;
		      m += FREE;
		      m = m << 8;
		      m += i - 4;
		      movelist[n].m[1] = m;
		      m = FREE;
		      m = m << 8;
		      m += (WHITE | MAN);
		      m = m << 8;
		      m += i;
		      movelist[n].m[0] = m;
		      n++;
		    }
		  if ((b[i - 5] & FREE) != 0)
		    {
		      movelist[n].n = 2;
		      if (i <= 13)
			m = (WHITE | KING);
		      else
			m = (WHITE | MAN);
		      m = m << 8;
		      m += FREE;
		      m = m << 8;
		      m += i - 5;
		      movelist[n].m[1] = m;
		      m = FREE;
		      m = m << 8;
		      m += (WHITE | MAN);
		      m = m << 8;
		      m += i;
		      movelist[n].m[0] = m;
		      n++;
		    }
		}
	      if ((b[i] & KING) != 0)	/* or else */
		{
		  if ((b[i + 4] & FREE) != 0)
		    {
		      movelist[n].n = 2;
		      m = (WHITE | KING);
		      m = m << 8;
		      m += FREE;
		      m = m << 8;
		      m += i + 4;
		      movelist[n].m[1] = m;
		      m = FREE;
		      m = m << 8;
		      m += (WHITE | KING);
		      m = m << 8;
		      m += i;
		      movelist[n].m[0] = m;
		      n++;
		    }
		  if ((b[i + 5] & FREE) != 0)
		    {
		      movelist[n].n = 2;
		      m = (WHITE | KING);
		      m = m << 8;
		      m += FREE;
		      m = m << 8;
		      m += i + 5;
		      movelist[n].m[1] = m;
		      m = FREE;
		      m = m << 8;
		      m += (WHITE | KING);
		      m = m << 8;
		      m += i;
		      movelist[n].m[0] = m;
		      n++;
		    }
		  if ((b[i - 4] & FREE) != 0)
		    {
		      movelist[n].n = 2;
		      m = (WHITE | KING);
		      m = m << 8;
		      m += FREE;
		      m = m << 8;
		      m += i - 4;
		      movelist[n].m[1] = m;
		      m = FREE;
		      m = m << 8;
		      m += (WHITE | KING);
		      m = m << 8;
		      m += i;
		      movelist[n].m[0] = m;
		      n++;
		    }
		  if ((b[i - 5] & FREE) != 0)
		    {
		      movelist[n].n = 2;
		      m = (WHITE | KING);
		      m = m << 8;
		      m += FREE;
		      m = m << 8;
		      m += i - 5;
		      movelist[n].m[1] = m;
		      m = FREE;
		      m = m << 8;
		      m += (WHITE | KING);
		      m = m << 8;
		      m += i;
		      movelist[n].m[0] = m;
		      n++;
		    }
		}
	    }
	}
    }
  return (n);
}

int
generatecapturelist (int b[46], struct move2 movelist[MAXMOVES], int color)
/*----------> purpose: generate all possible captures
  ----------> version: 1.0
  ----------> date: 25th october 97 */
{
  int n = 0;
  int m;
  int i;
  int tmp;

#ifdef STATISTICS
  generatecapturelists++;
#endif

  if (color == BLACK)
    {
      for (i = 5; i <= 40; i++)
	{
	  if ((b[i] & BLACK) != 0)
	    {
	      if ((b[i] & MAN) != 0)
		{
		  if ((b[i + 4] & WHITE) != 0)
		    {
		      if ((b[i + 8] & FREE) != 0)
			{
			  movelist[n].n = 3;
			  if (i >= 28)
			    m = (BLACK | KING);
			  else
			    m = (BLACK | MAN);
			  m = m << 8;
			  m += FREE;
			  m = m << 8;
			  m += i + 8;
			  movelist[n].m[1] = m;
			  m = FREE;
			  m = m << 8;
			  m += (BLACK | MAN);
			  m = m << 8;
			  m += i;
			  movelist[n].m[0] = m;
			  m = FREE;
			  m = m << 8;
			  m += b[i + 4];
			  m = m << 8;
			  m += i + 4;
			  movelist[n].m[2] = m;
			  blackmancapture (b, &n, movelist, i + 8);
			}
		    }
		  if ((b[i + 5] & WHITE) != 0)
		    {
		      if ((b[i + 10] & FREE) != 0)
			{
			  movelist[n].n = 3;
			  if (i >= 28)
			    m = (BLACK | KING);
			  else
			    m = (BLACK | MAN);
			  m = m << 8;
			  m += FREE;
			  m = m << 8;
			  m += i + 10;
			  movelist[n].m[1] = m;
			  m = FREE;
			  m = m << 8;
			  m += (BLACK | MAN);
			  m = m << 8;
			  m += i;
			  movelist[n].m[0] = m;
			  m = FREE;
			  m = m << 8;
			  m += b[i + 5];
			  m = m << 8;
			  m += i + 5;
			  movelist[n].m[2] = m;
			  blackmancapture (b, &n, movelist, i + 10);
			}
		    }
		}
	      else
		/* b[i] is a KING */
		{
		  if ((b[i + 4] & WHITE) != 0)
		    {
		      if ((b[i + 8] & FREE) != 0)
			{
			  movelist[n].n = 3;
			  m = (BLACK | KING);
			  m = m << 8;
			  m += FREE;
			  m = m << 8;
			  m += i + 8;
			  movelist[n].m[1] = m;
			  m = FREE;
			  m = m << 8;
			  m += (BLACK | KING);
			  m = m << 8;
			  m += i;
			  movelist[n].m[0] = m;
			  m = FREE;
			  m = m << 8;
			  m += b[i + 4];
			  m = m << 8;
			  m += i + 4;
			  movelist[n].m[2] = m;
			  tmp = b[i + 4];
			  b[i + 4] = FREE;
			  blackkingcapture (b, &n, movelist, i + 8);
			  b[i + 4] = tmp;
			}
		    }
		  if ((b[i + 5] & WHITE) != 0)
		    {
		      if ((b[i + 10] & FREE) != 0)
			{
			  movelist[n].n = 3;
			  m = (BLACK | KING);
			  m = m << 8;
			  m += FREE;
			  m = m << 8;
			  m += i + 10;
			  movelist[n].m[1] = m;
			  m = FREE;
			  m = m << 8;
			  m += (BLACK | KING);
			  m = m << 8;
			  m += i;
			  movelist[n].m[0] = m;
			  m = FREE;
			  m = m << 8;
			  m += b[i + 5];
			  m = m << 8;
			  m += i + 5;
			  movelist[n].m[2] = m;
			  tmp = b[i + 5];
			  b[i + 5] = FREE;
			  blackkingcapture (b, &n, movelist, i + 10);
			  b[i + 5] = tmp;
			}
		    }
		  if ((b[i - 4] & WHITE) != 0)
		    {
		      if ((b[i - 8] & FREE) != 0)
			{
			  movelist[n].n = 3;
			  m = (BLACK | KING);
			  m = m << 8;
			  m += FREE;
			  m = m << 8;
			  m += i - 8;
			  movelist[n].m[1] = m;
			  m = FREE;
			  m = m << 8;
			  m += (BLACK | KING);
			  m = m << 8;
			  m += i;
			  movelist[n].m[0] = m;
			  m = FREE;
			  m = m << 8;
			  m += b[i - 4];
			  m = m << 8;
			  m += i - 4;
			  movelist[n].m[2] = m;
			  tmp = b[i - 4];
			  b[i - 4] = FREE;
			  blackkingcapture (b, &n, movelist, i - 8);
			  b[i - 4] = tmp;
			}
		    }
		  if ((b[i - 5] & WHITE) != 0)
		    {
		      if ((b[i - 10] & FREE) != 0)
			{
			  movelist[n].n = 3;
			  m = (BLACK | KING);
			  m = m << 8;
			  m += FREE;
			  m = m << 8;
			  m += i - 10;
			  movelist[n].m[1] = m;
			  m = FREE;
			  m = m << 8;
			  m += (BLACK | KING);
			  m = m << 8;
			  m += i;
			  movelist[n].m[0] = m;
			  m = FREE;
			  m = m << 8;
			  m += b[i - 5];
			  m = m << 8;
			  m += i - 5;
			  movelist[n].m[2] = m;
			  tmp = b[i - 5];
			  b[i - 5] = FREE;
			  blackkingcapture (b, &n, movelist, i - 10);
			  b[i - 5] = tmp;
			}
		    }
		}
	    }
	}
    }
  else
    /* color is WHITE */
    {
      for (i = 5; i <= 40; i++)
	{
	  if ((b[i] & WHITE) != 0)
	    {
	      if ((b[i] & MAN) != 0)
		{
		  if ((b[i - 4] & BLACK) != 0)
		    {
		      if ((b[i - 8] & FREE) != 0)
			{
			  movelist[n].n = 3;
			  if (i <= 17)
			    m = (WHITE | KING);
			  else
			    m = (WHITE | MAN);
			  m = m << 8;
			  m += FREE;
			  m = m << 8;
			  m += i - 8;
			  movelist[n].m[1] = m;
			  m = FREE;
			  m = m << 8;
			  m += (WHITE | MAN);
			  m = m << 8;
			  m += i;
			  movelist[n].m[0] = m;
			  m = FREE;
			  m = m << 8;
			  m += b[i - 4];
			  m = m << 8;
			  m += i - 4;
			  movelist[n].m[2] = m;
			  whitemancapture (b, &n, movelist, i - 8);
			}
		    }
		  if ((b[i - 5] & BLACK) != 0)
		    {
		      if ((b[i - 10] & FREE) != 0)
			{
			  movelist[n].n = 3;
			  if (i <= 17)
			    m = (WHITE | KING);
			  else
			    m = (WHITE | MAN);
			  m = m << 8;
			  m += FREE;
			  m = m << 8;
			  m += i - 10;
			  movelist[n].m[1] = m;
			  m = FREE;
			  m = m << 8;
			  m += (WHITE | MAN);
			  m = m << 8;
			  m += i;
			  movelist[n].m[0] = m;
			  m = FREE;
			  m = m << 8;
			  m += b[i - 5];
			  m = m << 8;
			  m += i - 5;
			  movelist[n].m[2] = m;
			  whitemancapture (b, &n, movelist, i - 10);
			}
		    }
		}
	      else
		/* b[i] is a KING */
		{
		  if ((b[i + 4] & BLACK) != 0)
		    {
		      if ((b[i + 8] & FREE) != 0)
			{
			  movelist[n].n = 3;
			  m = (WHITE | KING);
			  m = m << 8;
			  m += FREE;
			  m = m << 8;
			  m += i + 8;
			  movelist[n].m[1] = m;
			  m = FREE;
			  m = m << 8;
			  m += (WHITE | KING);
			  m = m << 8;
			  m += i;
			  movelist[n].m[0] = m;
			  m = FREE;
			  m = m << 8;
			  m += b[i + 4];
			  m = m << 8;
			  m += i + 4;
			  movelist[n].m[2] = m;
			  tmp = b[i + 4];
			  b[i + 4] = FREE;
			  whitekingcapture (b, &n, movelist, i + 8);
			  b[i + 4] = tmp;
			}
		    }
		  if ((b[i + 5] & BLACK) != 0)
		    {
		      if ((b[i + 10] & FREE) != 0)
			{
			  movelist[n].n = 3;
			  m = (WHITE | KING);
			  m = m << 8;
			  m += FREE;
			  m = m << 8;
			  m += i + 10;
			  movelist[n].m[1] = m;
			  m = FREE;
			  m = m << 8;
			  m += (WHITE | KING);
			  m = m << 8;
			  m += i;
			  movelist[n].m[0] = m;
			  m = FREE;
			  m = m << 8;
			  m += b[i + 5];
			  m = m << 8;
			  m += i + 5;
			  movelist[n].m[2] = m;
			  tmp = b[i + 5];
			  b[i + 5] = FREE;
			  whitekingcapture (b, &n, movelist, i + 10);
			  b[i + 5] = tmp;
			}
		    }
		  if ((b[i - 4] & BLACK) != 0)
		    {
		      if ((b[i - 8] & FREE) != 0)
			{
			  movelist[n].n = 3;
			  m = (WHITE | KING);
			  m = m << 8;
			  m += FREE;
			  m = m << 8;
			  m += i - 8;
			  movelist[n].m[1] = m;
			  m = FREE;
			  m = m << 8;
			  m += (WHITE | KING);
			  m = m << 8;
			  m += i;
			  movelist[n].m[0] = m;
			  m = FREE;
			  m = m << 8;
			  m += b[i - 4];
			  m = m << 8;
			  m += i - 4;
			  movelist[n].m[2] = m;
			  tmp = b[i - 4];
			  b[i - 4] = FREE;
			  whitekingcapture (b, &n, movelist, i - 8);
			  b[i - 4] = tmp;
			}
		    }
		  if ((b[i - 5] & BLACK) != 0)
		    {
		      if ((b[i - 10] & FREE) != 0)
			{
			  movelist[n].n = 3;
			  m = (WHITE | KING);
			  m = m << 8;
			  m += FREE;
			  m = m << 8;
			  m += i - 10;
			  movelist[n].m[1] = m;
			  m = FREE;
			  m = m << 8;
			  m += (WHITE | KING);
			  m = m << 8;
			  m += i;
			  movelist[n].m[0] = m;
			  m = FREE;
			  m = m << 8;
			  m += b[i - 5];
			  m = m << 8;
			  m += i - 5;
			  movelist[n].m[2] = m;
			  tmp = b[i - 5];
			  b[i - 5] = FREE;
			  whitekingcapture (b, &n, movelist, i - 10);
			  b[i - 5] = tmp;
			}
		    }
		}
	    }
	}
    }
  return (n);
}

void
blackmancapture (int b[46], int *n, struct move2 movelist[MAXMOVES], int i)
{
  int m;
  int found = 0;
  struct move2 move, orgmove;

  orgmove = movelist[*n];
  move = orgmove;

  if ((b[i + 4] & WHITE) != 0)
    {
      if ((b[i + 8] & FREE) != 0)
	{
	  move.n++;
	  if (i >= 28)
	    m = (BLACK | KING);
	  else
	    m = (BLACK | MAN);
	  m = m << 8;
	  m += FREE;
	  m = m << 8;
	  m += (i + 8);
	  move.m[1] = m;
	  m = FREE;
	  m = m << 8;
	  m += b[i + 4];
	  m = m << 8;
	  m += (i + 4);
	  move.m[move.n - 1] = m;
	  found = 1;
	  movelist[*n] = move;
	  blackmancapture (b, n, movelist, i + 8);
	}
    }
  move = orgmove;
  if ((b[i + 5] & WHITE) != 0)
    {
      if ((b[i + 10] & FREE) != 0)
	{
	  move.n++;
	  if (i >= 28)
	    m = (BLACK | KING);
	  else
	    m = (BLACK | MAN);
	  m = m << 8;
	  m += FREE;
	  m = m << 8;
	  m += (i + 10);
	  move.m[1] = m;
	  m = FREE;
	  m = m << 8;
	  m += b[i + 5];
	  m = m << 8;
	  m += (i + 5);
	  move.m[move.n - 1] = m;
	  found = 1;
	  movelist[*n] = move;
	  blackmancapture (b, n, movelist, i + 10);
	}
    }
  if (!found)
    (*n)++;
}

void
blackkingcapture (int b[46], int *n, struct move2 movelist[MAXMOVES], int i)
{
  int m;
  int tmp;
  int found = 0;
  struct move2 move, orgmove;

  orgmove = movelist[*n];
  move = orgmove;

  if ((b[i - 4] & WHITE) != 0)
    {
      if ((b[i - 8] & FREE) != 0)
	{
	  move.n++;
	  m = (BLACK | KING);
	  m = m << 8;
	  m += FREE;
	  m = m << 8;
	  m += i - 8;
	  move.m[1] = m;
	  m = FREE;
	  m = m << 8;
	  m += b[i - 4];
	  m = m << 8;
	  m += i - 4;
	  move.m[move.n - 1] = m;
	  found = 1;
	  movelist[*n] = move;
	  tmp = b[i - 4];
	  b[i - 4] = FREE;
	  blackkingcapture (b, n, movelist, i - 8);
	  b[i - 4] = tmp;
	}
    }
  move = orgmove;
  if ((b[i - 5] & WHITE) != 0)
    {
      if ((b[i - 10] & FREE) != 0)
	{
	  move.n++;
	  m = (BLACK | KING);
	  m = m << 8;
	  m += FREE;
	  m = m << 8;
	  m += i - 10;
	  move.m[1] = m;
	  m = FREE;
	  m = m << 8;
	  m += b[i - 5];
	  m = m << 8;
	  m += i - 5;
	  move.m[move.n - 1] = m;
	  found = 1;
	  movelist[*n] = move;
	  tmp = b[i - 5];
	  b[i - 5] = FREE;
	  blackkingcapture (b, n, movelist, i - 10);
	  b[i - 5] = tmp;
	}
    }
  move = orgmove;
  if ((b[i + 4] & WHITE) != 0)
    {
      if ((b[i + 8] & FREE) != 0)
	{
	  move.n++;
	  m = (BLACK | KING);
	  m = m << 8;
	  m += FREE;
	  m = m << 8;
	  m += i + 8;
	  move.m[1] = m;
	  m = FREE;
	  m = m << 8;
	  m += b[i + 4];
	  m = m << 8;
	  m += i + 4;
	  move.m[move.n - 1] = m;
	  found = 1;
	  movelist[*n] = move;
	  tmp = b[i + 4];
	  b[i + 4] = FREE;
	  blackkingcapture (b, n, movelist, i + 8);
	  b[i + 4] = tmp;
	}
    }
  move = orgmove;
  if ((b[i + 5] & WHITE) != 0)
    {
      if ((b[i + 10] & FREE) != 0)
	{
	  move.n++;
	  m = (BLACK | KING);
	  m = m << 8;
	  m += FREE;
	  m = m << 8;
	  m += i + 10;
	  move.m[1] = m;
	  m = FREE;
	  m = m << 8;
	  m += b[i + 5];
	  m = m << 8;
	  m += i + 5;
	  move.m[move.n - 1] = m;
	  found = 1;
	  movelist[*n] = move;
	  tmp = b[i + 5];
	  b[i + 5] = FREE;
	  blackkingcapture (b, n, movelist, i + 10);
	  b[i + 5] = tmp;
	}
    }
  if (!found)
    (*n)++;
}

void
whitemancapture (int b[46], int *n, struct move2 movelist[MAXMOVES], int i)
{
  int m;
  int found = 0;
  struct move2 move, orgmove;

  orgmove = movelist[*n];
  move = orgmove;

  if ((b[i - 4] & BLACK) != 0)
    {
      if ((b[i - 8] & FREE) != 0)
	{
	  move.n++;
	  if (i <= 17)
	    m = (WHITE | KING);
	  else
	    m = (WHITE | MAN);
	  m = m << 8;
	  m += FREE;
	  m = m << 8;
	  m += i - 8;
	  move.m[1] = m;
	  m = FREE;
	  m = m << 8;
	  m += b[i - 4];
	  m = m << 8;
	  m += i - 4;
	  move.m[move.n - 1] = m;
	  found = 1;
	  movelist[*n] = move;
	  whitemancapture (b, n, movelist, i - 8);
	}
    }
  move = orgmove;
  if ((b[i - 5] & BLACK) != 0)
    {
      if ((b[i - 10] & FREE) != 0)
	{
	  move.n++;
	  if (i <= 17)
	    m = (WHITE | KING);
	  else
	    m = (WHITE | MAN);
	  m = m << 8;
	  m += FREE;
	  m = m << 8;
	  m += i - 10;
	  move.m[1] = m;
	  m = FREE;
	  m = m << 8;
	  m += b[i - 5];
	  m = m << 8;
	  m += i - 5;
	  move.m[move.n - 1] = m;
	  found = 1;
	  movelist[*n] = move;
	  whitemancapture (b, n, movelist, i - 10);
	}
    }
  if (!found)
    (*n)++;
}

void
whitekingcapture (int b[46], int *n, struct move2 movelist[MAXMOVES], int i)
{
  int m;
  int tmp;
  int found = 0;
  struct move2 move, orgmove;

  orgmove = movelist[*n];
  move = orgmove;

  if ((b[i - 4] & BLACK) != 0)
    {
      if ((b[i - 8] & FREE) != 0)
	{
	  move.n++;
	  m = (WHITE | KING);
	  m = m << 8;
	  m += FREE;
	  m = m << 8;
	  m += i - 8;
	  move.m[1] = m;
	  m = FREE;
	  m = m << 8;
	  m += b[i - 4];
	  m = m << 8;
	  m += i - 4;
	  move.m[move.n - 1] = m;
	  found = 1;
	  movelist[*n] = move;
	  tmp = b[i - 4];
	  b[i - 4] = FREE;
	  whitekingcapture (b, n, movelist, i - 8);
	  b[i - 4] = tmp;
	}
    }
  move = orgmove;
  if ((b[i - 5] & BLACK) != 0)
    {
      if ((b[i - 10] & FREE) != 0)
	{
	  move.n++;
	  m = (WHITE | KING);
	  m = m << 8;
	  m += FREE;
	  m = m << 8;
	  m += i - 10;
	  move.m[1] = m;
	  m = FREE;
	  m = m << 8;
	  m += b[i - 5];
	  m = m << 8;
	  m += i - 5;
	  move.m[move.n - 1] = m;
	  found = 1;
	  movelist[*n] = move;
	  tmp = b[i - 5];
	  b[i - 5] = FREE;
	  whitekingcapture (b, n, movelist, i - 10);
	  b[i - 5] = tmp;
	}
    }
  move = orgmove;
  if ((b[i + 4] & BLACK) != 0)
    {
      if ((b[i + 8] & FREE) != 0)
	{
	  move.n++;
	  m = (WHITE | KING);
	  m = m << 8;
	  m += FREE;
	  m = m << 8;
	  m += i + 8;
	  move.m[1] = m;
	  m = FREE;
	  m = m << 8;
	  m += b[i + 4];
	  m = m << 8;
	  m += i + 4;
	  move.m[move.n - 1] = m;
	  found = 1;
	  movelist[*n] = move;
	  tmp = b[i + 4];
	  b[i + 4] = FREE;
	  whitekingcapture (b, n, movelist, i + 8);
	  b[i + 4] = tmp;
	}
    }
  move = orgmove;
  if ((b[i + 5] & BLACK) != 0)
    {
      if ((b[i + 10] & FREE) != 0)
	{
	  move.n++;
	  m = (WHITE | KING);
	  m = m << 8;
	  m += FREE;
	  m = m << 8;
	  m += i + 10;
	  move.m[1] = m;
	  m = FREE;
	  m = m << 8;
	  m += b[i + 5];
	  m = m << 8;
	  m += i + 5;
	  move.m[move.n - 1] = m;
	  found = 1;
	  movelist[*n] = move;
	  tmp = b[i + 5];
	  b[i + 5] = FREE;
	  whitekingcapture (b, n, movelist, i + 10);
	  b[i + 5] = tmp;
	}
    }
  if (!found)
    (*n)++;
}

int
testcapture (int b[46], int color)
/*----------> purpose: test if color has a capture on b
  ----------> version: 1.0
  ----------> date: 25th october 97 */
{
  int i;

#ifdef STATISTICS
  testcaptures++;
#endif

  if (color == BLACK)
    {
      for (i = 5; i <= 40; i++)
	{
	  if ((b[i] & BLACK) != 0)
	    {
	      if ((b[i] & MAN) != 0)
		{
		  if ((b[i + 4] & WHITE) != 0)
		    {
		      if ((b[i + 8] & FREE) != 0)
			return (1);
		    }
		  if ((b[i + 5] & WHITE) != 0)
		    {
		      if ((b[i + 10] & FREE) != 0)
			return (1);
		    }
		}
	      else
		/* b[i] is a KING */
		{
		  if ((b[i + 4] & WHITE) != 0)
		    {
		      if ((b[i + 8] & FREE) != 0)
			return (1);
		    }
		  if ((b[i + 5] & WHITE) != 0)
		    {
		      if ((b[i + 10] & FREE) != 0)
			return (1);
		    }
		  if ((b[i - 4] & WHITE) != 0)
		    {
		      if ((b[i - 8] & FREE) != 0)
			return (1);
		    }
		  if ((b[i - 5] & WHITE) != 0)
		    {
		      if ((b[i - 10] & FREE) != 0)
			return (1);
		    }
		}
	    }
	}
    }
  else
    /* color is WHITE */
    {
      for (i = 5; i <= 40; i++)
	{
	  if ((b[i] & WHITE) != 0)
	    {
	      if ((b[i] & MAN) != 0)
		{
		  if ((b[i - 4] & BLACK) != 0)
		    {
		      if ((b[i - 8] & FREE) != 0)
			return (1);
		    }
		  if ((b[i - 5] & BLACK) != 0)
		    {
		      if ((b[i - 10] & FREE) != 0)
			return (1);
		    }
		}
	      else
		/* b[i] is a KING */
		{
		  if ((b[i + 4] & BLACK) != 0)
		    {
		      if ((b[i + 8] & FREE) != 0)
			return (1);
		    }
		  if ((b[i + 5] & BLACK) != 0)
		    {
		      if ((b[i + 10] & FREE) != 0)
			return (1);
		    }
		  if ((b[i - 4] & BLACK) != 0)
		    {
		      if ((b[i - 8] & FREE) != 0)
			return (1);
		    }
		  if ((b[i - 5] & BLACK) != 0)
		    {
		      if ((b[i - 10] & FREE) != 0)
			return (1);
		    }
		}
	    }
	}
    }
  return (0);
}
