#include "bam_file_xmsi.h"
#include "bam_data_xmsi.h"



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

	//now let's change the composition
	BAM::Data::XMSI::Layer *layer;
	BAM::Data::XMSI::Composition *composition;

	try {
		layer = new BAM::Data::XMSI::Layer(2.0, 0.5);
		layer->AddElement(26, 70.0);
		layer->AddElement(24, 18.0);
		layer->AddElement(28, 12.0);
		composition = new BAM::Data::XMSI::Composition();
		composition->AddLayer(*layer);
		composition->SetReferenceLayer(1);
		xmsi_file->ReplaceComposition(*composition);
		delete layer;
		delete composition;
	}
	catch (BAM::Exception &e) {
		std::cerr << "Exception occurred while creating a new layer -> " << e.what() << endl;;
		delete xmsi_file;
		delete layer;
		delete composition;
		return 1;
	}

	std::cout << *xmsi_file << endl;

	xmsi_file->SetFilename("In_copy.xmsi");
	try {
		xmsi_file->Write();
	}
	catch(BAM::Exception &e) {
		std::cerr << "Exception occurred while writing to file -> " << e.what() << endl;;
		delete xmsi_file;
		return 1;
	}


	delete xmsi_file;

	return 0;
}
