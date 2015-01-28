bin_PROGRAMS = app2

AM_CPPFLAGS = -I${top_srcdir}/include -I${top_builddir}/include -I${top_srcdir}/src -I${top_builddir}/src -I${top_srcdir}/bin

app2_SOURCES = main.cpp \
	       app2-assistant.h \
	       app2-assistant.cpp



app2_CPPFLAGS = @gtkmm_CFLAGS@ \
		$(AM_CPPFLAGS) \
		@xraylib_CFLAGS@ \
		@xmimsim_CFLAGS@ \
		@libxmlmm_CFLAGS@ \
		@gsl_CFLAGS@ \
		-DBAM_CATALOG=\"$(datadir)/bam-quant/bam-quant-cat.xml\"
app2_LDADD = @gtkmm_LIBS@ \
	     ${top_builddir}/src/libbam-quant.la \
	     @xmimsim_LIBS@ \
	     @libxmlmm_LIBS@ \
	     @gsl_LIBS@
