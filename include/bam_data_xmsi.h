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
#include <cmath>
#include "bam_data_xraylib.h"



namespace BAM {
	namespace File {
		class XMSI;
	}
	namespace Data {
		namespace XMSI {
			class Composition;
			class Layer : public Xraylib::CompoundNIST {
			private:
				double thickness;
				Layer(struct xmi_layer layer) : Xraylib::CompoundNIST(layer.density), thickness(layer.thickness) {
					SetComposition(layer.Z, layer.weight, layer.n_elements);
				}
				Layer() {};
			public:
			using BAM::Data::Xraylib::CompoundNIST::SetComposition;
				Layer(double density_new, double thickness_new);
				Layer(std::string compound, double density_new, double thickness_new);
				Layer(std::string nistcompound, double thickness_new);
				void SetThickness(double thickness_new) {
					thickness = thickness_new;
				}
				double GetThickness() {
					return thickness;
				}
				double BLB(double energy) {
					return exp(-1.0 * Mu(energy) * density * thickness);
				}
				struct xmi_layer Convert() const {
					struct xmi_layer rv;
					rv.n_elements = (int) composition.size();
					rv.Z = (int *) xmi_malloc(sizeof(int) * rv.n_elements);
					rv.weight = (double *) xmi_malloc(sizeof(double) * rv.n_elements);
					rv.density = density;
					rv.thickness = thickness;
					int counter(0);
					for (std::map<int,double>::const_iterator it = composition.begin() ; it != composition.end() ; ++it) {
						rv.Z[counter] = it->first;
						rv.weight[counter++] = it->second;
					}
					return rv;
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
				friend class BAM::Data::XMSI::Composition;
				friend std::ostream& operator<< (std::ostream &out, const Layer &layer);
				Layer& operator*=(double multiplier) {
					this->SetComposition(multiplier * *this);
					return *this;
				}
			};
			class Composition {
			private:
				unsigned int reference_layer;
				std::vector <Layer> layers;
				Composition(struct xmi_composition *composition) : 
					reference_layer(composition->reference_layer) {
					for (int i = 0 ; i < composition->n_layers ; i++)
						layers.push_back(Layer(composition->layers[i]));

				}
			public:
				Composition() : reference_layer(0) {}
				void AddLayer(const Layer &layer_new);
				void ReplaceLayer(const Layer &layer_new, unsigned int layer_index);
				void SetReferenceLayer(unsigned int reference_layer_new);
				Layer GetLayer(int layer_index) {
					try {
						return layers.at(layer_index-1);
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
				friend class Excitation;
			};

			class DiscreteEnergy : public BaseEnergy {
			private:
				int distribution_type;
				double scale_parameter;
				DiscreteEnergy(struct xmi_energy_discrete xed) : BaseEnergy(xed.energy, xed.horizontal_intensity, xed.vertical_intensity, xed.sigma_x, xed.sigma_xp, xed.sigma_y, xed.sigma_yp), distribution_type(xed.distribution_type), scale_parameter(xed.scale_parameter) {}
			public:
				DiscreteEnergy(double energy, double horizontal_intensity, double vertical_intensity, double sigma_x = 0.0, double sigma_xp = 0.0, double sigma_y = 0.0, double sigma_yp = 0.0, int distribution_type = XMI_DISCRETE_MONOCHROMATIC, double scale_parameter = 0.0) : BaseEnergy(energy, horizontal_intensity, vertical_intensity, sigma_x, sigma_xp, sigma_y, sigma_yp), distribution_type(distribution_type), scale_parameter(scale_parameter) {}
				friend class Excitation;
		
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
