#ifndef BAM_FILE_RXI_H
#define BAM_FILE_RXI_H


#include "bam_file_xmsi.h"
#include "bam_data_rxi.h"
#include <stdexcept>
#include <string>
#include <vector>
#include <map>



namespace BAM {
	namespace Job {
		class Quant;
	}
	namespace File {
		namespace RXI {
			class Common: public File::File {
			//abstract class since it doesnt implement Parse!!!
			private:
			protected:
				std::map<double,BAM::File::XMSI> xmimsim_input;
				//perhaps a constructor that takes a vector of a map would be useful??
				Common(const std::map<double,BAM::File::XMSI> &input, std::string filename="") : File::File(""), xmimsim_input(input) {
					if (!filename.empty())
						SetFilename(filename);
				}
				Common(std::string filename) : File::File(filename) {}
			public:
				virtual ~Common() {
				}
				BAM::File::XMSI GetFileXMSI(double excitation_energy) {
					if (xmimsim_input.find(excitation_energy) == xmimsim_input.end())
						throw BAM::Exception("BAM::File::RXI::Common::GetFileXMSI -> Invalid excitation_energy requested");
					return xmimsim_input[excitation_energy];
				}
				friend class BAM::Job::Quant;	
			};
			class Single: public Common {
			private:
				BAM::Data::RXI::Sample sample;
			public:
				Single(std::string);
				//Single() : File::File(""), xmimsim_input(0), sample() {}
				Single(const std::map<double,BAM::File::XMSI> &input, BAM::Data::RXI::Sample sample) : Common(input), sample(sample) {}
				void Open();
				void Close();
				void Parse();
				void Write();
				void Write(std::string filename);
				Single(const Single &single) : Common(single.xmimsim_input), sample(single.sample) {}
				Single& operator= (const Single &single) {
					if (this == &single)
						return *this;
					xmimsim_input = single.xmimsim_input;
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
				Multi(const std::map<double,BAM::File::XMSI> &input, std::string filename = "") : Common(input, filename) {}
				void Open();
				void Close();
				void Parse();
				void Write();
				void Write(std::string filename);
				Multi(const Multi &multi) : Common(multi.xmimsim_input, multi.filename), samples(multi.samples) {}
				Multi& operator= (const Multi &multi) {
					if (this == &multi)
						return *this;
					xmimsim_input = multi.xmimsim_input;
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
				size_t GetNumberOfSamples() {
					return samples.size();
				}
				void AddSample(BAM::Data::RXI::Sample sample) {
					samples.push_back(sample);
				}
				Single GetSingle(int index) {
					return Single(xmimsim_input, GetSample(index));
				}
			};
			Common* Parse(std::string filename);
		}
	}
}


#endif
