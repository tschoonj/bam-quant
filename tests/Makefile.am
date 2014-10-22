AM_CPPFLAGS = -I${top_srcdir}/include -I${top_builddir}/include

check_PROGRAMS = test_bam_file_asr \
		 test_bam_file_xmsi

test_bam_file_asr_SOURCES = test_bam_file_asr.cpp
test_bam_file_asr_LDADD = ../src/libbam-quant.la
test_bam_file_asr_CPPFLAGS = -DTEST_FILE_ASR=\"$(srcdir)/std20kev/QTD10029.ASR\" $(AM_CPPFLAGS)

test_bam_file_xmsi_SOURCES = test_bam_file_xmsi.cpp
test_bam_file_xmsi_LDADD = ../src/libbam-quant.la @xmimsim_LIBS@
test_bam_file_xmsi_CPPFLAGS = -DTEST_FILE_XMSI=\"$(srcdir)/In.xmsi\" $(AM_CPPFLAGS) @xmimsim_CFLAGS@


TESTS = $(check_PROGRAMS)

EXTRA_DIST = In.xmsi std20keV

clean-local:
	rm -rf In_copy.xmsi
