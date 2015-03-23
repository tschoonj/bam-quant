#ifndef BAM_DATA_XRAYLIB_H
#define BAM_DATA_XRAYLIB_H

#include "bam_data_base.h"
#include "bam_exception.h"
#include <xraylib.h>
#include <string>
#include <vector>

namespace BAM {
	namespace Data {
		namespace Xraylib {
			//wrapper around xraylib's CompoundParser
			class Compound : public Base::Composition {
			protected:
				Compound() {}
				void SetComposition(struct compoundData *cd) {
					if (cd == 0) {
						throw BAM::Exception("BAM::Data::Xraylib::Compound::SetComposition -> Invalid chemical formula provided");
					}
					Base::Composition::SetComposition(cd->Elements, cd->massFractions, cd->nElements);
				}
			public:	
				Compound(std::string compound) {
					struct compoundData *cd = CompoundParser(compound.c_str());
					SetComposition(cd);
					FreeCompoundData(cd);
				}
			};
			class CompoundNIST : public Compound {
			private:
			protected:
				double density;
				CompoundNIST() {}
				CompoundNIST(double density) : density(density) {}
				void SetComposition(struct compoundDataNIST *cdn) {
					if (cdn == 0) {
						throw BAM::Exception("BAM::Data::Xraylib::CompoundNIST::SetComposition -> Invalid NIST compound provided");
					}
					Base::Composition::SetComposition(cdn->Elements, cdn->massFractions, cdn->nElements);
					density = cdn->density;
				}
			public:
				CompoundNIST(std::string compoundNIST) {
					struct compoundDataNIST *cdn = GetCompoundDataNISTByName(compoundNIST.c_str());
					SetComposition(cdn);
					FreeCompoundDataNIST(cdn);
				}
				CompoundNIST(int compoundNIST) {
					struct compoundDataNIST *cdn = GetCompoundDataNISTByIndex(compoundNIST);
					SetComposition(cdn);
					FreeCompoundDataNIST(cdn);
				}
				static std::vector<std::string> GetCompoundDataNISTList() {
					int nCompounds;
					char **compoundsNIST = ::GetCompoundDataNISTList(&nCompounds);
					std::vector<std::string> rv(compoundsNIST, compoundsNIST + nCompounds);
					//for (int i = 0 ; i < nCompounds ; i
					return rv;
				}
				void SetDensity(double density_new) {
					density = density_new;
				}
				double GetDensity() {
					return density;
				}
				friend std::ostream& operator<< (std::ostream &out, const CompoundNIST &c);
			};
		}
	}
}

#endif
