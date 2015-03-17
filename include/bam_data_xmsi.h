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
#include <xraylib.h>
#include <cmath>



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
					for (unsigned int i = 0 ; i < Z.size() ; i++) {
						char *symbol = AtomicNumberToSymbol(Z[i]);
						std::string element(symbol);
						xrlFree(symbol);
						rv[element] = weight[i];
					}
					return rv;
				}
				double Mu(double energy) {
					double rv(0.0);
					for (unsigned int i = 0 ; i < Z.size() ; i++)
						rv += CS_Total_Kissel(Z[i], energy)*weight[i];

					return rv;
				}
				double Chi(double energy_exc, double energy_xrf, double angle_exc = M_PI_4, double angle_xrf = M_PI_4) {
					double rv(0.0);
					if (energy_xrf >= energy_exc)
						throw BAM::Exception("BAM::Data::XMSI::Chi -> energy_xrf must be less than energy_exc");	
					rv += Mu(energy_exc)/sin(angle_exc);
					rv += Mu(energy_xrf)/sin(angle_xrf);
					return rv;
				}
				double BLB(double energy) {
					return exp(-1.0 * Mu(energy) * density * thickness);
				}
				enum AcorrCases {
					ACORR_CASE_THIN = 1,
					ACORR_CASE_INTERMEDIATE = 2,
					ACORR_CASE_THICK =3
				};
				double Acorr(double energy_exc, double energy_xrf, enum AcorrCases *Acorr_case = 0, double angle_exc = M_PI_4, double angle_xrf = M_PI_4) {
					double rv(0.0);
					double chi(Chi(energy_exc, energy_xrf, angle_exc, angle_xrf));
					double denominator(chi*density*thickness);
					double numerator(-1.0*expm1(-1.0*denominator));
				
					if (Acorr_case) {
						if (numerator < 1E-4)
							*Acorr_case = ACORR_CASE_THIN;
						else if (numerator > 0.9999)
							*Acorr_case = ACORR_CASE_THICK;
						else
							*Acorr_case = ACORR_CASE_INTERMEDIATE;
					}					
	
					rv = numerator/denominator;
					return rv;
				}
				friend class Composition;
				friend std::ostream& operator<< (std::ostream &out, const Layer &layer);
			};
			class Composition {
			private:
				unsigned int reference_layer;
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
				void ReplaceLayer(const Layer &layer_new, unsigned int layer_index);
				void SetReferenceLayer(unsigned int reference_layer_new);
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
