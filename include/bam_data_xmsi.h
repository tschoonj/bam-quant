#ifndef BAM_DATA_XMSI_H
#define BAM_DATA_XMSI_H

#include <xmi_msim.h>
#include <vector>
#include <iterator>
#include <iostream>
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <map>
#include <xraylib.h>
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
				Layer(struct xmi_layer layer) : density(layer.density), thickness(layer.thickness) {
					Z.assign(layer.Z, layer.Z+layer.n_elements);
					weight.assign(layer.weight, layer.weight+layer.n_elements);
				}
			public:
				Layer(double density_new, double thickness_new);
				Layer(std::string compound, double density_new, double thickness_new);
				Layer(std::string nistcompound, double thickness_new);
				void AddElement(int Z_new, double weight_new);
				void AddElement(std::string Z_new, double weight_new);
				void Normalize();
				void RemoveElements() {
					Z.clear();
					weight.clear();
				}
				std::map<std::string,double> GetZandWeightMap() {
					std::map<std::string,double> rv;
					for (int i = 0 ; i < Z.size() ; i++) {
						char *symbol = AtomicNumberToSymbol(Z[i]);
						std::string element(symbol);
						xrlFree(symbol);
						rv[element] = weight[i];
					}
					return rv;
				}
				friend class Composition;
				friend std::ostream& operator<< (std::ostream &out, const Layer &layer);
			};
			class Composition {
			private:
				int reference_layer;
				std::vector <struct xmi_layer> layers;
				Composition(struct xmi_composition *composition) {
					reference_layer = composition->reference_layer;
					struct xmi_composition *composition_copy;
					xmi_copy_composition(composition, &composition_copy);
					layers.assign(composition_copy->layers, composition_copy->layers+composition_copy->n_layers);
					xmi_free(composition_copy->layers);
					xmi_free(composition_copy);

				}
			public:
				Composition() : reference_layer(0) {}
				~Composition() {
					for (std::vector<struct xmi_layer>::iterator it = layers.begin() ; it != layers.end() ; ++it) {
						xmi_free_layer(&(*it));	
					}
				}
				void AddLayer(const Layer &layer_new);
				void ReplaceLayer(const Layer &layer_new, int layer_index);
				void SetReferenceLayer(int reference_layer_new);
				Layer GetLayer(int layer_index) {
					try {
						return Layer(layers.at(layer_index-1));
					}
					catch (std::out_of_range &e) {
						throw BAM::Exception(std::string("BAM::Data:XMSI::GetLayer: ")+e.what());
					} 
				}
				friend class BAM::File::XMSI;
			};
		}
	}
}
#endif
