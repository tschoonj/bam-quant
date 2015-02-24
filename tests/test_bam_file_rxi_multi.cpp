#include "config.h"
#include "bam_file_rxi.h"
#include <xmi_msim.h>
#include "bam_exception.h"
#include <iostream>
#include <glibmm/convert.h>
#include <glibmm/exception.h>
#include <glibmm/miscutils.h>
#include <libxml/catalog.h>
#include <glib.h>

int main(int argc, char *argv[]) {
	BAM::File::RXI::Multi *rxi_multi;


	try {
		//catalog stuff first
		if (xmi_xmlLoadCatalog() == 0)
			throw BAM::Exception("Could not load XMI-MSIM XML catalog");
	
		Glib::ustring uriStartString("http://www.bam.de/xml/");
		Glib::ustring rewritePrefix(Glib::filename_to_uri(CATALOGPATH));
		
		if (xmlCatalogAdd(BAD_CAST "catalog",NULL,NULL) == -1) {
			throw BAM::Exception("xmlCatalogAdd error: catalog");
		}
		if (xmlCatalogAdd(BAD_CAST "rewriteURI", BAD_CAST uriStartString.c_str(), BAD_CAST rewritePrefix.c_str()) == -1) {
			throw BAM::Exception("xmlCatalogAdd error: rewriteURI");
		}
	
		try {
			std::cout << "Case 1" << std::endl;
			rxi_multi = new BAM::File::RXI::Multi(TEST_FILE_RXI_MULTI);
			BAM::Data::RXI::Sample sample = rxi_multi->GetSample(0);
			std::cout << "Sample density: " << sample.GetDensity() << std::endl;
			std::cout << "Sample density as string: " << sample.GetDensityString() << std::endl;
		}
		catch (BAM::Exception &e) {
			throw BAM::Exception(std::string(e.what()));
		}
		catch (...) {
			throw BAM::Exception("Unknown exception caught");
		}
		try {
			std::cout << "Case 2" << std::endl;
			BAM::File::RXI::Multi rxi_multi2(*rxi_multi);
			BAM::Data::RXI::Sample sample = rxi_multi2.GetSample(0);
			std::cout << "Sample density: " << sample.GetDensity() << std::endl;
			std::cout << "Sample density as string: " << sample.GetDensityString() << std::endl;
		}
		catch (BAM::Exception &e) {
			throw BAM::Exception(std::string(e.what()));
		}
		catch (...) {
			throw BAM::Exception("Unknown exception caught");
		}
		try {
			std::cout << "Case 3" << std::endl;
			BAM::File::RXI::Multi rxi_multi3;
			rxi_multi3 = *rxi_multi;
			BAM::Data::RXI::Sample sample = rxi_multi3.GetSample(0);
			std::cout << "Sample density: " << sample.GetDensity() << std::endl;
			std::cout << "Sample density as string: " << sample.GetDensityString() << std::endl;

			rxi_multi3.Write(std::string(Glib::path_get_basename(TEST_FILE_RXI_MULTI))+".copy");
		}
		catch (BAM::Exception &e) {
			throw BAM::Exception(std::string(e.what()));
		}
		catch (...) {
			throw BAM::Exception("Unknown exception caught");
		}
	}
	catch (BAM::Exception &e) {
		std::cerr << "BAM exception: " << e.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "Unknown exception caught: this should not happen! " << std::endl;
		return 1;
	}
	delete rxi_multi;

	return 0;
}

