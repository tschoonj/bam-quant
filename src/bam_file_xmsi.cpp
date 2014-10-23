#include "bam_exception.h"
#include <fstream>
#include <iostream>
#include <ext/stdio_filebuf.h>
#include "bam_file_xmsi.h"

using namespace BAM;
using namespace BAM::File;

XMSI::XMSI(string filename) : File::File(filename) {
	input = 0;
	Parse();
	Close();
}

XMSI::XMSI(struct xmi_input *new_input, string filename="") : File::File(filename) {
	if (xmi_validate_input(new_input) != 0) {
		throw BAM::Exception("BAM::File::XMSI::XMSI -> Could not validate input");
	}
	xmi_copy_input(new_input, &input);
}

XMSI::~XMSI() {
	xmi_free_input(input);
}

void XMSI::Open() {
	//do nothing
}

void XMSI::Close() {
	//do nothing
}

void XMSI::Parse() {
	if (xmi_xmlLoadCatalog() == 0) {
		throw BAM::Exception("BAM::File::XMSI::Parse -> Could not load XMI-MSIM XML catalog");
	}
	if (xmi_read_input_xml((char*)(filename.c_str()), &input) == 0) {
		throw BAM::Exception("BAM::File::XMSI::Parse -> Could not read file "+filename);
	}
}

void XMSI::Write() {
	if (xmi_validate_input(input) != 0) {
		throw BAM::Exception("BAM::File::XMSI::Write -> Could not validate input");
	}
	if (filename == "") {
		throw BAM::Exception("BAM::File::XMSI::Write -> Invalid filename");
	}
	if (xmi_write_input_xml((char*)filename.c_str(), input) == 0) {
		throw BAM::Exception("BAM::File::XMSI::Write -> Could not write to file "+filename);
	}	
}

void XMSI::Write(string filename) {
	try {
		SetFilename(filename);
		Write();
	}
	catch (BAM::Exception &e) {
		string message(e.what());
		size_t pos = message.find("->");
		if (pos == string::npos) {
			//this should not happen
			throw BAM::Exception(string("BAM::File::XMSI::Write -> ")+ e.what());
		}
		else {
			throw BAM::Exception(("BAM::File::XMSI::Write -> ")+ message.substr(pos+3));
		}
	}
}

//look here for more info regarding this very dirty trick : http://stackoverflow.com/questions/109449/getting-a-file-from-a-stdfstream
typedef std::basic_ofstream<char>::__filebuf_type buffer_t;
typedef __gnu_cxx::stdio_filebuf<char>            io_buffer_t; 
FILE* cfile_impl(buffer_t* const fb){
    return (static_cast<io_buffer_t* const>(fb))->file(); //type std::__c_file
}

FILE* cfile(std::ostream const& os){
    if(std::ofstream const* ofsP = dynamic_cast<std::ofstream const*>(&os)) return cfile_impl(ofsP->rdbuf());
    if(&os == &std::cerr) return stderr;
    if(&os == &std::cout) return stdout;
    if(&os == &std::clog) return stderr;
    return 0; // stream not recognized
}

//for some reason friend functions are really sensitive to the namespace thing
namespace BAM {
	namespace File {
		std::ostream& operator<< (std::ostream &out, const XMSI &xmsi) {
			if (xmi_validate_input(xmsi.input) != 0) {
				throw BAM::Exception("BAM::File::XMSI::operator<< -> Could not validate input");
			}
			FILE *filePtr = cfile(out);

			if (filePtr) {
				xmi_print_input(filePtr,xmsi.input);
			}
			else
				throw BAM::Exception("BAM::File::XMSI::operator<< -> Coult not get C filehandle for stream");
			return out;
		}
	}
}

void XMSI::ReplaceComposition(const BAM::Data::XMSI::Composition &composition_new) {
	//make sure input is valid
	if (composition_new.layers.size() == 0) {
		throw BAM::Exception("BAM::File::XMSI::ReplaceComposition -> No layers found in composition");
	}
	else if (composition_new.reference_layer < 1 || composition_new.reference_layer > composition_new.layers.size()) {
		throw BAM::Exception("BAM::File::XMSI::ReplaceComposition -> Invalid reference layer detected");
	}

	//first clear the current composition
	if (input->composition)
		xmi_free_composition(input->composition);
	struct xmi_composition composition;
	composition.reference_layer = composition_new.reference_layer;
	composition.layers = (struct xmi_layer*) xmi_memdup(&composition_new.layers[0], sizeof(struct xmi_layer)*composition_new.layers.size());
	composition.n_layers = composition_new.layers.size();
	xmi_copy_composition(&composition, &input->composition);
}

