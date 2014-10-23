#include "bam_exception.h"
#include <fstream>
#include <iostream>
#include "bam_file_xmso.h"

using namespace BAM;
using namespace BAM::File;

XMSO::XMSO(string filename) : File::File(filename) {
	output = 0;
	Parse();
	Close();
}

XMSO::XMSO(struct xmi_output *new_output, string filename="") : File::File(filename) {
	xmi_copy_output(new_output, &output);
}

XMSO::~XMSO() {
	xmi_free_output(output);
}

void XMSO::Open() {
	//do nothing
}

void XMSO::Close() {
	//do nothing
}

void XMSO::Parse() {
	if (xmi_xmlLoadCatalog() == 0) {
		throw BAM::Exception("BAM::File::XMSO::Parse -> Could not load XMI-MSIM XML catalog");
	}
	if (xmi_read_output_xml((char*)(filename.c_str()), &output) == 0) {
		throw BAM::Exception("BAM::File::XMSO::Parse -> Could not read file "+filename);
	}
}

