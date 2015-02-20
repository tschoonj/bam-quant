#ifndef BAM_FILE_RXI_H
#define BAM_FILE_RXI_H


#include "bam_file_xmsi.h"
#include "bam_data_rxi.h"



namespace BAM {
	namespace File {
		namespace RXI {
			class Single: public File {
			private:
				BAM::File::XMSI xmimsim_input;
				BAM::Data::RXI::Sample sample;
				
			public:
				Single(string);
				void Open() {}
				void Close() {}
				void Parse();
				//void Write();
				//void Write(string filename);
			};
			class Multi: public File {
			private:
				BAM::File::XMSI xmimsim_input;
				vector<BAM::Data::RXI::Sample> sample;
				
			public:
				Multi(string);
				void Open() {}
				void Close() {}
				void Parse();
				//void Write();
				//void Write(string filename);
			};
		}
	}
}


#endif
