#ifndef BAM_DATA_RXI_H
#define BAM_DATA_RXI_H

#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace BAM {
	namespace Data {
		namespace RXI {
			class SingleElement {
				private:
				std::string element;
				std::string linetype;
				std::string datatype;
				double rxi;
				public:
				SingleElement(std::string element, std::string linetype, std::string datatype, double rxi) : element(element), linetype(linetype), datatype(datatype), rxi(rxi) {}
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
				friend class Sample;
			};
			class Sample {
				private:
				static bool element_comp (std::string lhs, std::string rhs) {return SymbolToAtomicNumber((char *) lhs.c_str()) < SymbolToAtomicNumber((char *) rhs.c_str());}
				std::string asrfile;
				double density;
				double thickness;
				std::map<std::string,SingleElement,bool(*)(std::string,std::string)> single_elements;
				public:
				Sample(std::string asrfile, double density, double thickness) : asrfile(asrfile), density(density), thickness(thickness), single_elements(element_comp) {}
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
