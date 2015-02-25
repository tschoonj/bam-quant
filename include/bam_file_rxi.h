#ifndef BAM_FILE_RXI_H
#define BAM_FILE_RXI_H


#include "bam_file_xmsi.h"
#include "bam_data_rxi.h"
#include <stdexcept>
#include <string>
#include <vector>



namespace BAM {
	namespace File {
		namespace RXI {
			class Single: public File::File {
			private:
				BAM::File::XMSI *xmimsim_input;
				BAM::Data::RXI::Sample sample;
				
			public:
				Single(std::string);
				//Single() : File::File(""), xmimsim_input(0), sample() {}
				Single(BAM::File::XMSI input, BAM::Data::RXI::Sample sample) : File::File(""), sample(sample) {
					xmimsim_input = new BAM::File::XMSI(input);
				}
				~Single() {
					if (xmimsim_input)
						delete xmimsim_input;
				}
				void Open();
				void Close();
				void Parse();
				void Write();
				void Write(std::string filename);
				Single(const Single &single) : File(single.filename), sample(single.sample) {
					if (single.xmimsim_input)
						xmimsim_input = new BAM::File::XMSI(*single.xmimsim_input);
					else 
						xmimsim_input = 0;
				}
				Single& operator= (const Single &single) {
					if (this == &single)
						return *this;
					if (xmimsim_input)
						delete xmimsim_input;
					if (single.xmimsim_input)
						xmimsim_input = new BAM::File::XMSI(*single.xmimsim_input);
					else 
						xmimsim_input = 0;
					sample = single.sample;
					return *this;
				}
				BAM::Data::RXI::Sample GetSample() {
					return sample;
				}
			};
			class Multi: public File {
			private:
				BAM::File::XMSI *xmimsim_input;
				std::vector<BAM::Data::RXI::Sample> samples;
				
			public:
				Multi(std::string);
				Multi(BAM::File::XMSI input, std::string filename = "") : File::File(filename) {
					xmimsim_input = new BAM::File::XMSI(input);
				}
				~Multi() {
					if (xmimsim_input)
						delete xmimsim_input;
				}
				void Open();
				void Close();
				void Parse();
				void Write();
				void Write(std::string filename);
				Multi(const Multi &multi) : File(multi.filename), samples(multi.samples) {
					if (multi.xmimsim_input)
						xmimsim_input = new BAM::File::XMSI(*multi.xmimsim_input);
					else 
						xmimsim_input = 0;
				}
				Multi& operator= (const Multi &multi) {
					if (this == &multi)
						return *this;
					if (xmimsim_input)
						delete xmimsim_input;
					if (multi.xmimsim_input)
						xmimsim_input = new BAM::File::XMSI(*multi.xmimsim_input);
					else 
						xmimsim_input = 0;
					samples = multi.samples;
					return *this;
				}
				BAM::Data::RXI::Sample GetSample(int index) {
					try {
						return samples.at(index);
					}
					catch (std::out_of_range &e) {
						throw BAM::Exception(std::string("BAM::File::RXI::Multi::GetSample: ")+e.what());
					} 
				}
				void AddSample(BAM::Data::RXI::Sample sample) {
					samples.push_back(sample);
				}
				Single GetSingle(int index) {
					return Single(*xmimsim_input, GetSample(index));
				}
			};
		}
	}
}


#endif
