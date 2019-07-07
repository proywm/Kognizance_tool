export CC=gcc
export CXX=g++
export FC=gfortran
export F77=gfortran

ROOTDIR=$PWD 
LIBMONITORDIR=$ROOTDIR/libmonitor
echo "libmonitor Directory:"
echo $LIBMONITORDIR
cd $LIBMONITORDIR
make clean
./configure --prefix=$LIBMONITORDIR/installDir CC=gcc
make
make install
echo "Libmonitor installed *************"
