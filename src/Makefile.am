bam_quantincludedir=${includedir}/bam_quant

AM_CPPFLAGS = -I${top_srcdir}/include -I${top_builddir}/include -I.. @xraylib_CFLAGS@ @xmimsim_CFLAGS@


lib_LTLIBRARIES=libbam-quant.la
libbam_quant_la_SOURCES = bam_file.cpp \
			  bam_file_asr.cpp \
			  bam_file_xmsi.cpp \
			  bam_file_xmso.cpp \
			  bam_data_xmsi.cpp \
			  bam_catalog.c \
			  bam_file_rxi.cpp \
			  bam_job_xmsi.cpp \
			  bam_data_base.cpp \
			  bam_data_xraylib.cpp \
			  bam_job_quant.cpp

libbam_quant_la_LIBADD = @xraylib_LIBS@ @xmimsim_LIBS@ @libxmlmm_LIBS@
libbam_quant_la_CPPFLAGS = -DBAM_CATALOG=\"$(datadir)/bam-quant/bam-quant-cat.xml\" \
			   @libxmlmm_CFLAGS@ \
			   -I${top_srcdir}/include \
			   -I${top_builddir}/include \
			   -I.. \
			   @xraylib_CFLAGS@ \
			   @xmimsim_CFLAGS@
