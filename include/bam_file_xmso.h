#ifndef BAM_FILE_XMSO_H
#define BAM_FILE_XMSO_H

#include <xmi_msim.h>
#include "bam_file.h"
#include <iostream>

namespace BAM {
	namespace File {
		class XMSO : public File {
			private:
				struct xmi_output *output;
			public:
				XMSO(string);
				XMSO(struct xmi_output *, string);
				~XMSO();
				void Open();
				void Close();
				void Parse();
				//void Write();
				//void Write(string filename);
				//friend std::ostream& operator<< (std::ostream &out, const XMSI &xmsi);
		};
	}
}
#endif

