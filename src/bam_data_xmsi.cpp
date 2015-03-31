#include "config.h"
#include "bam_data_xmsi.h"
#include <xraylib.h>
#include <xmi_msim.h>

using namespace BAM;
using namespace BAM::Data;
using namespace BAM::Data::XMSI;


//Layer stuff
//three constructors
Layer::Layer(double density_new, double thickness_new) : Xraylib::CompoundNIST(density_new), thickness(thickness_new) {
	//NO elements added here -> invalid layer will result!!!
	if (thickness <= 0.0) {
		throw BAM::Exception("BAM::Data::XMSI::Layer::Layer -> Cannot construct layer with negative thickness");
	}
	else if (density <= 0.0) {
		throw BAM::Exception("BAM::Data::XMSI::Layer::Layer -> Cannot construct layer with negative density");
	}
}

Layer::Layer(std::string compound, double density_new, double thickness_new) : Xraylib::CompoundNIST(density_new), thickness(thickness_new) {
	if (thickness <= 0.0) {
		throw BAM::Exception("BAM::Data::XMSI::Layer::Layer -> Cannot construct layer with negative thickness");
	}
	else if (density <= 0.0) {
		throw BAM::Exception("BAM::Data::XMSI::Layer::Layer -> Cannot construct layer with negative density");
	}

	struct compoundData *cd;
	cd = CompoundParser(compound.c_str());
	SetComposition(cd);

	FreeCompoundData(cd);

}

Layer::Layer(std::string nistcompound, double thickness_new) : thickness(thickness_new) {
	if (thickness <= 0.0) {
		throw BAM::Exception("BAM::Data::XMSI::Layer::Layer -> Cannot construct layer with negative thickness");
	}

	struct compoundDataNIST *cd;
	cd = GetCompoundDataNISTByName(nistcompound.c_str());
	SetComposition(cd);

	FreeCompoundDataNIST(cd);

}


namespace BAM {
	namespace Data {
		namespace XMSI {
			std::ostream& operator<< (std::ostream &out, const Layer &layer) {
				const BAM::Data::Xraylib::CompoundNIST &c_base = layer;
				out << c_base;
                		out << "Thickness:" << layer.thickness << std::endl;
				return out;
			}
		}
	}
}


//Composition stuff
void Composition::AddLayer(const Layer &layer_new) {
	//ensure number of elements is greater than one
	if (layer_new.GetNumberOfElements() == 0) {
		throw BAM::Exception("BAM::Data::XMSI::Composition::AddLayer -> Cannot add layer with zero elements");
	}
	Layer layer_new_normalized(layer_new);
	layer_new_normalized.Normalize();
	layers.push_back(layer_new_normalized);
}

void Composition::SetReferenceLayer(unsigned int reference_layer_new) {
	if (reference_layer_new < 1 || reference_layer_new > layers.size()) {
		throw BAM::Exception("BAM::Data::XMSI::Composition::SetReferenceLayer -> Invalid reference layer detected");
	}	
	reference_layer = reference_layer_new;
}

void Composition::ReplaceLayer(const Layer &layer_new, unsigned int layer_index) {
	//make sure layer_index is valid!
	if (layer_index < 1 || layer_index > layers.size())
		throw BAM::Exception("BAM::Data::XMSI::Composition::ReplaceLayer -> Invalid layer_index detected");

	Layer layer_new_normalized(layer_new);
	layer_new_normalized.Normalize();
	layers[layer_index-1] = layer_new_normalized;
}

void Excitation::EnsureMonochromaticExcitation() {
	//all this function does is throw an exception if the input is not monochromatic
	if (discrete.size() != 1 || continuous.size() != 0)
		throw BAM::Exception("BAM::Data::XMSI::Excitation::EnsureMonochromaticExcitation -> input excitation must have exactly one discrete component and zero continuous components.");
}

