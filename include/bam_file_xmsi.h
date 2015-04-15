#ifndef BAM_FILE_XMSI_H
#define BAM_FILE_XMSI_H

#include <xmi_msim.h>
#include "bam_file.h"
#include "bam_data_xmsi.h"
#include <iostream>
#include <cstdlib>

//class BAM::Data::XMSI::Composition;

namespace BAM {
	namespace Data {
		namespace XMSI {
			//forward declaration
			class Composition;
			class Geometry;
			class Excitation;
		}
	}
	namespace File {
		class XMSI : public File {
		private:
			struct xmi_input *input;
		public:
			XMSI() : File(""), input(0) {}
			XMSI(std::string);
			XMSI(struct xmi_input *, std::string filename="");
			~XMSI();
			void Open();
			void Close();
			void Parse();
			void Write();
			void Write(std::string filename);
			struct xmi_input *GetInternalCopy() {
				struct xmi_input *rv;
				xmi_copy_input(input, &rv);
				//do not forget to free this memory afterwards with xmi_free_input
				return rv;
			}
			struct xmi_input *GetInternalPointer() {
				//this is dangerous!!!
				return input;
			}
			friend std::ostream& operator<< (std::ostream &out, const XMSI &xmsi);
			BAM::Data::XMSI::Composition GetComposition();
			BAM::Data::XMSI::Excitation  GetExcitation();
			BAM::Data::XMSI::Geometry    GetGeometry();
			void ReplaceComposition(const BAM::Data::XMSI::Composition &composition_new);
			void SetOutputFile(std::string file) {
				if (input->general->outputfile)
					free(input->general->outputfile);
				input->general->outputfile = (char *) xmi_memdup(file.c_str(), sizeof(char)*(file.length()+1));
			}
			std::string GetOutputFile() {
				return std::string(input->general->outputfile);
			}
			//copy constructor
			XMSI(const XMSI &xmsi) : File(""), input(0) {
				if (xmsi.input)
					xmi_copy_input(xmsi.input, &input);
				filename = xmsi.filename;
			}
			XMSI& operator= (const XMSI &xmsi) {
				if (this == &xmsi)
					return *this;
				if (input)
					xmi_free_input(input);
				if (xmsi.input)
					xmi_copy_input(xmsi.input, &input);
				else
					input = 0;
				filename = xmsi.filename;
				return *this;
			}
		};
	}
}
#endif
