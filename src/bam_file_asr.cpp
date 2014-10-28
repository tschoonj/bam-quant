#include "bam_file_asr.h"
#include "bam_exception.h"
#include <iostream>
#include <sstream>
#include <xraylib.h>

using namespace BAM;
using namespace BAM::File;
using namespace std;

ASR::ASR(string filename) : File::File(filename) {
	//cout << "Entering BAM::File::ASR constructor" << endl;
	if (fs) {
		this->Parse();
	}
	this->Close();
	//cout << "Leaving BAM::File::ASR constructor" << endl;
}

ASR::~ASR() {
	//cout << "Entering BAM::File::ASR destructor" << endl;

}

void ASR::Parse() {
	//cout << "Entering BAM::FileASR Parse" << endl;

	string line;
	bool peaks_found = false;
	bool normfactor_found = false;

	while (1) {
 		try {	
			getline(fs, line);
		}
		catch (...) {
			break;
		}
		//cout << line << endl;
		if (!peaks_found) {
			//process metadata
			stringstream ss;
			string identifier;

			ss << line;
			ss >> identifier;
			if (identifier.compare(0, 7, "$PEAKS:") == 0) {
				peaks_found = true;
				//cout << "peaks found!" << endl;
				//basically we ignore the line with the number of peaks
				getline(fs,line);
				continue;
			}
			else if (identifier == "Normfactor:") {
				ss >> normfactor;
				normfactor_found = true;
				cout << "Normfactor: " << normfactor << endl;
			}
		}
		else {
			int Z;
			int elem_line;
			double counts;
			double stddev;
			double chi;
			//double bg;
			double energy;
		
			stringstream ss;
			ss << line;
			ss >> Z >> elem_line >> energy >> counts >> stddev >> chi/* >> bg*/;
			//switch to xraylib's lines
			
			if (elem_line == 1) {
				elem_line = KA_LINE;
			}
			else if (elem_line == 2) {
				elem_line = LA_LINE;
			}
			else {
				throw BAM::Exception("BAM::File::ASR::Parse -> Invalid line identifier found");
			}
			data_asr.push_back(Data::ASR(Z, elem_line, counts, stddev, chi));
		}
	}

	if (!normfactor_found) {
		throw BAM::Exception("BAM::File::ASR::Parse -> Normfactor not found");
	}
	else if (!peaks_found) {
		throw BAM::Exception("BAM::File::ASR::Parse -> No $PEAKS: found");
	}
	else if (data_asr.size() == 0) {
		throw BAM::Exception("BAM::File::ASR::Parse -> No peaks found");
	}

	//cout << "Leaving BAM::FileASR Parse" << endl;

}
