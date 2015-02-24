#include "bam_file.h"
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
		fs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fs.open(filename.c_str());
	}
}


void File::Close() {
	if (fs.is_open()) {	
		fs.close();
	}
}
