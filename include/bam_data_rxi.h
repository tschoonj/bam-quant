#ifndef BAM_DATA_RXI_H
#define BAM_DATA_RXI_H

#include <vector>
#include <iostream>

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
			};
			class Sample {
				private:
				std::vector<SingleElement> elements;
				std::string asrfile;
				double density;
				double thickness;
				public:
				void SetASRfile(std::string file) {
					asrfile = file;	
				}
				void SetDensity(double value) {
					density = value;	
				}
				void SetThickness(double value) {
					thickness = value;	
				}
				void AddSingleElement(SingleElement single_element) {
					elements.push_back(single_element);
				}
			};
		}
	}
}

#endif
