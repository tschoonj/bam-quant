#ifndef BAM_FILE_ASR_H
#define BAM_FILE_ASR_H

#include "bam_file.h"
#include "bam_data_asr.h"
#include <vector>

using namespace std;

namespace BAM {
	namespace File {
		class ASR : public File {
			private:
				vector<Data::ASR> data_asr;
				double normfactor;
			public:
				ASR(string);
				~ASR();
				void Parse();
		};
	}
}

#endif
