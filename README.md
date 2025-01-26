# simplech
This is a fork of simplech, a checkers engine, from the CheckerBoard software package maintained by Ed Gilbert at https://github.com/eygilbert/CheckerBoard It was written by Martin Fierz in C.

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
