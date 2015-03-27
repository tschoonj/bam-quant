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

using namespace BAM::Data;


Base::Composition *BAM::Data::Xraylib::Parse(std::string compound) {
	Base::Composition *rv;

	try {
		rv = new BAM::Data::Xraylib::CompoundNIST(compound);
	}
	catch (BAM::Exception &e) {
		try {
			rv = new BAM::Data::Xraylib::Compound(compound);
		}
		catch (BAM::Exception &e) {
			throw BAM::Exception("BAM::Data::Xraylib::Parse -> could not parse compound");
		}
		catch (...) {
			throw BAM::Exception("BAM::Data::Xraylib::Parse -> unknown exception caught");
		}
	}
	catch (...) {
		throw BAM::Exception("BAM::Data::Xraylib::Parse -> unknown exception caught");
	}
	return rv;
}
