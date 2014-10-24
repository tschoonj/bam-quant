#include "bam_file_xmso.h"
#include <iostream>


int main(int argc, char *argv[]) {

	BAM::File::XMSO *xmso_file;

	try {
		xmso_file= new BAM::File::XMSO(TEST_FILE_XMSO);
	}
	catch (BAM::Exception &e) {
		std::cerr << "Exception occurred while reading in " << TEST_FILE_XMSO << " -> " << e.what();
		return 1;
	}
	delete xmso_file;
	return 0;
}
