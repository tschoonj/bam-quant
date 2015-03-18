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
				public:
				SingleElement(std::string element, std::string linetype, std::string datatype, double rxi) : element(element), linetype(linetype), datatype(datatype), rxi(rxi) {
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
				friend class Sample;
			};
			class Sample {
				private:
				static bool element_comp (std::string lhs, std::string rhs) {return SymbolToAtomicNumber((char *) lhs.c_str()) < SymbolToAtomicNumber((char *) rhs.c_str());}
				std::string asrfile;
				double density;
				double thickness;
				bool density_thickness_fixed;
				std::map<std::string,SingleElement,bool(*)(std::string,std::string)> single_elements;
				public:
				Sample(std::string asrfile, double density, double thickness, bool density_thickness_fixed) : asrfile(asrfile), density(density), thickness(thickness), density_thickness_fixed(density_thickness_fixed), single_elements(element_comp) {}
				std::string GetASRfile() {
					return asrfile;
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
			};
		}
	}
}

#endif
