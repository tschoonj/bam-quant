AM_CPPFLAGS = -I${top_srcdir}/include -I${top_builddir}/include

check_PROGRAMS = test_bam_file_asr \
		 test_bam_file_xmsi \
		 test_bam_file_xmso \
		 test_bam_file_rxi_single \
		 test_bam_file_rxi_multi \
		 test_bam_file_rxi_common \
		 test_bam_job_xmsi \
		 test_bam_data_xraylib

test_bam_file_asr_SOURCES = test_bam_file_asr.cpp
test_bam_file_asr_LDADD = ../src/libbam-quant.la
test_bam_file_asr_CPPFLAGS = -DTEST_FILE_ASR=\"$(srcdir)/std20kev/QTD10029.ASR\" $(AM_CPPFLAGS)

test_bam_file_xmsi_SOURCES = test_bam_file_xmsi.cpp
test_bam_file_xmsi_LDADD = ../src/libbam-quant.la @xmimsim_LIBS@
test_bam_file_xmsi_CPPFLAGS = -DTEST_FILE_XMSI=\"$(srcdir)/In.xmsi\" $(AM_CPPFLAGS) @xmimsim_CFLAGS@

test_bam_file_xmso_SOURCES = test_bam_file_xmso.cpp
test_bam_file_xmso_LDADD = ../src/libbam-quant.la @xmimsim_LIBS@
test_bam_file_xmso_CPPFLAGS = -DTEST_FILE_XMSO=\"$(srcdir)/In.xmso\" $(AM_CPPFLAGS) @xmimsim_CFLAGS@

test_bam_file_rxi_single_SOURCES = test_bam_file_rxi_single.cpp
test_bam_file_rxi_single_LDADD = ../src/libbam-quant.la @xmimsim_LIBS@ @glibmm_LIBS@
test_bam_file_rxi_single_CPPFLAGS = -DTEST_FILE_RXI_SINGLE=\"$(srcdir)/rxi_test_data_single.rxi\" $(AM_CPPFLAGS) @xmimsim_CFLAGS@ -DCATALOGPATH=\"$(abs_top_srcdir)/xml/\" @glibmm_CFLAGS@

test_bam_file_rxi_multi_SOURCES = test_bam_file_rxi_multi.cpp
test_bam_file_rxi_multi_LDADD = ../src/libbam-quant.la @xmimsim_LIBS@ @glibmm_LIBS@
test_bam_file_rxi_multi_CPPFLAGS = -DTEST_FILE_RXI_MULTI=\"$(srcdir)/rxi_test_data_multi.rxi\" $(AM_CPPFLAGS) @xmimsim_CFLAGS@ -DCATALOGPATH=\"$(abs_top_srcdir)/xml/\" @glibmm_CFLAGS@

test_bam_file_rxi_common_SOURCES = test_bam_file_rxi_common.cpp
test_bam_file_rxi_common_LDADD = ../src/libbam-quant.la @xmimsim_LIBS@ @glibmm_LIBS@
test_bam_file_rxi_common_CPPFLAGS = -DTEST_FILE_RXI_SINGLE=\"$(srcdir)/rxi_test_data_single.rxi\" -DTEST_FILE_RXI_MULTI=\"$(srcdir)/rxi_test_data_multi.rxi\" $(AM_CPPFLAGS) @xmimsim_CFLAGS@ -DCATALOGPATH=\"$(abs_top_srcdir)/xml/\" @glibmm_CFLAGS@

test_bam_job_xmsi_SOURCES = test_bam_job_xmsi.cpp
test_bam_job_xmsi_LDADD = ../src/libbam-quant.la @xmimsim_LIBS@
test_bam_job_xmsi_CPPFLAGS = -DTEST_JOB_XMSI=\"$(srcdir)/test_bam_job_xmsi.xmsi\" $(AM_CPPFLAGS) @xmimsim_CFLAGS@

test_bam_data_xraylib_SOURCES = test_bam_data_xraylib.cpp
test_bam_data_xraylib_LDADD = ../src/libbam-quant.la @xraylib_LIBS@
test_bam_data_xraylib_CPPFLAGS = $(AM_CPPFLAGS) @xraylib_CFLAGS@

TESTS = $(check_PROGRAMS)


EXTRA_DIST = In.xmsi std20kev In.xmso rxi_test_data_single.rxi rxi_test_data_multi.rxi test_bam_job_xmsi.xmsi

clean-local:
	rm -rf In_copy.xmsi rxi_test_data_single.rxi.copy rxi_test_data_multi.rxi.copy test_bam_job_xmsi.xmso
