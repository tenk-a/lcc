export BUILDDIR=bld
if [ ! -e $BUILDDIR ]; then
  mkdir $BUILDDIR
fi
make all HOSTFILE=etc/linux.c
export BUILDDIR=
