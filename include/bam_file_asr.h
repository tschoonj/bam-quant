#ifndef BAM_FILE_ASR_H
#define BAM_FILE_ASR_H

#include <bam_file.h>
#include <bam_data_asr.h>
#include <vector>

using namespace std;

namespace BAM {
	class FileASR : public File {
		private:
			vector<DataASR> data_asr;
			double normfactor;
		public:
			FileASR(string);
			~FileASR();
			void Parse();
	};
}

#endif
