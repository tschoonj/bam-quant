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

		BAM::File::RXI::Common *rxi_common = BAM::File::RXI::Parse(TEST_FILE_RXI_SINGLE);
		BAM::File::RXI::Single *rxi_single = dynamic_cast<BAM::File::RXI::Single*>(rxi_common);
		
		if (rxi_single == 0) {
			throw BAM::Exception("rxi_single is null after dynamic_cast");	
		}

		//this should be checked with valgrind for leaks
		delete rxi_single;

		rxi_common = BAM::File::RXI::Parse(TEST_FILE_RXI_MULTI);
		BAM::File::RXI::Multi *rxi_multi= dynamic_cast<BAM::File::RXI::Multi*>(rxi_common);
		
		if (rxi_multi == 0) {
			throw BAM::Exception("rxi_multi is null after dynamic_cast");	
		}

		//this should be checked with valgrind for leaks
		delete rxi_multi;

	}
	catch (BAM::Exception &e) {
		std::cerr << "BAM exception caught: " << e.what() << std::endl;
		return 1;	
	}
	catch (...) {
		std::cerr << "Unknown exception caught!" << std::endl;
		return 1;	
	}
	return 0;
}

