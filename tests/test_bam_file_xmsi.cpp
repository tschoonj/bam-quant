#include "bam_file_xmsi.h"



int main(int argc, char *argv[]) {

	BAM::File::XMSI *xmsi_file;

	try {
		xmsi_file= new BAM::File::XMSI(TEST_FILE_XMSI);
	}
	catch (BAM::Exception &e) {
		std::cerr << "Exception occurred while reading in " << TEST_FILE_XMSI << " -> " << e.what();
		delete xmsi_file;
		return 1;
	}

	//pointer deref is really important here, otherwise you will just get an address
	std::cout << *xmsi_file << endl;

	xmsi_file->SetFilename("In_copy.xmsi");
	xmsi_file->Write();

	delete xmsi_file;

	return 0;
}
