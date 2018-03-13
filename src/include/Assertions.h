#ifndef ASSERTIONS_H
#define ASSERTIONS_H

/******************************************************************************
 * PURPOSE: Assertions.h - Defines assertion macros for optionally verifying
 *          routine pre-conditions, post-conditions and class invariants -
 *          as inspired by Eiffel's 'Design By Contract'.
 *
 * NOTES:   
 *          This implementation is based on (uses) the standard C assert()
 *          macro and (if C++) the (required) local class member function:
 *
 *            virtual bool invariant() const // You must define this member
 *                                           // function for each class to
 *                                           // use PRE() and POST() properly.
 *
 *          Macros defined include:
 *
 *            INV                          Assert class invariant (if C++).
 *            PRE(c),  PRE2(c1,c2),  ...   Assert INV and pre-condition(s)
 *            POST(c), POST2(c1,c2), ...   Assert INV and post-condition(s)
 *            CHECK(c), CHECK2(c1,c2), ... Assert arbitrary condition(s)
 *            DEBUG(s)                     Add optional debug statements
 *
 *          Support macros include:
 *
 *            OLD(variable)                  For referring to previous values.
 *            REMEMBER(type,variable)        Required if OLD() is used...
 *            REMEMBER_F(type,function_name) Same as above but for functions.
 *
 *            IMPLIES(p,c)                   Logical implication.
 *            AND2(c1,c2), AND3(c1,c2,c3)... Logical and.
 *            OR2(c1,c2),  OR3(c1,c2,c3)...  Logical or.
 *            XOR2(c1,c2), XOR3(c1,c2,c3)... Logical exclusive-or.
 *            IN3(x,a,b),  IN4(x,a,b,c)...   Set membership: x in {a,b}?
 *            IN_RANGE(x,low,high)           x in [low,high]?
 *            IS_BOOL2(a,b), ...             Each in {0, 1}?
 *            NON_ZERO2(a,b), ...            Each == 0?
 *            IS_ZERO2(a,b), ...             Each != 0?
 *            GT_ZERO2(a,b), ...             Each >  0?
 *            GE_ZERO2(a,b), ...             Each >= 0?
 *
 *          There are three levels of assertion checking and they are
 *          specified on the command line or in the Makefile:
 *
 *            (default)            - Enables  all assertion checking.
 *            -DNO_ASSERTIONS      - Disables all assertion checking.
 *            -DPRECONDITIONS_ONLY - Enables PRE(c) checking only.
 *
 *          Similaraly, there are two levels of debugging:
 *
 *            (default)            - Disables DEBUG(s) statements.
 *            -DDEBUGGING          - Enables  DEBUG(s) statements.
 *
 *          If any asserted condition evaluates to false at runtime then
 *          the program will display a message such as:
 *
 *         Assertion failed: (int)( index >= 0 ), file Test.c, line 44
 *
 *          And then abort() producing a core file used for debugging.
 *          For example, to debug a bad call to a routine (violated
 *          pre-condition):
 *
 *            dbx Test
 *            dbx> up 4
 *            (this is the assertion line)
 *            dbx> up
 *            (this is the bad call line)
 *
 *          Note: Checking preconditions or postconditions imply checking the
 *          invariant also. If there is no precondition or postcondition then
 *          use PRE( true ) and POST( true ) so that the invariant is still
 *          checked. See search() below for an example of this.
 *          Also, the invariant should be a non-pure virtual function
 *          (one with a body that at least returns true) so that it can be
 *          called by decendents. See invariant() below.
 *
 *          Example C++ usage:
 *
 *            // Array10.h:
 *
 *            class Array10 : public Array, public FixedSize
 *            {
 *            private:
 *              int n;
 *              double a[ 10 ];
 *            public:
 *              virtual bool invariant() const; // This is called by INV macro.
 *              bool insert( int i, double x ); // Insert x at a[ i ].
 *              int search(  double x ); // Return index of x in a[] or -1.
 *              // ...
 *            };
 *
 *            // Array10.C:
 *
 *            #include <Assertions.h>
 *            #include <Array10.h>
 *
 *            bool Array10::invariant() const
 *            {
 *              // Should be written to call each of the immediate parent's
 *              // invariant() also (no variance):
 *
 *              return AND3( Array::invariant(), FixedSize::invariant(),
 *                           IN_RANGE( n, 0, 10 ) );
 *                           // The IN_RANGE() part is this class's invariant.
 *            }
 *
 *            bool Array10::insert( int i, double x )
 *            {
 *              PRE2( 0 <= i, i <= n ) // Semicolons not required (but OK).
 *              REMEMBER( int, n ) // REMEMBER() here if using OLD() in POST().
 *
 *              // ...
 *              a[ i ] = x;
 *              // ...
 *
 *              POST2( a[ i ] == x, n == OLD( n ) + 1 ) // Also implies INV.
 *            }
 *
 *            int Array10::search( double x )
 *            {
 *              PRE( true ) // Must check the invariant.
 *
 *              size_t i;
 *
 *              for ( i = 0; i < n; ++i ) if ( a[ i ] == x ) break;
 *
 *              if ( i == n ) i = -1;
 *
 *              POST( IMPLIES( i != -1, a[ i ] == x ) )
 *            }
 *
 *            The CHECK() macro can be used to assert a condition without
 *            implying the class invariant - e.g., inside a helper routine.
 *            The DEBUG() macro is used to optionally insert any statements.
 *
 *            Examples:
 *
 *            int X::helper( int i )
 *            {
 *              CHECK( i > 0 )
 *
 *              DEBUG( cout << "X::helper( int i = " << i << " )" << endl; )
 *
 *              // ...
 *
 *              CHECK( result > 0 )
 *
 *              DEBUG( cout << "X::helper() returning " << result << endl; )
 *
 *              return result;
 *            }
 *
 * VERSION "$Id: Assertions.h 84 2018-03-12 21:26:53Z coats $"
 ********************************************************************
 *
 * HISTORY:
 *            
 *      Created 02/1994, Todd Plessel, EPA/MMTSI
 *
 *      Version 2/2018 by Carlie J. Coats, Jr. for PAVE-2.4
 *****************************************************************************/

/*=============================== INCLUDES ==================================*/

#include <assert.h> /* For the standard assert() macro. */

/*=============================== DEFINES ===================================*/

#ifdef NO_ASSERTIONS

/* Disable all checking. */

#define INV

#define PRE(c)
#define PRE2(c1,c2)
#define PRE3(c1,c2,c3)
#define PRE4(c1,c2,c3,c4)
#define PRE5(c1,c2,c3,c4,c5)
#define PRE6(c1,c2,c3,c4,c5,c6)
#define PRE7(c1,c2,c3,c4,c5,c6,c7)
#define PRE8(c1,c2,c3,c4,c5,c6,c7,c8)
#define PRE9(c1,c2,c3,c4,c5,c6,c7,c8,c9)
#define PRE10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)
#define PRE11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)
#define PRE12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)
#define PRE13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)
#define PRE14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)
#define PRE15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)
#define PRE16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16)
#define PRE17(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17)
#define PRE18(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18)

#define PRE19(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19)

#define PRE20(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19,c20)

#define POST(c)
#define POST2(c1,c2)
#define POST3(c1,c2,c3)
#define POST4(c1,c2,c3,c4)
#define POST5(c1,c2,c3,c4,c5)
#define POST6(c1,c2,c3,c4,c5,c6)
#define POST7(c1,c2,c3,c4,c5,c6,c7)
#define POST8(c1,c2,c3,c4,c5,c6,c7,c8)
#define POST9(c1,c2,c3,c4,c5,c6,c7,c8,c9)
#define POST10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)
#define POST11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)
#define POST12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)
#define POST13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)
#define POST14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)
#define POST15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)
#define POST16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16)
#define POST17(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17)
#define POST18(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18)

#define POST19(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19)

#define POST20(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19,c20)

#define OLD(variable)
#define REMEMBER(type,variable)
#define REMEMBER_F(type,function_name)

#define CHECK(c)
#define CHECK2(c1,c2)
#define CHECK3(c1,c2,c3)
#define CHECK4(c1,c2,c3,c4)
#define CHECK5(c1,c2,c3,c4,c5)
#define CHECK6(c1,c2,c3,c4,c5,c6)
#define CHECK7(c1,c2,c3,c4,c5,c6,c7)
#define CHECK8(c1,c2,c3,c4,c5,c6,c7,c8)
#define CHECK9(c1,c2,c3,c4,c5,c6,c7,c8,c9)
#define CHECK10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)
#define CHECK11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)
#define CHECK12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)
#define CHECK13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)
#define CHECK14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)
#define CHECK15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)
#define CHECK16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16)
#define CHECK17(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17)
#define CHECK18(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18)

#define CHECK19(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19)

#define CHECK20(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19,c20)

#define CHECKING(s)

#else /* NO_ASSERTIONS */

/* Enable some checking... */

#ifdef PRECONDITIONS_ONLY

/* Enable pre-condition (and invariant) checking only. */

#ifdef __cplusplus
#define INV assert((int)invariant());
#else
#define INV
#endif

#define PRE(c) INV assert((int)(c));

#define PRE2(c1,c2) INV assert((int)(c1));assert((int)(c2));

#define PRE3(c1,c2,c3) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));

#define PRE4(c1,c2,c3,c4) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));

#define PRE5(c1,c2,c3,c4,c5) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));

#define PRE6(c1,c2,c3,c4,c5,c6) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));

#define PRE7(c1,c2,c3,c4,c5,c6,c7) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));

#define PRE8(c1,c2,c3,c4,c5,c6,c7,c8) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));

#define PRE9(c1,c2,c3,c4,c5,c6,c7,c8,c9) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));

#define PRE10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));

#define PRE11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));

#define PRE12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));

#define PRE13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));

#define PRE14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));assert((int)(c14));

#define PRE15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));assert((int)(c14));assert((int)(c15));

#define PRE16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));assert((int)(c14));assert((int)(c15));assert((int)(c16));

#define PRE17(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));assert((int)(c14));assert((int)(c15));assert((int)(c16));\
assert((int)(c17));

#define PRE18(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18) \
INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));assert((int)(c14));assert((int)(c15));assert((int)(c16));\
assert((int)(c17));assert((int)(c18));

#define PRE19(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));assert((int)(c14));assert((int)(c15));assert((int)(c16));\
assert((int)(c17));assert((int)(c18));assert((int)(c19));

#define PRE20(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19,c20) INV \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));assert((int)(c14));assert((int)(c15));assert((int)(c16));\
assert((int)(c17));assert((int)(c18));assert((int)(c19));assert((int)(c20));

#define POST(c)
#define POST2(c1,c2)
#define POST3(c1,c2,c3)
#define POST4(c1,c2,c3,c4)
#define POST5(c1,c2,c3,c4,c5)
#define POST6(c1,c2,c3,c4,c5,c6)
#define POST7(c1,c2,c3,c4,c5,c6,c7)
#define POST8(c1,c2,c3,c4,c5,c6,c7,c8)
#define POST9(c1,c2,c3,c4,c5,c6,c7,c8,c9)
#define POST10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)
#define POST11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)
#define POST12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)
#define POST13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)
#define POST14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)
#define POST15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)
#define POST16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16)
#define POST17(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17)
#define POST18(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18)

#define POST19(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19)

#define POST20(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19,c20)

#define OLD(variable)
#define REMEMBER(type,variable)
#define REMEMBER_F(type,function_name)

#define CHECK(c)
#define CHECK2(c1,c2)
#define CHECK3(c1,c2,c3)
#define CHECK4(c1,c2,c3,c4)
#define CHECK5(c1,c2,c3,c4,c5)
#define CHECK6(c1,c2,c3,c4,c5,c6)
#define CHECK7(c1,c2,c3,c4,c5,c6,c7)
#define CHECK8(c1,c2,c3,c4,c5,c6,c7,c8)
#define CHECK9(c1,c2,c3,c4,c5,c6,c7,c8,c9)
#define CHECK10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10)
#define CHECK11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11)
#define CHECK12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12)
#define CHECK13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13)
#define CHECK14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14)
#define CHECK15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15)
#define CHECK16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16)
#define CHECK17(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17)
#define CHECK18(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18)

#define CHECK19(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19)

#define CHECK20(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19,c20)

#define CHECKING(s)

#else /* PRECONDITIONS_ONLY */

/* Enable all checking. */

#ifdef __cplusplus
#define INV assert((int) invariant());
#else
#define INV
#endif

#define CHECK(c) assert((int)(c));

#define CHECK2(c1,c2) assert((int)(c1));assert((int)(c2));

#define CHECK3(c1,c2,c3) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));

#define CHECK4(c1,c2,c3,c4) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));

#define CHECK5(c1,c2,c3,c4,c5) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));

#define CHECK6(c1,c2,c3,c4,c5,c6) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));

#define CHECK7(c1,c2,c3,c4,c5,c6,c7) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));

#define CHECK8(c1,c2,c3,c4,c5,c6,c7,c8) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));

#define CHECK9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));

#define CHECK10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));

#define CHECK11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));

#define CHECK12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));

#define CHECK13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));

#define CHECK14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));assert((int)(c14));

#define CHECK15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));assert((int)(c14));assert((int)(c15));

#define CHECK16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));assert((int)(c14));assert((int)(c15));assert((int)(c16));

#define CHECK17(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));assert((int)(c14));assert((int)(c15));assert((int)(c16));\
assert((int)(c17));

#define CHECK18(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));assert((int)(c14));assert((int)(c15));assert((int)(c16));\
assert((int)(c17));assert((int)(c18));

#define CHECK19(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));assert((int)(c14));assert((int)(c15));assert((int)(c16));\
assert((int)(c17));assert((int)(c18));assert((int)(c19));

#define CHECK20(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19,c20) \
assert((int)(c1));assert((int)(c2));assert((int)(c3));assert((int)(c4));\
assert((int)(c5));assert((int)(c6));assert((int)(c7));assert((int)(c8));\
assert((int)(c9));assert((int)(c10));assert((int)(c11));assert((int)(c12));\
assert((int)(c13));assert((int)(c14));assert((int)(c15));assert((int)(c16));\
assert((int)(c17));assert((int)(c18));assert((int)(c19));assert((int)(c20));


#define PRE(c) INV CHECK(c);

#define PRE2(c1,c2) INV CHECK2(c1,c2);

#define PRE3(c1,c2,c3) INV CHECK3(c1,c2,c3);

#define PRE4(c1,c2,c3,c4) INV CHECK4(c1,c2,c3,c4);

#define PRE5(c1,c2,c3,c4,c5) INV CHECK5(c1,c2,c3,c4,c5);

#define PRE6(c1,c2,c3,c4,c5,c6) INV CHECK6(c1,c2,c3,c4,c5,c6);

#define PRE7(c1,c2,c3,c4,c5,c6,c7) INV CHECK7(c1,c2,c3,c4,c5,c6,c7);

#define PRE8(c1,c2,c3,c4,c5,c6,c7,c8) INV CHECK8(c1,c2,c3,c4,c5,c6,c7,c8);

#define PRE9(c1,c2,c3,c4,c5,c6,c7,c8,c9) INV CHECK9(c1,c2,c3,c4,c5,c6,c7,c8,c9);

#define PRE10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
INV   CHECK10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10);

#define PRE11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
INV   CHECK11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11);

#define PRE12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
INV   CHECK12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12);

#define PRE13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
INV   CHECK13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13);

#define PRE14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
INV   CHECK14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14);

#define PRE15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
INV   CHECK15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15);

#define PRE16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
INV   CHECK16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16);

#define PRE17(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17) \
INV   CHECK17(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17);

#define PRE18(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18) \
INV   CHECK18(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18);

#define PRE19(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19) \
INV   CHECK19(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19);

#define PRE20(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19,c20) \
INV   CHECK20(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19,c20);



#define POST(c) INV CHECK(c);

#define POST2(c1,c2) INV CHECK2(c1,c2);

#define POST3(c1,c2,c3) INV CHECK3(c1,c2,c3);

#define POST4(c1,c2,c3,c4) INV CHECK4(c1,c2,c3,c4);

#define POST5(c1,c2,c3,c4,c5) INV CHECK5(c1,c2,c3,c4,c5);

#define POST6(c1,c2,c3,c4,c5,c6) INV CHECK6(c1,c2,c3,c4,c5,c6);

#define POST7(c1,c2,c3,c4,c5,c6,c7) INV CHECK7(c1,c2,c3,c4,c5,c6,c7);

#define POST8(c1,c2,c3,c4,c5,c6,c7,c8) INV CHECK8(c1,c2,c3,c4,c5,c6,c7,c8);

#define POST9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
INV    CHECK9(c1,c2,c3,c4,c5,c6,c7,c8,c9);

#define POST10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
INV   CHECK10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10);

#define POST11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
INV   CHECK11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11);

#define POST12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
INV   CHECK12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12);

#define POST13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13) \
INV   CHECK13(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13);

#define POST14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14) \
INV   CHECK14(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14);

#define POST15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15) \
INV   CHECK15(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15);

#define POST16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16) \
INV   CHECK16(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16);

#define POST17(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17) \
INV   CHECK17(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17);

#define POST18(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18) \
INV   CHECK18(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18);

#define POST19(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19) \
INV   CHECK19(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19);

#define POST20(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19,c20) \
INV   CHECK20(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,\
c19,c20);


#define OLD(variable) variable##_old_
#define REMEMBER(type,variable) type OLD(variable) = variable;
#define REMEMBER_F(type,function_name) type OLD(function_name)=function_name();


#define CHECKING(s) s

#endif /* PRECONDITIONS_ONLY */

#endif /* NO_ASSERTIONS */


/* Handle optional debugging statements separately. */

#ifdef DEBUGGING
#define DEBUG(s) s
#else
#define DEBUG(s)
#endif




/*
 * Other macros useful in PRE() and POST() expressions:
 * E.g., PRE3( str, str[0], IN_RANGE( i, 0, 8 ) )
 *
 * Implies: p implies c: if p is true then c must be true.
 * Example:
 * void f( size_t count, int items[] ) { PRE( IMPLIES( count, items ) ) }
 */

#define IMPLIES(p,c) (!(p)||(c))

/* Boolean functions: */

#define AND2(a,b) ((a)&&(b))
#define AND3(a,b,c) ((a)&&(b)&&(c))
#define AND4(a,b,c,d) ((a)&&(b)&&(c)&&(d))
#define AND5(a,b,c,d,e) ((a)&&(b)&&(c)&&(d)&&(e))
#define AND6(a,b,c,d,e,f) ((a)&&(b)&&(c)&&(d)&&(e)&&(f))
#define AND7(a,b,c,d,e,f,g) ((a)&&(b)&&(c)&&(d)&&(e)&&(f)&&(g))
#define AND8(a,b,c,d,e,f,g,h) ((a)&&(b)&&(c)&&(d)&&(e)&&(f)&&(g)&&(h))

#define AND9(a,b,c,d,e,f,g,h,i) \
((a)&&(b)&&(c)&&(d)&&(e)&&(f)&&(g)&&(h)&&(i))

#define AND10(a,b,c,d,e,f,g,h,i,j) \
((a)&&(b)&&(c)&&(d)&&(e)&&(f)&&(g)&&(h)&&(i)&&(j))

#define AND11(a,b,c,d,e,f,g,h,i,j,k) \
((a)&&(b)&&(c)&&(d)&&(e)&&(f)&&(g)&&(h)&&(i)&&(j)&&(k))

#define AND12(a,b,c,d,e,f,g,h,i,j,k,l) \
((a)&&(b)&&(c)&&(d)&&(e)&&(f)&&(g)&&(h)&&(i)&&(j)&&(k)&&(l))

#define AND13(a,b,c,d,e,f,g,h,i,j,k,l,m) \
((a)&&(b)&&(c)&&(d)&&(e)&&(f)&&(g)&&(h)&&(i)&&(j)&&(k)&&(l)&&(m))

#define AND14(a,b,c,d,e,f,g,h,i,j,k,l,m,n) \
((a)&&(b)&&(c)&&(d)&&(e)&&(f)&&(g)&&(h)&&(i)&&(j)&&(k)&&(l)&&(m)&&(n))

#define AND15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) \
((a)&&(b)&&(c)&&(d)&&(e)&&(f)&&(g)&&(h)&&(i)&&(j)&&(k)&&(l)&&(m)&&(n)&&(o))

#define AND16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
((a)&&(b)&&(c)&&(d)&&(e)&&(f)&&(g)&&(h)&&(i)&&(j)&&(k)&&(l)&&(m)&&(n)&&(o)&&(p))

#define AND17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) \
((a)&&(b)&&(c)&&(d)&&(e)&&(f)&&(g)&&(h)&&(i)&&(j)&&(k)&&(l)&&(m)&&(n)&&(o)&&(p)\
&&(q))

#define AND18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) \
((a)&&(b)&&(c)&&(d)&&(e)&&(f)&&(g)&&(h)&&(i)&&(j)&&(k)&&(l)&&(m)&&(n)&&(o)&&(p)\
&&(q)&&(r))

#define AND19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s) \
((a)&&(b)&&(c)&&(d)&&(e)&&(f)&&(g)&&(h)&&(i)&&(j)&&(k)&&(l)&&(m)&&(n)&&(o)&&(p)\
&&(q)&&(r)&&(s))

#define AND20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t) \
((a)&&(b)&&(c)&&(d)&&(e)&&(f)&&(g)&&(h)&&(i)&&(j)&&(k)&&(l)&&(m)&&(n)&&(o)&&(p)\
&&(q)&&(r)&&(s)&&(t))


#define OR2(a,b) ((a)||(b))
#define OR3(a,b,c) ((a)||(b)||(c))
#define OR4(a,b,c,d) ((a)||(b)||(c)||(d))
#define OR5(a,b,c,d,e) ((a)||(b)||(c)||(d)||(e))
#define OR6(a,b,c,d,e,f) ((a)||(b)||(c)||(d)||(e)||(f))
#define OR7(a,b,c,d,e,f,g) ((a)||(b)||(c)||(d)||(e)||(f)||(g))
#define OR8(a,b,c,d,e,f,g,h) ((a)||(b)||(c)||(d)||(e)||(f)||(g)||(h))

#define OR9(a,b,c,d,e,f,g,h,i) \
((a)||(b)||(c)||(d)||(e)||(f)||(g)||(h)||(i))

#define OR10(a,b,c,d,e,f,g,h,i,j) \
((a)||(b)||(c)||(d)||(e)||(f)||(g)||(h)||(i)||(j))

#define OR11(a,b,c,d,e,f,g,h,i,j,k) \
((a)||(b)||(c)||(d)||(e)||(f)||(g)||(h)||(i)||(j)||(k))

#define OR12(a,b,c,d,e,f,g,h,i,j,k,l) \
((a)||(b)||(c)||(d)||(e)||(f)||(g)||(h)||(i)||(j)||(k)||(l))

#define OR13(a,b,c,d,e,f,g,h,i,j,k,l,m) \
((a)||(b)||(c)||(d)||(e)||(f)||(g)||(h)||(i)||(j)||(k)||(l)||(m))

#define OR14(a,b,c,d,e,f,g,h,i,j,k,l,m,n) \
((a)||(b)||(c)||(d)||(e)||(f)||(g)||(h)||(i)||(j)||(k)||(l)||(m)||(n))

#define OR15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) \
((a)||(b)||(c)||(d)||(e)||(f)||(g)||(h)||(i)||(j)||(k)||(l)||(m)||(n)||(o))

#define OR16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
((a)||(b)||(c)||(d)||(e)||(f)||(g)||(h)||(i)||(j)||(k)||(l)||(m)||(n)||(o)||(p))

#define OR17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) \
((a)||(b)||(c)||(d)||(e)||(f)||(g)||(h)||(i)||(j)||(k)||(l)||(m)||(n)||(o)||(p)\
||(q))

#define OR18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) \
((a)||(b)||(c)||(d)||(e)||(f)||(g)||(h)||(i)||(j)||(k)||(l)||(m)||(n)||(o)||(p)\
||(q)||(r))

#define OR19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s) \
((a)||(b)||(c)||(d)||(e)||(f)||(g)||(h)||(i)||(j)||(k)||(l)||(m)||(n)||(o)||(p)\
||(q)||(r)||(s))

#define OR20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t) \
((a)||(b)||(c)||(d)||(e)||(f)||(g)||(h)||(i)||(j)||(k)||(l)||(m)||(n)||(o)||(p)\
||(q)||(r)||(s)||(t))


/* Numeric tests: */

/* Range membership: */

#define IN_RANGE(x,low,high) ((low)<=(x)&&(x)<=(high))

/* For C int 'flag' usage: */

#define IS_BOOL(a) ((a)==0||(a)==1)
#define IS_BOOL2(a,b) AND2(IS_BOOL(a),IS_BOOL(b))
#define IS_BOOL3(a,b,c) AND3(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c))
#define IS_BOOL4(a,b,c,d) AND4(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d))

#define IS_BOOL5(a,b,c,d,e) \
AND5(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e))

#define IS_BOOL6(a,b,c,d,e,f) \
AND6(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e),IS_BOOL(f))

#define IS_BOOL7(a,b,c,d,e,f,g) \
AND7(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e),IS_BOOL(f),\
IS_BOOL(g))

#define IS_BOOL8(a,b,c,d,e,f,g,h) \
AND8(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e),IS_BOOL(f),\
IS_BOOL(g),IS_BOOL(h))

#define IS_BOOL9(a,b,c,d,e,f,g,h,i) \
AND9(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e),IS_BOOL(f),\
IS_BOOL(g),IS_BOOL(h),IS_BOOL(i))

#define IS_BOOL10(a,b,c,d,e,f,g,h,i,j) \
AND10(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e),IS_BOOL(f),\
IS_BOOL(g),IS_BOOL(h),IS_BOOL(i),IS_BOOL(j))

#define IS_BOOL11(a,b,c,d,e,f,g,h,i,j,k) \
AND11(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e),IS_BOOL(f),\
IS_BOOL(g),IS_BOOL(h),IS_BOOL(i),IS_BOOL(j),IS_BOOL(k))

#define IS_BOOL12(a,b,c,d,e,f,g,h,i,j,k,l) \
AND12(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e),IS_BOOL(f),\
IS_BOOL(g),IS_BOOL(h),IS_BOOL(i),IS_BOOL(j),IS_BOOL(k),IS_BOOL(l))

#define IS_BOOL13(a,b,c,d,e,f,g,h,i,j,k,l,m) \
AND13(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e),IS_BOOL(f),\
IS_BOOL(g),IS_BOOL(h),IS_BOOL(i),IS_BOOL(j),IS_BOOL(k),IS_BOOL(l),IS_BOOL(m))

#define IS_BOOL14(a,b,c,d,e,f,g,h,i,j,k,l,m,n) \
AND14(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e),IS_BOOL(f),\
IS_BOOL(g),IS_BOOL(h),IS_BOOL(i),IS_BOOL(j),IS_BOOL(k),IS_BOOL(l),IS_BOOL(m),\
IS_BOOL(n))

#define IS_BOOL15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) \
AND15(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e),IS_BOOL(f),\
IS_BOOL(g),IS_BOOL(h),IS_BOOL(i),IS_BOOL(j),IS_BOOL(k),IS_BOOL(l),IS_BOOL(m),\
IS_BOOL(n),IS_BOOL(o))

#define IS_BOOL16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
AND16(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e),IS_BOOL(f),\
IS_BOOL(g),IS_BOOL(h),IS_BOOL(i),IS_BOOL(j),IS_BOOL(k),IS_BOOL(l),IS_BOOL(m),\
IS_BOOL(n),IS_BOOL(o),IS_BOOL(p))

#define IS_BOOL17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) \
AND17(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e),IS_BOOL(f),\
IS_BOOL(g),IS_BOOL(h),IS_BOOL(i),IS_BOOL(j),IS_BOOL(k),IS_BOOL(l),IS_BOOL(m),\
IS_BOOL(n),IS_BOOL(o),IS_BOOL(p),IS_BOOL(q))

#define IS_BOOL18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) \
AND18(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e),IS_BOOL(f),\
IS_BOOL(g),IS_BOOL(h),IS_BOOL(i),IS_BOOL(j),IS_BOOL(k),IS_BOOL(l),IS_BOOL(m),\
IS_BOOL(n),IS_BOOL(o),IS_BOOL(p),IS_BOOL(q),IS_BOOL(r))

#define IS_BOOL19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s) \
AND19(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e),IS_BOOL(f),\
IS_BOOL(g),IS_BOOL(h),IS_BOOL(i),IS_BOOL(j),IS_BOOL(k),IS_BOOL(l),IS_BOOL(m),\
IS_BOOL(n),IS_BOOL(o),IS_BOOL(p),IS_BOOL(q),IS_BOOL(r),IS_BOOL(s))

#define IS_BOOL20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t) \
AND20(IS_BOOL(a),IS_BOOL(b),IS_BOOL(c),IS_BOOL(d),IS_BOOL(e),IS_BOOL(f),\
IS_BOOL(g),IS_BOOL(h),IS_BOOL(i),IS_BOOL(j),IS_BOOL(k),IS_BOOL(l),IS_BOOL(m),\
IS_BOOL(n),IS_BOOL(o),IS_BOOL(p),IS_BOOL(q),IS_BOOL(r),IS_BOOL(s),IS_BOOL(t))


#define NON_ZERO2(a,b) AND2(a,b)
#define NON_ZERO3(a,b,c) AND3(a,b,c)
#define NON_ZERO4(a,b,c,d) AND4(a,b,c,d)
#define NON_ZERO5(a,b,c,d,e) AND5(a,b,c,d,e)
#define NON_ZERO6(a,b,c,d,e,f) AND6(a,b,c,d,e,f)
#define NON_ZERO7(a,b,c,d,e,f,g) AND7(a,b,c,d,e,f,g)
#define NON_ZERO8(a,b,c,d,e,f,g,h) AND8(a,b,c,d,e,f,g,h)
#define NON_ZERO9(a,b,c,d,e,f,g,h,i) AND9(a,b,c,d,e,f,g,h,i)
#define NON_ZERO10(a,b,c,d,e,f,g,h,i,j) AND10(a,b,c,d,e,f,g,h,i,j)
#define NON_ZERO11(a,b,c,d,e,f,g,h,i,j,k) AND11(a,b,c,d,e,f,g,h,i,j,k)
#define NON_ZERO12(a,b,c,d,e,f,g,h,i,j,k,l) AND12(a,b,c,d,e,f,g,h,i,j,k,l)
#define NON_ZERO13(a,b,c,d,e,f,g,h,i,j,k,l,m) AND13(a,b,c,d,e,f,g,h,i,j,k,l,m)

#define NON_ZERO14(a,b,c,d,e,f,g,h,i,j,k,l,m,n) \
AND14(a,b,c,d,e,f,g,h,i,j,k,l,m,n)

#define NON_ZERO15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) \
AND15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)

#define NON_ZERO16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
AND16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)

#define NON_ZERO17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) \
AND17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q)

#define NON_ZERO18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) \
AND18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r)

#define NON_ZERO19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s) \
AND19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s)

#define NON_ZERO20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t) \
AND20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t)


#define IS_ZERO2(a,b) ((a)==0&&(b)==0)
#define IS_ZERO3(a,b,c) ((a)==0&&(b)==0&&(c)==0)
#define IS_ZERO4(a,b,c,d) ((a)==0&&(b)==0&&(c)==0&&(d)==0)
#define IS_ZERO5(a,b,c,d,e) ((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0)
#define IS_ZERO6(a,b,c,d,e,f) ((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0&&(f)==0)

#define IS_ZERO7(a,b,c,d,e,f,g) \
((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0&&(f)==0&&(g)==0)

#define IS_ZERO8(a,b,c,d,e,f,g,h) \
((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0&&(f)==0&&(g)==0&&(h)==0)

#define IS_ZERO9(a,b,c,d,e,f,g,h,i) \
((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0&&(f)==0&&(g)==0&&(h)==0&&(i)==0)

#define IS_ZERO10(a,b,c,d,e,f,g,h,i,j) \
((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0&&(f)==0&&(g)==0&&(h)==0&&(i)==0&&(j)==0)

#define IS_ZERO11(a,b,c,d,e,f,g,h,i,j,k) \
((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0&&(f)==0&&(g)==0&&(h)==0&&(i)==0&&\
(j)==0&&(k)==0)

#define IS_ZERO12(a,b,c,d,e,f,g,h,i,j,k,l) \
((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0&&(f)==0&&(g)==0&&(h)==0&&(i)==0&&\
(j)==0&&(k)==0&&(l)==0)

#define IS_ZERO13(a,b,c,d,e,f,g,h,i,j,k,l,m) \
((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0&&(f)==0&&(g)==0&&(h)==0&&(i)==0&&\
(j)==0&&(k)==0&&(l)==0&&(m)==0)

#define IS_ZERO14(a,b,c,d,e,f,g,h,i,j,k,l,m,n) \
((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0&&(f)==0&&(g)==0&&(h)==0&&(i)==0&&\
(j)==0&&(k)==0&&(l)==0&&(m)==0&&(n)==0)

#define IS_ZERO15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) \
((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0&&(f)==0&&(g)==0&&(h)==0&&(i)==0&&\
(j)==0&&(k)==0&&(l)==0&&(m)==0&&(n)==0&&(o)==0)

#define IS_ZERO16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0&&(f)==0&&(g)==0&&(h)==0&&(i)==0&&\
(j)==0&&(k)==0&&(l)==0&&(m)==0&&(n)==0&&(o)==0&&(p)==0)

#define IS_ZERO17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) \
((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0&&(f)==0&&(g)==0&&(h)==0&&(i)==0&&\
(j)==0&&(k)==0&&(l)==0&&(m)==0&&(n)==0&&(o)==0&&(p)==0&&(q)==0)

#define IS_ZERO18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) \
((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0&&(f)==0&&(g)==0&&(h)==0&&(i)==0&&\
(j)==0&&(k)==0&&(l)==0&&(m)==0&&(n)==0&&(o)==0&&(p)==0&&(q)==0&&(r)==0)

#define IS_ZERO19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s) \
((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0&&(f)==0&&(g)==0&&(h)==0&&(i)==0&&\
(j)==0&&(k)==0&&(l)==0&&(m)==0&&(n)==0&&(o)==0&&(p)==0&&(q)==0&&(r)==0&&\
(s)==0)

#define IS_ZERO20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t) \
((a)==0&&(b)==0&&(c)==0&&(d)==0&&(e)==0&&(f)==0&&(g)==0&&(h)==0&&(i)==0&&\
(j)==0&&(k)==0&&(l)==0&&(m)==0&&(n)==0&&(o)==0&&(p)==0&&(q)==0&&(r)==0&&\
(s)==0&&(t)==0)

#define GT_ZERO2(a,b) ((a)>0&&(b)>0)
#define GT_ZERO3(a,b,c) ((a)>0&&(b)>0&&(c)>0)
#define GT_ZERO4(a,b,c,d) ((a)>0&&(b)>0&&(c)>0&&(d)>0)
#define GT_ZERO5(a,b,c,d,e) ((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0)
#define GT_ZERO6(a,b,c,d,e,f) ((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0&&(f)>0)

#define GT_ZERO7(a,b,c,d,e,f,g) \
((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0&&(f)>0&&(g)>0)

#define GT_ZERO8(a,b,c,d,e,f,g,h) \
((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0&&(f)>0&&(g)>0&&(h)>0)

#define GT_ZERO9(a,b,c,d,e,f,g,h,i) \
((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0&&(f)>0&&(g)>0&&(h)>0&&(i)>0)

#define GT_ZERO10(a,b,c,d,e,f,g,h,i,j) \
((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0&&(f)>0&&(g)>0&&(h)>0&&(i)>0&&(j)>0)

#define GT_ZERO11(a,b,c,d,e,f,g,h,i,j,k) \
((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0&&(f)>0&&(g)>0&&(h)>0&&(i)>0&&(j)>0&&\
(k)>0)

#define GT_ZERO12(a,b,c,d,e,f,g,h,i,j,k,l) \
((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0&&(f)>0&&(g)>0&&(h)>0&&(i)>0&&(j)>0&&\
(k)>0&&(l)>0)

#define GT_ZERO13(a,b,c,d,e,f,g,h,i,j,k,l,m) \
((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0&&(f)>0&&(g)>0&&(h)>0&&(i)>0&&(j)>0&&\
(k)>0&&(l)>0&&(m)>0)

#define GT_ZERO14(a,b,c,d,e,f,g,h,i,j,k,l,m,n) \
((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0&&(f)>0&&(g)>0&&(h)>0&&(i)>0&&(j)>0&&\
(k)>0&&(l)>0&&(m)>0&&(n)>0)

#define GT_ZERO15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) \
((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0&&(f)>0&&(g)>0&&(h)>0&&(i)>0&&(j)>0&&\
(k)>0&&(l)>0&&(m)>0&&(n)>0&&(o)>0)

#define GT_ZERO16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0&&(f)>0&&(g)>0&&(h)>0&&(i)>0&&(j)>0&&\
(k)>0&&(l)>0&&(m)>0&&(n)>0&&(o)>0&&(p)>0)

#define GT_ZERO17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) \
((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0&&(f)>0&&(g)>0&&(h)>0&&(i)>0&&(j)>0&&\
(k)>0&&(l)>0&&(m)>0&&(n)>0&&(o)>0&&(p)>0&&(q)>0)

#define GT_ZERO18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) \
((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0&&(f)>0&&(g)>0&&(h)>0&&(i)>0&&(j)>0&&\
(k)>0&&(l)>0&&(m)>0&&(n)>0&&(o)>0&&(p)>0&&(q)>0&&(r)>0)

#define GT_ZERO19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s) \
((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0&&(f)>0&&(g)>0&&(h)>0&&(i)>0&&(j)>0&&\
(k)>0&&(l)>0&&(m)>0&&(n)>0&&(o)>0&&(p)>0&&(q)>0&&(r)>0&&(s)>0)

#define GT_ZERO20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t) \
((a)>0&&(b)>0&&(c)>0&&(d)>0&&(e)>0&&(f)>0&&(g)>0&&(h)>0&&(i)>0&&(j)>0&&\
(k)>0&&(l)>0&&(m)>0&&(n)>0&&(o)>0&&(p)>0&&(q)>0&&(r)>0&&(s)>0&&(t)>0)


#define GE_ZERO2(a,b) ((a)>=0&&(b)>=0)
#define GE_ZERO3(a,b,c) ((a)>=0&&(b)>=0&&(c)>=0)
#define GE_ZERO4(a,b,c,d) ((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0)
#define GE_ZERO5(a,b,c,d,e) ((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0)
#define GE_ZERO6(a,b,c,d,e,f) ((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0&&(f)>=0)

#define GE_ZERO7(a,b,c,d,e,f,g) \
((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0&&(f)>=0&&(g)>=0)

#define GE_ZERO8(a,b,c,d,e,f,g,h) \
((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0&&(f)>=0&&(g)>=0&&(h)>=0)

#define GE_ZERO9(a,b,c,d,e,f,g,h,i) \
((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0&&(f)>=0&&(g)>=0&&(h)>=0&&(i)>=0)

#define GE_ZERO10(a,b,c,d,e,f,g,h,i,j) \
((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0&&(f)>=0&&(g)>=0&&(h)>=0&&(i)>=0&&(j)>=0)

#define GE_ZERO11(a,b,c,d,e,f,g,h,i,j,k) \
((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0&&(f)>=0&&(g)>=0&&(h)>=0&&(i)>=0&&\
(j)>=0&&(k)>=0)

#define GE_ZERO12(a,b,c,d,e,f,g,h,i,j,k,l) \
((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0&&(f)>=0&&(g)>=0&&(h)>=0&&(i)>=0&&\
(j)>=0&&(k)>=0&&(l)>=0)

#define GE_ZERO13(a,b,c,d,e,f,g,h,i,j,k,l,m) \
((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0&&(f)>=0&&(g)>=0&&(h)>=0&&(i)>=0&&\
(j)>=0&&(k)>=0&&(l)>=0&&(m)>=0)

#define GE_ZERO14(a,b,c,d,e,f,g,h,i,j,k,l,m,n) \
((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0&&(f)>=0&&(g)>=0&&(h)>=0&&(i)>=0&&\
(j)>=0&&(k)>=0&&(l)>=0&&(m)>=0&&(n)>=0)

#define GE_ZERO15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) \
((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0&&(f)>=0&&(g)>=0&&(h)>=0&&(i)>=0&&\
(j)>=0&&(k)>=0&&(l)>=0&&(m)>=0&&(n)>=0&&(o)>=0)

#define GE_ZERO16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0&&(f)>=0&&(g)>=0&&(h)>=0&&(i)>=0&&\
(j)>=0&&(k)>=0&&(l)>=0&&(m)>=0&&(n)>=0&&(o)>=0&&(p)>=0)

#define GE_ZERO17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) \
((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0&&(f)>=0&&(g)>=0&&(h)>=0&&(i)>=0&&\
(j)>=0&&(k)>=0&&(l)>=0&&(m)>=0&&(n)>=0&&(o)>=0&&(p)>=0&&(q)>=0)

#define GE_ZERO18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) \
((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0&&(f)>=0&&(g)>=0&&(h)>=0&&(i)>=0&&\
(j)>=0&&(k)>=0&&(l)>=0&&(m)>=0&&(n)>=0&&(o)>=0&&(p)>=0&&(q)>=0&&(r)>=0)

#define GE_ZERO19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s) \
((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0&&(f)>=0&&(g)>=0&&(h)>=0&&(i)>=0&&\
(j)>=0&&(k)>=0&&(l)>=0&&(m)>=0&&(n)>=0&&(o)>=0&&(p)>=0&&(q)>=0&&(r)>=0&&(s)>=0)

#define GE_ZERO20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t) \
((a)>=0&&(b)>=0&&(c)>=0&&(d)>=0&&(e)>=0&&(f)>=0&&(g)>=0&&(h)>=0&&(i)>=0&&\
(j)>=0&&(k)>=0&&(l)>=0&&(m)>=0&&(n)>=0&&(o)>=0&&(p)>=0&&(q)>=0&&(r)>=0&&\
(s)>=0&&(t)>=0)



/* Set membership. */

#define IN3(x,a,b ) ((x)==(a)||(x)==(b))
#define IN4(x,a,b,c ) ((x)==(a)||(x)==(b)||(x)==(c))
#define IN5(x,a,b,c,d ) ((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d))

#define IN6(x,a,b,c,d,e) \
((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d)||(x)==(e))

#define IN7(x,a,b,c,d,e,f) \
((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d)||(x)==(e)||(x)==(f))

#define IN8(x,a,b,c,d,e,f,g) \
((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d)||(x)==(e)||(x)==(f)||(x)==(g))

#define IN9(x,a,b,c,d,e,f,g,h) \
((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d)||(x)==(e)||(x)==(f)||(x)==(g)||(x)==(h))

#define IN10(x,a,b,c,d,e,f,g,h,i) \
((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d)||(x)==(e)||(x)==(f)||(x)==(g)||(x)==(h)\
||(x)==(i))

#define IN11(x,a,b,c,d,e,f,g,h,i,j) \
((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d)||(x)==(e)||(x)==(f)||(x)==(g)||(x)==(h)\
||(x)==(i)||(x)==(j))

#define IN12(x,a,b,c,d,e,f,g,h,i,j,k) \
((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d)||(x)==(e)||(x)==(f)||(x)==(g)||(x)==(h)\
||(x)==(i)||(x)==(j)||(x)==(k))

#define IN13(x,a,b,c,d,e,f,g,h,i,j,k,l) \
((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d)||(x)==(e)||(x)==(f)||(x)==(g)||(x)==(h)\
||(x)==(i)||(x)==(j)||(x)==(k)||(x)==(l))

#define IN14(x,a,b,c,d,e,f,g,h,i,j,k,l,m) \
((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d)||(x)==(e)||(x)==(f)||(x)==(g)||(x)==(h)\
||(x)==(i)||(x)==(j)||(x)==(k)||(x)==(l)||(x)==(m))

#define IN15(x,a,b,c,d,e,f,g,h,i,j,k,l,m,n) \
((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d)||(x)==(e)||(x)==(f)||(x)==(g)||(x)==(h)\
||(x)==(i)||(x)==(j)||(x)==(k)||(x)==(l)||(x)==(m)||(x)==(n))

#define IN16(x,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) \
((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d)||(x)==(e)||(x)==(f)||(x)==(g)||(x)==(h)\
||(x)==(i)||(x)==(j)||(x)==(k)||(x)==(l)||(x)==(m)||(x)==(n)||(x)==(o))

#define IN17(x,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d)||(x)==(e)||(x)==(f)||(x)==(g)||(x)==(h)\
||(x)==(i)||(x)==(j)||(x)==(k)||(x)==(l)||(x)==(m)||(x)==(n)||(x)==(o)\
||(x)==(p))

#define IN18(x,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) \
((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d)||(x)==(e)||(x)==(f)||(x)==(g)||(x)==(h)\
||(x)==(i)||(x)==(j)||(x)==(k)||(x)==(l)||(x)==(m)||(x)==(n)||(x)==(o)\
||(x)==(p)||(x)==(q))

#define IN19(x,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) \
((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d)||(x)==(e)||(x)==(f)||(x)==(g)||(x)==(h)\
||(x)==(i)||(x)==(j)||(x)==(k)||(x)==(l)||(x)==(m)||(x)==(n)||(x)==(o)\
||(x)==(p)||(x)==(q)||(x)==(r))

#define IN20(x,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s) \
((x)==(a)||(x)==(b)||(x)==(c)||(x)==(d)||(x)==(e)||(x)==(f)||(x)==(g)||(x)==(h)\
||(x)==(i)||(x)==(j)||(x)==(k)||(x)==(l)||(x)==(m)||(x)==(n)||(x)==(o)\
||(x)==(p)||(x)==(q)||(x)==(r)||(x)==(s))


/* NON_ZERO_COUNT*() evaluates to the number of arguments that are non-zero: */

#define NON_ZERO_COUNT2(a,b) (((a)!=0)+((b)!=0))
#define NON_ZERO_COUNT3(a,b,c) (((a)!=0)+((b)!=0)+((c)!=0))
#define NON_ZERO_COUNT4(a,b,c,d) (((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0))

#define NON_ZERO_COUNT5(a,b,c,d,e) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0))

#define NON_ZERO_COUNT6(a,b,c,d,e,f) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0)+((f)!=0))

#define NON_ZERO_COUNT7(a,b,c,d,e,f,g) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0)+((f)!=0)+((g)!=0))

#define NON_ZERO_COUNT8(a,b,c,d,e,f,g,h) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0)+((f)!=0)+((g)!=0)+((h)!=0))

#define NON_ZERO_COUNT9(a,b,c,d,e,f,g,h,i) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0)+((f)!=0)+((g)!=0)+((h)!=0)\
+((i)!=0))

#define NON_ZERO_COUNT10(a,b,c,d,e,f,g,h,i,j) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0)+((f)!=0)+((g)!=0)+((h)!=0)\
+((i)!=0)+((j)!=0))

#define NON_ZERO_COUNT11(a,b,c,d,e,f,g,h,i,j,k) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0)+((f)!=0)+((g)!=0)+((h)!=0)\
+((i)!=0)+((j)!=0)+((k)!=0))

#define NON_ZERO_COUNT12(a,b,c,d,e,f,g,h,i,j,k,l) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0)+((f)!=0)+((g)!=0)+((h)!=0)\
+((i)!=0)+((j)!=0)+((k)!=0)+((l)!=0))

#define NON_ZERO_COUNT13(a,b,c,d,e,f,g,h,i,j,k,l,m) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0)+((f)!=0)+((g)!=0)+((h)!=0)\
+((i)!=0)+((j)!=0)+((k)!=0)+((l)!=0)+((m)!=0))

#define NON_ZERO_COUNT14(a,b,c,d,e,f,g,h,i,j,k,l,m,n) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0)+((f)!=0)+((g)!=0)+((h)!=0)\
+((i)!=0)+((j)!=0)+((k)!=0)+((l)!=0)+((m)!=0)+((n)!=0))

#define NON_ZERO_COUNT15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0)+((f)!=0)+((g)!=0)+((h)!=0)\
+((i)!=0)+((j)!=0)+((k)!=0)+((l)!=0)+((m)!=0)+((n)!=0)+((o)!=0))

#define NON_ZERO_COUNT16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0)+((f)!=0)+((g)!=0)+((h)!=0)\
+((i)!=0)+((j)!=0)+((k)!=0)+((l)!=0)+((m)!=0)+((n)!=0)+((o)!=0)+((p)!=0))

#define NON_ZERO_COUNT17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0)+((f)!=0)+((g)!=0)+((h)!=0)\
+((i)!=0)+((j)!=0)+((k)!=0)+((l)!=0)+((m)!=0)+((n)!=0)+((o)!=0)+((p)!=0)\
+((q)!=0))

#define NON_ZERO_COUNT18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0)+((f)!=0)+((g)!=0)+((h)!=0)\
+((i)!=0)+((j)!=0)+((k)!=0)+((l)!=0)+((m)!=0)+((n)!=0)+((o)!=0)+((p)!=0)\
+((q)!=0)+((r)!=0))

#define NON_ZERO_COUNT19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0)+((f)!=0)+((g)!=0)+((h)!=0)\
+((i)!=0)+((j)!=0)+((k)!=0)+((l)!=0)+((m)!=0)+((n)!=0)+((o)!=0)+((p)!=0)\
+((q)!=0)+((r)!=0)+((s)!=0))

#define NON_ZERO_COUNT20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t) \
(((a)!=0)+((b)!=0)+((c)!=0)+((d)!=0)+((e)!=0)+((f)!=0)+((g)!=0)+((h)!=0)\
+((i)!=0)+((j)!=0)+((k)!=0)+((l)!=0)+((m)!=0)+((n)!=0)+((o)!=0)+((p)!=0)\
+((q)!=0)+((r)!=0)+((s)!=0)+((t)!=0))


/* Exclusive-or does not do short-circuited boolean evaluation: */

#define XOR2(a,b) (NON_ZERO_COUNT2(a,b)==1)
#define XOR3(a,b,c) (NON_ZERO_COUNT3(a,b,c)==1)
#define XOR4(a,b,c,d) (NON_ZERO_COUNT4(a,b,c,d)==1)
#define XOR5(a,b,c,d,e) (NON_ZERO_COUNT5(a,b,c,d,e)==1)
#define XOR6(a,b,c,d,e,f) (NON_ZERO_COUNT6(a,b,c,d,e,f)==1)
#define XOR7(a,b,c,d,e,f,g) (NON_ZERO_COUNT7(a,b,c,d,e,f,g)==1)
#define XOR8(a,b,c,d,e,f,g,h) (NON_ZERO_COUNT8(a,b,c,d,e,f,g,h)==1)
#define XOR9(a,b,c,d,e,f,g,h,i) (NON_ZERO_COUNT9(a,b,c,d,e,f,g,h,i)==1)
#define XOR10(a,b,c,d,e,f,g,h,i,j) (NON_ZERO_COUNT10(a,b,c,d,e,f,g,h,i,j)==1)

#define XOR11(a,b,c,d,e,f,g,h,i,j,k) \
(NON_ZERO_COUNT11(a,b,c,d,e,f,g,h,i,j,k)==1)

#define XOR12(a,b,c,d,e,f,g,h,i,j,k,l) \
(NON_ZERO_COUNT12(a,b,c,d,e,f,g,h,i,j,k,l)==1)

#define XOR13(a,b,c,d,e,f,g,h,i,j,k,l,m) \
(NON_ZERO_COUNT13(a,b,c,d,e,f,g,h,i,j,k,l,m)==1)

#define XOR14(a,b,c,d,e,f,g,h,i,j,k,l,m,n) \
(NON_ZERO_COUNT14(a,b,c,d,e,f,g,h,i,j,k,l,m,n)==1)

#define XOR15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) \
(NON_ZERO_COUNT15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)==1)

#define XOR16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
(NON_ZERO_COUNT16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)==1)

#define XOR17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) \
(NON_ZERO_COUNT17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q)==1)

#define XOR18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) \
(NON_ZERO_COUNT18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r)==1)

#define XOR19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s) \
(NON_ZERO_COUNT19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s)==1)

#define XOR20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t) \
(NON_ZERO_COUNT20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t)==1)


#endif /* ASSERTIONS_H */

