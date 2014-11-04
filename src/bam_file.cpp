#include "bam_file.h"
#include <iostream>

using namespace BAM::File;

File::File(string filename) : filename(filename) {
	//cout << "Entering BAM::File constructor" << endl;
	this->Open();	
}

File::~File() {
	//cout << "Entering BAM::File destructor" << endl;
	this->Close();
}

void File::Open(){
	//cout << "Entering BAM::File Open" << endl;
	if (filename != "") {
		fs.exceptions(ifstream::failbit | ifstream::badbit);
		fs.open(filename.c_str());
		//cout << "Leaving BAM::File Open" << endl;
	}
}


void File::Close() {
	//cout << "Entering BAM::File Close" << endl;
	if (fs.is_open()) {	
		//cout << "Entering BAM::File Close: is open" << endl;
		fs.close();
	}
	/*else {
		cout << "Entering BAM::File Close: is not open" << endl;
	}*/
}
