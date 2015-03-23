#include <bam_data_xraylib.h>



namespace BAM {
	namespace Data {
		namespace Xraylib {
			std::ostream& operator<< (std::ostream &out, const CompoundNIST &c) {
				const BAM::Data::Base::Composition &c_base = c;
				out << c_base;
				out << "Density: " << c.density << std::endl; 

				return out;
			}
		}
	}
}
