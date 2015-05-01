#ifndef BAM_JOB_QUANT_H
#define BAM_JOB_QUANT_H

#include "bam_job_xmsi.h"
#include "bam_data_rxi.h"
#include "bam_file_rxi.h"
#include <map>

#define BAM_QUANT_MAX_ITERATIONS 100
#define BAM_QUANT_CONV_THRESHOLD 0.01

namespace BAM {
	namespace Job {
		class Quant {
		private:
			//std::map<std::string,BAM::File::XMSO,bool(*)(std::string,std::string)> pure_map(element_comp);
			std::map<std::string,BAM::File::XMSO,bool(*)(std::string,std::string)> pure_map;
			struct xmi_main_options options;
			BAM::File::RXI::Common *common;

			static bool element_comp (std::string lhs, std::string rhs) {return SymbolToAtomicNumber(const_cast<char *> (lhs.c_str())) < SymbolToAtomicNumber(const_cast<char *> (rhs.c_str()));}
			static bool rxi_match(std::pair<std::string,double> rxi_rel) {return rxi_rel.second < BAM_QUANT_CONV_THRESHOLD;}
			
			double calculate_rxi(std::string element, std::map<double,BAM::File::XMSO> &sample_output, BAM::Data::RXI::SingleElement single_element);

			void SimulatePure(BAM::Data::RXI::SingleElement single_element);
			BAM::File::XMSO SimulateSample(BAM::Data::RXI::Sample &sample);
		public:
			Quant(BAM::File::RXI::Common *common, std::string outputfile, struct xmi_main_options options);
			





		};
	}
}
#endif
