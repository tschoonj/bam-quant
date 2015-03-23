#include "bam_data_base.h"
#include <xraylib.h>

namespace BAM {
	namespace Data {
		namespace Base {
			Composition operator+(const Composition &c1, const Composition &c2) {
				Composition rv(c1);
				for (std::map<int,double>::const_iterator it = c2.composition.begin(); it != c2.composition.end() ; ++it) {
					if (rv.composition.find(it->first) == rv.composition.end())
						rv.composition[it->first] = it->second;
					else
						rv.composition[it->first] += it->second;
				}
				return rv;
			}
			std::ostream& operator<< (std::ostream &out, const Composition &c) {
				out << "Composition" << std::endl;
				for (std::map<int,double>::const_iterator it = c.composition.begin(); it != c.composition.end() ; ++it) {
					char *element = AtomicNumberToSymbol(it->first);
                        		out << element << " -> weight: " << it->second << std::endl;
					xrlFree(element);
                		}
				return out;
			}
		}
	}
}

using namespace BAM::Data::Base;

void Composition::SetComposition(int *Z_new, double *weight_new, int n_elements) {
	for (int i = 0 ; i < n_elements ; i++) {
		AddElement(Z_new[i], weight_new[i]);
	}
}

void Composition::AddElement(int Z_new, double weight_new) {
	if (Z_new < 1 || Z_new > 95) 
		throw BAM::Exception("BAM::Data::Base::Composition::AddElement -> Z_new must be a number between 1 and 94");

	if (weight_new <= 0.0)
		throw BAM::Exception("BAM::Data::Base::Composition::AddElement -> weight_new must be a greater than 0");

	if (composition.find(Z_new) != composition.end()) {
		composition[Z_new] += weight_new;
	}
	else {
		composition[Z_new] = weight_new;
	}
}

void Composition::AddElement(std::string Z_new, double weight_new) {
	int Z = SymbolToAtomicNumber((char *) Z_new.c_str());
	if (Z == 0) {
		throw BAM::Exception("BAM::Data::Base::Composition::AddElement -> Z_new must be a chemical symbol");
	}
	AddElement(Z, weight_new);
}

void Composition::Normalize() {
	
	double sum(0.0);
	
	for (std::map<int,double>::iterator it = composition.begin() ; it != composition.end() ; ++it)
		sum += it->second;
	
	for (std::map<int,double>::iterator it = composition.begin() ; it != composition.end() ; ++it)
		it->second /= sum;
}
