#include "config.h"
#include "bam_exception.h"
#include <fstream>
#include <iostream>
#include "bam_file_xmsi.h"
#include "bam_file_rxi.h"
#include <libxml++/libxml++.h>
#include <cstdlib>
#include <glibmm/exception.h>

using namespace BAM;
using namespace BAM::File;
using namespace BAM::File::RXI;
using namespace BAM::Data::RXI;

static Sample ConvertXMLToSample(xmlpp::Element *element) {
	Glib::ustring asrfile = dynamic_cast<xmlpp::Element*>(element->get_first_child("asrfile"))->get_child_text()->get_content();
	Glib::ustring density_str = dynamic_cast<xmlpp::Element*>(element->get_first_child("density"))->get_child_text()->get_content();
	Glib::ustring thickness_str = dynamic_cast<xmlpp::Element*>(element->get_first_child("thickness"))->get_child_text()->get_content();
 	Glib::ustring density_thickness_str = element->get_attribute_value("density_thickness");

	bool density_thickness_fixed;

	if (density_thickness_str == "fixed") {
		density_thickness_fixed = true;
	}
	else {
		density_thickness_fixed = false;
	}

	double density;
	double thickness;
	{
		std::stringstream ss;
		ss << density_str;
		ss >> density;
	}
	{
		std::stringstream ss;
		ss << thickness_str;
		ss >> thickness;
	}
	Sample rv(asrfile, density, thickness, density_thickness_fixed);
	
	xmlpp::Node::NodeList list = element->get_children("element_rxi");
	for (xmlpp::Node::NodeList::iterator it = list.begin() ; it != list.end() ; ++it) {
 		Glib::ustring element = dynamic_cast<xmlpp::Element*>(*it)->get_attribute_value("element");
                Glib::ustring datatype= dynamic_cast<xmlpp::Element*>(*it)->get_attribute_value("datatype");
                Glib::ustring linetype = dynamic_cast<xmlpp::Element*>(*it)->get_attribute_value("linetype");
		Glib::ustring rxi_str = dynamic_cast<xmlpp::Element*>(*it)->get_child_text()->get_content();
		std::stringstream ss;
		double rxi;
		ss << rxi_str;
		ss >> rxi;
		rv.AddSingleElement(SingleElement(element, linetype, datatype, rxi));
	}

	return rv;
}

static void ConvertSampleToXML(xmlpp::Element *sampleXML, Sample &sampleBAM) {
	std::vector<std::string> elements(sampleBAM.GetElements());

	for (std::vector<std::string>::iterator it = elements.begin() ; it != elements.end() ; ++it) {
		SingleElement single_element = sampleBAM.GetSingleElement(*it);
		xmlpp::Element *element_rxi = sampleXML->add_child("element_rxi");
		element_rxi->set_attribute("element", single_element.GetElement());
		element_rxi->set_attribute("linetype", single_element.GetLineType());
		element_rxi->set_attribute("datatype", single_element.GetDataType());
		element_rxi->add_child_text(single_element.GetRXIString());
	}
	sampleXML->add_child("asrfile")->add_child_text(sampleBAM.GetASRfile());
	sampleXML->add_child("density")->add_child_text(sampleBAM.GetDensityString());
	sampleXML->add_child("thickness")->add_child_text(sampleBAM.GetThicknessString());
	sampleXML->set_attribute("density_thickness", sampleBAM.GetDensityThicknessFixedString());

	return;
}

Single::Single(std::string filename) : Common(filename) ,sample("", 0.0, 0.0, true) {
	try {
		Parse();
	}
	catch (BAM::Exception &e) {
		throw BAM::Exception(e.what());
	}
        catch (std::exception &e) {
		throw BAM::Exception(std::string("BAM::File::RXI::Single::Single -> ")+e.what());
        }
	catch (Glib::Exception &e) {
		throw BAM::Exception(std::string("BAM::File::RXI::Single::Single -> ")+e.what());
	}
        catch (...) {
		throw BAM::Exception(std::string("BAM::File::RXI::Single::Single -> Unknown exception caught"));
        }
	Close();
}

void Single::Open() {}
void Single::Close() {}

void Single::Parse() {
	xmlpp::DomParser parser;
	parser.set_validate();
       	parser.parse_file(filename);
       	xmlpp::Document *document = parser.get_document();
       	xmlpp::Element *root = document->get_root_node();

	if (root->get_name() != "bam-quant-rxi-single") {
		throw BAM::Exception("BAM::File::RXI::Single::Parse -> root element must be bam-quant-rxi-single");
	}
	xmlpp::Element *samples = dynamic_cast<xmlpp::Element*>(root->get_first_child("samples"));
	if (samples == 0) {
		throw BAM::Exception("BAM::File::RXI::Single::Parse -> root element does not contain the samples element");
	}
	sample = ConvertXMLToSample(samples);
	xmlpp::Node *xmimsim_input_node = root->get_first_child("xmimsim-input");
        struct xmi_input *input = (struct xmi_input*) malloc(sizeof(struct xmi_input));
        if (xmi_read_input_xml_body(document->cobj(), xmimsim_input_node->cobj(), input) == 0) {
                throw BAM::Exception("BAM::File::RXI::Single::Parse -> Error in xmi_read_input_xml_body");
        }
	xmimsim_input = new BAM::File::XMSI(input);
	xmi_free_input(input);
	xmimsim_input->GetExcitation().EnsureMonochromaticExcitation();
}

void Single::Write() {
	if (filename.empty()) {
		throw BAM::Exception("BAM::File::RXI::Single::Write -> Invalid filename");
	}
	try {
		xmlpp::Document document;
		document.set_internal_subset("bam-quant-rxi-single", "", "http://www.bam.de/xml/bam-quant-rxi.dtd");
		//document.add_comment("this is a comment");
		xmlDocPtr doc = document.cobj();

		xmlpp::Element *rootnode = document.create_root_node("bam-quant-rxi-single");

		if (xmi_write_default_comments(doc, rootnode->cobj()) == 0) {
			throw BAM::Exception("BAM::File::RXI::Single::Write -> Could not write XMI-MSIM default comments");
		}
		ConvertSampleToXML(rootnode->add_child("samples"), sample);

		xmlpp::Element *xmimsim_element = rootnode->add_child("xmimsim-input");
		//xmlpp::Node *nodepp = dynamic_cast<xmlpp::Node *>(xmimsim);
		xmlNodePtr nodePtr = xmimsim_element->cobj();
		struct xmi_input *xmsi_raw = xmimsim_input->GetInternalCopy();
		if (xmi_write_input_xml_body(doc, nodePtr, xmsi_raw) == 0) {
			throw BAM::Exception("BAM::File::RXI::Single::Write -> Could not write XMI-MSIM input body");
		}

		document.write_to_file_formatted(filename);
		xmi_free_input(xmsi_raw);
	}
	catch (BAM::Exception &e) {
		throw BAM::Exception(e.what());
	}
	catch (std::exception &e) {
		throw BAM::Exception(std::string("BAM::File::RXI::Single::Write -> ")+e.what());
	}
	catch (Glib::Exception &e) {
		throw BAM::Exception(std::string("BAM::File::RXI::Single::Write -> ")+e.what());
	}
	catch (...) {
		throw BAM::Exception(std::string("BAM::File::RXI::Single::Write -> Unknown exception"));
	}
}

void Single::Write(std::string new_filename) {
	SetFilename(new_filename);
	Write();
}

Multi::Multi(std::string filename) : Common(filename) {
	try {
		Parse();
	}
	catch (BAM::Exception &e) {
		throw BAM::Exception(e.what());
	}
        catch (std::exception &e) {
		throw BAM::Exception(std::string("BAM::File::RXI::Multi::Multi -> ")+e.what());
        }
	catch (Glib::Exception &e) {
		throw BAM::Exception(std::string("BAM::File::RXI::Multi::Multi -> ")+e.what());
	}
        catch (...) {
		throw BAM::Exception(std::string("BAM::File::RXI::Multi::Multi -> Unknown exception caught"));
        }
	Close();
}


void Multi::Open() {}
void Multi::Close() {}

void Multi::Parse() {
	xmlpp::DomParser parser;
	parser.set_validate();
       	parser.parse_file(filename);
       	xmlpp::Document *document = parser.get_document();
       	xmlpp::Element *root = document->get_root_node();

	if (root->get_name() != "bam-quant-rxi-multi") {
		throw BAM::Exception("BAM::File::RXI::Multi::Parse -> root element must be bam-quant-rxi-multi");
	}
	xmlpp::Node::NodeList list = root->get_children("samples");
	for (xmlpp::Node::NodeList::iterator it = list.begin() ; it != list.end() ; ++it) {
		samples.push_back(ConvertXMLToSample(dynamic_cast<xmlpp::Element*>(*it)));
	}

	xmlpp::Node *xmimsim_input_node = root->get_first_child("xmimsim-input");
        struct xmi_input *input = (struct xmi_input*) malloc(sizeof(struct xmi_input));
        if (xmi_read_input_xml_body(document->cobj(), xmimsim_input_node->cobj(), input) == 0) {
                throw BAM::Exception("BAM::File::RXI::Multi::Parse -> Error in xmi_read_input_xml_body");
       }
	xmimsim_input = new BAM::File::XMSI(input);
	xmi_free_input(input);
	xmimsim_input->GetExcitation().EnsureMonochromaticExcitation();
}

void Multi::Write() {
	if (filename.empty()) {
		throw BAM::Exception("BAM::File::RXI::Multi::Write -> Invalid filename");
	}
	try {
		xmlpp::Document document;
		document.set_internal_subset("bam-quant-rxi-multi", "", "http://www.bam.de/xml/bam-quant-rxi.dtd");
		xmlDocPtr doc = document.cobj();

		xmlpp::Element *rootnode = document.create_root_node("bam-quant-rxi-multi");

		if (xmi_write_default_comments(doc, rootnode->cobj()) == 0) {
			throw BAM::Exception("BAM::File::RXI::Multi::Write -> Could not write XMI-MSIM default comments");
		}
		for (std::vector<BAM::Data::RXI::Sample>::iterator it = samples.begin(); it != samples.end() ; ++it) {
			ConvertSampleToXML(rootnode->add_child("samples"), *it);
		}
		xmlpp::Element *xmimsim_element = rootnode->add_child("xmimsim-input");
		//xmlpp::Node *nodepp = dynamic_cast<xmlpp::Node *>(xmimsim);
		xmlNodePtr nodePtr = xmimsim_element->cobj();
		struct xmi_input *xmsi_raw = xmimsim_input->GetInternalCopy();
		if (xmi_write_input_xml_body(doc, nodePtr, xmsi_raw) == 0) {
			throw BAM::Exception("BAM::File::RXI::Multi::Write -> Could not write XMI-MSIM input body");
		}

		document.write_to_file_formatted(filename);
		xmi_free_input(xmsi_raw);
	}
	catch (BAM::Exception &e) {
		throw BAM::Exception(e.what());
	}
	catch (std::exception &e) {
		throw BAM::Exception(std::string("BAM::File::RXI::Multi::Write -> ")+e.what());
	}
	catch (Glib::Exception &e) {
		throw BAM::Exception(std::string("BAM::File::RXI::Multi::Write -> ")+e.what());
	}
	catch (...) {
		throw BAM::Exception(std::string("BAM::File::RXI::Multi::Write -> Unknown exception"));
	}
}

void Multi::Write(std::string new_filename) {
	SetFilename(new_filename);
	Write();
}


namespace BAM {
	namespace File {
		namespace RXI {
			Common* Parse(std::string filename) {
				Common *rv;
				try {
					//first try Single
					rv = new Single(filename);	
				}
				catch (BAM::Exception &e){
					try {
						//then try multi
						rv = new Multi(filename);	
					}
					catch (BAM::Exception &e){
						throw BAM::Exception("BAM::File::RXI::Common::Parse -> file is neither Single nor Multi");
					}
				}
				return rv;
			}
		}
	}
}
