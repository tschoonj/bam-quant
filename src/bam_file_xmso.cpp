#include "bam_exception.h"
#include <fstream>
#include <iostream>
#include "bam_file_xmso.h"

using namespace BAM;
using namespace BAM::File;

XMSO::XMSO(std::string filename) : File::File(filename), output(0) {
	Parse();
	Close();
}

XMSO::XMSO(struct xmi_output *new_output, std::string filename) : File::File("") {
	xmi_copy_output(new_output, &output);
	if (filename.empty())
		SetFilename(std::string(output->input->general->outputfile));
	else 
		SetFilename(filename);
}

XMSO::~XMSO() {
	if (output)
		xmi_free_output(output);
}

void XMSO::Open() {
	//do nothing
}

void XMSO::Close() {
	//do nothing
}

void XMSO::Parse() {
	if (xmi_read_output_xml((char*)(filename.c_str()), &output) == 0) {
		throw BAM::Exception("BAM::File::XMSO::Parse -> Could not read file "+filename);
	}
}

void XMSO::Write() {
	if (filename == "") {
		throw BAM::Exception("BAM::File::XMSO::Write -> Invalid filename");
	}
	if (xmi_write_output_xml((char*)filename.c_str(), output) == 0) {
		throw BAM::Exception("BAM::File::XMSO::Write -> Could not write to file ");
	}	
}

void XMSO::Write(std::string new_filename) {
	SetFilename(new_filename);
	Write();
}

