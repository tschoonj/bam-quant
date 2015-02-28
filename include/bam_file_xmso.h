#ifndef BAM_FILE_XMSO_H
#define BAM_FILE_XMSO_H

#include <xmi_msim.h>
#include "bam_file.h"
#include <iostream>
#include <vector>

namespace BAM {
	namespace File {
		class XMSO : public File {
		private:
			struct xmi_output *output;
		public:
			XMSO() : File::File(""), output(0) {} 
			XMSO(std::string);
			XMSO(struct xmi_output *, std::string filename="");
			XMSO(const XMSO &xmso) : File(xmso.filename), output(0) {
				if (xmso.output)
					xmi_copy_output(xmso.output, &output);
			}
			XMSO& operator= (const XMSO &xmso) {
				if (this == &xmso)
					return *this;
				if (output)
					xmi_free_output(output);
				if (xmso.output)
					xmi_copy_output(xmso.output, &output);
				else
					output = 0;
				return *this;
			}
			~XMSO();
			void Open();
			void Close();
			void Parse();
			struct xmi_output *GetInternalCopy() {
				struct xmi_output *rv;
				xmi_copy_output(output, &rv);
				return rv;
			}
			std::vector<int> GetElements() {
				std::vector<int> elements;
				for (int i = 0 ; i < output->nvar_red_history ; i++) {
					elements.push_back(output->var_red_history[i].atomic_number);
				}	
				return elements;
			}
			double GetCountsForElement(int Z) {
				for (int i = 0 ; i < output->nvar_red_history ; i++) {
					if (Z == output->var_red_history[i].atomic_number)
						return output->var_red_history[i].total_counts;
				}
				throw BAM::Exception(std::string("BAM::File::XMSI::GetCountsForElement -> Requested element not found"));
				return 0;
			}
			double GetCountsForElementForLine(int Z, std::string line) {
				for (int i = 0 ; i < output->nvar_red_history ; i++) {
					if (Z == output->var_red_history[i].atomic_number) {
						for (int j = 0 ; j < output->var_red_history[i].n_lines ; j++) {
							if (line == output->var_red_history[i].lines[j].line_type)
								return output->var_red_history[i].lines[j].total_counts;
						}
					}
				}
				throw BAM::Exception(std::string("BAM::File::XMSI::GetCountsForElementForLine -> Requested element or line not found"));
				return 0;
			}
			double GetCountsForElementForLineForInteraction(int Z, std::string line, int interaction) {
				for (int i = 0 ; i < output->nvar_red_history ; i++) {
					if (Z == output->var_red_history[i].atomic_number) {
						for (int j = 0 ; j < output->var_red_history[i].n_lines ; j++) {
							if (line == output->var_red_history[i].lines[j].line_type) {
								for (int k = 0 ; k < output->var_red_history[i].lines[j].n_interactions ; k++) {
									if (interaction == output->var_red_history[i].lines[j].interactions[k].interaction_number) {
										return output->var_red_history[i].lines[j].interactions[k].counts;
									}
								}
							}
						}
					}
				}
				throw BAM::Exception(std::string("BAM::File::XMSI::GetCountsForElementForLineForInteraction -> Requested element, line or interaction not found"));
				return 0;
			}
			void Write();
			void Write(std::string filename);
		};
	}
}

#endif

