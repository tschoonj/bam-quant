if ENABLE_GUI
bin_PROGRAMS = app2
endif

AM_CPPFLAGS = -I${top_srcdir}/include -I${top_builddir}/include -I${top_srcdir}/src -I${top_builddir}/src -I${top_srcdir}/bin

app2_SOURCES = main.cpp \
	           app2-assistant.h \
	           app2-assistant.cpp \
	           app2-matrixdialog.h \
	           app2-matrixdialog.cpp \
		   app2-energiesgrid.h \
		   app2-energiesgrid.cpp \
		   app2-puresgrid.h \
		   app2-puresgrid.cpp \
		   app2-samplesgrid.h \
		   app2-samplesgrid.cpp \
		   app2-samplessummarygrid.h \
		   app2-samplessummarygrid.cpp \
		   app2-simulategrid.h \
		   app2-simulategrid.cpp \
		   app2-outputfilegrid.h \
		   app2-outputfilegrid.cpp \
		   app2-confirmationlabel.h \
		   app2-confirmationlabel.cpp



app2_CPPFLAGS = @gtkmm_CFLAGS@ \
		    $(AM_CPPFLAGS) \
		    @xraylib_CFLAGS@ \
		    @xmimsim_CFLAGS@ \
		    @gsl_CFLAGS@
app2_LDADD = @gtkmm_LIBS@ \
	         ${top_builddir}/src/libbam-quant.la \
	         @xmimsim_LIBS@ \
	         @gsl_LIBS@
