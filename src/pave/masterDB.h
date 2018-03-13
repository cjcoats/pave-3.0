/* so that this header isn't included twice... */
#ifndef SBUS_MASTERDB_H_INCLUDED
#define SBUS_MASTERDB_H_INCLUDED
/********************************************************************
 *                                                                  *
 *                        EDSS Software Bus                         *
 *                     Copyright (c) 1994 by                        *
 *                                                                  *
 *                  North Carolina State University                 *
 *              Department of Computer Science, Box 8206            *
 *                      Raleigh, NC 27695-8206                      *
 *                               and                                *
 *           MCNC, North Carolina Supercomputing Center             *
 *              Environmental Programs, P.O. Box 12889              *
 *                 Research Triangle Park, 27709                    *
 *                                                                  *
 *      Made available under terms of the GNU Public License        *
 *                                                                  *
 ********************************************************************
 *    About:   masterDB.h
 *
 *        This include file contains structures, defines and prototypes for
 *        Functions used internally by the Bus Master Code
 *
 *    Version "$Id: masterDB.h 83 2018-03-12 19:24:33Z coats $"
 ****************************************************************************
 *  REVISION HISTORY - masterDB.h
 *
 * Date: 14-Dec-94
 * Version: 0.1
 * Change Description: ORIGINAL VERSION
 * Change author: Leland Morrison, NCSU, CSC
 *
 * Date: 15-Dec-94
 * Version: 0.2
 * Change Description: Added some comments and main headers
 * Change author: Vouk
 *
 *  Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 *
 ********************************************************************/

/* Database functions (masterDB.c) */

/* Types */
struct BusTypeNode *
BusMasterAddType ( struct BusTypeNode *last,
                   const char *name,
                   int id );
struct BusTypeNode *
BusMasterFindTypeById ( struct BusTypeNode *last,
                        int id );
struct BusTypeNode *
BusMasterFindTypeByName ( struct BusTypeNode *last,
                          const char *name );

/* Modules */
struct BusModuleNode *
BusMasterAddModule ( struct BusModuleNode *last,
                     const char *name,
                     int id,
                     int fd );
struct BusModuleNode *
BusMasterFindModuleByName ( struct BusModuleNode *last,
                            const char *name );
struct BusModuleNode *
BusMasterFindModuleById  ( struct BusModuleNode *last,
                           int id );
struct BusModuleNode *
BusMasterFindModuleByFd ( struct BusModuleNode *last,
                          int fd );


int BusMasterAddRegisteredType ( struct BusModuleNode *bmn, int typeId );
int BusMasterCheckRegisteredTypes ( struct BusModuleNode *bmn, int typeId );

#endif
