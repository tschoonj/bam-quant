#include "bam_file_asr.h"
#include "bam_exception.h"
#include <iostream>

int main(int argc, char **argv) {

	BAM::FileASR *asr;

	try {
		asr = new BAM::FileASR(TEST_FILE_ASR);
	}
	catch (ifstream::failure &e) {
		cerr << "Exception reading in file: " << e.what() << endl;	
		delete asr;
		return 1;
	}
	catch (BAM::bam_exception &e) {
		cerr << "Some other exception detected: " << e.what() << endl;
		delete asr;
		return 1;
	}

	delete asr;
	return 0;
}
