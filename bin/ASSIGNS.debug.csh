#! /bin/csh
#
#   Environment variables for operating "pave"
#   (but see ${PAVEDIR}/src/Makeinclude for building "pave")
#  Copyright 2018 Carlie J. Coats, Jr., Ph.D.
#  Released under the GNU General Public License, Version 2
#
# Script version "$Id: ASSIGNS.debug.csh 84 2018-03-12 21:26:53Z coats $"
#########################################################################

setenv  BUILD           Linux2_x86_64gfort_mediumdbg

setenv  PAVEDIR         ${HOME}/apps/pave-3.0

setenv  PAVE_DIR        ${PAVEDIR}
setenv  PAVE_BINDIR     ${PAVEDIR}/${BUILD}
setenv  PAVE_MAPDIR     ${PAVEDIR}/maps
setenv  PAVE_DOCS       ${PAVEDIR}/Docs

setenv  PAVE_ENV        ${BUILD}
setenv  PAVE_EXE        ${PAVE_BINDIR}/pave.exe

if ( ! $?PATH             ) setenv PATH            ""
if ( ! $?MANPATH          ) setenv MANPATH         ""
if ( ! $?LD_LIBRARY_PATH  ) setenv LD_LIBRARY_PATH ""

setenv PATH             "${PAVE_BINDIR}:${PATH}"
setenv MANPATH          "${PAVE_DIR}/man:${MANPATH}"
setenv LD_LIBRARY_PATH  "${PAVE_DIR}/lib:${LD_LIBRARY_PATH}"
