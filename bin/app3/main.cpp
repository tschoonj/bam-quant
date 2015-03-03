#include <glibmm/miscutils.h>
#include <glibmm/optioncontext.h>
#include <glib.h>
#include "app3-optionentry.h"
#include <xmi_msim.h>
#include <iostream>
#include "bam_catalog.h"
#include "bam_file_rxi.h" 
#include "bam_job_xmsi.h" 


#define BAM_QUANT_MAX_ITERATIONS 100
#define BAM_QUANT_CONV_THRESHOLD 0.001

bool element_comp (std::string lhs, std::string rhs) {return SymbolToAtomicNumber((char *) lhs.c_str()) < SymbolToAtomicNumber((char *) rhs.c_str());}

double calculate_rxi(std::string element, BAM::File::XMSO sample_output, std::map<std::string, BAM::File::XMSO, bool(*)(std::string,std::string)> pure_map, BAM::Data::RXI::SingleElement single_element) {
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



int main(int argc, char **argv) {
	Glib::set_application_name("app3");
#if defined(G_OS_WIN32)
        setlocale(LC_ALL,"English_United States");
        //g_setenv("LANG","en_US",TRUE);
#else
        Glib::setenv("LANG","en_US",TRUE);
#endif
        setbuf(stdout,NULL);

	if (bam_xmlLoadCatalog() == 0) {
		return 1;
	}
	if (xmi_xmlLoadCatalog() == 0) {
		return 1;
	}

	static struct xmi_main_options options;
        static std::string spe_file_noconv;
        static std::string spe_file_conv;
        static std::string csv_file_noconv;
        static std::string csv_file_conv;
        static std::string svg_file_noconv;
        static std::string svg_file_conv;
        static std::string htm_file_noconv;
        static std::string htm_file_conv;
	static std::string custom_detector_response;
	static std::string hdf5_file;
        static std::string xmimsim_hdf5_solid_angles;
        static std::string xmimsim_hdf5_escape_ratios;
        bool version = false;
	std::vector<std::string> argv_files;


        options.use_M_lines = 1;
        options.use_cascade_auger = 1;
        options.use_cascade_radiative = 1;
        options.use_variance_reduction = 1;
        options.use_sum_peaks = 0;
        options.use_escape_peaks = 1;
        options.use_poisson = 0;
        options.verbose = 0;
        options.use_opencl = 0;
        options.extra_verbose = 0;
        options.omp_num_threads = xmi_omp_get_max_threads();
        options.custom_detector_response = NULL;
        options.use_advanced_compton = 0;



	Glib::OptionContext option_context("inputfile XMSO-file");
	option_context.set_help_enabled();
	Glib::OptionGroup option_group("Options", "Options");

	//option_group.add_entry(App3OptionEntry( "enable-M-lines", 0, "Enable M lines (default)"), (bool&) options.use_M_lines);
	option_group.add_entry(App3OptionEntry( "disable-M-lines", 0, "Disable M lines", "", Glib::OptionEntry::FLAG_REVERSE), (bool&) options.use_M_lines);
	//option_group.add_entry(App3OptionEntry( "enable-auger-cascade", 0, "Enable Auger (non radiative) cascade effects (default)"), (bool&) options.use_cascade_auger);
	option_group.add_entry(App3OptionEntry( "disable-auger-cascade", 0, "Disable Auger (non radiative) cascade effects", "", Glib::OptionEntry::FLAG_REVERSE), (bool&) options.use_cascade_auger);
	//option_group.add_entry(App3OptionEntry( "enable-radiative-cascade", 0, "Enable radiative cascade effects (default)"), (bool&) options.use_cascade_radiative);
	option_group.add_entry(App3OptionEntry( "disable-radiative-cascade", 0, "Disable radiative cascade effects", "", Glib::OptionEntry::FLAG_REVERSE), (bool&) options.use_cascade_radiative);
	//option_group.add_entry(App3OptionEntry( "enable-variance-reduction", 0, "Enable variance reduction (default)"), (bool&) options.use_variance_reduction);
	option_group.add_entry(App3OptionEntry( "disable-variance-reduction", 0, "Disable variance reduction", "", Glib::OptionEntry::FLAG_REVERSE), (bool&) options.use_variance_reduction);
	option_group.add_entry_filename(App3OptionEntry( "with-hdf5-data", 0, "Select a HDF5 data file (advanced usage)", "file", Glib::OptionEntry::FLAG_HIDDEN), hdf5_file);
	option_group.add_entry_filename(App3OptionEntry( "with-solid-angles-data", 0, "Select a HDF5 solid angles file (advanced usage)", "file", Glib::OptionEntry::FLAG_HIDDEN), xmimsim_hdf5_solid_angles);
	option_group.add_entry_filename(App3OptionEntry( "with-escape-ratios-data", 0, "Select a HDF5 escape ratios file (advanced usage)", "file", Glib::OptionEntry::FLAG_HIDDEN), xmimsim_hdf5_escape_ratios);
	option_group.add_entry_filename(App3OptionEntry( "spe-file", 0, "Write detector convoluted spectra to file", "file"), spe_file_conv);
	option_group.add_entry_filename(App3OptionEntry( "spe-file-unconvoluted", 0, "Write detector unconvoluted spectra to file", "file"), spe_file_noconv);
	option_group.add_entry_filename(App3OptionEntry( "csv-file", 0, "Write detector convoluted spectra to CSV file", "file"), csv_file_conv);
	option_group.add_entry_filename(App3OptionEntry( "csv-file-unconvoluted", 0, "Write detector unconvoluted spectra to CSV file", "file"), csv_file_noconv);
	option_group.add_entry_filename(App3OptionEntry( "svg-file", 0, "Write detector convoluted spectra to SVG file", "file"), svg_file_conv);
	option_group.add_entry_filename(App3OptionEntry( "svg-file-unconvoluted", 0, "Write detector unconvoluted spectra to SVG file", "file"), svg_file_noconv);
	option_group.add_entry_filename(App3OptionEntry( "htm-file", 0, "Write detector convoluted spectra to HTML file", "file"), htm_file_conv);
	option_group.add_entry_filename(App3OptionEntry( "htm-file-unconvoluted", 0, "Write detector unconvoluted spectra to HTML file", "file"), htm_file_noconv);
	option_group.add_entry(App3OptionEntry( "enable-pile-up", 0, "Enable pile-up"), (bool&) options.use_sum_peaks);
	//option_group.add_entry(App3OptionEntry( "disable-pile-up", 0, "Disable pile-up (default)", "", Glib::OptionEntry::FLAG_REVERSE), (bool&) options.use_sum_peaks);
	//option_group.add_entry(App3OptionEntry( "enable-escape-peaks", 0, "Enable escape peaks (default)"), (bool&) options.use_escape_peaks);
	option_group.add_entry(App3OptionEntry( "disable-escape-peaks", 0, "Disable escape peaks", "", Glib::OptionEntry::FLAG_REVERSE), (bool&) options.use_escape_peaks);
	option_group.add_entry(App3OptionEntry( "enable-poisson", 0, "Generate Poisson noise in the spectra"), (bool&) options.use_poisson);
	//option_group.add_entry(App3OptionEntry( "disable-poisson", 0, "Disable the generating of spectral Poisson noise (default)", "", Glib::OptionEntry::FLAG_REVERSE), (bool&) options.use_poisson);
	option_group.add_entry(App3OptionEntry( "enable-opencl", 0, "Enable OpenCL"), (bool&) options.use_opencl);
	//option_group.add_entry(App3OptionEntry( "disable-opencl", 0, "Disable OpenCL (default)", "", Glib::OptionEntry::FLAG_REVERSE), (bool&) options.use_opencl);
	option_group.add_entry(App3OptionEntry( "enable-advanced-compton", 0, "Enable advanced yet slower Compton simulation"), (bool&) options.use_advanced_compton);
	//option_group.add_entry(App3OptionEntry( "disable-advanced-compton", 0, "Disable advanced yet slower Compton simulation (default)", "", Glib::OptionEntry::FLAG_REVERSE), (bool&) options.use_advanced_compton);
	option_group.add_entry_filename(App3OptionEntry( "custom-detector-response", 0, "Use the supplied library for the detector response routine", "file"), custom_detector_response);
	option_group.add_entry(App3OptionEntry( "set-threads", 0, "Set the number of threads (default=max)", "N"), options.omp_num_threads);
	option_group.add_entry(App3OptionEntry( "verbose", 'v', "Verbose mode"), (bool&) options.verbose);
	option_group.add_entry(App3OptionEntry( "very-verbose", 'V', "Even more verbose mode"), (bool&) options.extra_verbose);
	option_group.add_entry(App3OptionEntry( "version", 0, "Display version information"), version);
	option_group.add_entry_filename(App3OptionEntry( G_OPTION_REMAINING), argv_files);

	option_context.set_main_group(option_group);
	option_context.set_ignore_unknown_options();

	option_context.parse(argc, argv);

        if (version) {
                std::cout << xmi_version_string() << std::endl;
                return 0;
        }

        if (argv_files.size() != 2) {
		std::cerr << option_context.get_help(true) << std::endl;
                return 1;
        }		

	if (options.extra_verbose) {
                options.verbose = 1;
		std::cout << "Option M-lines: " << options.use_M_lines << std::endl;
                std::cout << "Option non-radiative cascade: " << options.use_cascade_auger << std::endl;
                std::cout << "Option radiative cascade: " << options.use_cascade_radiative << std::endl;
                std::cout << "Option variance reduction: " << options.use_variance_reduction << std::endl;
                std::cout << "Option pile-up: " << options.use_sum_peaks << std::endl;
                std::cout << "Option Poisson noise: " << options.use_poisson << std::endl;
                std::cout << "Option escape peaks: " << options.use_escape_peaks << std::endl;
                std::cout << "Option OpenCL: " << options.use_opencl << std::endl;
                std::cout << "Option number of threads: " << options.omp_num_threads << std::endl;
                std::cout << "Option advanced Compton: " << options.use_advanced_compton << std::endl;
        }


	try {
		BAM::Job::XMSI::RandomNumberAcquisitionStart();

		//start by reading in the inputfile
		BAM::File::RXI::Single single_rxi(argv_files[0]);
		BAM::Data::RXI::Sample sample = single_rxi.GetSample();

		//now next -> produce the initial XMSI
		BAM::File::XMSI initial_input(single_rxi.GetFileXMSI());

		/*
		 *
		 * Calculate the intensities of the pure element samples
		 *
		 */
		//bool(*element_comp_pt)(std::string,std::string) = element_comp;
		std::map<std::string,BAM::File::XMSO,bool(*)(std::string,std::string)> pure_map(element_comp);

		std::vector<std::string> sample_elements(sample.GetElements());

		if (options.verbose) 
			std::cout << "Start simulations of the pure elements" << std::endl <<
				     "======================================" << std::endl << std::endl;
		for (std::vector<std::string>::iterator it = sample_elements.begin() ; it != sample_elements.end() ; ++it) {
			BAM::Data::RXI::SingleElement single_element = sample.GetSingleElement(*it);
			BAM::Data::XMSI::Composition composition_pure;
			BAM::File::XMSI input_pure(initial_input);

			BAM::Data::XMSI::Layer layer_pure1("Air, Dry (near sea level)", 5.0);
			composition_pure.AddLayer(layer_pure1);

			BAM::Data::XMSI::Layer layer_pure2(ElementDensity(SymbolToAtomicNumber((char *) single_element.GetElement().c_str())), 5.0);
			layer_pure2.AddElement(single_element.GetElement(), 1.0);
			composition_pure.AddLayer(layer_pure2);
			composition_pure.SetReferenceLayer(2);
			input_pure.ReplaceComposition(composition_pure);
			//input_pure.SetOutputFile(Glib::build_filename(Glib::get_tmp_dir(), "bam-quant-" + Glib::get_user_name() + "-" + static_cast<ostringstream*>( &(ostringstream() << getpid()))->str() +  single_element.GetElement()+ ".xmso");

			BAM::Job::XMSI job(input_pure, options);
			job.Start();
			pure_map[*it] = job.GetFileXMSO();
		}

		//now change its composition
		BAM::Data::XMSI::Composition composition;

		BAM::Data::XMSI::Layer layer1("Air, Dry (near sea level)", 5.0);
		composition.AddLayer(layer1);

		BAM::Data::XMSI::Layer layer2(sample.GetDensity(), sample.GetThickness());
		
		//starting weights determined by normalizing the array of RXI's
		for (std::vector<std::string>::iterator it = sample_elements.begin() ; it != sample_elements.end() ; ++it) {
			BAM::Data::RXI::SingleElement single_element(sample.GetSingleElement(*it));
			layer2.AddElement(single_element.GetElement(), single_element.GetRXI());
		}
		layer2.Normalize();
		composition.AddLayer(layer2);
		composition.SetReferenceLayer(2);
		initial_input.ReplaceComposition(composition);

		double sum_all;
		int iteration = 0;

		sum_all = BAM_QUANT_CONV_THRESHOLD*10.0;

		if (options.verbose) 
			std::cout << "Start simulations of the sample" << std::endl <<
				     "===============================" << std::endl << std::endl;
		
		while ((sum_all > BAM_QUANT_CONV_THRESHOLD)) {
			//the big while loop
			if (iteration++ > BAM_QUANT_MAX_ITERATIONS) {
				throw BAM::Exception("No convergence after 100 iterations");
			}
			if (options.verbose)
				std::cout << "Iteration: " << iteration << std::endl << std::endl;
			//create job
			BAM::Job::XMSI job(initial_input, options);
			job.Start();
		
			//get FileXMSO 
			BAM::File::XMSO output(job.GetFileXMSO());

			sum_all = 0;

			//calculate RXIs
			for (std::vector<std::string>::iterator it = sample_elements.begin() ; it != sample_elements.end() ; ++it) {
				BAM::Data::RXI::SingleElement single_element = sample.GetSingleElement(*it);
				double rxi = calculate_rxi(*it, output, pure_map, single_element);	
				sum_all += (rxi-single_element.GetRXI())*(rxi-single_element.GetRXI());
			}
		}

		BAM::Job::XMSI::RandomNumberAcquisitionStop();
	}
	catch (BAM::Exception &e) {
		std::cerr << "Fatal BAM exception: " << e.what() << std::endl;
		return 1;
	}
	catch (std::exception &e) {
		std::cerr << "Fatal non-BAM exception: " << e.what() << std::endl;
		return 1;
	}
	catch (Glib::Exception &e) {
		std::cerr << "Fatal Glib exception: " << e.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "Unknown exception occurred" << std::endl;
		return 1;
	}


	return 0;
}

