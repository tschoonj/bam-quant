#include "bam_file_xmso.h"
#include <iostream>


int main(int argc, char *argv[]) {

	BAM::File::XMSO *xmso_file;

	try {
		if (xmi_xmlLoadCatalog() == 0)
			throw BAM::Exception("Could not load XMI-MSIM XML catalog");

		xmso_file= new BAM::File::XMSO(TEST_FILE_XMSO);
	}
	catch (BAM::Exception &e) {
		std::cerr << "Exception occurred while reading in " << TEST_FILE_XMSO << " -> " << e.what();
		return 1;
	}
	catch (...) {
		std::cerr << "Some other exception occurred" << std::endl;
		return 1;
	}

	//let's check some values
	try {
		std::cout << "Ar total counts: " << xmso_file->GetCountsForElement(18) << std::endl;
		std::cout << "In KL3 total counts: " << xmso_file->GetCountsForElementForLine(49, "KL3") << std::endl;
		std::cout << "In L3M5 counts generated by the second interaction: " << xmso_file->GetCountsForElementForLineForInteraction(49, "L3M5", 2) << std::endl;
	}
	catch (BAM::Exception &e) {
		std::cerr << "BAM::Exception -> " << e.what();
		delete xmso_file;
		return 1;
	}

	delete xmso_file;
	return 0;
}
