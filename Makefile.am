#pkgconfigdir=$(libdir)/pkgconfig
#pkgconfig_DATA=libxrmc.pc

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src include tests bin xml .


#EXTRA_DIST = test libxrmc.pc.in xrmc.spec.in
#DISTCHECK_CONFIGURE_FLAGS = --disable-openmp --disable-xmi-msim
DISTCHECK_CONFIGURE_FLAGS = CXX=clang-omp++ CC=clang-omp
