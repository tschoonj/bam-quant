AC_INIT([bam-quant],[0.1],[tom.schoonjans@me.com],,[http://github.com/tschoonj/bam-quant])
AC_PREREQ([2.60])
#AC_CONFIG_SRCDIR([src/main/xrmc_main.cpp])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CANONICAL_HOST



LIB_CURRENT=0
LIB_REVISION=0
LIB_AGE=0
AC_SUBST(LIB_CURRENT)
AC_SUBST(LIB_REVISION)
AC_SUBST(LIB_AGE)

#m4_pattern_allow([AS_TR_SH])
AC_CONFIG_MACRO_DIR([m4])

AC_USE_SYSTEM_EXTENSIONS
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

#libtool
LT_PREREQ([2.0.0])

LT_INIT()
AC_PROG_LIBTOOL

AC_PROG_CC
if test `AS_BASENAME([$CC])` = $CC ; then
  AC_CHECK_PROG(CC_FULL, [$CC], $CC, [none])
  #this next line may never be reached...
  if test x$CC_FULL = "xnone" ; then
          AC_MSG_ERROR([no C compiler was found on the system.])
  fi
fi
AM_PROG_CC_C_O


AC_PROG_CXX
if test `AS_BASENAME([$CXX])` = $CXX ; then
  AC_CHECK_PROG(CXX_FULL, [$CXX], $CXX, [none])
  #this next line may never be reached...
  if test x$CXX_FULL = "xnone" ; then
          AC_MSG_ERROR([no C++ compiler was found on the system.])
  fi
fi
AC_PROG_CXX_C_O


LDFLAGS_EXTRA=""
case "$host" in
	*mingw*)
		LDFLAGS_EXTRA+="-no-undefined"
		;;
esac
AC_SUBST(LDFLAGS_EXTRA)

#look for openmp support in C++
#AC_LANG_PUSH(C++)
#AC_OPENMP
#AC_LANG_POP(C++)

#AC_CHECK_LIB([m], [exp], [], [AC_MSG_ERROR([math library does not contain exp])])
#AC_CHECK_FUNC(expm1,,[AC_MSG_ERROR([Required libc function not found])])
#AC_CHECK_FUNC(log1p,,[AC_MSG_ERROR([Required libc function not found])])



AC_LANG_PUSH(C++)
AC_CHECK_HEADERS([ext/stdio_filebuf.h])
AC_LANG_POP(C++)


#look for xraylib
#initialize pkg-config
PKG_PROG_PKG_CONFIG


#search for xraylib and other modules
PKG_CHECK_MODULES([xraylib],libxrl >= 3.1.0)
PKG_CHECK_MODULES([xmimsim],libxmimsim >= 5.1)
PKG_CHECK_MODULES([glibmm],glibmm-2.4)
PKG_CHECK_MODULES([libxmlmm], libxml++-2.6 >= 2.30.0)
PKG_CHECK_MODULES([gsl], gsl)
ENABLE_GUI=
PKG_CHECK_MODULES([gtkmm],gtkmm-3.0 >= 3.12.0, [ENABLE_GUI=1], 
	[ENABLE_GUI=0]
	AC_MSG_WARN([Building without graphical user interface!!!]))
AM_CONDITIONAL([ENABLE_GUI],[test "x$ENABLE_GUI" = x1])



#search for xmlcatalog
AC_CHECK_PROG([XMLCATALOG],[xmlcatalog],[xmlcatalog],[fail])
if test x$XMLCATALOG = xfail ; then
	AC_MSG_ERROR([xmlcatalog was not found. This should be a part of the libxml2 installation which was detected on the system. Consider verifying your PATH variable.])
fi


AC_CONFIG_FILES([	Makefile
			src/Makefile
			include/Makefile
			tests/Makefile
			bin/Makefile
			bin/app1/Makefile
			bin/app2/Makefile
			bin/app3/Makefile
			xml/Makefile
			])
AC_CONFIG_HEADERS([config.h])


abs_top_builddir=`pwd -P`
AC_SUBST(abs_top_builddir)
abs_top_srcdir=`AS_DIRNAME([$0])`
cd $abs_top_srcdir
abs_top_srcdir=`pwd -P`
cd $abs_top_builddir
AC_SUBST(abs_top_srcdir)

AM_CONDITIONAL([ABS_SRC_BUILD_EQUAL],[test x$abs_top_builddir = x$abs_top_srcdir])


AC_OUTPUT

