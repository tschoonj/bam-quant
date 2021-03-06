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
	Glib::ustring density_str = dynamic_cast<xmlpp::Element*>(*(element->get_children("density").begin()))->get_child_text()->get_content();
	Glib::ustring thickness_str = dynamic_cast<xmlpp::Element*>(*(element->get_children("thickness").begin()))->get_child_text()->get_content();
 	Glib::ustring density_thickness_str = element->get_attribute_value("density_thickness");
 	Glib::ustring matrix_str = element->get_attribute_value("matrix");

	//the following is necessary to circumvent a bug in libxml++ versions before 2.36.0 regarding the handling of default attribute values	
	if (matrix_str == "") {
		matrix_str = "none";
	}

	xmlpp::Node::NodeList asrfile_nodes = element->get_children("asrfile");
	std::map<std::string,double> asrfiles;

	for (xmlpp::Node::NodeList::iterator iter = asrfile_nodes.begin() ;
	     iter != asrfile_nodes.end() ;
	     ++iter) {
		Glib::ustring asrfile = dynamic_cast<xmlpp::Element*>(*iter)->get_child_text()->get_content();
		Glib::ustring excitation_energy_str = dynamic_cast<xmlpp::Element*>(*iter)->get_attribute_value("excitation_energy");
		std::stringstream ss;
		double excitation_energy;
		ss << excitation_energy_str;
		ss >> excitation_energy;
		asrfiles[asrfile] = excitation_energy;
	}

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
	Sample rv(asrfiles, density, thickness, density_thickness_fixed, matrix_str);
	
	xmlpp::Node::NodeList list = element->get_children("element_rxi");
	for (xmlpp::Node::NodeList::iterator it = list.begin() ; it != list.end() ; ++it) {
 		Glib::ustring element = dynamic_cast<xmlpp::Element*>(*it)->get_attribute_value("element");
                Glib::ustring datatype= dynamic_cast<xmlpp::Element*>(*it)->get_attribute_value("datatype");
                Glib::ustring linetype = dynamic_cast<xmlpp::Element*>(*it)->get_attribute_value("linetype");
                Glib::ustring excitation_energy_str = dynamic_cast<xmlpp::Element*>(*it)->get_attribute_value("excitation_energy");
		Glib::ustring rxi_str = dynamic_cast<xmlpp::Element*>(*it)->get_child_text()->get_content();
		double rxi, excitation_energy;
		{
			std::stringstream ss;
			ss << rxi_str;
			ss >> rxi;
		}
		{
			std::stringstream ss;
			ss << excitation_energy_str;
			ss >> excitation_energy;
		}
		rv.AddSingleElement(SingleElement(element, linetype, datatype, rxi, excitation_energy));
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
		element_rxi->set_attribute("excitation_energy", single_element.GetExcitationEnergyString());
		element_rxi->add_child_text(single_element.GetRXIString());
	}
	std::map<std::string,double> asrfiles = sampleBAM.GetASRfiles();
	for (std::map<std::string,double>::iterator iter = asrfiles.begin() ; 
		iter != asrfiles.end() ;
		++iter) {
		xmlpp::Element *new_asrfile = sampleXML->add_child("asrfile");
		new_asrfile->add_child_text(iter->first);
		std::stringstream ss;
		ss << iter->second;
		new_asrfile->set_attribute("excitation_energy", ss.str());	
	}
	sampleXML->add_child("density")->add_child_text(sampleBAM.GetDensityString());
	sampleXML->add_child("thickness")->add_child_text(sampleBAM.GetThicknessString());
	sampleXML->set_attribute("density_thickness", sampleBAM.GetDensityThicknessFixedString());
	sampleXML->set_attribute("matrix", sampleBAM.GetMatrixString());

	return;
}

Single::Single(std::string filename) : Common(filename) ,sample() {
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
	xmlpp::Element *samples = dynamic_cast<xmlpp::Element*>(*(root->get_children("samples").begin()));
	if (samples == 0) {
		throw BAM::Exception("BAM::File::RXI::Single::Parse -> root element does not contain the samples element");
	}
	sample = ConvertXMLToSample(samples);

	xmlpp::Node::NodeList xmimsim_input_nodes = root->get_children("xmimsim-input");

	for (xmlpp::Node::NodeList::iterator iter = xmimsim_input_nodes.begin() ; 
		iter != xmimsim_input_nodes.end() ;
		++iter) {
        	struct xmi_input *input = (struct xmi_input*) malloc(sizeof(struct xmi_input));
        	if (xmi_read_input_xml_body(document->cobj(), (*iter)->cobj(), input) == 0) {
			throw BAM::Exception("BAM::File::RXI::Single::Parse -> Error in xmi_read_input_xml_body");
        	}
		BAM::File::XMSI *xmimsim_input_temp = new BAM::File::XMSI(input);
		xmi_free_input(input);
		xmimsim_input_temp->GetExcitation().EnsureMonochromaticExcitation();
		xmimsim_input[xmimsim_input_temp->GetExcitation().GetDiscreteEnergy(0).GetEnergy()] = *xmimsim_input_temp;
		delete xmimsim_input_temp;

	}
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

		for (std::map<double,BAM::File::XMSI>::iterator iter = xmimsim_input.begin() ;
			iter != xmimsim_input.end() ;
			++iter) {

			xmlpp::Element *xmimsim_element = rootnode->add_child("xmimsim-input");
			//xmlpp::Node *nodepp = dynamic_cast<xmlpp::Node *>(xmimsim);
			xmlNodePtr nodePtr = xmimsim_element->cobj();
			struct xmi_input *xmsi_raw = iter->second.GetInternalCopy();
			if (xmi_write_input_xml_body(doc, nodePtr, xmsi_raw) == 0) {
				throw BAM::Exception("BAM::File::RXI::Single::Write -> Could not write XMI-MSIM input body");
			}
			xmi_free_input(xmsi_raw);
		}
		document.write_to_file_formatted(filename);
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

	xmlpp::Node::NodeList xmimsim_input_nodes = root->get_children("xmimsim-input");

	for (xmlpp::Node::NodeList::iterator iter = xmimsim_input_nodes.begin() ; 
		iter != xmimsim_input_nodes.end() ;
		++iter) {
        	struct xmi_input *input = (struct xmi_input*) malloc(sizeof(struct xmi_input));
        	if (xmi_read_input_xml_body(document->cobj(), (*iter)->cobj(), input) == 0) {
			throw BAM::Exception("BAM::File::RXI::Single::Parse -> Error in xmi_read_input_xml_body");
        	}
		BAM::File::XMSI *xmimsim_input_temp = new BAM::File::XMSI(input);
		xmi_free_input(input);
		xmimsim_input_temp->GetExcitation().EnsureMonochromaticExcitation();
		xmimsim_input[xmimsim_input_temp->GetExcitation().GetDiscreteEnergy(0).GetEnergy()] = *xmimsim_input_temp;
		delete xmimsim_input_temp;

	}
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

		for (std::map<double,BAM::File::XMSI>::iterator iter = xmimsim_input.begin() ;
			iter != xmimsim_input.end() ;
			++iter) {

			xmlpp::Element *xmimsim_element = rootnode->add_child("xmimsim-input");
			//xmlpp::Node *nodepp = dynamic_cast<xmlpp::Node *>(xmimsim);
			xmlNodePtr nodePtr = xmimsim_element->cobj();
			struct xmi_input *xmsi_raw = iter->second.GetInternalCopy();
			if (xmi_write_input_xml_body(doc, nodePtr, xmsi_raw) == 0) {
				throw BAM::Exception("BAM::File::RXI::Multi::Multi -> Could not write XMI-MSIM input body");
			}
			xmi_free_input(xmsi_raw);
		}

		document.write_to_file_formatted(filename);
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


Common* BAM::File::RXI::Parse(std::string filename) {
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
