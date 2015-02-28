#include "bam_job_xmsi.h"
#include <string>

int main(int argc, char *argv[]) {

	BAM::Job::XMSI *job;
	struct xmi_main_options options = xmi_get_default_main_options();
	options.verbose = 1;
	options.extra_verbose = 1;


	try {
		//catalog stuff first
		if (xmi_xmlLoadCatalog() == 0)
			throw BAM::Exception("Could not load XMI-MSIM XML catalog");
	
		job = new BAM::Job::XMSI(TEST_JOB_XMSI, options);
		job->Start();
		job->Write();

	}
	catch (BAM::Exception &e) {
		std::cerr << "BAM exception: " << e.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "Unknown exception caught: this should not happen! " << std::endl;
		return 1;
	}
	delete job;

	return 0;





}
