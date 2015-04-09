#include "bam_file.h"
#include "bam_exception.h"
#include <iostream>

using namespace BAM::File;

File::File(std::string filename) : filename(filename) {
	this->Open();	
}

File::~File() {
	this->Close();
}

void File::Open(){
	if (filename != "") {
		try {
			fs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			fs.open(filename.c_str());
		}
		catch (std::ifstream::failure &e) {
			throw BAM::Exception(std::string("BAM::File::File::Open -> I/O error: ")+e.what());
		}
		catch (...) {
			throw BAM::Exception(std::string("BAM::File::File::Open -> unknown exception caught: this should not happen!!!"));
		}
	}
}


void File::Close() {
	if (fs.is_open()) {	
		fs.close();
	}
}
