#ifndef BAM_DATA_RXI_H
#define BAM_DATA_RXI_H

#include <vector>
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
				SingleElement(std::string element = "", std::string linetype = "", std::string datatype = "", double rxi = 0.0) : element(element), linetype(linetype), datatype(datatype), rxi(rxi) {}
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
			};
			class Sample {
				private:
				std::vector<SingleElement> elements;
				std::string asrfile;
				double density;
				double thickness;
				public:
				Sample() : asrfile(""), density(0.0), thickness(0.0) {}
				void SetASRfile(std::string file) {
					asrfile = file;	
				}
				void SetDensity(double value) {
					density = value;	
				}
				void SetThickness(double value) {
					thickness = value;	
				}
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
					elements.push_back(single_element);
				}
				SingleElement GetSingleElement(int index) {
					SingleElement element;
					try {
						element = elements.at(index);
					}
					catch (std::out_of_range &e) {
						throw BAM::Exception(std::string("BAM::Data::RXI::Sample::GetSingleElement: ")+e.what());
					}
					return element;
				}
				size_t GetNumberOfSingleElements() {
					return elements.size();
				}
			};
		}
	}
}

#endif
