#include "bam_file_asr.h"
#include "bam_exception.h"
#include <iostream>

int main(int argc, char **argv) {

	BAM::File::ASR *asr;

	try {
		asr = new BAM::File::ASR(TEST_FILE_ASR);
	}
	catch (std::ifstream::failure &e) {
		std::cerr << "Exception reading in file: " << e.what() << std::endl;	
		return 1;
	}
	catch (BAM::Exception &e) {
		std::cerr << "Some other exception detected: " << e.what() << std::endl;
		return 1;
	}

	delete asr;
	return 0;
}
