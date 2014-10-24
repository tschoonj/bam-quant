bin_PROGRAMS = app1

AM_CPPFLAGS = -I${top_srcdir}/include -I${top_builddir}/include -I${top_srcdir}/src -I${top_builddir}/src -I${top_srcdir}/bin

app1_SOURCES = main.cpp \
	       window.h \
	       window.cpp \
	       application.cpp \
	       application.h \
	       mendeleev-button.h \
	       mendeleev-button.cpp



app1_CPPFLAGS = @gtkmm_CFLAGS@ $(AM_CPPFLAGS) @xraylib_CFLAGS@ @xmimsim_CFLAGS@
app1_LDADD = @gtkmm_LIBS@ ${top_builddir}/src/libbam-quant.la @xmimsim_LIBS@
