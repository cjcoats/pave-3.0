#! /bin/csh
#
#  Un-tar the packages under ${PAVEDIR}/external_libs/tars,
#
# Version "$Id: untar.csh 83 2018-03-12 19:24:33Z coats $"

if ( ! $?SCRIPT_DEBUG ) set echo

if ( ! $?PAVE_DIR ) setenv PAVE_DIR  ${HOME}/apps/pave-2.4

setenv  EXTERN  ${PAVE_DIR}/external_libs
setenv  TARS    ${EXTERN}/tars

cd ${EXTERN}

tar xvfj ${TARS}/netcdf-3.6.3.tar.bz2
tar xvfj ${TARS}/libX11-1.6.5.tar.bz2
tar xvfj ${TARS}/libXt-1.1.5.tar.bz2
tar xvfj ${TARS}/motif-2.3.8.tar.bz2
tar xvfj ${TARS}/tcl8.4.9-src.tar.bz2
tar xvfj ${TARS}/tk8.4.9-src.tar.bz2
tar xvfj ${TARS}/BLT2.4z.tar.bz2
tar xvfj ${TARS}/proj-4.9.3.tar.g
tar xvfj ${TARS}/plplot4p99i.tar.bz2
tar xvfj ${TARS}/plplot-5.13.0.tar.bz2
