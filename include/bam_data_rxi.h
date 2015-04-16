#ifndef BAM_DATA_RXI_H
#define BAM_DATA_RXI_H

#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <xraylib.h>

namespace BAM {
	namespace Data {
		namespace RXI {
			class SingleElement {
				private:
				std::string element;
				std::string linetype;
				std::string datatype;
				double rxi;
				double line_energy;
				double excitation_energy;
				public:
				SingleElement(std::string element, std::string linetype, std::string datatype, double rxi, double excitation_energy) : element(element), linetype(linetype), datatype(datatype), rxi(rxi), excitation_energy(excitation_energy) {
					int atomic_number = SymbolToAtomicNumber((char*) element.c_str());
					if (atomic_number == 0)
						throw BAM::Exception("BAM::Data::RXI::SingleElement -> Unknown element");
					if (linetype == "KA_LINE") {
						line_energy = LineEnergy(atomic_number, KA_LINE);
					}
					else if (linetype == "LA_LINE") {
						line_energy = LineEnergy(atomic_number, LA_LINE);
					}
					else
						throw BAM::Exception("BAM::Data::RXI::SingleElement -> Unknown linetype: must be KA_LINE or LA_LINE");
				}
				std::string GetElement() {
					return element;
				}
				std::string GetLineType() {
					return linetype;
				}
				std::string GetDataType() {
					return datatype;
				}
				std::string GetRXIString() {
					std::stringstream ss;
					ss << rxi;
					return ss.str();
				}
				double GetRXI() {
					return rxi;
				}
				double GetLineEnergy() {
					return line_energy;
				}
				double GetExcitationEnergy() {
					return excitation_energy;
				}
				std::string GetExcitationEnergyString() {
					std::stringstream ss;
					ss << excitation_energy;
					return ss.str();
				}
				friend class Sample;
			};
			class Sample {
				private:
				static bool element_comp (std::string lhs, std::string rhs) {return SymbolToAtomicNumber((char *) lhs.c_str()) < SymbolToAtomicNumber((char *) rhs.c_str());}
				std::map<std::string,double> asrfiles;
				double density;
				double thickness;
				bool density_thickness_fixed;
				std::string matrix;
				BAM::Data::Base::Composition *matrix_composition;
				std::map<std::string,SingleElement,bool(*)(std::string,std::string)> single_elements;
				public:
				Sample() : density(0.0), thickness(0.0), density_thickness_fixed(true), matrix("none"), matrix_composition(0), single_elements(element_comp) {}
				Sample(std::map<std::string,double> asrfiles, double density, double thickness, bool density_thickness_fixed, std::string matrix) : asrfiles(asrfiles), density(density), thickness(thickness), density_thickness_fixed(density_thickness_fixed), matrix(matrix), matrix_composition(0), single_elements(element_comp) {
					if (matrix != "none") {
						matrix_composition = BAM::Data::Xraylib::Parse(matrix);
					}
				}
				~Sample() {
					if (matrix_composition)
						delete matrix_composition;
				}
				Sample(const Sample &sample) : asrfiles(sample.asrfiles), density(sample.density), thickness(sample.thickness), density_thickness_fixed(sample.density_thickness_fixed), matrix(sample.matrix), matrix_composition(0), single_elements(sample.single_elements) {
					if (matrix != "none") {
						matrix_composition = BAM::Data::Xraylib::Parse(matrix);
					}
				}
				Sample& operator= (const Sample &sample) {
					if (this == &sample)
						return *this;
					asrfiles = sample.asrfiles;
					density = sample.density;
					thickness = sample.thickness;
					density_thickness_fixed = sample.density_thickness_fixed;
					matrix = sample.matrix;
					single_elements = sample.single_elements;

					if (matrix_composition)
						delete matrix_composition;

					matrix_composition = 0;

					if (matrix != "none") {
						matrix_composition = BAM::Data::Xraylib::Parse(matrix);
					}
					return *this;
				}
				std::map<std::string,double> GetASRfiles() {
					return asrfiles;
				}
				double GetDensity() {
					return density;
				}
				double GetThickness() {
					return thickness;
				}
				std::string GetDensityString() {
					std::stringstream ss;
					ss << density;
					return ss.str();
				}
				std::string GetThicknessString() {
					std::stringstream ss;
					ss << thickness;
					return ss.str();
				}
				bool GetDensityThicknessFixed() {
					return density_thickness_fixed;
				}
				std::string GetDensityThicknessFixedString() {
					return density_thickness_fixed ? "fixed" : "variable";
				}
				void AddSingleElement(SingleElement single_element) {
					single_elements.insert(std::pair<std::string,SingleElement>(single_element.element, single_element));
				}
				SingleElement GetSingleElement(std::string requested_element) {
					if (single_elements.find(requested_element) == single_elements.end())
						throw BAM::Exception(std::string("BAM::Data::RXI::Sample::GetSingleElement -> Element not found "));

					return SingleElement(single_elements.find(requested_element)->second);
				}
				size_t GetNumberOfSingleElements() {
					return single_elements.size();
				}
				std::vector<std::string> GetElements() {
					std::vector<std::string> rv;
					for(std::map<std::string,SingleElement>::iterator it = single_elements.begin(); it != single_elements.end(); ++it) {
						rv.push_back(it->first);
					}
					return rv;
				}
				std::string GetMatrixString() {
					return matrix;
				}
				const BAM::Data::Base::Composition *GetMatrix() {
					//do not delete this!!!!
					return matrix_composition;
				}
			};
		}
	}
}

#endif
