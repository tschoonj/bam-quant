#ifndef BAM_FILE_XMSI_H
#define BAM_FILE_XMSI_H

#include <xmi_msim.h>
#include "bam_file.h"
#include <iostream>

namespace BAM {
	namespace File {
		class XMSI : public File{
			private:
				struct xmi_input *input;
			public:
				XMSI(string);
				XMSI(struct xmi_input *, string);
				~XMSI();
				void Open();
				void Close();
				void Parse();
				void Write();
				void Write(string filename);
				friend std::ostream& operator<< (std::ostream &out, const XMSI &xmsi);
				//void ReplaceComposition()
		};
	}
}
#endif
