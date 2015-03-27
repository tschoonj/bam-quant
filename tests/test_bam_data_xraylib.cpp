#include "bam_data_xraylib.h"
#include <iostream>
#include "bam_exception.h"
#include <algorithm>
#include <iterator>

int main(int argc, char **argv) {

	BAM::Data::Xraylib::Compound *compound;
	BAM::Data::Xraylib::CompoundNIST *compoundNIST;



	try {
		compound = new BAM::Data::Xraylib::Compound("CaSO4");
		std::cout << *compound << std::endl; 

		compoundNIST = new BAM::Data::Xraylib::CompoundNIST("Calcium Tungstate");
		std::cout << *compoundNIST << std::endl; 

		//try summing them
		BAM::Data::Base::Composition compound_sum = *compound + *compoundNIST;
		std::cout << compound_sum << std::endl; 
	
		std::vector<std::string> compoundNISTnames = BAM::Data::Xraylib::CompoundNIST::GetList();
		std::copy(compoundNISTnames.begin(), compoundNISTnames.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
	
		delete compound;
		delete compoundNIST;
	}
	catch (BAM::Exception &e) {
		std::cerr << "BAM::Exception caught: " << e.what() << std::endl; 
		return 1;
	}
	catch (std::exception &e) {
		std::cerr << "std::exception caught: " << e.what() << std::endl; 
		return 1;
	}
	catch (...) {
		std::cerr << "Unknown exception caught: " << std::endl; 
		return 1;
	}

	return 0;
}
