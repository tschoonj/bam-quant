#include "config.h"
#include "bam_file_xmsi.h"
#include "bam_data_xmsi.h"



int main(int argc, char *argv[]) {

	BAM::File::XMSI *xmsi_file;

	try {
		xmsi_file= new BAM::File::XMSI(TEST_FILE_XMSI);
	}
	catch (BAM::Exception &e) {
		std::cerr << "Exception occurred while reading in " << TEST_FILE_XMSI << " -> " << e.what();
		return 1;
	}

	//pointer deref is really important here, otherwise you will just get an address
#ifdef HAVE_EXT_STDIO_FILEBUF_H
	//you need gcc for this to work
	std::cout << *xmsi_file << endl;
#endif
	//now let's change the composition
	BAM::Data::XMSI::Layer *layer;
	BAM::Data::XMSI::Composition *composition;

	try {
		composition = new BAM::Data::XMSI::Composition();
		layer = new BAM::Data::XMSI::Layer("Air, Dry (near sea level)", 5.0);
		composition->AddLayer(*layer);
		delete layer;

		layer = new BAM::Data::XMSI::Layer(2.0, 0.5);
		layer->AddElement(26, 70.0);
		layer->AddElement(24, 18.0);
		layer->AddElement(28, 12.0);
		composition->AddLayer(*layer);
		delete layer;

		composition->SetReferenceLayer(2);
		xmsi_file->ReplaceComposition(*composition);
		delete composition;
	}
	catch (BAM::Exception &e) {
		std::cerr << "Exception occurred while creating a new layer -> " << e.what() << endl;;
		delete xmsi_file;
		//delete layer;
		//delete composition;
		return 1;
	}

#ifdef HAVE_EXT_STDIO_FILEBUF_H
	std::cout << *xmsi_file << endl;
#endif

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
