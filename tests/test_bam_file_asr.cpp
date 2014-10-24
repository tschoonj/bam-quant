#include "bam_file_asr.h"
#include "bam_exception.h"
#include <iostream>

int main(int argc, char **argv) {

	BAM::File::ASR *asr;

	try {
		asr = new BAM::File::ASR(TEST_FILE_ASR);
	}
	catch (ifstream::failure &e) {
		cerr << "Exception reading in file: " << e.what() << endl;	
		return 1;
	}
	catch (BAM::Exception &e) {
		cerr << "Some other exception detected: " << e.what() << endl;
		return 1;
	}

	delete asr;
	return 0;
}
