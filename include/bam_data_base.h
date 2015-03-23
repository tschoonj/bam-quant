#ifndef BAM_DATA_BASE_H
#define BAM_DATA_BASE_H

#include <vector>
#include <map>
#include <iostream>
#include <xraylib.h>
#include <cmath>
#include "bam_exception.h"

namespace BAM {
	namespace Data {
		namespace Base {
			class Composition {
			private:
			protected:
				std::map <int,double> composition;
				Composition() {}
				void SetComposition(int *Z_new, double *weight_new, int n_elements);
			public:
				void AddElement(int Z_new, double weight_new);
				void AddElement(std::string Z_new, double weight_new);
				size_t GetNumberOfElements() const {
					return composition.size();
				}
				void Normalize();
				void RemoveElements() {
					composition.clear();
				}
				std::map<std::string,double> GetZandWeightMap() {
					std::map<std::string,double> rv;
					for (std::map<int,double>::iterator it = composition.begin() ; it != composition.end() ; ++it) {
						char *symbol = AtomicNumberToSymbol(it->first);
						std::string element(symbol);
						xrlFree(symbol);
						rv[element] = it->second;
					}
					return rv;
				}
				double Mu(double energy) {
					if (energy <= 0.0)
						throw BAM::Exception("BAM::Data::Base::Composition::Mu -> energy must be greater than 0");
					double rv(0.0);
					for (std::map<int,double>::iterator it = composition.begin() ; it != composition.end() ; ++it) {
						rv += CS_Total_Kissel(it->first, energy)*it->second;
					}

					return rv;
				}
				double Chi(double energy_exc, double energy_xrf, double angle_exc = M_PI_4, double angle_xrf = M_PI_4) {
					double rv(0.0);
					if (energy_xrf >= energy_exc)
						throw BAM::Exception("BAM::Data::Base::Composition::Chi -> energy_xrf must be less than energy_exc");	
					rv += Mu(energy_exc)/sin(angle_exc);
					rv += Mu(energy_xrf)/sin(angle_xrf);
					return rv;
				}
				friend Composition operator+(const Composition &c1, const Composition &c2);
				friend std::ostream& operator<< (std::ostream &out, const Composition &c);
			};
		}
	}
}
#endif
