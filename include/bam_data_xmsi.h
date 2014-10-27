#ifndef BAM_DATA_XMSI_H
#define BAM_DATA_XMSI_H

#include <xmi_msim.h>
#include <vector>
#include <iterator>
#include <iostream>
#include <numeric>
#include <algorithm>
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
					Layer(double density_new, double thickness_new);
					Layer(string compound, double density_new, double thickness_new);
					Layer(string nistcompound, double thickness_new);
					void AddElement(int Z_new, double weight_new);
					void Normalize();
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
					void AddLayer(const Layer &layer_new);
					void SetReferenceLayer(int reference_layer_new);
					friend class BAM::File::XMSI;
			};
		}
	}
}
#endif
