#include "config.h"
#include "bam_job_quant.h"
#include "bam_exception.h"
#include <libxml++/libxml++.h>
#include <cmath>

using namespace BAM::Job;

namespace BAM {
	class ExceptionLocal : public Exception {
	public:
		explicit ExceptionLocal(const std::string &s) : Exception(s) {}
		virtual ~ExceptionLocal() throw() {};
			
	};
}


double Quant::calculate_rxi(std::string element, BAM::File::XMSO &sample_output, BAM::Data::RXI::SingleElement single_element) {
	double rxi;
	//everything depends on linetype!
	if (single_element.GetLineType() == "KA_LINE") {
		double numerator = sample_output.GetCountsForElementForLine(element, "KL2") +
				   sample_output.GetCountsForElementForLine(element, "KL3");
		double denominator = pure_map[element].GetCountsForElementForLine(element, "KL2") +
				     pure_map[element].GetCountsForElementForLine(element, "KL3");
		if (numerator == 0.0) 
			throw BAM::Exception(std::string("Error in calculate_rxi: zero counts detected for ") + element + " in sample_output");
		if (denominator == 0.0) 
			throw BAM::Exception(std::string("Error in calculate_rxi: zero counts detected for ") + element + " in pure_map");
		rxi = numerator / denominator;
	}
	else if (single_element.GetLineType() == "LA_LINE") {
		double numerator = sample_output.GetCountsForElementForLine(element, "L3M4") +
				   sample_output.GetCountsForElementForLine(element, "L3M5");
		double denominator = pure_map[element].GetCountsForElementForLine(element, "L3M4") +
				     pure_map[element].GetCountsForElementForLine(element, "L3M5");
		if (numerator == 0.0) 
			throw BAM::Exception(std::string("Error in calculate_rxi: zero counts detected for ") + element + " in sample_output");
		if (denominator == 0.0) 
			throw BAM::Exception(std::string("Error in calculate_rxi: zero counts detected for ") + element + " in pure_map");
		rxi = numerator / denominator;
	}
	else {
		throw BAM::Exception("Error in calculate_rxi: unknown linetype");
	}
	return rxi;
}

Quant::Quant(BAM::File::RXI::Common *common, std::string outputfile, struct xmi_main_options options) : pure_map(element_comp), options(options), initial_input(common->GetFileXMSI()) {
	//only constructor in the class
	//check if we are dealing with single or multi
	if (dynamic_cast<BAM::File::RXI::Single*>(common)) {
		if (options.verbose)
			std::cout << "Single RXI mode active" << std::endl 
				  << "======================" << std::endl << std::endl;

		BAM::File::RXI::Single *single_rxi = dynamic_cast<BAM::File::RXI::Single*>(common);

		BAM::Data::RXI::Sample sample = single_rxi->GetSample();

		SimulateSample(sample).Write(outputfile);
	}
	else if (dynamic_cast<BAM::File::RXI::Multi*>(common)) {
		if (options.verbose)
			std::cout << "Multi RXI mode active" << std::endl 
				  << "======================" << std::endl << std::endl;

		BAM::File::RXI::Multi *multi_rxi = dynamic_cast<BAM::File::RXI::Multi*>(common);

		//open xml file
		try {
			xmlpp::Document document;
			document.set_internal_subset("bam-quant-rxi-multi-output", "", "http://www.bam.de/xml/bam-quant-rxi.dtd");
			//document.add_comment("this is a comment");
			xmlDocPtr doc = document.cobj();

			xmlpp::Element *rootnode = document.create_root_node("bam-quant-rxi-multi-output");

			if (xmi_write_default_comments(doc, rootnode->cobj()) == 0) {
				throw BAM::Exception("BAM::Job::Quant::Quant -> Could not write XMI-MSIM default comments");
			}
		
			std::stringstream exit_message;
			exit_message << "Summary" << std::endl;
			

			for (int i = 0 ; i < multi_rxi->GetNumberOfSamples() ; i++) {
				BAM::Data::RXI::Sample sample = multi_rxi->GetSample(i);

				if (options.verbose)
					std::cout << "Now processing: " << sample.GetASRfile() << std::endl << std::endl;				

				BAM::File::XMSO *xmso_file;
				try {
					xmso_file = new BAM::File::XMSO(SimulateSample(sample));
				}
				catch (BAM::ExceptionLocal &e) {
					//no convergence for our threshold
					delete xmso_file;
					exit_message << sample.GetASRfile() << ": no convergence" << std::endl;
					continue;
				}
				catch (BAM::Exception &e) {
					throw BAM::Exception(e.what());
				}
				exit_message << sample.GetASRfile() << ": converged successfully" << std::endl;
				xmso_file->SetInputfile(sample.GetASRfile());
				xmlpp::Element *xmimsim_results = rootnode->add_child("xmimsim-results");
				xmlNodePtr node = xmimsim_results->cobj();
				struct xmi_output *xmso_raw = xmso_file->GetInternalPointer();

				if (xmi_write_output_xml_body(doc, node, xmso_raw, -1, -1, 0) == 0) {
					throw BAM::Exception("BAM::Job::Quant::Quant -> Could not write XMI-MSIM output body");
				}
				delete xmso_file;
			}
			document.write_to_file_formatted(outputfile);
			std::cout << exit_message.str();
		}
		catch (BAM::Exception &e) {
			throw BAM::Exception(e.what());
		}
		catch (xmlpp::exception &e) {
			throw BAM::Exception(std::string("BAM::Job::Quant::Quant -> XML++ exception -> ") + e.what());
		}
		catch (std::exception &e) {
			throw BAM::Exception(std::string("BAM::Job::Quant::Quant -> ") + e.what());
		}
		catch (...) {
			throw BAM::Exception(std::string("BAM::Job::Quant::Quant -> Unknown exception occurred"));
		}
	}
	else {
		BAM::Exception("BA::Job::Quant::Quant -> Invalid classtype of common");
	}


}


void Quant::SimulatePure(BAM::Data::RXI::SingleElement single_element) {
	BAM::Data::XMSI::Composition composition_pure;
	BAM::File::XMSI input_pure(initial_input);

	BAM::Data::XMSI::Layer layer_pure1("Air, Dry (near sea level)", 5.0);
	composition_pure.AddLayer(layer_pure1);

	BAM::Data::XMSI::Layer layer_pure2(ElementDensity(SymbolToAtomicNumber(const_cast<char *> (single_element.GetElement().c_str()))), 5.0);
	layer_pure2.AddElement(single_element.GetElement(), 1.0);
	composition_pure.AddLayer(layer_pure2);
	composition_pure.SetReferenceLayer(2);
	input_pure.ReplaceComposition(composition_pure);
	//input_pure.SetOutputFile(Glib::build_filename(Glib::get_tmp_dir(), "bam-quant-" + Glib::get_user_name() + "-" + static_cast<ostringstream*>( &(ostringstream() << getpid()))->str() +  single_element.GetElement()+ ".xmso");

	if (options.verbose)
		std::cout << std::endl << "Simulating " << single_element.GetElement() << std::endl;

	BAM::Job::XMSI job(input_pure, options);
	job.Start();
	pure_map[single_element.GetElement()] = job.GetFileXMSO();
}

BAM::File::XMSO Quant::SimulateSample(BAM::Data::RXI::Sample &sample) {
	/*
	 *
	 * Calculate the intensities of the pure element samples
	 *
	 */

	std::vector<std::string> sample_elements(sample.GetElements());
	BAM::File::XMSI input_sample(initial_input);

	// start preparing for the simulations of the sample
	BAM::Data::XMSI::Composition composition;

	BAM::Data::XMSI::Layer layer1("Air, Dry (near sea level)", 5.0);
	composition.AddLayer(layer1);

	BAM::Data::XMSI::Layer layer2(sample.GetDensity(), sample.GetThickness());
	

	if (options.verbose) 
		std::cout << "Start simulations of the pure elements" << std::endl <<
				     "======================================" << std::endl;
	for (std::vector<std::string>::iterator it = sample_elements.begin() ; it != sample_elements.end() ; ++it) {
		if (pure_map.find(*it) == pure_map.end())
			SimulatePure(sample.GetSingleElement(*it));
		BAM::Data::RXI::SingleElement single_element(sample.GetSingleElement(*it));
		layer2.AddElement(single_element.GetElement(), single_element.GetRXI());
	}

	layer2.Normalize();

	if (options.verbose) {
		std::cout << std::endl << "Initial composition" << std::endl << layer2;
	}
	composition.AddLayer(layer2);
	composition.SetReferenceLayer(2);
	input_sample.ReplaceComposition(composition);

	int iteration = 0;

	if (options.verbose) 
		std::cout << "Start simulations of the sample" << std::endl <<
			     "===============================" << std::endl << std::endl;
		
	std::map<std::string, double, bool(*)(std::string,std::string)> rxi_rel(element_comp);
	for (std::vector<std::string>::iterator it = sample_elements.begin() ; it != sample_elements.end() ; ++it) {
		rxi_rel[*it] = BAM_QUANT_CONV_THRESHOLD*10.0;
	}

	BAM::Job::XMSI *job(0);
	int counted = 0;

	do {
		//the big do while loop
		if (iteration++ > BAM_QUANT_MAX_ITERATIONS) {
			if (job)
				delete job;
			throw BAM::ExceptionLocal("BAM::File::XMSO Quant::SimulateSample -> No convergence after 100 iterations");
		}
		if (options.verbose)
			std::cout << "Iteration: " << iteration << std::endl << std::endl;
		//create job
		if (job)
			delete job;

		job = new BAM::Job::XMSI(input_sample, options);
		job->Start();
	
		//get FileXMSO 
		BAM::File::XMSO output(job->GetFileXMSO());

		//first get the composition back
		BAM::Data::XMSI::Composition composition_old(input_sample.GetComposition());
		BAM::Data::XMSI::Layer layer_old(composition_old.GetLayer(2));
		std::map<std::string, double> layer_old_map(layer_old.GetZandWeightMap());

		BAM::Data::XMSI::Layer layer_new(layer_old);
		layer_new.RemoveElements();

		//calculate RXIs
		//and update the concentrations
		for (std::vector<std::string>::iterator it = sample_elements.begin() ; it != sample_elements.end() ; ++it) {
			BAM::Data::RXI::SingleElement single_element = sample.GetSingleElement(*it);
			double rxi = calculate_rxi(*it, output, single_element);	
			rxi_rel[*it] = fabs(rxi-single_element.GetRXI())/single_element.GetRXI();
			double rxi_scale = single_element.GetRXI()/rxi;

			//make sure that rxi_scale doesnt go wild!
			double max_scale;
			if (layer_old_map[*it] <= 0.0001)
				max_scale = 100.0;
			else if (layer_old_map[*it] <= 0.01)
				max_scale = 10.0;
			else if (layer_old_map[*it] <= 0.1)
				max_scale = 2.5;
			else if (layer_old_map[*it] <= 0.25)
				max_scale = 1.5;
			else if (layer_old_map[*it] <= 0.375)
				max_scale = 1.2;
			else if (layer_old_map[*it] <= 0.50)
				max_scale = 1.1;
			else if (layer_old_map[*it] <= 0.6)
				max_scale = 1.05;
			else if (layer_old_map[*it] <= 0.7)
				max_scale = 1.025;
			else 
				max_scale = 1.01;
			
			if (options.verbose)
				std::cout << *it << ": RXI -> " << rxi << " [" << single_element.GetRXI() << "]" << std::endl;

			double new_weight = layer_old_map[*it]*std::min(rxi_scale, max_scale);
			layer_new.AddElement(*it, new_weight);
		}
		layer_new.Normalize();
		if (options.verbose) {
			std::cout << std::endl << "New composition" << std::endl << layer_new;
		}
		composition_old.ReplaceLayer(layer_new, 2);
		input_sample.ReplaceComposition(composition_old);

		counted = std::count_if(rxi_rel.begin(), rxi_rel.end(), rxi_match);

		if (options.verbose) {
			std::cout << "Composition matched: " << counted << "/" << rxi_rel.size() << std::endl;
		}

	}
	while (counted != rxi_rel.size());

	BAM::File::XMSO rv = job->GetFileXMSO();
	delete job;

	return rv;
}
