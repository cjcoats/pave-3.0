/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: LinkedList.cc 85 2018-03-13 13:17:36Z coats $
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
 ****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
//
// LinkedList.C
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 23, 1995
//
/////////////////////////////////////////////////////////////
//
// LinkedList Class
//
// Portions from Roger Sessions' "Class Construction in C
// and C++", copyright 1992 by Prentice-Hall
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT  950523  Implemented
//
/////////////////////////////////////////////////////////////


#include "LinkedList.h"


linkedList::linkedList()
    {
    currentLink = headLink = tailLink = 0;
    nlinks = 0;
    max = MAX_INT;
    }


linkedList::~linkedList()
    {
    // SRT replace with freeContents()    while(removeHead());
    freeContents();
    }


void linkedList::freeContents()
    {
    baseType *thisItem;
    while ( thisItem = removeHead() ) delete thisItem;
    }


baseType *linkedList::head()
    {
    if ( !nlinks ) return 0;
    currentLink = headLink;
    return ( ( linkp * ) currentLink )->contents;
    }


baseType *linkedList::tail()
    {
    if ( !nlinks ) return 0;
    currentLink = tailLink;
    return ( ( linkp * ) currentLink )->contents;
    }

int linkedList::length()
    {
    return nlinks;
    }


void linkedList::setMax ( int newMax )
    {
    max = newMax;
    }


int linkedList::left()
    {
    return max - length();
    }


baseType *linkedList::next()
    {
    if ( !nlinks ) return 0;
    if ( ( ( linkp * ) currentLink )->next )
        {
        currentLink = ( ( linkp * ) currentLink )->next;
        return ( ( linkp * ) currentLink )->contents;
        }
    else
        return 0;
    }


baseType *linkedList::previous()
    {
    if ( !nlinks ) return 0;
    if ( ( ( linkp * ) currentLink )->previous )
        {
        currentLink = ( ( linkp * ) currentLink )->previous;
        return ( ( linkp * ) currentLink )->contents;
        }
    else
        return 0;
    }


baseType *linkedList::retrieve()
    {
    if ( !nlinks ) return 0;
    return ( ( linkp * ) currentLink )->contents;
    }


baseType *linkedList::replace ( baseType *newElement )
    {
    if ( !nlinks ) return 0;
    ( ( linkp * ) currentLink )->contents = newElement;
    return ( ( linkp * ) currentLink )->contents;
    }


baseType *linkedList::promoteTail()
    {
    linkp *oldTail;

    if ( !nlinks ) return 0;
    if ( nlinks == 1 ) return ( ( linkp * ) headLink )->contents;
    oldTail = ( linkp * ) tailLink;
    tailLink = ( ( linkp * ) tailLink )->previous;
    ( ( linkp * ) oldTail->previous )->next = 0;
    oldTail->previous  = 0;
    oldTail->next = headLink;
    ( ( linkp * ) headLink )->previous = oldTail;
    headLink = oldTail;
    currentLink = headLink;
    return ( ( linkp * ) currentLink )->contents; // added SRT
    }


baseType *linkedList::addHead ( baseType *newElement )
    {
    linkp *newLink = new linkp ( newElement );
    assert ( newLink );

    if ( !left() )
        {
        head();
        return ( replace ( newElement ) );
        }

    if ( head() )
        ( ( linkp * ) currentLink )->previous = newLink;
    else
        tailLink = newLink;

    newLink->next = currentLink;
    headLink = currentLink = newLink;
    nlinks++;

    return ( ( linkp * ) currentLink )->contents;
    }


baseType *linkedList::addTail ( baseType *newElement )
    {
    linkp *newLink = new linkp ( newElement );
    assert ( newLink );

    if ( !left() )
        {
        tail();
        return ( replace ( newElement ) );
        }

    if ( tail() )
        ( ( linkp * ) currentLink )->next = newLink;
    else
        headLink = newLink;

    newLink->previous = currentLink;
    tailLink = currentLink = newLink;
    nlinks++;

    return ( ( linkp * ) currentLink )->contents;
    }


baseType *linkedList::findAndRemoveLink ( void *findme ) // SRT DOES THIS WORK?
    {
    baseType *nextElement;
    head();
    for ( ;; )
        {
        nextElement = retrieve();
        if ( !nextElement ) return ( baseType * ) 0;
        if ( nextElement->match ( findme ) )
            {
            if ( nlinks == 1 )
                {
                delete headLink;
                headLink = tailLink = currentLink = 0;
                }
            if ( nlinks > 1 )
                {
                if ( ( ( linkp * ) currentLink )->previous )
                    ( ( linkp * ) ( ( linkp * ) currentLink )->previous )->next =
                        ( ( linkp * ) currentLink )->next;
                else
                    headLink = ( ( linkp * ) currentLink )->next;

                if ( ( ( linkp * ) currentLink )->next )
                    ( ( linkp * ) ( ( linkp * ) currentLink )->next )->previous =
                        ( ( linkp * ) currentLink )->previous;
                else
                    tailLink = ( ( linkp * ) currentLink )->previous;
                delete ( ( linkp * ) currentLink );
                head();
                }
            nlinks--;

#ifdef DIAGNOSTICS
            fprintf ( stderr, "in findAndRemoveLink:\n" );
            nextElement->print ( stderr );
#endif // DIAGNOSTICS

            return nextElement; // match
            }
        if ( isTail() ) return ( baseType * ) 0;            // no match
        next();
        }
    }





baseType *linkedList::removeHead()
    {
    baseType *thisItem;

    if ( !nlinks ) return 0;
    thisItem = head();
    if ( nlinks == 1 )
        {
        delete headLink;
        headLink = tailLink = currentLink = 0;
        }
    if ( nlinks > 1 )
        {
        next();
        delete headLink;
        headLink = currentLink;
        ( ( linkp * ) headLink )->previous = 0;
        }
    nlinks--;
    return thisItem;
    }


int linkedList::isTail()
    {
    return ( currentLink == tailLink );
    }


void linkedList::print ( FILE *output )
    {
    baseType *thisItem;

    thisItem = head();
    if ( thisItem )
        {
        while ( thisItem )
            {
            thisItem->print ( output );
            thisItem = next();
            }
        }
    }



baseType *linkedList::find ( void *findme )
    {
    baseType *nextElement;
    head();
    for ( ;; )
        {
        nextElement = retrieve();
        if ( !nextElement ) return ( baseType * ) 0;
        if ( nextElement->match ( findme ) ) return nextElement; // match
        if ( isTail() ) return ( baseType * ) 0;            // no match
        next();
        }
    }


// override baseType's method
char *linkedList::getClassName()
    {
    static char *myName = "linkedList";
    return myName;
    }
