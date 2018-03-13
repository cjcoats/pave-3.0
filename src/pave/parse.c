/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: parse.c 83 2018-03-12 19:24:33Z coats $
 *  Copyright (C) 1996-2004 MCNC
 *            (C) 2004-2010 UNC Institute for the Environment
 *            (C) 2018-     Carlie J. Coats, Jr., Ph.D.
 *
 *  Licensed under the GNU General Public License Version 2.
 *  See enclosed gpl.txt for more details
 *
 *  For further information on PAVE:
 *      Usage: type -usage in PAVE's standard input
 *      User Guide: https://cjcoats.github.io/pave/PaveManual.html
 *      FAQ:        https://cjcoats.github.io/pave/Pave.FAQ.html
 *
 ****************************************************************************
 *                           C R E D I T S
 *
 *   Development of this Software was supported in part through the
 *   MCNC-EPA cooperative agreement number CR822066-01.
 *
 *   Portions of this software were developed by the State University
 *   of New York at Albany, and funded by the United States Environmental
 *   Protection Agency under Contract 68D80016 with the Research Foundation
 *   of the State University of New York.
 *
 ****************************************************************************

FILE:       parse.c

AUTHOR:     Steve Thorpe
        MCNC Environmental Programs
        thorpe@ncsc.org

DATE:       12/9/94

PURPOSE:
    To translate a formula string from infix to
    postfix notation.

    This C program will be used to implement
    a processor for an arithmetic-expression translation
    grammer, as discussed in COMPILER DESIGN THEORY,
    by P.M.Lewis II, D.J.Rosenkrantz, and R.E.Stearns,
    published by Addison-Wesley Publishing Company, Inc.,
    1976.  This will process a slightly modified grammar
    as figure 9.13 on page 332 does; here the operators
    +, -, **, <, <=, >, >=, !=, ==, &&, ||, abs, sqrt, sqr, log,
    exp, ln, sin, cos, tan, sind, cosd, tand, minx, miny, minz,
    maxx, maxy, maxz, mean, min, max, and sum will also be handled.
    No computation will actually be done; but rather a postfix form of the
    infix formula will be output.  Also, the atomic I input
    terminal symbol will have three cases:

        a) it may be a floating point constant
           ('E', 'PI', 'NROWS', 'NCOLS', and 'NLEVELS'
            are also acceptable here)
        b) it may be an array of data
        c) it may be "sigma", which denotes sigma-half values

    Given terminal symbols I, (, ), abs, sqrt, sqr, log, exp,
    ln, sin, cos, tan, sind, cosd, tand, minx, miny, minz, maxx,
    maxy, maxz, mean, min, max, sum, mint, maxt, +, -, **, *, /,
    <, <=, >, >=, !=, ==, &&, ||, -| (end of input string), and e the null
    string, the following context free grammar will be implemented:


    ------------            --------------
    Productions:                Selection Set:
    ------------            --------------
    A     -> B ALIST            (, I, abs, log, sqr, sqrt, exp, ln,
                    sin, cos, tan, sind, cosd, tand,
                    minx, miny, minz, maxx, maxy, maxz,
                    mean, min, max, sum, mint, maxt
    ALIST -> || B { || } ALIST  ||
    ALIST -> e          -|, )
    B     -> C BLIST            (, I, abs, log, sqr, sqrt, exp, ln,
                    sin, cos, tan, sind, cosd, tand,
                    minx, miny, minz, maxx, maxy, maxz,
                    mean, min, max, sum, mint, maxt
    BLIST -> && C { && } BLIST  &&
    BLIST -> e          -|, ), ||

    C     -> D CLIST            (, I, abs, log, sqr, sqrt, exp, ln,
                    sin, cos, tan, sind, cosd, tand,
                    minx, miny, minz, maxx, maxy, maxz,
                    mean, min, max, sum, mint, maxt
    CLIST -> != D { != } CLIST  !=
    CLIST -> == D { == } CLIST  ==
    CLIST -> e          -|, ), &&, ||
    D     -> E DLIST            (, I, abs, log, sqr, sqrt, exp, ln,
                    sin, cos, tan, sind, cosd, tand,
                    minx, miny, minz, maxx, maxy, maxz,
                    mean, min, max, sum, mint, maxt
    DLIST -> <= { <= } DLIST    <=
    DLIST -> >= { >= } DLIST    >=
    DLIST -> >  { >  } DLIST    >
    DLIST -> <  { <  } DLIST    <
    DLIST -> e          -|, ), !=, ==, &&, ||
    E     -> T ELIST            (, I, abs, log, sqr, sqrt, exp, ln,
                    sin, cos, tan, sind, cosd, tand,
                    minx, miny, minz, maxx, maxy, maxz,
                    mean, min, max, sum, mint, maxt
    ELIST -> + T  { + } ELIST   +
    ELIST -> - T  { - } ELIST   -
    ELIST -> e          -|, ), <, <=, >, >=, !=, == , &&, ||
    T     -> Q TLIST        (, I, abs, log, sqr, sqrt, exp, ln,
                                    sin, cos, tan, sind, cosd, tand,
                                    minx, miny, minz, maxx, maxy, maxz,
                                    mean, min, max, sum, mint, maxt
    TLIST -> * Q  { * } TLIST   *
    TLIST -> / Q  { / } TLIST   /
    TLIST -> e          -|, ), +, -, <, <=, >, >=, !=, ==, &&, ||
    Q     -> P QLIST        (, I, abs, log, sqr, sqrt, exp, ln,
                                    sin, cos, tan, sind, cosd, tand,
                                    minx, miny, minz, maxx, maxy, maxz,
                                    mean, min, max, sum, mint, maxt
    QLIST -> ** P { ** } QLIST  **
    QLIST -> e          -|, ), +, -, *, /, <, <=, >, >=, !=, ==, &&, ||
    P     -> abs P  { abs }     abs
    P     -> log P  { log }     log
    P     -> sqr P  { sqr }     sqr
    P     -> sqrt P { sqrt }    sqrt
    P     -> exp P  { exp }     exp
    P     -> ln P   { ln  }     ln
    P     -> sin P  { sin  }    sin
    P     -> cos P  { cos  }    cos
    P     -> tan P  { tan  }    tan
    P     -> sind P { sind  }   sind
    P     -> cosd P { cosd  }   cosd
    P     -> tand P { tand  }   tand
    P     -> minx P { minx  }   tand
    P     -> miny P { miny  }   miny
    P     -> minz P { minz  }   minz
    P     -> maxx P { maxx  }   maxx
    P     -> maxy P { maxy  }   maxy
    P     -> maxz P { maxz  }   maxz
    P     -> mean P { mean  }   mean
    P     -> min P  { min   }   min
    P     -> max P  { max   }   max
    P     -> sum P  { sum   }   sum
    P     -> mint P { mint   }  mint
    P     -> maxt P { maxt   }  maxt
    P     -> ( A )          (
    P     -> I  { I }       I


    The LIFO push down stack processor to implement
    the above translation grammar is as follows:

    Stack                     INPUT SYMBOLS
    Symb. I  || && == != <  <= >  >= +  -  *  /  ** (  ) abs sqr sqrt log exp ln sin -|
         --------------------------------------------------------------------------------
    A    |43 x  x  x  x  x  x  x  x  x  x  x  x  x  43 43 43 43  43   43  43  43 43  x
    B    |44 x  x  x  x  x  x  x  x  x  x  x  x  x  44 44 44 44  44   44  44  44 44  x
    C    |35 x  x  x  x  x  x  x  x  x  x  x  x  x  35 35 35 35  35   35  35  35 35  x
    D    |36 x  x  x  x  x  x  x  x  x  x  x  x  x  36 36 36 36  36   36  36  36 36  x
    E    |1  x  x  x  x  x  x  x  x  x  x  x  x  x  1  x  1  1   1    1   1   1  1   x
    T    |2  x  x  x  x  x  x  x  x  x  x  x  x  x  2  x  2  2   2    2   2   2  2   x
    Q    |22 x  x  x  x  x  x  x  x  x  x  x  x  x  22 x  22 22  22   22  22  22 22  x
    P    |3  x  x  x  x  x  x  x  x  x  x  x  x  x  4  x  13 5   6    7   14  15 16  x
    ALIST|x  45 x  x  x  x  x  x  x  x  x  x  x  x  x  PR x  x   x    x   x   x  x   PR
    BLIST|x  PR 46 x  x  x  x  x  x  x  x  x  x  x  x  PR x  x   x    x   x   x  x   PR
    CLIST|x  PR PR 41 42 x  x  x  x  x  x  x  x  x  x  PR x  x   x    x   x   x  x   PR
    DLIST|x  PR PR PR PR 37 38 39 40 x  x  x  x  x  x  PR x  x   x    x   x   x  x   PR
    ELIST|x  PR PR PR PR PR PR PR PR 8  9  x  x  x  x  PR x  x   x    x   x   x  x   PR
    TLIST|x  PR PR PR PR PR PR PR PR PR PR 10 11 x  x  PR x  x   x    x   x   x  x   PR
    QLIST|x  PR PR PR PR PR PR PR PR PR PR PR PR 12 x  PR x  x   x    x   x   x  x   PR
    )    |x  x  x  x  x  x  x  x  x  x  x  x  x  x  PA x  x  x   x    x   x   x  x   x
    empty|x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x   x    x   x   x  x   ACC
         --------------------------------------------------------------------------------

    Stack                     INPUT SYMBOLS
    Symb. cos tan sind cosd tand minx miny minz maxx maxy maxz mean min max sum mint maxt
         --------------------------------------------------------------------------------
    A    |43  43  43   43   43   43   43   43   43   43   43   43   43  43  43  43   43
    B    |44  44  44   44   44   44   44   44   44   44   44   44   44  44  44  44   44
    C    |35  35  35   35   35   35   35   35   35   35   35   35   35  35  35  35   35
    D    |36  36  36   36   36   36   36   36   36   36   36   36   36  36  36  36   36
    E    |1   1   1    1    1    1    1    1    1    1    1    1    1   1   1   1    1
    T    |2   2   2    2    2    2    2    2    2    2    2    2    2   2   2   2    2
    Q    |22  22  22   22   22   22   22   22   22   22   22   22   22  22  22  22   22
    P    |17  18  19   20   21   23   24   25   26   27   28   29   30  31  32  33   34
    ALIST|x   x   x    x    x    x    x    x    x    x    x    x    x   x   x   x    x
    BLIST|x   x   x    x    x    x    x    x    x    x    x    x    x   x   x   x    x
    CLIST|x   x   x    x    x    x    x    x    x    x    x    x    x   x   x   x    x
    DLIST|x   x   x    x    x    x    x    x    x    x    x    x    x   x   x   x    x
    ELIST|x   x   x    x    x    x    x    x    x    x    x    x    x   x   x   x    x
    TLIST|x   x   x    x    x    x    x    x    x    x    x    x    x   x   x   x    x
    QLIST|x   x   x    x    x    x    x    x    x    x    x    x    x   x   x   x    x
    )    |x   x   x    x    x    x    x    x    x    x    x    x    x   x   x   x    x
    empty|x   x   x    x    x    x    x    x    x    x    x    x    x   x   x   x    x
         --------------------------------------------------------------------------------

    Stack                     INPUT SYMBOLS
    Symb.                   < any input symbol >
         ---------------------------------------------------------------------------
    ||   |                     <-  output "||", PR    ->
    &&   |                     <-  output "&&", PR    ->
    !=   |                     <-  output "!=", PR    ->
    ==   |                     <-  output "==", PR    ->
    >    |                     <-  output ">", PR     ->
    >=   |                     <-  output ">=", PR    ->
    <    |                     <-  output "<", PR     ->
    <=   |                     <-  output "<=", PR    ->
    +    |                     <-  output "+", PR     ->
    -    |                     <-  output "-", PR     ->
    *    |                     <-  output "*", PR     ->
    /    |                     <-  output "/", PR     ->
    **   |                     <-  output "**", PR    ->
    abs  |                     <-  output "abs", PR   ->
    sqr  |                     <-  output "sqr", PR   ->
    sqrt |                     <-  output "sqrt", PR  ->
    log  |                     <-  output "log", PR   ->
    exp  |                     <-  output "exp", PR   ->
    ln   |                     <-  output "ln", PR    ->
    sin  |                     <-  output "sin", PR   ->
    cos  |                     <-  output "cos", PR   ->
    tan  |                     <-  output "tan", PR   ->
    sind |                     <-  output "sind", PR  ->
    cosd |                     <-  output "cosd", PR  ->
    tand |                     <-  output "tand", PR  ->
    minx |                     <-  output "minx", PR  ->
    miny |                     <-  output "miny", PR  ->
    minz |                     <-  output "minz", PR  ->
    maxx |                     <-  output "maxx", PR  ->
    maxy |                     <-  output "maxy", PR  ->
    maxz |                     <-  output "maxz", PR  ->
    mean |                     <-  output "mean", PR  ->
    min  |                     <-  output "min", PR   ->
    max  |                     <-  output "max", PR   ->
    sum  |                     <-  output "sum", PR   ->
    mint |                     <-  output "mint", PR  ->
    maxt |                     <-  output "maxt", PR  ->
         ---------------------------------------------------------------------------
                           Starting Stack: A

     The actions denoted by the above symbols are:

       PR   pop, retain
       PA   pop, advance
       ACC  accept
       43   pop, push ALIST, push B, retain
       44   pop, push BLIST, push C, retain
       35   pop, push CLIST, push D, retain
       36   pop, push DLIST, push E, retain
       1    pop, push ELIST, push T, retain
       2    pop, push TLIST, push Q, retain
       22   pop, push QLIST, push P, retain
       3    pop, output I, advance
       4    pop, push ), push A, advance
       5    pop, push sqr, push P, advance
       6    pop, push sqrt, push P, advance
       7    pop, push log, push P, advance
       45   pop, push ALIST, push ||, push B, advance
       46   pop, push BLIST, push &&, push C, advance
       41   pop, push CLIST, push ==, push D, advance
       42   pop, push CLIST, push !=, push D, advance
       37   pop, push DLIST, push <, push E, advance
       38   pop, push DLIST, push <=, push E, advance
       39   pop, push DLIST, push >, push E, advance
       40   pop, push DLIST, push >=, push E, advance
       8    pop, push ELIST, push +, push T, advance
       9    pop, push ELIST, push -, push T, advance
       10   pop, push TLIST, push *, push Q, advance
       11   pop, push TLIST, push /, push Q, advance
       12   pop, push QLIST, push **, push P, advance
       13   pop, push abs, push P, advance
       14   pop, push exp, push P, advance
       15   pop, push ln, push P, advance
       16   pop, push sin, push P, advance
       17   pop, push cos, push P, advance
       18   pop, push tan, push P, advance
       19   pop, push sind, push P, advance
       20   pop, push cosd, push P, advance
       21   pop, push tand, push P, advance
       23   pop, push minx, push P, advance
       24   pop, push miny, push P, advance
       25   pop, push minz, push P, advance
       26   pop, push maxx, push P, advance
       27   pop, push maxy, push P, advance
       28   pop, push maxz, push P, advance
       29   pop, push mean, push P, advance
       30   pop, push min, push P, advance
       31   pop, push max, push P, advance
       32   pop, push sum, push P, advance
       33   pop, push mint, push P, advance
       34   pop, push maxt, push P, advance
       x    reject

     The operator precedence (highest to lowest) is as follows:

       1) abs, log, sqr, sqrt, exp, ln, sin, cos, tan, sind, cosd, tand,
          minx, miny, minz, max, maxy, maxz, mean, min, max, sum, mint, maxt
       2) **
       3) /, *
       4) +, -
       5) <, <=, >, >=
       6) ==, !=
       7) &&
       8) ||

    This code development originally began at Atmospheric Sciences
    Research Center's (ASRC) Acid Deposition Modeling Project (ADMP),
    based at the State University of New York at Albany (SUNYA),
    (otherwise affectionately known as the alphabet place). It was
    originally developed under the direction of Dr. Julius Chang as part
    of the Regional Acid Deposition Model Analysis Package (RAP), a
    Macintosh based analysis and visualization system which runs under a
    HyperCard front end.

     ****************************************************************************
    NOTES
    -----
    OPERATORS can't be used as species names

    will need checking with deps & mixing 2D w/ 3D in a formula

    eventually caldat should deal with minutes & seconds,
    not just year, date, and hour - also should deal with
    time increment (RAP assumed same for all files)

 ****************************************************************************
 *  REVISION HISTORY
    ---  ----       ----
    WHO  WHEN   WHAT
    ---  ----       ----

    SRT  03/01/90   Implemented this code (parseSRT.c) to replace
            parse.c.  This will now handle the unary operators
            sqr, sqrt, and log.

    SRT  04/16/90   Changed position of output symbol placement in
            productions 2,3,6,and 7.  The output symbols
            are now 2nd from the right rather than on the
            right.  The code has been changed accordingly,
            and now a formula such as a-b+c will be evaluated
            correctly as (a-b)+c rather than as a-(b+c).

    SRT  05/07/90   Error message for invalid species is more understandable.

    SRT  05/22/90   "WindVectors" special case added.

    SRT  05/29/90   Allowed to attach a specific hour to a species within
            a formula, by adding a colon and the hour on the end.

    SRT  07/01/90   Changed DRY:HNO2 to the correct DRY:HNO3
            Changed ??? units to empty units
            Added ** and abs operators.

    SRT  07/18/90   Allowed to compute the rate of change of a species (atomic
            only, *NOT* a combined species, also *NOT* a species with a
            specific hour attached to it as in 5/29/90 change)
            per hour within a formula, specified as in "d[O3a]/dt"

    DAH  08/03/90   1) All comments preceded by dh. Changes to handle two
               and three dimensional data in the same formula.
            2) change postfixque for
                old: SXi (species, specie index, case)
                new: SXi (species, absolute leve, case)
                old: DX.Yi (deposition, specie index, level, case
                new: DXi (species, absolute level, case)
                where absolute level means the starting
                level in the data set
            3) get kmax from HCard

    DAH  08/08/90   add variable dimension.. if every specie is 2d then will
            be 2 if any specie is 3d then is 3

    DAH  09/07/90   add oldDep... so if old type files are used error message
            old 2d and 3d mixed in a formula are not allowed.
            NOTE: need to modify when all new data format is used
            take out oldDep, depname, and depflag

    DAH  09/10/90   took out space after log, sqr, sqrt, abs because was
            not working when reloaded formulas

    SRT  09/18/90   updated HC variable I/O list above, added dim and KMAX

    SRT  09/19/90   Modified the U and V position check (used for wind
            vectors) to work with the new method of representing
            species internally Changed a couple of places below,
            marked them SRT 9/19/90, so both "SO2a+dry:so2a" *and*
            "dry:so2a+so2a" would be rejected.

    SRT  11/07/90   Changed the code setting newformula[11] and
            postfixqueue[11] so its in the right order.

    SRT  11/09/90   Changed the initialization of dim[] so it would have a
            terminating character.  Changed the special case of
            setting to 3d with an old file - search for "11/9/90"
            in code

    SRT  11/28/90   Added the option to handle "sigma" atom.

    SRT  12/09/94   Implemented this code (parse.c) to replace
                    RAP's parseSRT.c.

    SRT  01/12/95   Added exp, ln, sin, cos, tan, sind, cosd, and
            tand operators, as well as pi and e constants.
            Also added logic to "pretty up" the input formula
            for storage in choice browswers, dialog boxes, on
            graphs/plots, etc.

    SRT  01/13/95   Increase precedence of '**' operator to be greater
            than (rather than equal to) that of '*' and '/'

    SRT  02/22/95   Add capability to handle minx, miny, minz, maxx,
                    maxy, maxz, mean, min, max, sum, nrows, ncols, nlevels,
            mint, maxt

    SRT  02/24/95   parseFormula() no longer accepts a formula denoting
            the rate of change of a species (specified as in "d[O3a]/dt")
            since retrieveData() currently doesn't handle it.

    SRT  08/30/95   Allow mixing variables w/ different map_info strings

    SRT  10/04/96   Added <, <=, >, >=, !=, ==, &&, and || operators

    SRT  10/21/96   Added makeSureIts_netCDF() calls
    
    CJC  02/2018    Version for PAVE-3.0
 ****************************************************************************/

#include "bts.h"


/* structs for use by this file only */

struct stackItem
    {
    int      itype;              /* stack item type */
    struct   stackItem *sptr;    /* pointer to the item below on stack */
    };


/* global (ugh!) variables for this file only */

static
struct  stackItem *stack;

static
int     atomType,     /* type of current atom being processed */
        fpos,         /* where in the formula the input is now */
        dt,           /* whether we're doing d<spec>/dt, 0 or 1 */
        ncases,       /* how many cases are loaded in */
        oldDep,
        *mixCase,     /* if set to non-zero hour can't id hour for this formula */
        *dim,         /* number dimensions of output formula */
        kmax,
        jmax,
        imax,
        oldmm,
        olddd,
        oldyy,
        oldhh,
        *dateDay,
        *dateMonth,
        *dateYear,
        *hourStart,
        doLousyCheck;


static
char    atom[256],          /* the current atom being processed */
        *postFixQueue,      /* resulting postfix version of formula */
        *formula,           /* the formula typed in by the user */
        newformula[512],    /* place to store a new version of the formula */
        patom[256],         /* buffer to store atom for postFixQueue */
        *caseList,          /* list of cases separated by commas */
        *hostList,          /* list of hosts separated by commas */
        arrayType,          /* 'D' for deposition, 'S' for 3D */
        *whichUnit,         /* units for the formula */
        *depNames[12],      /* dh remove when all files have 2d/3d indicator */
        *caseUsed,          /* "010" if ncases = 3 and only case b used */
        *map_info;          /* will point to map_info (domain) char string */

static
VIS_DATA *caseInfo;

static
struct BusData *bd;


/* constants for this file only */

/* possible processor actions */
#define     ACT1        (1)
#define     ACT2        (2)
#define     ACT3        (3)
#define     ACT4        (4)
#define     ACT5        (5)
#define     ACT6        (6)
#define     ACT7        (7)
#define     ACT8        (8)
#define     ACT9        (9)
#define     ACT10       (10)
#define     ACT11       (11)
#define     ACT12       (12)
#define     ACT13       (13)
#define     ACT14       (14)
#define     ACT15       (15)
#define     ACT16       (16)
#define     ACT17       (17)
#define     ACT18       (18)
#define     ACT19       (19)
#define     ACT20       (20)
#define     ACT21       (21)
#define     ACT22       (22)
#define     ACT23       (23)
#define     ACT24       (24)
#define     ACT25       (25)
#define     ACT26       (26)
#define     ACT27       (27)
#define     ACT28       (28)
#define     ACT29       (29)
#define     ACT30       (30)
#define     ACT31       (31)
#define     ACT32       (32)
#define     ACT33       (33)
#define     ACT34       (34)
#define     ACT35       (35)
#define     ACT36       (36)
#define     ACT37       (37)
#define     ACT38       (38)
#define     ACT39       (39)
#define     ACT40       (40)
#define     ACT41       (41)
#define     ACT42       (42)
#define     ACT43       (43)
#define     ACT44       (44)
#define     ACT45       (45)
#define     ACT46       (46)
#define     ADDACT      (130)
#define     SUBACT      (131)
#define     MULACT      (132)
#define     DIVACT      (133)
#define     LOGACT      (134)
#define     SQRACT      (135)
#define     SQRTACT     (136)
#define     POPRET      (137)
#define     REJECT      (138)
#define     ACCEPT      (139)
#define     POPADV      (140)
#define     ABSACT      (141)
#define     POWACT      (142)
#define     EXPACT      (143)
#define     LNACT       (144)
#define     SINACT      (145)
#define     COSACT      (146)
#define     TANACT      (147)
#define     SINDACT     (148)
#define     COSDACT     (149)
#define     TANDACT     (150)
#define     MINXACT     (151)
#define     MINYACT     (152)
#define     MINZACT     (153)
#define     MAXXACT     (154)
#define     MAXYACT     (155)
#define     MAXZACT     (156)
#define     MEANACT     (157)
#define     MINACT      (158)
#define     MAXACT      (159)
#define     SUMACT      (160)
#define     MINTACT     (161)
#define     MAXTACT     (162)
#define     LTACT       (163)
#define     LTEACT      (164)
#define     GTACT       (165)
#define     GTEACT      (166)
#define     EQACT       (167)
#define     NEQACT      (168)
#define     ANDACT      (169)
#define     ORACT       (170)

/* stack item and input atom types */
#define     UNKNOWN     (0)
#define     E           (1)
#define     T           (2)
#define     P           (3)
#define     ELIST       (4)
#define     TLIST       (5)
#define     RPAREN      (6)
#define     EMPTY       (7)
#define     ADD         (8)
#define     SUB         (9)
#define     MUL         (10)
#define     DIV         (11)
#define     LOG         (12)
#define     SQRT        (13)
#define     SQR         (14)
#define     I           (15)
#define     LPAREN      (16)
#define     END         (17)
#define     ABS         (18)
#define     POW         (19)
#define     EXP         (20)
#define     LN          (21)
#define     SIN         (22)
#define     COS         (23)
#define     TAN         (24)
#define     SIND        (25)
#define     COSD        (26)
#define     TAND        (27)
#define     Q           (28)
#define     QLIST       (29)
#define     MINX        (30)
#define     MINY        (31)
#define     MINZ        (32)
#define     MAXX        (33)
#define     MAXY        (34)
#define     MAXZ        (35)
#define     MEAN        (36)
#define     MINIMUM     (37)
#define     MAXIMUM     (38)
#define     SUM         (39)
#define     MINT        (40)
#define     MAXT        (41)
#define     LT          (42)
#define     LTE         (43)
#define     GT          (44)
#define     GTE         (45)
#define     EQ          (46)
#define     NEQ         (47)
#define     C           (48)
#define     D           (49)
#define     CLIST       (50)
#define     DLIST       (51)
#define     A           (52)
#define     B           (53)
#define     ALIST       (54)
#define     BLIST       (55)
#define     AND         (56)
#define     OR          (57)


/* function prototypes for routines for this file only */

static char *actiontype ( int );
static char *itemtype ( int );
static void dumpStack ( void );
static void output ( char * );
static int  caldat ( char * );
static int  push ( int );
static int  pop ( void );
static int  checkAtom ( char *, char * );
static int  advance ( void );
static int  get_action ( void );
static int  process ( void );
static void freeCaseInfo ( void );




/***********************************************************
ACTIONTYPE - returns the given item type as a string
************************************************************/
static
char    *actiontype ( int t )
    {
    switch ( t )
        {
        case ACT1:
            return ( "pop, push ELIST, push T, retain" );
        case ACT2:
            return ( "pop, push TLIST, push Q, retain" );
        case ACT3:
            return ( "pop, output I, advance" );
        case ACT4:
            return ( "pop, push ), push E, advance" );
        case ACT5:
            return ( "pop, push sqr, push P, advance" );
        case ACT6:
            return ( "pop, push sqrt, push P, advance" );
        case ACT7:
            return ( "pop, push log, push P, advance" );
        case ACT8:
            return ( "pop, push ELIST, push +, push T, advance" );
        case ACT9:
            return ( "pop, push ELIST, push -, push T, advance" );
        case ACT10:
            return ( "pop, push TLIST, push *, push Q, advance" );
        case ACT11:
            return ( "pop, push TLIST, push /, push Q, advance" );
        case ACT12:
            return ( "pop, push QLIST, push **, push P, advance" );
        case ACT13:
            return ( "pop, push abs, push P, advance" );
        case ACT14:
            return ( "pop, push exp, push P, advance" );
        case ACT15:
            return ( "pop, push ln, push P, advance" );
        case ACT16:
            return ( "pop, push sin, push P, advance" );
        case ACT17:
            return ( "pop, push cos, push P, advance" );
        case ACT18:
            return ( "pop, push tan, push P, advance" );
        case ACT19:
            return ( "pop, push sind, push P, advance" );
        case ACT20:
            return ( "pop, push cosd, push P, advance" );
        case ACT21:
            return ( "pop, push tand, push P, advance" );
        case ACT22:
            return ( "pop, push QLIST, push P, retain" );
        case ACT23:
            return ( "pop, push minx, push P, advance" );
        case ACT24:
            return ( "pop, push miny, push P, advance" );
        case ACT25:
            return ( "pop, push minz, push P, advance" );
        case ACT26:
            return ( "pop, push maxx, push P, advance" );
        case ACT27:
            return ( "pop, push maxy, push P, advance" );
        case ACT28:
            return ( "pop, push maxz, push P, advance" );
        case ACT29:
            return ( "pop, push mean, push P, advance" );
        case ACT30:
            return ( "pop, push min, push P, advance" );
        case ACT31:
            return ( "pop, push max, push P, advance" );
        case ACT32:
            return ( "pop, push sum, push P, advance" );
        case ACT33:
            return ( "pop, push mint, push P, advance" );
        case ACT34:
            return ( "pop, push maxt, push P, advance" );
        case ACT35:
            return ( "pop, push CLIST, push D, retain" );
        case ACT36:
            return ( "pop, push DLIST, push E, retain" );
        case ACT37:
            return ( "pop, push DLIST, push <, push E, advance" );
        case ACT38:
            return ( "pop, push DLIST, push <=, push E, advance" );
        case ACT39:
            return ( "pop, push DLIST, push >, push E, advance" );
        case ACT40:
            return ( "pop, push DLIST, push >=, push E, advance" );
        case ACT41:
            return ( "pop, push CLIST, push ==, push D, advance" );
        case ACT42:
            return ( "pop, push CLIST, push !=, push D, advance" );
        case ACT43:
            return ( "pop, push ALIST, push B, retain" );
        case ACT44:
            return ( "pop, push BLIST, push C, retain" );
        case ACT45:
            return ( "pop, push ALIST, push ||, push B, advance" );
        case ACT46:
            return ( "pop, push BLIST, push &&, push C, advance" );
        case ADDACT:
            return ( "ADDACT" );
        case SUBACT:
            return ( "SUBACT" );
        case MULACT:
            return ( "MULACT" );
        case DIVACT:
            return ( "DIVACT" );
        case LOGACT:
            return ( "LOGACT" );
        case SQRACT:
            return ( "SQRACT" );
        case SQRTACT:
            return ( "SQRTACT" );
        case POPRET:
            return ( "POPRET" );
        case REJECT:
            return ( "REJECT" );
        case ACCEPT:
            return ( "ACCEPT" );
        case POPADV:
            return ( "POPADV" );
        case ABSACT:
            return ( "ABSACT" );
        case POWACT:
            return ( "POWACT" );
        case GTACT:
            return ( "GTACT" );
        case LTACT:
            return ( "LTACT" );
        case GTEACT:
            return ( "GTEACT" );
        case LTEACT:
            return ( "LTEACT" );
        case EQACT:
            return ( "EQACT" );
        case NEQACT:
            return ( "NEQACT" );
        case ANDACT:
            return ( "ANDACT" );
        case ORACT:
            return ( "ORACT" );
        case EXPACT:
            return ( "EXPACT" );
        case LNACT:
            return ( "LNACT" );
        case SINACT:
            return ( "SINACT" );
        case COSACT:
            return ( "COSACT" );
        case TANACT:
            return ( "TANACT" );
        case SINDACT:
            return ( "SINDACT" );
        case COSDACT:
            return ( "COSDACT" );
        case TANDACT:
            return ( "TANDACT" );
        case MINXACT:
            return ( "MINXACT" );
        case MINYACT:
            return ( "MINYACT" );
        case MINZACT:
            return ( "MINZACT" );
        case MAXXACT:
            return ( "MAXXACT" );
        case MAXYACT:
            return ( "MAXYACT" );
        case MAXZACT:
            return ( "MAXZACT" );
        case MEANACT:
            return ( "MEANACT" );
        case MINACT:
            return ( "MINACT" );
        case MAXACT:
            return ( "MAXACT" );
        case SUMACT:
            return ( "SUMACT" );
        case MINTACT:
            return ( "MINTACT" );
        case MAXTACT:
            return ( "MAXTACT" );
        default:
            return ( "???" );
        }
    }


/***********************************************************
ITEMTYPE - returns the given item type as a string
************************************************************/
static
char    *itemtype ( int t )
    {
    switch ( t )
        {
        case A:
            return ( "A" );
        case B:
            return ( "B" );
        case C:
            return ( "C" );
        case D:
            return ( "D" );
        case E:
            return ( "E" );
        case T:
            return ( "T" );
        case P:
            return ( "P" );
        case Q:
            return ( "Q" );
        case ALIST:
            return ( "ALIST" );
        case BLIST:
            return ( "BLIST" );
        case CLIST:
            return ( "CLIST" );
        case DLIST:
            return ( "DLIST" );
        case ELIST:
            return ( "ELIST" );
        case TLIST:
            return ( "TLIST" );
        case QLIST:
            return ( "QLIST" );
        case RPAREN:
            return ( ")" );
        case EMPTY:
            return ( "" );
        case AND:
            return ( "&&" );
        case OR:
            return ( "||" );
        case EQ:
            return ( "==" );
        case NEQ:
            return ( "!=" );
        case LT:
            return ( "<" );
        case LTE:
            return ( "<=" );
        case GT:
            return ( ">" );
        case GTE:
            return ( ">=" );
        case ADD:
            return ( "+" );
        case SUB:
            return ( "-" );
        case MUL:
            return ( "*" );
        case DIV:
            return ( "/" );
        case LOG:
            return ( "log" );
        case SQRT:
            return ( "sqrt" );
        case SQR:
            return ( "sqr" );
        case I:
            return ( "I" );
        case LPAREN:
            return ( "(" );
        case END:
            return ( "-|" );
        case ABS:
            return ( "abs:" );
        case POW:
            return ( "**" );
        case EXP:
            return ( "exp" );
        case LN:
            return ( "ln" );
        case SIN:
            return ( "sin" );
        case COS:
            return ( "cos" );
        case TAN:
            return ( "tan" );
        case SIND:
            return ( "sind" );
        case COSD:
            return ( "cosd" );
        case TAND:
            return ( "tand" );
        case MINX:
            return ( "minx" );
        case MINY:
            return ( "miny" );
        case MINZ:
            return ( "minz" );
        case MAXX:
            return ( "maxx" );
        case MAXY:
            return ( "maxy" );
        case MAXZ:
            return ( "maxz" );
        case MEAN:
            return ( "mean" );
        case MINIMUM:
            return ( "min" );
        case MAXIMUM:
            return ( "max" );
        case SUM:
            return ( "sum" );
        case MINT:
            return ( "mint" );
        case MAXT:
            return ( "maxt" );
        default:
            return ( "???" );
        }
    }



/***********************************************************
DUMPSTACK -
************************************************************/
static void dumpStack ( void )
    {
    char tstring[256], tstring2[256];
    struct stackItem *tstack = stack;
    tstring[0] = '\0';
    while ( tstack != NULL )
        {
        strcpy ( tstring2, itemtype ( tstack->itype ) );
        strcat ( tstring2, " " );
        strcat ( tstring2, tstring );
        strcpy ( tstring, tstring2 );
        tstack = tstack->sptr;
        }

#ifdef DIAGNOSTICS
    strcpy ( tstring2, "                                    Current stack: " );
    strcat ( tstring2, tstring );
    diagmsg ( tstring2 );
#endif /* DIAGNOSTICS */
    }



/***********************************************************
OUTPUT -
************************************************************/
static void output ( char *s )
    {
    char tstring[512];
    strcat ( postFixQueue, s );

#ifdef DIAGNOSTICS
    strcpy ( tstring, "Current postfix formula: " );
    strcat ( tstring, postFixQueue );
    diagmsg ( tstring );
#endif /* DIAGNOSTICS */
    }



/***********************************************************
CALDAT - for the given formula being evaluated and for
     the julian date of the given atom within the
     formula, this routine:

     1) updates *mixCase to 1 if no date can be
        assigned for this formula due to mixing dates

     2) updates *dateYear, *dateMonth, *dateDay,
        and *hourStart for the given formula
************************************************************/
static int caldat ( char *julian /* YYYYDDDHHMMSS */ )
    {
    char    temp[12], tstring[255];
    int     jdate, i, sum = 0;
    static  int days_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int mm, dd, yy, hh;

    temp[0] = julian[0];
    temp[1] = julian[1];
    temp[2] = julian[2];
    temp[3] = julian[3];
    temp[4] = '\0';
    yy = atoi ( temp );
    temp[0] = julian[4];
    temp[1] = julian[5];
    temp[2] = julian[6];
    temp[3] = '\0';
    jdate = atoi ( temp );
    if ( ! ( yy % 4 ) ) days_month[1] = 29;

    for ( i = 0; i < 12; i++ )
        {
        sum += days_month[i];
        if ( sum >= jdate ) break;
        }
    mm = i + 1;
    if ( mm > 12 )
        {
        mm -= 12;
        yy++;
        }

    sum -= days_month[i];
    dd = jdate - sum;
    temp[0] = julian[7];
    temp[1] = julian[8];
    temp[2] = '\0';
    hh = atoi ( temp ) % 24;

    if ( ( ! ( ( oldyy == -1 ) && ( oldmm == -1 ) && ( olddd == -1 ) && ( oldhh == -1 ) ) ) &&
            ( ( oldyy != yy ) || ( oldmm != mm ) || ( olddd != dd ) || ( oldhh != hh ) ) )
        {
        *mixCase = 1;
        *dateYear = -1;
        *dateMonth = -1;
        *dateDay = -1;
        *hourStart = -1;
        }
    else
        {
        oldyy = yy;
        oldmm = mm;
        olddd = dd;
        oldhh = hh;
        *dateYear = yy;
        *dateMonth = mm;
        *dateDay = dd;
        *hourStart = hh;
        }
    return 0;
    }



/************************************************************
PUSH - returns 1 if there was a failure
************************************************************/
static  int push ( int type )
    {
    struct  stackItem *tstack;
    char    tstring[80];
    if ( ( tstack = ( struct stackItem * )
                    malloc ( ( unsigned ) sizeof ( struct stackItem ) ) ) == NULL )
        return errmsg ( "Need more memory!" );

#ifdef DIAGNOSTICS
    sprintf ( tstring, "PUSH %s", itemtype ( type ) );
    diagmsg ( tstring );
#endif /* DIAGNOSTICS */

    tstack->sptr = stack;
    stack = tstack;
    stack->itype = type;
    dumpStack();
    return 0;
    }



/************************************************************
POP - returns 1 if there was nothing to pop
************************************************************/
static  int pop ( void )
    {
    struct  stackItem *tstack;
    char    tstring[80];

    if ( stack == NULL ) return 1;

#ifdef DIAGNOSTICS
    sprintf ( tstring, "POP %s", itemtype ( stack->itype ) );
    diagmsg ( tstring );
#endif /* DIAGNOSTICS */

    tstack = stack;
    stack = stack->sptr;
    free ( tstack );
    tstack = NULL;
    dumpStack();
    return 0;
    }



/***********************************************************
CHECKATOM - if OK produces an atom for the postFixQueue and
        returns 0 (also perhaps changes s to be in a more
        readable form for when the user is reading the
        formula) otherwise returns 1
************************************************************/
static  int
checkAtom ( char *s,   /* the species as entered by the user */
            char *sp ) /* the species as translated for postFixQueue */
    {
    int  i,  j, k, m, n, pos, len, seenPeriod, stillNum,
         nspecs, thisHour, colonPos, index, absLevels ;
    char c, tstring[512], numSpecsStr[256], 
         tspec[6], depFlag, cs[3], tts[20], ermsg[512], olds[256],
         julian[16];
    /*dh remove depFlag when all new files with 2d/3d indicator */

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter checkAtom('%s') with len=%d\n", s, len );
#endif

    n          = 0 ;
    pos        = 0 ;
    len        = strlen ( s ) ;
    seenPeriod = 0 ;
    stillNum   = 1 ;
    thisHour   = -1 ;
    colonPos   = -1 ;
    c          = toupper ( s[len-1] ) ;
    sp[0]      = '\0';

    strcpy ( olds, s );
    sprintf ( ermsg, "Can not understand part of forumula \"%s\"!", s );

    if ( dt )
        {
        int tp = 0;
        if ( ( strncmp ( "DRY:", s, 4 ) == 0 ) || ( strncmp ( "WET:", s, 4 ) == 0 ) ) tp=4;
        if ( strchr ( &s[tp], ( int ) ':' ) )
            return ( errmsg ( ermsg ) );
        }

    /* if its nrows then its an easy way out */
    if ( ( strncmp ( s, "NROWS", 5 ) == 0 ) && ( strlen ( s ) == 5 ) )
        {
        strcpy ( s,  "nrows" );
        strcpy ( sp, "nrows" );
        return 0;
        }

    /* if its ncols then its an easy way out */
    if ( ( strncmp ( s, "NCOLS", 5 ) == 0 ) && ( strlen ( s ) == 5 ) )
        {
        strcpy ( s,  "ncols" );
        strcpy ( sp, "ncols" );
        return 0;
        }

    /* if its nlevels then its an easy way out */
    if ( ( strncmp ( s, "NLEVELS", 7 ) == 0 ) && ( strlen ( s ) == 7 ) )
        {
        strcpy ( s,  "nlevels" );
        strcpy ( sp, "nlevels" );
        return 0;
        }

    /* if its sigma or a number then its an easy way out */
    if ( ( strncmp ( s, "SIGMA", 5 ) == 0 ) && ( strlen ( s ) == 5 ) )
        {
        strcpy ( s,  "sigma" );
        strcpy ( sp, "T" );
        return 0;
        }

    /* if its E then its an easy way out */
    if ( ( strncasecmp ( atom, "E", 1 ) == 0 ) && ( strlen ( atom ) == 1 ) )
        {
        strcpy ( s,  "e" );
        strcpy ( sp, "e" );
        return 0;
        }

    /* if its PI then its an easy way out */
    if ( ( strncasecmp ( atom, "PI", 2 ) == 0 ) && ( strlen ( atom ) == 2 ) )
        {
        strcpy ( s,  "pi" );
        strcpy ( sp, "pi" );
        return 0;
        }

    for ( i = 0; ( ( i<len ) && ( stillNum ) ); i++ )
        {
        if ( ! ( ( ( s[i] <= '9' ) && ( s[i] >= '0' ) ) || ( s[i] == '.' ) ) )
            stillNum = 0;
        else if ( s[i] == '.' )
            {
            if ( seenPeriod == 1 )
                return errmsg ( ermsg );
            else
                seenPeriod = 1;
            }
        }
    if ( stillNum )
        {
        double d = atof ( s );
        /* doing this changes large or small number to exponential notation */
        /*sprintf(s, "%g", d);*/
        sprintf ( s,"%s",olds );
        sprintf ( sp, "C%.10f", d );
        if ( dt ) return ( errmsg ( "No rate of change for a constant!" ) );
        return 0;
        }
    if ( ( c >= '0' ) && ( c <= '9' ) )
        /* check for hour specific case */
        for ( i = len-1; ( ( i >= 0 ) && ( colonPos == -1 ) ); i-- )
            if ( s[i] == ':' )
                {
                int allNums = 1;
                colonPos = i;
                for ( j = colonPos+1; j < len; j++ )
                    if ( ! ( ( s[j]>='0' ) && ( s[j]<='9' ) ) ) allNums = 0;
                if ( allNums )
                    {
                    if ( allNums ) thisHour = atoi ( &s[colonPos+1] );
                    s[colonPos] = '\0';
                    len = strlen ( s );
                    c = toupper ( s[len-1] );
                    }
                }
    if ( doLousyCheck ) return 0;
    if ( ( c < 'A' ) || ( c-'A'+1 > ncases ) )
        {
        strcpy ( s, olds );
        return errmsg ( ermsg );
        }


    /* get the information for that case if
       we don't already have it */

    if ( caseInfo[c-'A'].filename == NULL )
        {
        if ( ( getNthItem ( ( int ) ( c-'A'+1 ), caseList, tstring ) ) ||
                ( ! ( caseInfo[c-'A'].filename = ( char * ) malloc ( strlen ( tstring )+1 ) ) ) )
            {
            strcpy ( s, olds );
            return 1;
            }
        strcpy ( caseInfo[c-'A'].filename, tstring );

        if ( ( getNthItem ( ( int ) ( c-'A'+1 ), hostList, tstring ) ) ||
                ( ! ( caseInfo[c-'A'].filehost.name =
                          ( char * ) malloc ( strlen ( tstring )+1 ) ) ) )
            {
            strcpy ( s, olds );
            return 1;
            }
        strcpy ( caseInfo[c-'A'].filehost.name, tstring );

#ifdef DIAGNOSTICS
        /*  fprintf(stderr, "parse.c just before get_info, caseInfo[%d] == \n",
                (int)(c-'A'));
                dump_VIS_DATA(&caseInfo[c-'A'], NULL, NULL); fflush(stderr); fflush(stdout);(stdout);
        */
#endif /* DIAGNOSTICS */
        if ( !get_info ( bd, &caseInfo[c-'A'], errorString ) )
            {
            strcpy ( s, olds );
#ifdef DIAGNOSTICS
            fprintf ( stderr, "parse.c just after get_info, caseInfo[%d] == \n",
                      ( int ) ( c-'A' ) );
            dump_VIS_DATA ( &caseInfo[c-'A'], NULL, NULL );
            fflush ( stderr );
            fflush ( stdout );
#endif /* DIAGNOSTICS */
            return 1;
            }

        /* convert to netCDF (really, IO/API map_info information)
           data if it isn't already */
        if ( makeSureIts_netCDF ( &caseInfo[c-'A'], errorString ) )
            {
            strcpy ( s, olds );
            return 1;
            }

        caseUsed[c-'A'] = '1';

#ifdef DIAGNOSTICS
        fprintf ( stderr, "parse.c just after get_info, caseInfo[%d] == \n",
                  ( int ) ( c-'A' ) );
        if ( dump_VIS_DATA ( &caseInfo[c-'A'], NULL, NULL ) ) return 1;
        fflush ( stdout );
#endif /* DIAGNOSTICS */
        }

    if ( imax == -1 ) imax = caseInfo[c-'A'].ncol;
    if ( jmax == -1 ) jmax = caseInfo[c-'A'].nrow;
    if ( ( kmax == -1 ) ||
         ( kmax == 1 ) ) /* allowed because can mix KMAX == 1 w/ KMAX > 1 */
        kmax = caseInfo[c-'A'].nlevel;

    if ( ( imax < 1 ) || ( jmax < 1 ) || ( kmax < 1 ) )
        {
        strcpy ( s, olds );
        return errmsg ( "Non-positive imax, jmax, or kmax in parseFormula!" );
        }

    if ( ( caseInfo[c-'A'].ncol != imax ) ||
            ( caseInfo[c-'A'].nrow != jmax ) )
        {
        strcpy ( s, olds );
        return errmsg ( "Can't mix species with different IMAX x JMAX !" );
        }

    if ( caseInfo[c-'A'].nlevel != kmax ) /* 960404 allowed as per Alison Eyth */
        fprintf ( stderr, "\nWARNING: mixing variables with different KMAX!\n"
                  "Are you sure you want to do this?\n" );

    /* make sure the map info is OK */
    if ( map_info == NULL ) map_info = caseInfo[c-'A'].map_info;

    /* do a time and date check -
       NOTE: Kathy's VIS_DATA structs have
       *int* first_date stored as YYYYDDD, and
       *int* first_time stored as HHMMSS  */

    sprintf ( julian, "%07d%06d", caseInfo[c-'A'].first_date,
              caseInfo[c-'A'].first_time );
    caldat ( julian );


    nspecs = caseInfo[c-'A'].nspecies;

    /*dh remove when all new files with 2d/3d indicator */
    if ( strncmp ( "DRY:", s, 4 ) == 0 ) depFlag = 'D';
    else if ( strncmp ( "WET:", s, 4 ) == 0 ) depFlag = 'W';
    else
        /* SRT 951012
        if (len > 6)
            {
            strcpy(s, olds);
            return errmsg(ermsg);
            }
        else
           SRT 951012 */
        depFlag = 0;

    for ( i = 0; i < nspecs; i++ )
        {
        j = strlen ( caseInfo[c-'A'].species_short_name[i] )-1;

        /* SRT replaced by above 4/67/95
        j = 4;
        while (caseInfo[c-'A'].species_short_name[i][j] == ' ' ||
               caseInfo[c-'A'].species_short_name[i][j] == '\0') j--;
        */

        if ( j < 0 )
            {
            strcpy ( s, olds );
            return errmsg ( ermsg );
            }
        for ( k = 0; k <= j; k++ )
            tspec[k] = toupper ( caseInfo[c-'A'].species_short_name[i][k] );

        tspec[j+1] = '\0';

        /* if its a dry or wet dep we must do some extra checking */
        /*dh remove when all new files with 2d/3d indicator */
        if ( ( ( ( depFlag == 'W' ) && ( strncmp ( "WET", tspec, 3 ) == 0 ) ) ||
                ( ( depFlag == 'D' ) && ( strncmp ( "DRY", tspec, 3 ) == 0 ) ) ) && ( j == 2 ) )
            {
            for ( k = 0+6* ( depFlag=='D' ); k < 6+6* ( depFlag=='D' ); k++ )
                {
                strcpy ( tts, depNames[k] );
                for ( m = 0; m < ( int ) strlen ( tts ); m++ ) tts[m] = toupper ( tts[m] );
                if ( ( strncmp ( tts, s, strlen ( tts ) ) == 0 ) &&
                        ( strlen ( s ) == 1 + strlen ( tts ) ) )
                    {
                    oldDep = TRUE; /* added this line here instead of just below
                                  SRT 9/19/90.  Before "dry:so2a+so2a" was allowed
                                  but "so2a+dry:so2a" was not */
                    if ( arrayType == 'S' )
                        {
                        /* oldDep = TRUE; dh mark if old deps.. cannot mix old ones */
                        strcpy ( s, olds );
                        return errmsg ( "Can't mix concs with old deps!" );
                        }
                    arrayType = 'D';
                    sp[0] = 'D';
                    /* dh make it the absolute starting level */
                    itoa ( ( i*kmax ), &sp[1] );
                    strcpy ( cs, ".  " );
                    cs[1] = '0' + k;
                    if ( tspec[0] == 'D' ) cs[1] -= 6;
                    cs[2] = tolower ( c );
                    strcat ( sp, cs );
                    for ( m = 0; m < ( int ) strlen ( tts ); m++ ) s[m] = depNames[k][m];
                    s[len-1] = tolower ( c );
                    if ( thisHour >= 0 )
                        {
                        tstring[0] = ':';
                        itoa ( thisHour, &tstring[1] );
                        strcat ( sp, tstring );
                        strcat ( s, tstring );
                        *mixCase = 1; /* to denote can't identify hour of formula */
                        }
                    if ( dt ) /* change with respect to time case */
                        {
                        strcat ( sp, ":dt" );
                        strcpy ( tstring, "d[" );
                        strcat ( tstring, s );
                        strcat ( tstring, "]/dt" );
                        strcpy ( s, tstring );
                        }
                    if ( whichUnit[0] == '\0' )
                        {
                        /* ADDED 951020 SRT */        if ( caseInfo[c-'A'].units_name[i] )
                            strcpy ( whichUnit,
                                     caseInfo[c-'A'].units_name[i] );
                        /* ADDED 951020 SRT */            else
                            /* ADDED 951020 SRT */          strcpy ( whichUnit, " " );
                        if ( dt ) strcat ( whichUnit, "/hr" );
#ifdef DIAGNOSTICS
                        fprintf ( stderr,
                                  "parse.c 1 set whichUnit to '%s'\n",
                                  whichUnit );
#endif /* DIAGNOSTICS */
                        }
                    else
                        {
                        /* note: this assumes that
                           units are case insensitive */
                        if ( strcasecmp ( whichUnit,
                                          caseInfo[c-'A'].units_name[i] ) )
                            strcpy ( whichUnit, "???" );
#ifdef DIAGNOSTICS
                        fprintf ( stderr,
                                  "parse.c 2 set whichUnit to '%s'\n",
                                  whichUnit );
#endif /* DIAGNOSTICS */
                        }
                    return 0;
                    }
                }
            strcpy ( s, olds );
            return errmsg ( ermsg );
            }
        /* dh end remove*/

        /* it had better be just a regular species at this point */
        if ( ( strlen ( tspec ) == len-1 ) && ( strncmp ( s, tspec, len-1 ) == 0 ) )
            {
            if ( ( arrayType == 'D' ) && ( oldDep ) ) /*dh only if have an old type */
                {
                strcpy ( s, olds );
                return errmsg ( "Can't mix concs with old deps!" );
                }

            strcpy ( s, caseInfo[c-'A'].species_short_name[i] );

#ifdef DIAGNOSTICS
            fprintf ( stderr,"setting s to '%s' in checkAtom()\n", s );
#endif /* DIAGNOSTICS */

            /* dh set to D for 2d specie, S for 3d specie */
            /* if (headers[c-'A']->s[i].sfil[0]=='2') */
            if ( caseInfo[c-'A'].nlevel == 1 )
                {
                sp[0] = 'D';
                arrayType = 'D';
                }
            else /* if (headers[c-'A']->s[i].sfil[0]=='3') */
                {
                if ( oldDep ) /*  Added this line here SRT 9/19/90.
                            Before "dry:so2a+so2a" was allowed
                            but "so2a+dry:so2a" was not */
                    return errmsg ( "Can't mix concs with old deps!" );
                sp[0] = 'S';
                arrayType = 'S';
                /* if any of the arrays are 3dim species then act as if whole thing is*/
                }

            /* dh put in absolute starting level info */
            /* dh i contains the specie index, have header sfil with the 2 & 3d info */
            for ( index = 0, absLevels=0; index<i; index++ )
                {
                /* if (headers[c-'A']->s[index].sfil[0] == '2')
                    ++absLevels;
                   else
                   if (headers[c-'A']->s[index].sfil[0] == '3')
                    absLevels += kmax; */

                if ( caseInfo[c-'A'].nlevel == 1 )
                    ++absLevels;
                else
                    absLevels += kmax;
                }

            /* dh remove when all files have 2 & 3d indicators */
            if ( ( absLevels==0 ) && /* have a 3d old format file */
                    /*(headers[c-'A']->s[0].sfil[0] != '2'))*/ /*   added 11/9/90 SRT
                            for met 2d constant files, which have KMAX = 6 or 15 but
                            ALL species are 2d despite the KMAX */
                    ( caseInfo[c-'A'].nlevel > 1 ) )
                {
                arrayType = 'S';
                sp[0] = 'S';
                absLevels = kmax * i;
                }

            /* itoa (absLevels, &sp[1]); */
            /* dh remove itoa(i, &sp[1]); */
            /* SRT 12/16/94  add back itoa(i, &sp[1]); */
            itoa ( i, &sp[1] );
            cs[0] = tolower ( c );
            cs[1] = '\0';
            strcat ( sp, cs );
            len = strlen ( s );
            s[len] = tolower ( c );
            s[++len]='\0';
#ifdef DIAGNOSTICS
            fprintf ( stderr,"setting s to '%s' in checkAtom()\n", s );
#endif /* DIAGNOSTICS */
            if ( thisHour >= 0 )
                {
                tstring[0] = ':';
                itoa ( thisHour, &tstring[1] );
                strcat ( sp, tstring );
                strcat ( s, tstring );
                *mixCase = 1; /* to denote can't identify hour of formula */
                }
            if ( dt ) /* change with respect to time case */
                {
                strcat ( sp, ":dt" );
                strcpy ( tstring, "d[" );
                strcat ( tstring, s );
                strcat ( tstring, "]/dt" );
                strcpy ( s, tstring );
                }
            if ( whichUnit[0] == '\0' )
                {
                /* ADDED 951020 SRT */  if ( caseInfo[c-'A'].units_name[i] )
                    strcpy ( whichUnit, caseInfo[c-'A'].units_name[i] );
                /* ADDED 951020 SRT */          else
                    /* ADDED 951020 SRT */  strcpy ( whichUnit, " " );

                /* SRT 950911 whichUnit[n] = '\0'; */
                if ( dt ) strcat ( whichUnit, "/time step" );
#ifdef DIAGNOSTICS
                fprintf ( stderr,
                          "parse.c 3 set whichUnit to '%s'\n",
                          whichUnit );
#endif /* DIAGNOSTICS */
                }
            else
                {
                char u1[256], u2[256], ch;

                /* note: this assumes that
                   units are case insensitive */
                strcpy ( u1, whichUnit );
                strcpy ( u2, caseInfo[c-'A'].units_name[i] );
                for ( ch = 0; ch < ( char ) strlen ( u1 ); ch++ )
                    {
                    u1[ch] = toupper ( u1[ch] );
                    u2[ch] = toupper ( u2[ch] );
                    }
                if ( strncmp ( u1, u2, strlen ( u1 ) ) != 0 )
                    strcpy ( whichUnit, "???" );
#ifdef DIAGNOSTICS
                fprintf ( stderr,
                          "parse.c 4 set whichUnit to '%s'\n",
                          whichUnit );
#endif /* DIAGNOSTICS */
                }
            return 0;
            }
        }

    strcpy ( s, olds );
    return errmsg ( ermsg );
    }


static  int lparen_check ( int offset )
    {
    int fp;
    int status;

    fp = fpos;
    fpos += offset;
    status = 0;
    if ( !advance() )
        {
        status = ( atomType == LPAREN ) ;
        }
    fpos = fp;
    return status;
    }


/************************************************************
ADVANCE - advances the input pointer, grabbing the next atom
************************************************************/
static  int advance ( void )
    {
    int atompos = 0;    /* current position in the current atom */
    char    tstring[256];

    atomType = UNKNOWN;
    dt = 0;
    while ( formula[fpos] == ' ' ) fpos++;
    if ( formula[fpos] == '\0' )
        {
        atom[0] = '\0';
        atomType = END;
        }
    else if ( formula[fpos] == '+' )
        {
        strcpy ( atom, "+" );
        atomType = ADD;
        fpos++;
        }
    else if ( formula[fpos] == '-' )
        {
        strcpy ( atom, "-" );
        atomType = SUB;
        fpos++;
        }
    else if ( ( formula[fpos] == '&' ) && ( formula[fpos+1] == '&' ) )
        {
        strcpy ( atom, "&&" );
        atomType = AND;
        fpos+=2;
        }
    else if ( ( formula[fpos] == '|' ) && ( formula[fpos+1] == '|' ) )
        {
        strcpy ( atom, "||" );
        atomType = OR;
        fpos+=2;
        }
    else if ( ( formula[fpos] == '<' ) && ( formula[fpos+1] == '=' ) )
        {
        strcpy ( atom, "<=" );
        atomType = LTE;
        fpos+=2;
        }
    else if ( ( formula[fpos] == '>' ) && ( formula[fpos+1] == '=' ) )
        {
        strcpy ( atom, ">=" );
        atomType = GTE;
        fpos+=2;
        }
    else if ( ( formula[fpos] == '!' ) && ( formula[fpos+1] == '=' ) )
        {
        strcpy ( atom, "!=" );
        atomType = NEQ;
        fpos+=2;
        }
    else if ( ( formula[fpos] == '=' ) && ( formula[fpos+1] == '=' ) )
        {
        strcpy ( atom, "==" );
        atomType = EQ;
        fpos+=2;
        }
    else if ( formula[fpos] == '<' )
        {
        strcpy ( atom, "<" );
        atomType = LT;
        fpos++;
        }
    else if ( formula[fpos] == '>' )
        {
        strcpy ( atom, ">" );
        atomType = GT;
        fpos++;
        }
    else if ( ( formula[fpos] == '*' ) && ( formula[fpos+1] == '*' ) )
        {
        strcpy ( atom, "**" );
        atomType = POW;
        fpos+=2;
        }
    else if ( formula[fpos] == '*' )
        {
        strcpy ( atom, "*" );
        atomType = MUL;
        fpos++;
        }
    else if ( formula[fpos] == '/' )
        {
        strcpy ( atom, "/" );
        atomType = DIV;
        fpos++;
        }
    else if ( formula[fpos] == '(' )
        {
        strcpy ( atom, "(" );
        atomType = LPAREN;
        fpos++;
        }
    else if ( formula[fpos] == ')' )
        {
        strcpy ( atom, ")" );
        atomType = RPAREN;
        fpos++;
        }
    else if ( strncmp ( &formula[fpos], "SQRT", 4 ) == 0 )
        {
        if ( !lparen_check ( 4 ) ) goto maybe_species;
        strcpy ( atom, "sqrt" );
        atomType = SQRT;
        fpos += 4;
        }
    else if ( strncmp ( &formula[fpos], "SQR", 3 ) == 0 )
        {
        if ( !lparen_check ( 3 ) ) goto maybe_species;
        strcpy ( atom, "sqr" );
        atomType = SQR;
        fpos += 3;
        }
    else if ( strncmp ( &formula[fpos], "SIND", 4 ) == 0 )
        {
        if ( !lparen_check ( 4 ) ) goto maybe_species;
        strcpy ( atom, "sind" );
        atomType = SIND;
        fpos += 4;
        }
    else if ( strncmp ( &formula[fpos], "COSD", 4 ) == 0 )
        {
        if ( !lparen_check ( 4 ) ) goto maybe_species;
        strcpy ( atom, "cosd" );
        atomType = COSD;
        fpos += 4;
        }
    else if ( strncmp ( &formula[fpos], "TAND", 4 ) == 0 )
        {
        if ( !lparen_check ( 4 ) ) goto maybe_species;
        strcpy ( atom, "tand" );
        atomType = TAND;
        fpos += 4;
        }
    else if ( strncmp ( &formula[fpos], "SIN", 3 ) == 0 )
        {
        if ( !lparen_check ( 3 ) ) goto maybe_species;
        strcpy ( atom, "sin" );
        atomType = SIN;
        fpos += 3;
        }
    else if ( strncmp ( &formula[fpos], "COS", 3 ) == 0 )
        {
        if ( !lparen_check ( 3 ) ) goto maybe_species;
        strcpy ( atom, "cos" );
        atomType = COS;
        fpos += 3;
        }
    else if ( strncmp ( &formula[fpos], "TAN", 3 ) == 0 )
        {
        if ( !lparen_check ( 3 ) ) goto maybe_species;
        strcpy ( atom, "tan" );
        atomType = TAN;
        fpos += 3;
        }
    else if ( strncmp ( &formula[fpos], "LOG", 3 ) == 0 )
        {
        if ( !lparen_check ( 3 ) ) goto maybe_species;
        strcpy ( atom, "log" );
        atomType = LOG;
        fpos += 3;
        }
    else if ( strncmp ( &formula[fpos], "ABS", 3 ) == 0 )
        {
        if ( !lparen_check ( 3 ) ) goto maybe_species;
        strcpy ( atom, "abs" );
        atomType = ABS;
        fpos += 3;
        }
    else if ( strncmp ( &formula[fpos], "EXP", 3 ) == 0 )
        {
        if ( !lparen_check ( 3 ) ) goto maybe_species;
        strcpy ( atom, "exp" );
        atomType = EXP;
        fpos += 3;
        }
    else if ( strncmp ( &formula[fpos], "LN", 2 ) == 0 )
        {
        if ( !lparen_check ( 2 ) ) goto maybe_species;
        strcpy ( atom, "ln" );
        atomType = LN;
        fpos += 2;
        }
    else if ( strncmp ( &formula[fpos], "MINX", 4 ) == 0 )
        {
        if ( !lparen_check ( 4 ) ) goto maybe_species;
        strcpy ( atom, "minx" );
        atomType = MINX;
        fpos += 4;
        }
    else if ( strncmp ( &formula[fpos], "MINY", 4 ) == 0 )
        {
        if ( !lparen_check ( 4 ) ) goto maybe_species;
        strcpy ( atom, "miny" );
        atomType = MINY;
        fpos += 4;
        }
    else if ( strncmp ( &formula[fpos], "MINZ", 4 ) == 0 )
        {
        if ( !lparen_check ( 4 ) ) goto maybe_species;
        strcpy ( atom, "minz" );
        atomType = MINZ;
        fpos += 4;
        }
    else if ( strncmp ( &formula[fpos], "MINT", 4 ) == 0 )
        {
        if ( !lparen_check ( 4 ) ) goto maybe_species;
        strcpy ( atom, "mint" );
        atomType = MINT;
        fpos += 4;
        }
    else if ( strncmp ( &formula[fpos], "MAXT", 4 ) == 0 )
        {
        if ( !lparen_check ( 4 ) ) goto maybe_species;
        strcpy ( atom, "maxt" );
        atomType = MAXT;
        fpos += 4;
        }
    else if ( strncmp ( &formula[fpos], "MAXX", 4 ) == 0 )
        {
        if ( !lparen_check ( 4 ) ) goto maybe_species;
        strcpy ( atom, "maxx" );
        atomType = MAXX;
        fpos += 4;
        }
    else if ( strncmp ( &formula[fpos], "MAXY", 4 ) == 0 )
        {
        if ( !lparen_check ( 4 ) ) goto maybe_species;
        strcpy ( atom, "maxy" );
        atomType = MAXY;
        fpos += 4;
        }
    else if ( strncmp ( &formula[fpos], "MAXZ", 4 ) == 0 )
        {
        if ( !lparen_check ( 4 ) ) goto maybe_species;
        strcpy ( atom, "maxz" );
        atomType = MAXZ;
        fpos += 4;
        }
    else if ( strncmp ( &formula[fpos], "MEAN", 4 ) == 0 )
        {
        if ( !lparen_check ( 4 ) ) goto maybe_species;
        strcpy ( atom, "mean" );
        atomType = MEAN;
        fpos += 4;
        }
    else if ( strncmp ( &formula[fpos], "MIN", 3 ) == 0 )
        {
        if ( !lparen_check ( 3 ) ) goto maybe_species;
        strcpy ( atom, "min" );
        atomType = MINIMUM;
        fpos += 3;
        }
    else if ( strncmp ( &formula[fpos], "MAX", 3 ) == 0 )
        {
        if ( !lparen_check ( 3 ) ) goto maybe_species;
        strcpy ( atom, "max" );
        atomType = MAXIMUM;
        fpos += 3;
        }
    else if ( strncmp ( &formula[fpos], "SUM", 3 ) == 0 )
        {
        if ( !lparen_check ( 3 ) ) goto maybe_species;
        strcpy ( atom, "sum" );
        atomType = SUM;
        fpos += 3;
        }

    else if ( strncasecmp ( &formula[fpos], "d[", 2 ) == 0 )
        {
        /* we must have the d[<spec-name>]/dt case */
        int i;
        /*
            char *tp = (char *) strstr(&formula[fpos+2], "]/dt");
            if (tp > &formula[fpos+2])
        */
        char *tp = ( char * ) strstr ( &formula[fpos+2], "]/" );
        if ( tp != NULL )
            {
            if ( strncasecmp ( tp+2, "dt", 2 ) == 0 )
                {
                for ( i = 0; formula[fpos+2+i] != ']'; i++ )
                    atom[i] = formula[fpos+2+i];
                atom[i] = '\0';
                fpos = tp + 4 - &formula[0];
                dt = 1;
                if ( checkAtom ( atom, patom ) == 0 )
                    atomType = I;
                }
            }
        else
            return ( errmsg ( "Can't understand 'd['!" ) );
        }

    else /* it must be a species array name or a constant */
        {
maybe_species:
        while ( ( formula[fpos] != ' ' ) &&
                ( formula[fpos] != '\0' ) &&
                ( formula[fpos] != '+' ) &&
                ( formula[fpos] != '-' ) &&
                ( formula[fpos] != '*' ) &&
                ( formula[fpos] != '/' ) &&
                ( formula[fpos] != '(' ) &&
                ( formula[fpos] != ')' ) &&
                ( formula[fpos] != '<' ) &&
                ( formula[fpos] != '>' ) &&
                ( formula[fpos] != '=' ) &&
                ( formula[fpos] != '&' ) &&
                ( formula[fpos] != '|' ) &&
                ( formula[fpos] != '!' ) )
            atom[atompos++] = formula[fpos++];

        atom[atompos] = '\0';

        if ( checkAtom ( atom, patom ) == 0 )
            atomType = I;
        }

    strcat ( newformula, atom );
    strcat ( newformula, " " );

#ifdef DIAGNOSTICS
    strcpy ( tstring, "Advanced to atom >>>" );
    strcat ( tstring, atom );
    strcat ( tstring, "<<<" );
    diagmsg ( tstring );

    /*fprintf(stderr,"DEBUG:advance:%s\n",tstring);*/

#endif /* DIAGNOSTICS */


    return ( atomType == UNKNOWN );
    }



/************************************************************
GET_ACTION -    from the current item type on top of the stack
        (stack->itype) and the current input atom type
        (atomType), determines and returns the
        appropriate processor action.
************************************************************/
static int get_action ( void )
    {
    char tstring[256];

    if ( stack == NULL )
        {
        if ( atomType == END )
            return ACCEPT;
        else
            return REJECT;
        }

    switch ( stack->itype )
        {
        case AND:
            return ANDACT;

        case OR:
            return ORACT;

        case ADD:
            return ADDACT;

        case SUB:
            return SUBACT;

        case MUL:
            return MULACT;

        case EQ:
            return EQACT;

        case NEQ:
            return NEQACT;

        case GT:
            return GTACT;

        case GTE:
            return GTEACT;

        case LT:
            return LTACT;

        case LTE:
            return LTEACT;

        case DIV:
            return DIVACT;

        case POW:
            return POWACT;

        case LOG:
            return LOGACT;

        case SQR:
            return SQRACT;

        case SQRT:
            return SQRTACT;

        case ABS:
            return ABSACT;

        case EXP:
            return EXPACT;

        case LN:
            return LNACT;

        case SIN:
            return SINACT;

        case COS:
            return COSACT;

        case TAN:
            return TANACT;

        case SIND:
            return SINDACT;

        case COSD:
            return COSDACT;

        case TAND:
            return TANDACT;

        case MINX:
            return MINXACT;

        case MINY:
            return MINYACT;

        case MINZ:
            return MINZACT;

        case MAXX:
            return MAXXACT;

        case MAXY:
            return MAXYACT;

        case MAXZ:
            return MAXZACT;

        case MEAN:
            return MEANACT;

        case MINIMUM:
            return MINACT;

        case MAXIMUM:
            return MAXACT;

        case SUM:
            return SUMACT;

        case MINT:
            return MINTACT;

        case MAXT:
            return MAXTACT;

        case A:
            switch ( atomType )
                {
                case I:
                case LPAREN:
                case ABS:
                case SQR:
                case SQRT:
                case LOG:
                case EXP:
                case LN:
                case SIN:
                case COS:
                case TAN:
                case SIND:
                case COSD:
                case TAND:
                case MINX:
                case MINY:
                case MINZ:
                case MAXX:
                case MAXY:
                case MAXZ:
                case MEAN:
                case MINIMUM:
                case MAXIMUM:
                case MINT:
                case MAXT:
                case SUM:
                    return ACT43;
                }
            strcpy ( tstring, "Can't understand '" );
            strcat ( tstring, atom );
            strcat ( tstring, "'!" );
            errmsg ( tstring );
            break;

        case B:
            switch ( atomType )
                {
                case I:
                case LPAREN:
                case ABS:
                case SQR:
                case SQRT:
                case LOG:
                case EXP:
                case LN:
                case SIN:
                case COS:
                case TAN:
                case SIND:
                case COSD:
                case TAND:
                case MINX:
                case MINY:
                case MINZ:
                case MAXX:
                case MAXY:
                case MAXZ:
                case MEAN:
                case MINIMUM:
                case MAXIMUM:
                case MINT:
                case MAXT:
                case SUM:
                    return ACT44;
                }
            strcpy ( tstring, "Can't understand '" );
            strcat ( tstring, atom );
            strcat ( tstring, "'!" );
            errmsg ( tstring );
            break;

        case C:
            switch ( atomType )
                {
                case I:
                case LPAREN:
                case ABS:
                case SQR:
                case SQRT:
                case LOG:
                case EXP:
                case LN:
                case SIN:
                case COS:
                case TAN:
                case SIND:
                case COSD:
                case TAND:
                case MINX:
                case MINY:
                case MINZ:
                case MAXX:
                case MAXY:
                case MAXZ:
                case MEAN:
                case MINIMUM:
                case MAXIMUM:
                case MINT:
                case MAXT:
                case SUM:
                    return ACT35;
                }
            strcpy ( tstring, "Can't understand '" );
            strcat ( tstring, atom );
            strcat ( tstring, "'!" );
            errmsg ( tstring );
            break;

        case D:
            switch ( atomType )
                {
                case I:
                case LPAREN:
                case ABS:
                case SQR:
                case SQRT:
                case LOG:
                case EXP:
                case LN:
                case SIN:
                case COS:
                case TAN:
                case SIND:
                case COSD:
                case TAND:
                case MINX:
                case MINY:
                case MINZ:
                case MAXX:
                case MAXY:
                case MAXZ:
                case MEAN:
                case MINIMUM:
                case MAXIMUM:
                case MINT:
                case MAXT:
                case SUM:
                    return ACT36;
                }
            strcpy ( tstring, "Can't understand '" );
            strcat ( tstring, atom );
            strcat ( tstring, "'!" );
            errmsg ( tstring );
            break;

        case E:
            switch ( atomType )
                {
                case I:
                case LPAREN:
                case ABS:
                case SQR:
                case SQRT:
                case LOG:
                case EXP:
                case LN:
                case SIN:
                case COS:
                case TAN:
                case SIND:
                case COSD:
                case TAND:
                case MINX:
                case MINY:
                case MINZ:
                case MAXX:
                case MAXY:
                case MAXZ:
                case MEAN:
                case MINIMUM:
                case MAXIMUM:
                case MINT:
                case MAXT:
                case SUM:
                    return ACT1;
                }
            strcpy ( tstring, "Can't understand '" );
            strcat ( tstring, atom );
            strcat ( tstring, "'!" );
            errmsg ( tstring );
            break;

        case T:
            switch ( atomType )
                {
                case I:
                case LPAREN:
                case ABS:
                case SQR:
                case SQRT:
                case LOG:
                case EXP:
                case LN:
                case SIN:
                case COS:
                case TAN:
                case SIND:
                case COSD:
                case MINX:
                case MINY:
                case MINZ:
                case MAXX:
                case MAXY:
                case MAXZ:
                case MEAN:
                case MINIMUM:
                case MAXIMUM:
                case SUM:
                case MINT:
                case MAXT:
                case TAND:
                    return ACT2;
                }
            strcpy ( tstring, "Can't understand '" );
            strcat ( tstring, atom );
            strcat ( tstring, "'!" );
            errmsg ( tstring );
            break;

        case Q:
            switch ( atomType )
                {
                case I:
                case LPAREN:
                case ABS:
                case SQR:
                case SQRT:
                case LOG:
                case EXP:
                case LN:
                case SIN:
                case COS:
                case TAN:
                case SIND:
                case COSD:
                case MINX:
                case MINY:
                case MINZ:
                case MAXX:
                case MAXY:
                case MAXZ:
                case MEAN:
                case MINIMUM:
                case MAXIMUM:
                case SUM:
                case MINT:
                case MAXT:
                case TAND:
                    return ACT22;
                }
            strcpy ( tstring, "Can't understand '" );
            strcat ( tstring, atom );
            strcat ( tstring, "'!" );
            errmsg ( tstring );
            break;

        case P:
            switch ( atomType )
                {
                case I:
                    return ACT3;
                case LPAREN:
                    return ACT4;
                case ABS:
                    return ACT13;
                case SQR:
                    return ACT5;
                case SQRT:
                    return ACT6;
                case LOG:
                    return ACT7;
                case EXP:
                    return ACT14;
                case LN:
                    return ACT15;
                case SIN:
                    return ACT16;
                case COS:
                    return ACT17;
                case TAN:
                    return ACT18;
                case SIND:
                    return ACT19;
                case COSD:
                    return ACT20;
                case TAND:
                    return ACT21;
                case MINX:
                    return ACT23;
                case MINY:
                    return ACT24;
                case MINZ:
                    return ACT25;
                case MAXX:
                    return ACT26;
                case MAXY:
                    return ACT27;
                case MAXZ:
                    return ACT28;
                case MEAN:
                    return ACT29;
                case MINIMUM:
                    return ACT30;
                case MAXIMUM:
                    return ACT31;
                case SUM:
                    return ACT32;
                case MINT:
                    return ACT33;
                case MAXT:
                    return ACT34;
                }
            strcpy ( tstring, "Can't understand '" );
            strcat ( tstring, atom );
            strcat ( tstring, "'!" );
            errmsg ( tstring );
            break;

        case ALIST:
            switch ( atomType )
                {
                case OR:
                    return ACT45;
                case RPAREN:
                case END:
                    return POPRET;
                }
            strcpy ( tstring, "Found '" );
            strcat ( tstring, atom );
            strcat ( tstring, "'; expected ) or || !" );
            errmsg ( tstring );
            break;

        case BLIST:
            switch ( atomType )
                {
                case AND:
                    return ACT46;
                case OR:
                case RPAREN:
                case END:
                    return POPRET;
                }
            strcpy ( tstring, "Found '" );
            strcat ( tstring, atom );
            strcat ( tstring, "'; expected &&, ||, or ) !" );
            errmsg ( tstring );
            break;

        case CLIST:
            switch ( atomType )
                {
                case EQ:
                    return ACT41;
                case NEQ:
                    return ACT42;
                case AND:
                case OR:
                case RPAREN:
                case END:
                    return POPRET;
                }
            strcpy ( tstring, "Found '" );
            strcat ( tstring, atom );
            strcat ( tstring, "'; expected ),&&,||,==,or != !" );
            errmsg ( tstring );
            break;

        case DLIST:
            switch ( atomType )
                {
                case LT:
                    return ACT37;
                case LTE:
                    return ACT38;
                case GT:
                    return ACT39;
                case GTE:
                    return ACT40;
                case AND:
                case OR:
                case EQ:
                case NEQ:
                case RPAREN:
                case END:
                    return POPRET;
                }
            strcpy ( tstring, "Found '" );
            strcat ( tstring, atom );
            strcat ( tstring, "'; expected ),<,<=,>,>=,&&,||,==,or != !" );
            errmsg ( tstring );
            break;

        case ELIST:
            switch ( atomType )
                {
                case ADD:
                    return ACT8;
                case SUB:
                    return ACT9;
                case AND:
                case OR:
                case RPAREN:
                case LT:
                case LTE:
                case GT:
                case GTE:
                case EQ:
                case NEQ:
                case END:
                    return POPRET;
                }
            strcpy ( tstring, "Found '" );
            strcat ( tstring, atom );
            strcat ( tstring, "'; expected +,-,),<,<=,>,>=,&&,||,==,or != !" );
            errmsg ( tstring );
            break;

        case TLIST:
            switch ( atomType )
                {
                case MUL:
                    return ACT10;
                case DIV:
                    return ACT11;
                case AND:
                case OR:
                case LT:
                case LTE:
                case GT:
                case GTE:
                case EQ:
                case NEQ:
                case ADD:
                case SUB:
                case RPAREN:
                case END:
                    return POPRET;
                }
            strcpy ( tstring, "Found '" );
            strcat ( tstring, atom );
            strcat ( tstring,
                     "'; expected '+','-',')','*','/'"
                     "<,<=,>,>=,&&,||,==,or != !" );
            errmsg ( tstring );
            break;

        case QLIST:
            switch ( atomType )
                {
                case POW:
                    return ACT12;
                case AND:
                case OR:
                case LT:
                case LTE:
                case GT:
                case GTE:
                case EQ:
                case NEQ:
                case ADD:
                case SUB:
                case MUL:
                case DIV:
                case RPAREN:
                case END:
                    return POPRET;
                }
            strcpy ( tstring, "Found '" );
            strcat ( tstring, atom );
            strcat ( tstring,
                     "'; expected '+','-',')','*','**','/',"
                     "<,<=,>,>=,&&,||,==,or != !" );
            errmsg ( tstring );
            break;

        case RPAREN:
            if ( atomType == RPAREN ) return POPADV;
            strcpy ( tstring, "Expected ')' but found '" );
            strcat ( tstring, atom );
            strcat ( tstring, "'!" );
            errmsg ( tstring );
            break;
        }

    return REJECT;
    }



/************************************************************
PROCESS -   the main event loop for the expression processor.
            this returns:

            0 if processing should continue
            1 if it has completed processing successfully
            2 if processing has encountered an error

            the basic algorithm is to call get_action() to
            determine the next step to take, then perform
            that correct action in this subroutine.
************************************************************/
static  int process ( void )
    {
    int     action;
    char    tstring[512];

    action = get_action();

    if ( action != ACCEPT )
        if ( action != REJECT )
            if ( pop() )
                return 1+errmsg ( "Pop in parse failed!" );
    switch ( action )
        {
        case POPRET:
            break;

        case POPADV:
            if ( advance() ) return 2;
            break;

        case ACCEPT:
            diagmsg ( "ACCEPT!" );
            return 1;

        case ACT1:
            if ( push ( ELIST ) ) return 2;
            if ( push ( T ) ) return 2;
            break;

        case ACT2:
            if ( push ( TLIST ) ) return 2;
            if ( push ( Q ) ) return 2;
            break;

        case ACT22:
            if ( push ( QLIST ) ) return 2;
            if ( push ( P ) ) return 2;
            break;

        case ACT3:
            strcat ( patom, " " );
            output ( patom );
            if ( advance() ) return 2;
            break;

        case ACT4:
            if ( push ( RPAREN ) ) return 2;
            if ( push ( A ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT5:
            if ( push ( SQR ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT6:
            if ( push ( SQRT ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT7:
            if ( push ( LOG ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT8:
            if ( push ( ELIST ) ) return 2;
            if ( push ( ADD ) ) return 2;
            if ( push ( T ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT9:
            if ( push ( ELIST ) ) return 2;
            if ( push ( SUB ) ) return 2;
            if ( push ( T ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT10:
            if ( push ( TLIST ) ) return 2;
            if ( push ( MUL ) ) return 2;
            if ( push ( Q ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT11:
            if ( push ( TLIST ) ) return 2;
            if ( push ( DIV ) ) return 2;
            if ( push ( Q ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT12:
            if ( push ( QLIST ) ) return 2;
            if ( push ( POW ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT13:
            if ( push ( ABS ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT14:
            if ( push ( EXP ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT15:
            if ( push ( LN ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT16:
            if ( push ( SIN ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT17:
            if ( push ( COS ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT18:
            if ( push ( TAN ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT19:
            if ( push ( SIND ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT20:
            if ( push ( COSD ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT21:
            if ( push ( TAND ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT23:
            if ( push ( MINX ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT24:
            if ( push ( MINY ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT25:
            if ( push ( MINZ ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT26:
            if ( push ( MAXX ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT27:
            if ( push ( MAXY ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT28:
            if ( push ( MAXZ ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT29:
            if ( push ( MEAN ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT30:
            if ( push ( MINIMUM ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT31:
            if ( push ( MAXIMUM ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT32:
            if ( push ( SUM ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT33:
            if ( push ( MINT ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT34:
            if ( push ( MAXT ) ) return 2;
            if ( push ( P ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT43:
            if ( push ( ALIST ) ) return 2;
            if ( push ( B ) ) return 2;
            break;

        case ACT44:
            if ( push ( BLIST ) ) return 2;
            if ( push ( C ) ) return 2;
            break;

        case ACT35:
            if ( push ( CLIST ) ) return 2;
            if ( push ( D ) ) return 2;
            break;

        case ACT36:
            if ( push ( DLIST ) ) return 2;
            if ( push ( E ) ) return 2;
            break;

        case ACT37:
            if ( push ( DLIST ) ) return 2;
            if ( push ( LT ) ) return 2;
            if ( push ( E ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT38:
            if ( push ( DLIST ) ) return 2;
            if ( push ( LTE ) ) return 2;
            if ( push ( E ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT39:
            if ( push ( DLIST ) ) return 2;
            if ( push ( GT ) ) return 2;
            if ( push ( E ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT40:
            if ( push ( DLIST ) ) return 2;
            if ( push ( GTE ) ) return 2;
            if ( push ( E ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT45:
            if ( push ( ALIST ) ) return 2;
            if ( push ( OR ) ) return 2;
            if ( push ( B ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT46:
            if ( push ( BLIST ) ) return 2;
            if ( push ( AND ) ) return 2;
            if ( push ( C ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT41:
            if ( push ( CLIST ) ) return 2;
            if ( push ( EQ ) ) return 2;
            if ( push ( D ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ACT42:
            if ( push ( CLIST ) ) return 2;
            if ( push ( NEQ ) ) return 2;
            if ( push ( D ) ) return 2;
            if ( advance() ) return 2;
            break;

        case ORACT:
            output ( "|| " );
            break;

        case ANDACT:
            output ( "&& " );
            break;

        case ADDACT:
            output ( "+ " );
            break;

        case SUBACT:
            output ( "- " );
            break;

        case MULACT:
            output ( "* " );
            break;

        case DIVACT:
            output ( "/ " );
            break;

        case POWACT:
            output ( "** " );
            break;

        case ABSACT:
            output ( "abs " );
            break;

        case SQRACT:
            output ( "sqr " );
            break;

        case SQRTACT:
            output ( "sqrt " );
            break;

        case LOGACT:
            output ( "log " );
            break;

        case EXPACT:
            output ( "exp " );
            break;

        case LNACT:
            output ( "ln " );
            break;

        case SINACT:
            output ( "sin " );
            break;

        case COSACT:
            output ( "cos " );
            break;

        case TANACT:
            output ( "tan " );
            break;

        case SINDACT:
            output ( "sind " );
            break;

        case COSDACT:
            output ( "cosd " );
            break;

        case TANDACT:
            output ( "tand " );
            break;

        case MINXACT:
            output ( "minx " );
            break;

        case MINYACT:
            output ( "miny " );
            break;

        case MINZACT:
            output ( "minz " );
            break;

        case MAXXACT:
            output ( "maxx " );
            break;

        case MAXYACT:
            output ( "maxy " );
            break;

        case MAXZACT:
            output ( "maxz " );
            break;

        case MEANACT:
            output ( "mean " );
            break;

        case MINACT:
            output ( "min " );
            break;

        case MAXACT:
            output ( "max " );
            break;

        case MINTACT:
            output ( "mint " );
            break;

        case MAXTACT:
            output ( "maxt " );
            break;

        case SUMACT:
            output ( "sum " );
            break;

        case GTACT:
            output ( "> " );
            break;

        case GTEACT:
            output ( ">= " );
            break;

        case LTACT:
            output ( "< " );
            break;

        case LTEACT:
            output ( "<= " );
            break;

        case EQACT:
            output ( "== " );
            break;

        case NEQACT:
            output ( "!= " );
            break;

        case REJECT:
        default:
            diagmsg ( "REJECT!" );
            return 2;
        }

    return 0;
    }


/************************************************************
FREECASEINFO -  frees up storage used for case information structs
************************************************************/
static
void    freeCaseInfo ( void )
    {
    int i;

    if ( caseInfo != NULL )
        {
        for ( i = 0; i < ncases; i++ )
            if ( caseInfo[i].filename != NULL )
                free_vis ( &caseInfo[i] );


        free ( caseInfo );
        caseInfo = NULL;
        }
    }




/************************************************************
PARSEFORMULA -  subroutine to do the infix -> postfix translation
        returns 0 if success, if failure returns
        non-zero and stuffs the errString argument
        with an appropriate error msg
************************************************************/
int     parseFormula

(

    /* INPUTS TO parseFormula */

    char *formulaP,       /* infix formula typed in by user;
                     NOTE:  this is possibly
                     "prettied up" to look better
                     on graphs and in dialog boxes */

    char *caseListP,      /* a list of cases (ie data file
                     names) in order (a..?),
                     separated by commas */

    char *hostListP,      /* a list of hosts (ie
                     "todd.hpcc.epa.gov,ozone,
                     flyer.ncsc.org") separated by
                                         commas, one for each case in
                     caseList */

    struct BusData *bdP,  /* needed to communicate with
                     the SW Bus; this should already
                     have been initialized with
                     initVisDataClient() */



    /* MODIFIED BY parseFormula */
    char *errString,      /* errormsg if any */
    char *postFixQueueP,  /* postfix formula result */
    char *caseUsedP,      /* "010" if ncases = 3 and only
                         case b in formula */
    char *whichUnitP,     /* the units of the formula's output */

    int  *dimP,           /* ndim of formula result's data */
    int  *dateDayP,       /* the day of formula result */
    int  *dateMonthP,     /* the month of formula result */
    int  *dateYearP,      /* the year of formula result */
    int  *hourStartP,     /* starting hour */
    int  *mixCaseP,       /* 1 if can't put a time on the
                     starting hour, otherwise 0 */
    int  *IMAX,       /* IMAX for formula */
    int  *JMAX,       /* JMAX for formula */
    int  *KMAX        /* KMAX for formula */
)
    {
    int returnval, i;
    char    tstring[512], tstring2[512], tstring3[512];

#ifdef DIAGNOSTICS
    fprintf ( stderr, "\nEnter parseFormula()\n" );
    fprintf ( stderr, "formulaP == '%s'\n", formulaP );
    fprintf ( stderr, "caseListP == '%s'\n", caseListP );
    fprintf ( stderr, "hostListP == '%s'\n", hostListP );
#endif /* DIAGNOSTICS */

    /* miscellaneous initializations */

    /* dh remove when have all files with 2d/3d indicators */
    depNames[ 0] = "WET:Hplus";
    depNames[ 1] = "WET:SO4";
    depNames[ 2] = "WET:H2SO4";
    depNames[ 3] = "WET:HNO3";
    depNames[ 4] = "WET:NH3";
    depNames[ 5] = "WET:PAN";
    depNames[ 6] = "DRY:SO2";
    depNames[ 7] = "DRY:SO4";
    depNames[ 8] = "DRY:NO2";
    depNames[ 9] = "DRY:NO";
    depNames[10] = "DRY:O3";
    depNames[11] = "DRY:HNO3";
    /* end remove dh */

    oldDep = FALSE;
    dt = 0;
    formula = formulaP;
    /* fprintf(stderr,"DEBUGG:parsing FORMULA \"%s\" for aliases\n",formulaP); */

    caseList = caseListP;
    hostList = hostListP;
    bd = bdP;
    postFixQueue = postFixQueueP;
    caseUsed = caseUsedP;
    whichUnit = whichUnitP;
    dateDay = dateDayP;
    dateMonth = dateMonthP;
    dateYear = dateYearP;
    hourStart = hourStartP;
    errorString = errString;
    mixCase = mixCaseP;
    oldmm = olddd = oldyy = oldhh = -1;
    imax = jmax = kmax = -1;
    dim = dimP;
    newformula[0] = errString[0] = postFixQueue[0] =
                                       caseUsed[0] = whichUnit[0] = '\0';
    *dim = *dateDay = *dateMonth = *dateYear = *hourStart = *mixCase = 0;
    arrayType = 0;
    map_info = NULL;


    if ( hostListP == NULL )
        return errmsg ( "NULL hostListP in parseFormula() !! \n" );

    if ( bd == NULL )
        fprintf ( stderr, "NULL bdP in parseFormula() !! Assuming local data\n" );

    doLousyCheck = 0;

    /* how many cases do we have available? */
    ncases = 0;
    while ( !getNthItem ( ncases+1, caseList, tstring ) ) ncases++;

    /* allocate space to hold info on each case */
    if ( ( caseInfo = ( VIS_DATA * ) malloc ( ncases * sizeof ( VIS_DATA ) ) ) == NULL )
        return errmsg ( "caseInfo malloc() failed in parseFormula!" );

    /* set case info records to all NULL values */
    memset ( ( void * ) caseInfo, 0, ( size_t ) ( ncases * sizeof ( VIS_DATA ) ) );


    strcpy ( tstring, "Formula: " );
    strcat ( tstring, formula );
    diagmsg ( tstring );

    /* strcpy(dim, "2");  dim[0]='2'; SRT changed 11/9/90 */ /*dh indicator of 2 or 3 dimension*/
    for ( i = 0; i < ( int ) strlen ( formula ); i++ )
        formula[i] = toupper ( formula[i] );
    for ( i = 0; i < ncases; i++ )
        caseUsed[i] = '0';
    caseUsed[ncases] = '\0';

#ifdef SQUAT
    for ( i = 0; i < 10; i++ )          /* initialize header pointers to null */
        headers[i] = 0L;
#endif

    fpos = 0;                           /* set the initial position in the formula */
    stack = NULL;                       /* initialize stack */
    if ( push ( A ) )
        {
        freeCaseInfo();
        return 1;       /* set up start stack */
        }

    /* check for special case of wind vectors */
    if ( ( strlen ( formula ) == 12 ) && ( strncmp ( formula, "WINDVECTORS", 11 ) == 0 ) )
        {
        strcpy ( atom, "U " );
        strcpy ( tstring, "V " );
        atom[1] = tstring[1] = formula[11];
        if ( ( checkAtom ( atom, tstring2 ) ) || ( checkAtom ( tstring, tstring3 ) ) )
            returnval = 2;
        else
            {
            /* SRT 9/19/90 modified this U and V position check to work
               with the new method of representing species internally */
            int len2, len3;
            char last2, last3;
            len2 = strlen ( tstring2 );
            len3 = strlen ( tstring3 );
            last2 = tstring2[len2-1];
            last3 = tstring3[len3-1];
            tstring3[len3-1] = tstring2[len2-1] = '\0';
            if ( ( tstring2[0] != 'S' ) ||
                    ( tstring3[0] != 'S' ) ||
                    ( atoi ( &tstring2[1] ) != 0 ) ||
                    ( atoi ( &tstring3[1] ) != kmax ) )
                returnval = 1 + errmsg ( "U and V wrong position in file!" );
            else
                {
                strcpy ( postFixQueue, "WindVectors  " );
                strcpy ( newformula, "WindVectors " );
                newformula[11] = postFixQueue[11] = /* 'a' + 'A' - formula[11] SRT 11/7/90 */
                                     'a' + formula[11] - 'A';
                returnval = 1;
                }
            }
        }
    else
        {
        if ( advance() )    /* find initial atom in formula */
            returnval = 2;
        else
            /* process until done with formula */
            while ( ! ( returnval = process() ) );
        }
    if ( ( arrayType == 0 ) && ( returnval <= 1 ) )
        {
        returnval = 2;
        errmsg ( "Formulas must include a species!" );
        }

    /* if returnval is > 1 then there was an error */
    if ( returnval > 1 ) while ( pop() == 0 );

    /* get rid of the last unnecessary space */
    postFixQueue[strlen ( postFixQueue )-1]='\0';

    if ( strncmp ( "???", whichUnit, 3 ) == 0 )
        {
#ifdef DIAGNOSTICS
        fprintf ( stderr, "Don't know units !\n" );
#endif /* DIAGNOSTICS */
        strcpy ( whichUnit, "" );
        }

    /* put the "prettied up" version of the formula back into formula arg */
    strcpy ( formula, newformula );

    freeCaseInfo();
    *dim = 2 + ( kmax > 1 );
    *IMAX = imax;
    *JMAX = jmax;
    *KMAX = kmax;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "formula == '%s'\n", formula );
    fprintf ( stderr, "postFixQueue == '%s'\n", postFixQueue );
#endif /* DIAGNOSTICS */

    return ( returnval - 1 );
    }

int renameFormula ( char *f, int ds,   char *newFormula )
    {
    int i;
    int len;
    char c;


    formula = f;
    fpos = 0;
    newFormula[0]='\0';
    doLousyCheck = 1;

    while ( !advance() )
        {
        if ( atomType == END ) break;

        if ( atomType == I )
            {
            if ( strcmp ( atom, patom ) )
                {
                if ( patom[0] != 'C' )
                    {
                    len = strlen ( atom );
                    if ( len <= 0 )
                        {
                        newFormula[0]='\0';
                        return 0;
                        }
                    len --;
                    c = atom[len];
                    i = tolower ( c ) - 'a';
                    if ( i==ds )
                        {
                        newFormula[0]='\0';
                        return 0;
                        }
                    if ( i > ds ) atom[len] = 'a'+i-1;
                    }
                }
            }
        strcat ( newFormula, atom );
        }
    return 1;

    }


int evalTokens ( char *f, char *token[], int *tflag )
    {
    int i;
    int len;
    char c;
    int flag;
    int count;


    formula = f;
    fpos = 0;
    doLousyCheck = 1;
    count = 0;

    while ( !advance() )
        {
        if ( atomType == END ) break;
        if ( dt )
            {
            tflag[count] = 0;
            token[count] = strdup ( "d[" );
            count++;
            }

        flag = 0;
        if ( atomType == I )
            {
            if ( strcmp ( atom, patom ) )
                {
                if ( patom[0] != 'C' )
                    {
                    /* it's a constant */
                    len = strlen ( atom );
                    if ( len <= 0 )
                        {
                        goto error;
                        }
                    flag = 1;
                    }
                }
            }
        tflag[count] = flag;
        token[count] = strdup ( atom );
        count++;

        if ( dt )
            {
            tflag[count] = 0;
            token[count] = strdup ( "]/dt" );
            count++;
            }
        }
    return count;

error:
    while ( count>=0 )
        {
        free ( token[count] );
        count--;
        }
    return 0;

    }
