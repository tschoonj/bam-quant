#include "config.h"
#include "bam_job_quant.h"
#include "bam_exception.h"
#include <libxml++/libxml++.h>
#include <cmath>
#include <xraylib.h>

using namespace BAM::Job;

namespace BAM {
	class ExceptionLocal : public Exception {
	public:
		explicit ExceptionLocal(const std::string &s) : Exception(s) {}
		virtual ~ExceptionLocal() throw() {};
			
	};
}


double Quant::calculate_rxi(std::string element, std::map<double,BAM::File::XMSO> &sample_output, BAM::Data::RXI::SingleElement single_element) {
	double rxi;
	std::map<double,BAM::File::XMSO>::iterator iter;

	if ((iter = sample_output.find(single_element.GetExcitationEnergy())) == sample_output.end()) {
		throw BAM::Exception("BAM::Quant::calculate_rxi -> Excitation energy not found in sample_output");
	}

	BAM::File::XMSO sample_output_local(iter->second);

	//everything depends on linetype!
	if (single_element.GetLineType() == "KA_LINE") {
		double numerator = sample_output_local.GetCountsForElementForLine(element, "KL2") +
				   sample_output_local.GetCountsForElementForLine(element, "KL3");
		double denominator = pure_map[element].GetCountsForElementForLine(element, "KL2") +
				     pure_map[element].GetCountsForElementForLine(element, "KL3");
		if (numerator == 0.0) 
			throw BAM::Exception(std::string("BAM::Job::Quant::calculate_rxi -> zero counts detected for ") + element + " in sample_output");
		if (denominator == 0.0) 
			throw BAM::Exception(std::string("BAM::Job::Quant::calculate_rxi -> zero counts detected for ") + element + " in pure_map");
		rxi = numerator / denominator;
	}
	else if (single_element.GetLineType() == "LA_LINE") {
		double numerator = sample_output_local.GetCountsForElementForLine(element, "L3M4") +
				   sample_output_local.GetCountsForElementForLine(element, "L3M5");
		double denominator = pure_map[element].GetCountsForElementForLine(element, "L3M4") +
				     pure_map[element].GetCountsForElementForLine(element, "L3M5");
		if (numerator == 0.0) 
			throw BAM::Exception(std::string("BAM::Job::Quant::calculate_rxi -> zero counts detected for ") + element + " in sample_output");
		if (denominator == 0.0) 
			throw BAM::Exception(std::string("BAM::Job::Quant::calculate_rxi -> zero counts detected for ") + element + " in pure_map");
		rxi = numerator / denominator;
	}
	else {
		throw BAM::Exception("BAM::Job::Quant::calculate_rxi -> unknown linetype");
	}
	return rxi;
}

Quant::Quant(BAM::File::RXI::Common *common_arg, std::string outputfile, struct xmi_main_options options, double conv_threshold) : pure_map(element_comp), options(options), common(common_arg), conv_threshold(conv_threshold) {

	if (options.verbose) {
		std::cout << "BAM::Job::Quant started" << std::endl;
		std::cout << "=======================" << std::endl;

		std::cout << "RXI convergence threshold: " << conv_threshold << std::endl << std::endl;
	}

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
			

			for (unsigned int i = 0 ; i < multi_rxi->GetNumberOfSamples() ; i++) {
				BAM::Data::RXI::Sample sample = multi_rxi->GetSample(i);

				if (options.verbose)
					std::cout << "Now processing Sample " << i << std::endl << std::endl;				

				BAM::File::XMSO *xmso_file(0);
				try {
					xmso_file = new BAM::File::XMSO(SimulateSample(sample));
				}
				catch (BAM::ExceptionLocal &e) {
					//no convergence for our threshold
					if (xmso_file)
						delete xmso_file;
					exit_message << "Sample " << i << ": no convergence" << std::endl;
					continue;
				}
				catch (BAM::Exception &e) {
					throw BAM::Exception(e.what());
				}
				exit_message << "Sample " << i << ": converged successfully" << std::endl;
				std::stringstream ss;
				ss << "Sample " << i;
				xmso_file->SetInputfile(ss.str());
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
	//first we need to check if there is an XMSI file available for our excitation energy
	if (common->xmimsim_input.find(single_element.GetExcitationEnergy()) == common->xmimsim_input.end())
		throw BAM::Exception("BAM::Job::Quant::SimulatePure -> excitation energy not found in xmimsim_input");
	BAM::File::XMSI input_pure(common->xmimsim_input[single_element.GetExcitationEnergy()]);

	BAM::Data::XMSI::Layer layer_pure1("Air, Dry (near sea level)", 5.0);
	composition_pure.AddLayer(layer_pure1);

	BAM::Data::XMSI::Layer layer_pure2(ElementDensity(SymbolToAtomicNumber(const_cast<char *> (single_element.GetElement().c_str()))), 5.0);
	layer_pure2.AddElement(single_element.GetElement(), 1.0);
	composition_pure.AddLayer(layer_pure2);
	composition_pure.SetReferenceLayer(2);
	input_pure.ReplaceComposition(composition_pure);
	//input_pure.SetOutputFile(Glib::build_filename(Glib::get_tmp_dir(), "bam-quant-" + Glib::get_user_name() + "-" + static_cast<ostringstream*>( &(ostringstream() << getpid()))->str() +  single_element.GetElement()+ ".xmso");

	if (options.verbose)
		std::cout << std::endl << "Simulating " << single_element.GetElement() << " at " << single_element.GetExcitationEnergy() << " keV" << std::endl;

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
	std::map<double, BAM::File::XMSI> input_sample(common->xmimsim_input);

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

	//if matrix available -> intervene
	const BAM::Data::Base::Composition *matrix = sample.GetMatrix();

	if (matrix) {
		double current_sum = layer2.GetSum();
		if (current_sum < 1.0) {
			layer2 *= 0.5/current_sum;
			current_sum = 0.5;
		}
		layer2.SetComposition(layer2 + (1.0-current_sum) * *matrix);
	}
	else {
		layer2.Normalize();
	}

	if (options.verbose) {
		std::cout << std::endl << "Initial composition" << std::endl << layer2 << std::endl;
	}
	composition.AddLayer(layer2);
	composition.SetReferenceLayer(2);
	for (std::map<double,BAM::File::XMSI>::iterator iter = input_sample.begin() ; 
	     iter != input_sample.end() ;
	     ++iter) {
		iter->second.ReplaceComposition(composition);     
	}

	int iteration = 0;

	if (options.verbose) {
		std::cout << "Start simulations of the sample" << std::endl <<
			     "===============================" << std::endl << std::endl;
		if (sample.GetDensityThicknessFixed())
			std::cout << "Density and thickness are fixed!" << std::endl << std::endl;
		else
			std::cout << "Density and thickness are variable!" << std::endl << std::endl;
		
		if (matrix)
			std::cout << "Matrix detected: " << sample.GetMatrixString() << std::endl << std::endl;
		else 
			std::cout << "No Matrix detected" << std::endl << std::endl;
	}

	std::map<std::string, double, bool(*)(std::string,std::string)> rxi_rel(element_comp);
	for (std::vector<std::string>::iterator it = sample_elements.begin() ; it != sample_elements.end() ; ++it) {
		rxi_rel[*it] = conv_threshold*10.0;
	}

	//BAM::Job::XMSI *job(0);
	std::map<double, BAM::Job::XMSI *> jobs;
	int counted = 0;

	int layer_trouble = 0;

	do {
		//the big do while loop
		if (iteration++ > BAM_QUANT_MAX_ITERATIONS) {
			for (std::map<double, BAM::Job::XMSI *>::iterator iter = jobs.begin() ;
			     iter != jobs.end() ;
			     ++iter) {
			     	if (iter->second) {
					delete iter->second;
					iter->second = 0;
				}
			}
			throw BAM::ExceptionLocal("BAM::File::XMSO Quant::SimulateSample -> No convergence after 100 iterations");
		}
		if (options.verbose)
			std::cout << "Iteration: " << iteration << std::endl << std::endl;
		//create job
		for (std::map<double, BAM::Job::XMSI *>::iterator iter = jobs.begin() ;
		     iter != jobs.end() ;
		     ++iter) {
		     	if (iter->second) {
				delete iter->second;
				iter->second = 0;
			}
		}

		std::map<double,BAM::File::XMSO> output;

		for (std::map<double, BAM::File::XMSI>::iterator iter = input_sample.begin() ;
		     iter != input_sample.end() ;
		     ++iter) {
			BAM::Job::XMSI *job = new BAM::Job::XMSI(iter->second, options);
			if (options.verbose)
				std::cout << std::endl << "Simulating sample at " << iter->first << " keV" << std::endl;
			job->Start();
	
			//get FileXMSO 
			output[iter->first] = job->GetFileXMSO();
			jobs[iter->first] = job;
		}

		//first get the composition back
		BAM::Data::XMSI::Composition composition_old(input_sample.begin()->second.GetComposition());
		BAM::Data::XMSI::Layer layer_old(composition_old.GetLayer(2));
		std::map<std::string, double> layer_old_map(layer_old.GetZandWeightMap());

		BAM::Data::XMSI::Layer layer_new(layer_old);

		if (!sample.GetDensityThicknessFixed() && iteration % 2 == 0) {
			//variable density/thickness mode
		}
		else {
			layer_new.RemoveElements();

		}

		//calculate RXIs
		//and update the concentrations
		std::string it_max_xrf_energy;
		double max_xrf_energy(0.0);
		double max_xrf_energy_rxi_scale(0.0);

		for (std::vector<std::string>::iterator it = sample_elements.begin() ; it != sample_elements.end() ; ++it) {
			BAM::Data::RXI::SingleElement single_element = sample.GetSingleElement(*it);
			double rxi = calculate_rxi(*it, output, single_element);	
			rxi_rel[*it] = fabs(rxi-single_element.GetRXI())/single_element.GetRXI();
			double rxi_scale = single_element.GetRXI()/rxi;

			if (!sample.GetDensityThicknessFixed() && iteration % 2 == 0) {
				//variable density/thickness mode
				//basically look for the element whose XRF has the highest energy and therefore the highest Acorr	
				if (single_element.GetLineEnergy() > max_xrf_energy) {
					it_max_xrf_energy = *it;
					max_xrf_energy = single_element.GetLineEnergy();
					max_xrf_energy_rxi_scale = rxi_scale;
				}
			}
			else {
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
				double new_weight = layer_old_map[*it]*std::min(rxi_scale, max_scale);
				layer_new.AddElement(*it, new_weight);
				//std::cout << "old weight: " << layer_old_map[*it] << std::endl;
				//std::cout << "new weight: " << new_weight << std::endl;
				//std::cout << "rxi_scale: " << rxi_scale << std::endl;
			}
			
			if (options.verbose)
				std::cout << *it << ": RXI -> " << rxi << " [" << single_element.GetRXI() << "]" << std::endl;

		}

		counted = std::count_if(rxi_rel.begin(), rxi_rel.end(), rxi_match(conv_threshold));

		if (options.verbose) {
			std::cout << "Composition matched: " << counted << "/" << rxi_rel.size() << std::endl;
		}
		if (counted == (int) rxi_rel.size()) {
			std::cout << std::endl << "Definitive composition" << std::endl << layer_old << std::endl;
			break;
		}

		if (!sample.GetDensityThicknessFixed() && iteration % 2 == 0) {
			//variable density/thickness mode
			//get new density/thickness
			//for this we will use the line with the highest energy
			BAM::Data::RXI::SingleElement single_element = sample.GetSingleElement(it_max_xrf_energy);
			//calculate Acorr to see what kind of sample we are dealing with
			BAM::Data::XMSI::Layer::AcorrCases a_corr_case;
			double excitation_energy = single_element.GetExcitationEnergy();
			double a_corr = layer_new.Acorr(excitation_energy, single_element.GetLineEnergy(), &a_corr_case, input_sample[excitation_energy].GetGeometry().GetAlpha(), input_sample[excitation_energy].GetGeometry().GetBeta()); 

			if (options.verbose) {
				std::cout << "Current Acorr: " << a_corr << std::endl;
				std::cout << "Current Acorr case : " << a_corr_case << std::endl;
			}
			
			if (options.verbose) {
				std::cout << "Old density: " << layer_new.GetDensity() << std::endl;
			}

			//calculate Chi
			double chi = layer_new.Chi(excitation_energy, single_element.GetLineEnergy(), input_sample[excitation_energy].GetGeometry().GetAlpha(), input_sample[excitation_energy].GetGeometry().GetBeta());

			double thickness_density_old = layer_new.GetDensity() * layer_new.GetThickness();
			double thickness_density_new = a_corr *  thickness_density_old * max_xrf_energy_rxi_scale * chi;
			thickness_density_new = 1.0 - thickness_density_new;
			thickness_density_new = -1.0 * log(thickness_density_new) / chi;
			layer_new.SetDensity(layer_new.GetDensity() * thickness_density_new / thickness_density_old);	
		}
		else if (matrix) {
			//check current sum
			double current_sum = layer_new.GetSum();
			if (current_sum >= 1.0) {
				//this should not happen -> it would indicate that something very fishy is going on with the RXI's
				layer_trouble++;
				layer_new.Normalize();
				std::cout << "BAM::Job::Quant::SimulateSample -> matrix detected but sum of quantifiable elements greater than 100%!!!" << std::endl;
				if (layer_trouble == 5) {
					throw BAM::Exception("BAM::Job::Quant::SimulateSample -> sum of quantifiable elements was 5 times greater than 100%");
				}
			}
			else {
				if (layer_trouble > 0)
					layer_trouble--;
				layer_new.SetComposition(layer_new + (1.0-current_sum) * *matrix);
			}
		}
		else {
			layer_new.Normalize();
		}

		if (options.verbose) {
			std::cout << std::endl << "New composition" << std::endl << layer_new << std::endl;
		}

		composition_old.ReplaceLayer(layer_new, 2);
		for (std::map<double,BAM::File::XMSI>::iterator iter = input_sample.begin() ; 
		     iter != input_sample.end() ;
		     ++iter) {
			iter->second.ReplaceComposition(composition_old);
		}


	}
	while (1);

	BAM::File::XMSO rv = jobs.begin()->second->GetFileXMSO();
	for (std::map<double, BAM::Job::XMSI *>::iterator iter = jobs.begin() ;
	     iter != jobs.end() ;
	     ++iter) {
	     	if (iter->second) {
			delete iter->second;
			iter->second = 0;
		}
	}

	return rv;
}
