#include "bam_data_xmsi.h"
#include <xraylib.h>

using namespace BAM;
using namespace BAM::Data;
using namespace BAM::Data::XMSI;


//Layer stuff
//three constructors
Layer::Layer(double density_new, double thickness_new) : density(density_new), thickness(thickness_new) {
	//NO elements added here -> invalid layer will result!!!
	if (thickness <= 0.0) {
		throw BAM::Exception("BAM::Data::XMSI::Layer::Layer -> Cannot construct layer with negative thickness");
	}
	else if (density <= 0.0) {
		throw BAM::Exception("BAM::Data::XMSI::Layer::Layer -> Cannot construct layer with negative density");
	}
}

Layer::Layer(std::string compound, double density_new, double thickness_new) : density(density_new), thickness(thickness_new) {
	if (thickness <= 0.0) {
		throw BAM::Exception("BAM::Data::XMSI::Layer::Layer -> Cannot construct layer with negative thickness");
	}
	else if (density <= 0.0) {
		throw BAM::Exception("BAM::Data::XMSI::Layer::Layer -> Cannot construct layer with negative density");
	}

	struct compoundData *cd;
	cd = CompoundParser(compound.c_str());
	if (cd == 0) {
		throw BAM::Exception("BAM::Data::XMSI::Layer::Layer -> Invalid chemical compound could not be parsed");
	}
	Z.assign(cd->Elements, cd->Elements+cd->nElements);
	weight.assign(cd->massFractions, cd->massFractions+cd->nElements);

	FreeCompoundData(cd);

}

Layer::Layer(std::string nistcompound, double thickness_new) : thickness(thickness_new) {
	if (thickness <= 0.0) {
		throw BAM::Exception("BAM::Data::XMSI::Layer::Layer -> Cannot construct layer with negative thickness");
	}

	struct compoundDataNIST *cd;
	cd = GetCompoundDataNISTByName(nistcompound.c_str());
	if (cd == 0) {
		throw BAM::Exception("BAM::Data::XMSI::Layer::Layer -> Unknown NIST compound");
	}
	Z.assign(cd->Elements, cd->Elements+cd->nElements);
	weight.assign(cd->massFractions, cd->massFractions+cd->nElements);
	density = cd->density;

	FreeCompoundDataNIST(cd);

}

void Layer::AddElement(int Z_new, double weight_new) {
	//ensure Z isn't already present!
	std::vector<int>::iterator it = std::find(Z.begin(), Z.end(), Z_new);
	if (it == Z.end()) {
		//not present -> add it to the list
		Z.push_back(Z_new);
		weight.push_back(weight_new);
	}
	else {
		int index = std::distance(Z.begin(), it);	
		weight[index] += weight_new;
	}
}

void Layer::Normalize() {
	double sum = std::accumulate(weight.begin(), weight.end(), 0.0);
	std::transform(weight.begin(), weight.end(), weight.begin(), std::bind1st(std::multiplies<double>(),1.0/sum));
}

//Composition stuff
void Composition::AddLayer(const Layer &layer_new) {
	//ensure number of elements is greater than one
	if (layer_new.Z.size() == 0) {
		throw BAM::Exception("BAM::Data::XMSI::Composition::AddLayer -> Cannot add layer with zero elements");
	}
	struct xmi_layer layer;
	layer.Z = (int *) xmi_memdup(&layer_new.Z[0], sizeof(int)*layer_new.Z.size());
	layer.weight = (double *) xmi_memdup(&layer_new.weight[0], sizeof(double)*layer_new.Z.size());
	xmi_scale_double(layer.weight, layer_new.Z.size(), 1.0/xmi_sum_double(layer.weight, layer_new.Z.size()));
	layer.density = layer_new.density;
	layer.thickness = layer_new.thickness;
	layer.n_elements = layer_new.Z.size();
	layers.push_back(layer);
}

void Composition::SetReferenceLayer(int reference_layer_new) {
	if (reference_layer_new < 1 || reference_layer_new > layers.size()) {
		throw BAM::Exception("BAM::Data::XMSI::Composition::SetReferenceLayer -> Invalid reference layer detected");
	}	
	reference_layer = reference_layer_new;
}
