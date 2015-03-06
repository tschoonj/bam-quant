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
			class Common: public File::File {
			//abstract class since it doesnt implement Parse!!!
			private:
			protected:
				BAM::File::XMSI *xmimsim_input;
				Common(BAM::File::XMSI input, std::string filename="") : File::File(filename) {
					xmimsim_input = new BAM::File::XMSI(input);
				}
				Common(std::string filename) : File::File(filename), xmimsim_input(0) {}
			public:
				virtual ~Common() {
					if (xmimsim_input)
						delete xmimsim_input;
				}
				BAM::File::XMSI GetFileXMSI() {
					return *xmimsim_input;
				}
			
			};
			class Single: public Common {
			private:
				BAM::Data::RXI::Sample sample;
			public:
				Single(std::string);
				//Single() : File::File(""), xmimsim_input(0), sample() {}
				Single(BAM::File::XMSI input, BAM::Data::RXI::Sample sample) : Common(input), sample(sample) {}
				void Open();
				void Close();
				void Parse();
				void Write();
				void Write(std::string filename);
				Single(const Single &single) : Common(*single.xmimsim_input, single.filename), sample(single.sample) {}
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
					SetFilename(single.GetFilename());
					return *this;
				}
				BAM::Data::RXI::Sample GetSample() {
					return sample;
				}
			};
			class Multi: public Common {
			private:
				std::vector<BAM::Data::RXI::Sample> samples;
			public:
				Multi(std::string);
				Multi(BAM::File::XMSI input, std::string filename = "") : Common(input, filename) {}
				void Open();
				void Close();
				void Parse();
				void Write();
				void Write(std::string filename);
				Multi(const Multi &multi) : Common(*multi.xmimsim_input, multi.filename), samples(multi.samples) {}
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
					SetFilename(multi.GetFilename());
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
			Common* Parse(std::string filename);
		}
	}
}


#endif
