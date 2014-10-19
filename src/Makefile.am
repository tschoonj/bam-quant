bam_quantincludedir=${includedir}/bam_quant

AM_CPPFLAGS = -I${top_srcdir}/include -I${top_builddir}/include -I.. @xraylib_CFLAGS@


lib_LTLIBRARIES=libbam-quant.la
libbam_quant_la_SOURCES = bam_file.cpp \
			  bam_file_asr.cpp
libbam_quant_la_LIBADD = @xraylib_LIBS@
