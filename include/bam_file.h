#ifndef BAM_FILE_H
#define BAM_FILE_H

#include <fstream>
#include "bam_exception.h"

using namespace std;

namespace BAM {
	namespace File {
		class File {
			private:
			protected:
				ifstream fs;
				string filename;
			public:
				//constructor
				File(string);
				//destructor
				virtual ~File();
				virtual void Open();
				virtual void Close();
				virtual void Parse() = 0;
				string GetFilename() {return filename;}
				void SetFilename(string new_filename) {
					if (filename != "")
						filename = new_filename;
					else
						throw BAM::Exception("BAM::File::File:SetFilename -> Invalid filename");
				}
		};
	}
}
#endif
