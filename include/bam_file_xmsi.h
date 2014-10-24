#ifndef BAM_FILE_XMSI_H
#define BAM_FILE_XMSI_H

#include <xmi_msim.h>
#include "bam_file.h"
#include "bam_data_xmsi.h"
#include <iostream>
#include <cstdlib>

namespace BAM {
	namespace File {
		class XMSI : public File {
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
				void ReplaceComposition(const BAM::Data::XMSI::Composition &composition_new);
				void SetOutputFile(string file) {
					if (input->general->outputfile)
						free(input->general->outputfile);
					input->general->outputfile = (char *) xmi_memdup(file.c_str(), sizeof(char)*(file.length()+1));
				}
				//copy constructor
				XMSI(const XMSI &xmsi) : File(xmsi.filename) {
					xmi_copy_input(xmsi.input, &input);
				}
				XMSI& operator= (const XMSI &xmsi) {
					if (this == &xmsi)
						return *this;
					if (input)
						xmi_free_input(input);
					xmi_copy_input(xmsi.input, &input);
					return *this;
				}
		};
	}
}
#endif
