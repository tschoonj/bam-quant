#ifndef BAM_DATA_XMSI_H
#define BAM_DATA_XMSI_H

#include <xmi_msim.h>
#include <vector>
#include <iterator>
#include <iostream>
#include <numeric>
#include "bam_file_xmsi.h"



namespace BAM {
	namespace File {
		class XMSI;
	}
	namespace Data {
		namespace XMSI {
			class Layer {
				private:
					std::vector <int> Z;
					std::vector <double> weight;
					double density;
					double thickness;
				public:
					Layer(double density_new, double thickness_new) : density(density_new), thickness(thickness_new) {
						if (thickness <= 0.0) {
							throw BAM::Exception("BAM::Data::XMSI::Layer::Layer -> Cannot construct layer with negative thickness");
						}
						else if (density <= 0.0) {
							throw BAM::Exception("BAM::Data::XMSI::Layer::Layer -> Cannot construct layer with negative density");
						}
					}
					void AddElement(int Z_new, double weight_new) {
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
					void Normalize() {
						double sum = std::accumulate(weight.begin(), weight.end(), 0.0);
						std::transform(weight.begin(), weight.end(), weight.begin(), std::bind1st(std::multiplies<double>(),1.0/sum));
					}
					friend class Composition;
			};
			class Composition {
				private:
					int reference_layer;
					std::vector <struct xmi_layer> layers;
				public:
					Composition() : reference_layer(0) {}
					~Composition() {
						for (std::vector<struct xmi_layer>::iterator it = layers.begin() ; it != layers.end() ; ++it) {
							xmi_free_layer(&(*it));	
						}
					}
					void AddLayer(const Layer &layer_new) {
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
					void SetReferenceLayer(int reference_layer_new) {
						if (reference_layer_new < 1 || reference_layer_new > layers.size()) {
							throw BAM::Exception("BAM::Data::XMSI::Composition::SetReferenceLayer -> Invalid reference layer detected");
						}	
						reference_layer = reference_layer_new;
					}
					friend class BAM::File::XMSI;
			};
		}
	}
}
#endif
