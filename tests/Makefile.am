AM_CPPFLAGS = -I${top_srcdir}/include -I${top_builddir}/include

check_PROGRAMS = test_bam_file_asr

test_bam_file_asr_SOURCES = test_bam_file_asr.cpp
test_bam_file_asr_LDADD = ../src/libbam-quant.la
test_bam_file_asr_CPPFLAGS = -DTEST_FILE_ASR=\"$(srcdir)/std20kev/QTD10029.ASR\" $(AM_CPPFLAGS)

TESTS = $(check_PROGRAMS)
