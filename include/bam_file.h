#ifndef BAM_FILE_H
#define BAM_FILE_H

#include <fstream>
#include "bam_exception.h"

namespace BAM {
	namespace File {
		class File {
		private:
		protected:
			std::ifstream fs;
			std::string filename;
		public:
			//constructor
			File(std::string);
			//destructor
			virtual ~File();
			virtual void Open();
			virtual void Close();
			virtual void Parse() = 0;
			std::string GetFilename() {return filename;}
			void SetFilename(std::string new_filename) {
				if (new_filename != "")
					filename = new_filename;
				else
					throw BAM::Exception("BAM::File::File:SetFilename -> Invalid filename");
			}
		};
	}
}
#endif
