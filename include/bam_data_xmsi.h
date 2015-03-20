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
				void SetThickness(double thickness_new) {
					thickness = thickness_new;
				}
				void SetDensity(double density_new) {
					density = density_new;
				}
				double GetThickness() {
					return thickness;
				}
				double GetDensity() {
					return density;
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
					ACORR_CASE_THICK = 3
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
						throw BAM::Exception(std::string("BAM::Data:XMSI::Composition::GetLayer: ")+e.what());
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
					double dot_prod = std::inner_product(beam, beam + 3, n_sample_orientation.begin(), double(0.0));
					//std::cout << "n_sample_orientation[2]: " << n_sample_orientation[2] << std::endl;
					//std::cout << "beam[2]: " << beam[2] << std::endl;
					//std::cout << "Alpha dot_prod: " << dot_prod << std::endl;
					alpha = M_PI_2 - acos(dot_prod);
					dot_prod = std::inner_product(n_detector_orientation.begin(), n_detector_orientation.end(), n_sample_orientation.begin(), double(0.0));
					//std::cout << "Beta dot_prod: " << dot_prod << std::endl;
					beta = M_PI_2 - acos(dot_prod);
					//std::cout << "Geometry alpha: " << alpha << std::endl;
					//std::cout << "Geometry beta: " << beta << std::endl;
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
			class BaseEnergy {
			private:
				//basically I kill of the default constructor with this statement
				BaseEnergy();
			protected:
				double energy;
				double horizontal_intensity;
				double vertical_intensity;
				double sigma_x;
				double sigma_xp;
				double sigma_y;
				double sigma_yp;
				BaseEnergy(double energy, double horizontal_intensity, double vertical_intensity, double sigma_x = 0.0, double sigma_xp = 0.0, double sigma_y = 0.0, double sigma_yp = 0.0) : energy(energy), horizontal_intensity(horizontal_intensity), vertical_intensity(vertical_intensity), sigma_x(sigma_x), sigma_xp(sigma_xp), sigma_y(sigma_y), sigma_yp(sigma_yp) {
					//ideally I check here the validity of the parameters
				}
			public:
				double GetEnergy() {
					return energy;
				}
			};
			//forward declaration
			class Excitation;

			class ContinuousEnergy : public BaseEnergy {
			private:
				ContinuousEnergy(struct xmi_energy_continuous xec) : BaseEnergy(xec.energy, xec.horizontal_intensity, xec.vertical_intensity, xec.sigma_x, xec.sigma_xp, xec.sigma_y, xec.sigma_yp) {}
			public:
				ContinuousEnergy(double energy, double horizontal_intensity, double vertical_intensity, double sigma_x = 0.0, double sigma_xp = 0.0, double sigma_y = 0.0, double sigma_yp = 0.0) : BaseEnergy(energy, horizontal_intensity, vertical_intensity, sigma_x, sigma_xp, sigma_y, sigma_yp) {}
				friend Excitation;
			};

			class DiscreteEnergy : public BaseEnergy {
			private:
				int distribution_type;
				double scale_parameter;
				DiscreteEnergy(struct xmi_energy_discrete xed) : BaseEnergy(xed.energy, xed.horizontal_intensity, xed.vertical_intensity, xed.sigma_x, xed.sigma_xp, xed.sigma_y, xed.sigma_yp), distribution_type(xed.distribution_type), scale_parameter(xed.scale_parameter) {}
			public:
				DiscreteEnergy(double energy, double horizontal_intensity, double vertical_intensity, double sigma_x = 0.0, double sigma_xp = 0.0, double sigma_y = 0.0, double sigma_yp = 0.0, int distribution_type = XMI_DISCRETE_MONOCHROMATIC, double scale_parameter = 0.0) : BaseEnergy(energy, horizontal_intensity, vertical_intensity, sigma_x, sigma_xp, sigma_y, sigma_yp), distribution_type(distribution_type), scale_parameter(scale_parameter) {}
				friend Excitation;
		
			};

			class Excitation {
			private:
				std::vector<DiscreteEnergy> discrete;
				std::vector<ContinuousEnergy> continuous;
				
				Excitation(struct xmi_excitation *excitation) {
					//ideally one would also check for duplicates and sort after every push_back
					//however, this constructor is fed data straight from XMI-MSIM which is already sorted and verified for dupes
					
					for (int i = 0 ; i < excitation->n_discrete ; i++) {
						discrete.push_back(DiscreteEnergy(excitation->discrete[i]));
					}
					for (int i = 0 ; i < excitation->n_continuous; i++) {
						continuous.push_back(ContinuousEnergy(excitation->continuous[i]));
					}
				} 
			public:
				void EnsureMonochromaticExcitation();
				DiscreteEnergy GetDiscreteEnergy(int index) {
					try {
						return discrete.at(index);
					}
					catch (std::out_of_range &e) {
						throw BAM::Exception(std::string("BAM::Data:XMSI::Excitation::GetDiscreteEnergy: ")+e.what());
					} 
				}
				ContinuousEnergy GetContinuousEnergy(int index) {
					try {
						return continuous.at(index);
					}
					catch (std::out_of_range &e) {
						throw BAM::Exception(std::string("BAM::Data:XMSI::Excitation::GetContinuousEnergy: ")+e.what());
					} 
				}
				
				friend class BAM::File::XMSI;
			};
		}
	}
}
#endif
