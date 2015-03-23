#ifndef BAM_FILE_ASR_H
#define BAM_FILE_ASR_H

#include "bam_file.h"
#include "bam_data_asr.h"
#include "bam_exception.h"
#include <vector>

namespace BAM {
	namespace File {
		class ASR : public File {
		private:
			std::vector<Data::ASR> data_asr;
			double normfactor;
			bool keep_negative_counts;
			void Parse();
		public:
			ASR(std::string, bool keep_negative_counts = false);
			ASR(double normfactor);
			~ASR();
			int GetNPeaks() {
				return static_cast<int> (data_asr.size());
			}
			Data::ASR GetData(int i) {
				if (i < 0 || i >= (int) data_asr.size()) {
					throw BAM::Exception("Index out of bounds in BAM::File::GetData");
				}			
				return data_asr[i];
			}
			double GetNormfactor() {
				return normfactor;
			}
		};
	}
}

#endif
