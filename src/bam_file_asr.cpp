#include "bam_file_asr.h"
#include "bam_exception.h"
#include <iostream>
#include <sstream>

using namespace BAM;
using namespace BAM::File;
using namespace std;

ASR::ASR(string filename) : File::File(filename) {
	cout << "Entering BAM::FileASR constructor" << endl;
	if (fs) {
		this->Parse();
	}
	this->Close();
	cout << "Leaving BAM::FileASR constructor" << endl;
}

ASR::~ASR() {
	cout << "Entering BAM::FileASR destructor" << endl;

}

void ASR::Parse() {
	cout << "Entering BAM::FileASR Parse" << endl;

	string line;
	bool peaks_found = false;
	bool normfactor_found = false;

 	try {	
		while (getline(fs, line)) {
			//cout << line << endl;
			if (!peaks_found) {
				//process metadata
				stringstream ss;
				string identifier;

				ss << line;
				ss >> identifier;
				if (identifier.compare(0, 7, "$PEAKS:") == 0) {
					peaks_found = true;
					cout << "peaks found!" << endl;
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
				double bg;
			
				stringstream ss;
				ss << line;
				ss >> Z >> elem_line >> counts >> stddev >> chi >> bg;
				data_asr.push_back(Data::ASR(Z, elem_line, counts, stddev, chi, bg));
			}
		}
	}
	catch (...) {}

	if (!normfactor_found) {
		throw Exception("Normfactor not found in "+filename);
	}
	else if (!peaks_found) {
		throw Exception("No $PEAKS: found in "+filename);
	}
	else if (data_asr.size() == 0) {
		throw Exception("No peaks found in "+filename);
	}

	cout << "Leaving BAM::FileASR Parse" << endl;

}
