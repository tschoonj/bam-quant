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
			XMSO(string);
			XMSO(struct xmi_output *, string filename="");
			~XMSO();
			void Open();
			void Close();
			void Parse();
			struct xmi_output *GetInternalCopy() {
				struct xmi_output *rv;
				xmi_copy_output(output, &rv);
				return rv;
			}
			vector<int> GetElements() {
				vector<int> elements;
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
				throw BAM::Exception(string("BAM::File::XMSI::GetCountsForElement -> Requested element not found"));
				return 0;
			}
			double GetCountsForElementForLine(int Z, string line) {
				for (int i = 0 ; i < output->nvar_red_history ; i++) {
					if (Z == output->var_red_history[i].atomic_number) {
						for (int j = 0 ; j < output->var_red_history[i].n_lines ; j++) {
							if (line == output->var_red_history[i].lines[j].line_type)
								return output->var_red_history[i].lines[j].total_counts;
						}
					}
				}
				throw BAM::Exception(string("BAM::File::XMSI::GetCountsForElementForLine -> Requested element or line not found"));
				return 0;
			}
			double GetCountsForElementForLineForInteraction(int Z, string line, int interaction) {
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
				throw BAM::Exception(string("BAM::File::XMSI::GetCountsForElementForLineForInteraction -> Requested element, line or interaction not found"));
				return 0;
			}
			//void Write();
			//void Write(string filename);
			//friend std::ostream& operator<< (std::ostream &out, const XMSI &xmsi);
		};
	}
}

#endif

