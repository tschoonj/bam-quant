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
				Composition(struct xmi_composition *composition) : 
					reference_layer(composition->reference_layer) {
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
				Composition(const Composition &composition) : reference_layer(composition.reference_layer) {
					for (std::vector<struct xmi_layer>::const_iterator it = composition.layers.begin() ; it != composition.layers.end() ; ++it) {
						AddLayer(Layer(*it));
					}
				}
				Composition& operator= (const Composition &composition) {
					if (this == &composition)
						return *this;

					for (std::vector<struct xmi_layer>::iterator it = layers.begin() ; it != layers.end() ; ++it) {
						xmi_free_layer(&(*it));	
					}
					layers.clear();

					for (std::vector<struct xmi_layer>::const_iterator it = composition.layers.begin() ; it != composition.layers.end() ; ++it) {
						AddLayer(Layer(*it));
					}
				
					reference_layer = composition.reference_layer;
					return *this;
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
			class Geometry {
			private:
				double d_sample_source;
				std::vector <double> n_sample_orientation;
				std::vector <double> p_detector_window;
				std::vector <double> n_detector_orientation;
				double area_detector;
				double collimator_height;
				double collimator_diameter;
				double d_source_slit;
				double slit_size_x;
				double slit_size_y;

				double alpha;
				double beta;

				Geometry(struct xmi_geometry *geometry) :
					d_sample_source(geometry->d_sample_source),
					n_sample_orientation(geometry->n_sample_orientation, geometry->n_sample_orientation+3),
					p_detector_window(geometry->p_detector_window, geometry->p_detector_window+3),
					n_detector_orientation(geometry->n_detector_orientation, geometry->n_detector_orientation+3),
					area_detector(geometry->area_detector),
					collimator_height(geometry->collimator_height),
					collimator_diameter(geometry->collimator_diameter),
					d_source_slit(geometry->d_source_slit),
					slit_size_x(geometry->slit_size_x),
					slit_size_y(geometry->slit_size_y) {

					double beam[3] = {0, 0, 1};
					double dot_prod = std::inner_product(beam, beam + 3, n_sample_orientation.begin(), 0);
					alpha = M_PI_2 - acos(dot_prod);
					dot_prod = std::inner_product(n_detector_orientation.begin(), n_detector_orientation.end(), n_sample_orientation.begin(), 0);
					beta = M_PI - acos(dot_prod);
				}
			public:
				double GetAlpha() {
					return alpha;
				}
				double GetBeta() {
					return beta;
				}
				friend class BAM::File::XMSI;
			};
			class Excitation {
			private:
				std::vector<struct xmi_energy_discrete> discrete;
				std::vector<struct xmi_energy_continuous> continuous;
				
				Excitation(struct xmi_excitation *excitation) {
					if (excitation->n_discrete)
						discrete.assign(excitation->discrete, excitation->discrete + excitation->n_discrete);
					if (excitation->n_continuous)
						continuous.assign(excitation->continuous, excitation->continuous + excitation->n_continuous);
				} 
			public:
				void EnsureMonochromaticExcitation();
				friend class BAM::File::XMSI;
			};
		}
	}
}
#endif
