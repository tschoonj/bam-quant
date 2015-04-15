#include "config.h"
#include "bam_exception.h"
#include <fstream>
#include <iostream>
#ifdef HAVE_EXT_STDIO_FILEBUF_H
#include <ext/stdio_filebuf.h>
#endif
#include "bam_file_xmsi.h"

using namespace BAM;
using namespace BAM::File;


XMSI::XMSI(std::string filename) : File::File(filename) {
	input = 0;
	Parse();
	Close();
}

XMSI::XMSI(struct xmi_input *new_input, std::string filename) : File::File(filename) {
	if (xmi_validate_input(new_input) != 0) {
		throw BAM::Exception("BAM::File::XMSI::XMSI -> Could not validate input");
	}
	xmi_copy_input(new_input, &input);
}

XMSI::~XMSI() {
	if (input)
		xmi_free_input(input);
}

void XMSI::Open() {
	//do nothing
}

void XMSI::Close() {
	//do nothing
}

void XMSI::Parse() {
	if (xmi_read_input_xml((char*)(filename.c_str()), &input) == 0) {
		//std::cout << "BAM::File::XMSI::Parse -> Could not read file "+filename << std::endl;
		throw BAM::Exception("BAM::File::XMSI::Parse -> Could not read file ");
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
		throw BAM::Exception("BAM::File::XMSI::Write -> Could not write to file ");
	}	
}

void XMSI::Write(std::string new_filename) {
	SetFilename(new_filename);
	Write();
}

#ifdef HAVE_EXT_STDIO_FILEBUF_H
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
#endif

//for some reason friend functions are really sensitive to the namespace thing
namespace BAM {
	namespace File {
		std::ostream& operator<< (std::ostream &out, const XMSI &xmsi) {
			if (xmi_validate_input(xmsi.input) != 0) {
				throw BAM::Exception("BAM::File::XMSI::operator<< -> Could not validate input");
			}
#ifdef HAVE_EXT_STDIO_FILEBUF_H
			FILE *filePtr = cfile(out);
#else
			FILE *filePtr = 0;
#endif

			if (filePtr) {
				xmi_print_input(filePtr,xmsi.input);
			}
			else
				throw BAM::Exception("BAM::File::XMSI::operator<< -> Could not get C filehandle for stream");
			return out;
		}
	}
}

BAM::Data::XMSI::Composition XMSI::GetComposition() {
	return BAM::Data::XMSI::Composition(input->composition);
}

BAM::Data::XMSI::Geometry XMSI::GetGeometry() {
	return BAM::Data::XMSI::Geometry(input->geometry);
}

BAM::Data::XMSI::Excitation XMSI::GetExcitation() {
	return BAM::Data::XMSI::Excitation(input->excitation);
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
	struct xmi_composition *composition = (struct xmi_composition *) xmi_malloc(sizeof(struct xmi_composition));
	composition->reference_layer = composition_new.reference_layer;
	composition->layers = (struct xmi_layer*) xmi_malloc(sizeof(struct xmi_layer)*composition_new.layers.size());
	composition->n_layers = composition_new.layers.size();
	
	//std::cout << "composition->n_layers: " << composition->n_layers << std::endl;
	
	for (int i = 0 ; i < composition->n_layers ; i++) {
		composition->layers[i] = composition_new.layers[i].Convert();
	}
	//xmi_print_layer(stdout, composition->layers, composition->n_layers);

	xmi_copy_composition(composition, &input->composition);
	xmi_free_composition(composition);
}


