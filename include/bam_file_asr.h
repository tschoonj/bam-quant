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
			ASR(double normfactor = 0.0);
			~ASR();
			int GetNPeaks() {
				return static_cast<int> (data_asr.size());
			}
			ASR(const ASR &asr) : File(asr.filename), data_asr(asr.data_asr), normfactor(asr.normfactor), keep_negative_counts(asr.keep_negative_counts) {}
			ASR& operator= (const ASR &asr) {
				if (this == &asr)
					return *this;
				data_asr = asr.data_asr;
				normfactor = asr.normfactor;
				keep_negative_counts = asr.keep_negative_counts;
				filename = asr.filename;
				return *this;
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
