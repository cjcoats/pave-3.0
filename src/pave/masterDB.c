/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: masterDB.c 83 2018-03-12 19:24:33Z coats $
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
 *
 * ABOUT:  masterDB.c
 *
 *     Routines and structures that handle bus master data-base calls.
 *     Will have to be updated and/or replaced once EDSS data-base is
 *     defined.
 *
 * KNOWN BUGS:  :-(
 *
 * OTHER NOTES: :-)
 *
 ****************************************************************************
 *  REVISION HISTORY - masterDB.c
 *
 * Date: 14-Dec-94
 * Version: 0.1
 * Change Description: ORIGINAL CODE
 * Change author: Leland Morrison, NCSU, CSC
 *
 * Date: 15-Dec-94
 * Version: 0.2
 * Change Description: Added some comments and main headers
 * Change author: M. Vouk, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

char *vcmasterDB= "$Id: masterDB.c 83 2018-03-12 19:24:33Z coats $" ;


#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "busMaster.h"
#include "masterDB.h"
#include "busError.h"

struct BusTypeNode *
BusMasterAddType ( struct BusTypeNode *last,
                   const char *name,
                   int id )
    {
    struct BusTypeNode *newNode;

    newNode = ( struct BusTypeNode * ) malloc ( sizeof ( struct BusTypeNode ) );
    if ( newNode == NULL )
        return NULL;

    newNode->typeId = id;

    newNode->typeName = ( char * ) malloc ( strlen ( name ) + 1 );
    if ( newNode->typeName == NULL )
        {
        free ( newNode );
        return NULL;
        }
    strcpy ( newNode->typeName, name );

    if ( last!=NULL )
        {
        newNode->next = last->next;
        last->next    = newNode;
        }
    else
        newNode->next = newNode;
    return newNode;
    }

struct BusTypeNode *
BusMasterFindTypeById ( struct BusTypeNode *last,
                        int id )
    {
    struct BusTypeNode *current;

    current = last->next;
    while ( ( current != last ) && ( current->typeId != id ) )
        current = current->next;

    if ( current->typeId == id )
        return current;
    else
        return NULL;
    }

struct BusTypeNode *
BusMasterFindTypeByName ( struct BusTypeNode *last,
                          const char *name )
    {
    struct BusTypeNode *current;

    current=last->next;
    while ( ( current!=last ) && strcmp ( name, current->typeName ) )
        {
        current = current->next;
        }

    if ( strcmp ( name,current->typeName ) == 0 )
        {
        return current;
        }
    else
        return NULL;
    }


struct BusModuleNode *
BusMasterAddModule ( struct BusModuleNode *last,
                     const char *name,
                     int id,
                     int fd )
    {
    struct BusModuleNode *newModule;

    newModule = ( struct BusModuleNode * )
                malloc ( sizeof ( struct BusModuleNode ) );

    if ( newModule == NULL )
        return NULL;
    else
        {
        newModule->moduleName = ( char * ) malloc ( strlen ( name )+1 );
        if ( newModule->moduleName == NULL )
            {
            free ( newModule );
            return NULL;
            }
        strcpy ( newModule->moduleName,name );

        if ( last )
            newModule->next = last->next;
        else
            newModule->next = newModule;

        if ( last )
            last->next = newModule;


        newModule->moduleId = id;
        newModule->fd = fd;

        newModule->Types = NULL;
        newModule->Messages = NULL;

        return newModule;
        }
    }

struct BusModuleNode *
BusMasterFindModuleByName ( struct BusModuleNode *last,
                            const char *name )
    {
    struct BusModuleNode *current;

    if ( last )
        current = last->next;
    else
        return NULL;

    while ( current && current!=last && strcmp ( current->moduleName,name ) )
        current = current->next;

    if ( strcmp ( current->moduleName,name ) )
        return NULL;
    else
        return current;
    }

struct BusModuleNode *
BusMasterFindModuleById  ( struct BusModuleNode *last,
                           int id )
    {
    struct BusModuleNode *current;

    if ( last )
        current = last->next;
    else
        return NULL;

    while ( current && current!=last && ( current->moduleId != id ) )
        current = current->next;

    if ( current->moduleId != id )
        return NULL;
    else
        return current;
    }

struct BusModuleNode *
BusMasterFindModuleByFd ( struct BusModuleNode *last,
                          int fd )
    {
    struct BusModuleNode *cur;

    if ( last == NULL )
        return NULL;
    else
        cur=last->next;

    while ( cur && ( cur != last ) && ( fd != cur -> fd ) )
        cur = cur->next;

    if ( fd == cur->fd )
        return cur;
    else
        return NULL;
    }

int BusMasterAddRegisteredType ( struct BusModuleNode *bmn,
                                 int typeId )
    {
    struct BusRegisteredTypeNode *newNode;

    newNode = ( struct BusRegisteredTypeNode * )
              malloc ( sizeof ( struct BusRegisteredTypeNode ) );

    if ( newNode != NULL )
        {
        newNode->next = bmn->Types;
        newNode->typeId = typeId;

        bmn->Types = newNode;

        return SBUSERROR_NOT;
        }
    else
        return SBUSERROR_NOMEMORY;
    }

int BusMasterCheckRegisteredTypes ( struct BusModuleNode *bmn,
                                    int typeId )
/* is a given type understood by a module? */
/* returns 1 if the type is understood, 0 if the type is not */
    {
    struct BusRegisteredTypeNode *current;

    if ( bmn == NULL )
        return 0;
    if ( bmn->Types == NULL )
        return 0;

    /*printf("Checking to see if module %s understands type %d.\n",
     bmn->moduleName, typeId);*/

    if ( bmn->Types->typeId == typeId )
        return 1;

    /*printf("Not type %d\n",bmn->Types->typeId);*/

    current = bmn->Types;

    do
        {
        current = current->next;
        }
    while ( current && ( current != bmn->Types ) &&
            ( current->typeId != typeId ) );

    /* we have either found the type, or reached the end of the understood
       types list */

    if ( current && ( current->typeId == typeId ) )
        return 1;
    else
        return 0;
    }

