#! /bin/csh
#
#  Un-tar the packages under ${PAVEDIR}/external_libs/tars,
#  configure and build them under, ${PAVEDIR}/external_libs,
#  and install them under ${PAVEDIR}
#
#  Note that there will be quite a few "extras" installed 
#  in/under ${PAVEDIR}/lib and ${PAVEDIR}/bin...
#
# Version "$Id: buildinstall.csh 85 2018-03-13 13:17:36Z coats $"

if ( ! $?SCRIPT_DEBUG ) set echo


####################  Directories  ###############################

if ( ! $?PAVEDIR ) setenv PAVEDIR  ${HOME}/apps/pave-3.0
if ( ! $?BUILD   ) setenv BUILD    Linux2_x86_64gfort_medium
if ( ! $?IOBIN   ) setenv IOBIN    ${HOME}/ioapi-3.2/${BUILD}

setenv  EXTERN  ${PAVEDIR}/external_libs
setenv  TARS    ${EXTERN}/tars

mkdir -p ${PAVEDIR}/lib
mkdir -p ${PAVEDIR}/bin
mkdir -p ${PAVEDIR}/include

if ( -e  ${PAVEDIR}/lib/libioapi.a ) then
    echo "libioapi.a already installed"
elseif ( -f  ${IOBIN}/libioapi.a ) then
    ln -s  ${IOBIN}/libioapi.a  ${PAVEDIR}/lib/
else
    echo "********************************************"
    echo "You need to build I/O API-3.2 for BIN=$BUILD"
    echo "********************************************"
endif

set errors = 0


####################  Compilers and flags  ###############################

setenv CC       gcc
setenv CXX      g++
setenv FC       gfortran
setenv F77      gfortran
setenv F90      gfortran
setenv CFLAGS   "-O3 -fopenmp -mcmodel=medium -funroll-loops -m64"
setenv FFLAGS   "-O3 -fopenmp -mcmodel=medium -funroll-loops -m64"
setenv FCLAGS   "-O3 -fopenmp -mcmodel=medium -funroll-loops -m64"
setenv F77LAGS  "-O3 -fopenmp -mcmodel=medium -funroll-loops -m64"
setenv CPPFLAGS "-I${PAVEDIR}/include -D_GNU_SOURCE=1"
setenv LDFLAGS  "-L/usr/lib64 -L${PAVEDIR}/lib"


####################  Packages  ###############################

echo '-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'
cd ${EXTERN}/netcdf-3.6.3/
./configure --prefix=${PAVEDIR} 
make && make install
set foo = ${status}
if ( ${foo} != 0 ) then
    echo "### ERROR ${foo} building or installing netcdf"
    @ errors = ${errors} + 1
endif

echo '-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'
cd ${EXTERN}/libX11-1.6.5
./configure --prefix=${PAVEDIR} --enable-static
make && make install
set foo = ${status}
if ( ${foo} != 0 ) then
    echo "### ERROR ${foo} building or installing libX11"
    @ errors = ${errors} + 1
endif

echo '-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'
cd ${EXTERN}/libXt-1.1.5
./configure --prefix=${PAVEDIR} --enable-static --disable-shared
make && make install
set foo = ${status}
if ( ${foo} != 0 ) then
    echo "### ERROR ${foo} building or installing libXt"
    @ errors = ${errors} + 1
endif

echo '-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'
cd ${EXTERN}/motif-2.3.8
./configure --prefix=${PAVEDIR} --enable-static --disable-shared
make && make install
set foo = ${status}
if ( ${foo} != 0 ) then
    echo "### ERROR ${foo} building or installing motif"
    @ errors = ${errors} + 1
endif
rm ${PAVEDIR}/bin/mwm; 

echo '-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'
cd ${EXTERN}/tcl8.4.9/unix/
./configure --prefix=${PAVEDIR} --enable-threads --disable-shared --enable-64bit
make && make install
set foo = ${status}
if ( ${foo} != 0 ) then
    echo "### ERROR ${foo} building or installing TCL"
    @ errors = ${errors} + 1
endif
cd ${PAVEDIR}/lib; ln -s libtcl8.4.a libtcl.a

echo '-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'
cd ${EXTERN}/tk8.4.9/unix/
./configure --prefix=${PAVEDIR} --enable-threads --disable-shared --enable-64bit
make && make install
set foo = ${status}
if ( ${foo} != 0 ) then
    echo "### ERROR ${foo} building or installing TK"
endif
cd ${PAVEDIR}/lib; ln -s libtk8.4.a libtk.a

echo '-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'
cd ${EXTERN}/blt2.4z/
./configure --prefix=${PAVEDIR} --enable-threads --disable-shared --enable-64bit --with-tcl=${PAVEDIR}
make && make install
set foo = ${status}
if ( ${foo} != 0 ) then
    echo "### ERROR ${foo} building or installing BLT"
    @ errors = ${errors} + 1
endif
cd ${PAVEDIR}/lib; ln -s libBLT24.a libBLT.a

echo '-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'
cd ${EXTERN}/proj-4.9.3
./configure --prefix=${PAVEDIR} --enable-static --disable-shared
make && make install
set foo = ${status}
if ( ${foo} != 0 ) then
    echo "### ERROR ${foo} building or installing proj"
    @ errors = ${errors} + 1
endif

echo '-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'
cd ${EXTERN}/plplot4p99i/
./configure --prefix=${PAVEDIR} --enable-static --with-x
make && make install
set foo = ${status}
if ( ${foo} != 0 ) then
    echo "### ERROR ${foo} building or installing PLPLOT-4"
    @ errors = ${errors} + 1
endif

echo '-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'
cd ${EXTERN}/plplot-5.3.1/
./configure --prefix=${PAVEDIR} --enable-static --with-x
make -i
cp plrender plserver pltcl ${PAVEDIR}/bin/

echo '-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'
if ( ${errors} > 0 ) then
    echo "buildinstall.csh FAILED with ${errors} errors\n"
else
    echo "buildinstall.csh completed successfully\n"
endif

