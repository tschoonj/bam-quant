bam_quantincludedir=${includedir}/bam_quant

AM_CPPFLAGS = -I${top_srcdir}/include -I${top_builddir}/include -I.. @xraylib_CFLAGS@ @xmimsim_CFLAGS@


lib_LTLIBRARIES=libbam-quant.la
libbam_quant_la_SOURCES = bam_file.cpp \
			  bam_file_asr.cpp \
			  bam_file_xmsi.cpp \
			  bam_file_xmso.cpp \
			  bam_data_xmsi.cpp
libbam_quant_la_LIBADD = @xraylib_LIBS@ @xmimsim_LIBS@
