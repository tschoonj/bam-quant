#include "bam_job_xmsi.h"
#include <iostream>
#include <libxml/tree.h>
#include <glibmm/module.h>
#include <cstring>


using namespace BAM;
using namespace BAM::Job;

bool XMSI::random_acquisition_started = false;
int XMSI::ObjectCounter = 0;

void XMSI::Initialize() {
	

	xmi_init_hdf5();

	//start random number acquisition
	if (!random_acquisition_started) {
		if (xmi_start_random_acquisition() == 0) {
			BAM::Exception("BAM::Job::XMSI::Initialize -> Could not start random number acquisition");
		}
		random_acquisition_started = true;
	}

	//check number of threads
	if (options.omp_num_threads > xmi_omp_get_max_threads() || options.omp_num_threads < 1) {
		options.omp_num_threads = xmi_omp_get_max_threads();
	}

	//check the hdf5 files
	if (!hdf5_file.empty())
		hdf5_file_c = strdup(hdf5_file.c_str());

	if (xmi_get_hdf5_data_file(&hdf5_file_c) == 0) {
		BAM::Exception("BAM::Job::XMSI::Initialize -> Error in xmi_get_hdf5_data_file");
	}


	//get Fortran counterpart of input
	xmi_input_C2F(xmimsim_input->GetInternalPointer(), &inputFPtr);

	if (xmi_init_input(&inputFPtr) == 0) {
		BAM::Exception("BAM::Job::XMSI::Initialize -> Error in xmi_init_input");
	}
	
	if (options.verbose)
                std::cout <<"Reading HDF5 datafile" << std::endl;

        //read from HDF5 file what needs to be read in
        if (xmi_init_from_hdf5(hdf5_file_c, inputFPtr, &hdf5FPtr, options) == 0) {
                BAM::Exception("Could not initialize from hdf5 data file");
        }
        else if (options.verbose)
                std::cout << "HDF5 datafile "<< hdf5_file <<" successfully processed" << std::endl;

        xmi_update_input_from_hdf5(inputFPtr, hdf5FPtr);	

}

void XMSI::Start() {

	/*
	 *
	 * 1) solid angle grid
	 * 2) run the actual simulation
	 * 3)
	 *
	 */
	if (options.use_variance_reduction) {
		if (!xmimsim_hdf5_solid_angles.empty())
			xmimsim_hdf5_solid_angles_c = strdup(xmimsim_hdf5_solid_angles.c_str());

		if (xmi_get_solid_angle_file(&xmimsim_hdf5_solid_angles_c, 1) == 0)
			BAM::Exception("BAM::Job::XMSI::Start-> Error in xmi_get_solid_angle_file");

	        //check if solid angles are already precalculated
                if (options.verbose)
                        std::cout << "Querying " << xmimsim_hdf5_solid_angles << " for solid angle grid" << std::endl;

                if (xmi_find_solid_angle_match(xmimsim_hdf5_solid_angles_c , xmimsim_input->GetInternalPointer(), &solid_angles, options) == 0)
			BAM::Exception("BAM::Job::XMSI::Start -> Error in xmi_find_solid_angle_match");

                if (solid_angles == 0) {
                        if (options.verbose)
                                std::cout << "Precalculating solid angle grid" << std::endl;
                        //doesn't exist yet
                        //convert input to string
			char *xmi_input_string = 0;

                        if (xmi_write_input_xml_to_string(&xmi_input_string, xmimsim_input->GetInternalPointer()) == 0) {
				BAM::Exception("BAM::Job::XMSI::Start -> Error in xmi_write_input_xml_to_string");
                        }

                        xmi_solid_angle_calculation(inputFPtr, &solid_angles, xmi_input_string, options);

                        //update hdf5 file
                        if( xmi_update_solid_angle_hdf5_file(xmimsim_hdf5_solid_angles_c, solid_angles) == 0)
				BAM::Exception("BAM::Job::XMSI::Start -> Error in xmi_update_solid_angle_hdf5_file");
                        else if (options.verbose)
                                std::cout << xmimsim_hdf5_solid_angles_c << " was successfully updated with new solid angle grid" << std::endl;

			xmlFree(xmi_input_string);
                }
                else if (options.verbose)
                        std::cout << "Solid angle grid already present in " << xmimsim_hdf5_solid_angles_c << std::endl;

	}
	else if (options.verbose)
		std::cout << "Operating in brute-force mode: solid angle grid is redundant" << std::endl;
	
	if (options.verbose)
		std::cout << "Simulating interactions" << std::endl;

	double *channels;
	double **channels_conv;
	double *brute_history;
	double *var_red_history;
	double zero_sum;

	if (xmi_main_msim(inputFPtr, hdf5FPtr, 1, &channels, options, &brute_history, &var_red_history, solid_angles) == 0) {
		BAM::Exception("BAM::Job::XMSI::Start -> Error in xmi_main_msim");
	}
	
	if (options.verbose)
		std::cout << "Interactions simulation finished" << std::endl;

#define ARRAY3D_FORTRAN(array,i,j,k,Ni,Nj,Nk) (array[(Nj)*(Nk)*(i-1)+(Nk)*(j-1)+(k-1)])
//watch out, I'm doing something naughty here :-)
#define ARRAY2D_FORTRAN(array,i,j,Ni,Nj) (array[(Nj)*(i)+(j)])

	zero_sum = xmi_sum_double(channels, xmimsim_input->GetInternalPointer()->detector->nchannels);

	//convolute spectrum
	channels_conv = (double **) malloc(sizeof(double *)*(xmimsim_input->GetInternalPointer()->general->n_interactions_trajectory+1));

	if (options.use_escape_peaks) {
		if (!xmimsim_hdf5_escape_ratios.empty())
			xmimsim_hdf5_escape_ratios_c = strdup(xmimsim_hdf5_escape_ratios.c_str());

		if (xmi_get_escape_ratios_file(&xmimsim_hdf5_escape_ratios_c, 1) == 0)
			BAM::Exception("BAM::Job::XMSI::Initialize -> Error in xmi_get_escape_ratios_file");

		//check if escape ratios are already precalculated
               if (xmi_find_escape_ratios_match(xmimsim_hdf5_escape_ratios_c , xmimsim_input->GetInternalPointer(), &escape_ratios, options) == 0)
			BAM::Exception("BAM::Job::XMSI::Initialize -> Error in xmi_find_escape_ratios_match");

               if (escape_ratios == 0) {
			if (options.verbose)
				std::cout << "Precalculating escape peak ratios" << std::endl;
                        //doesn't exist yet
                        //convert input to string
			char *xmi_input_string = 0;

                        if (xmi_write_input_xml_to_string(&xmi_input_string, xmimsim_input->GetInternalPointer()) == 0) {
				BAM::Exception("BAM::Job::XMSI::Initialize -> Error in xmi_write_input_xml_to_string");
                        }

			xmi_escape_ratios_calculation(xmimsim_input->GetInternalPointer(), &escape_ratios, xmi_input_string, hdf5_file_c, options, xmi_get_default_escape_ratios_options());

			//update hdf5 file
			if(xmi_update_escape_ratios_hdf5_file(xmimsim_hdf5_escape_ratios_c , escape_ratios) == 0)
				BAM::Exception("BAM::Job::XMSI::Initialize -> Error in xmi_update_escape_ratios_hdf5_file");
			else if (options.verbose)
				std::cout << xmimsim_hdf5_escape_ratios_c << " was successfully updated with new escape peak ratios" << std::endl;
			xmlFree(xmi_input_string);
		}
		else if (options.verbose)
			std::cout << "Escape peak ratios already present in " << xmimsim_hdf5_escape_ratios_c << std::endl;


	}
	else if (options.verbose)
		std::cout << "No escape peaks requested: escape peak calculation is redundant" << std::endl;

	double **channels_ptrs = (double **) malloc(sizeof(double *) * (xmimsim_input->GetInternalPointer()->general->n_interactions_trajectory+1));
	for (int i = 0 ; i <= xmimsim_input->GetInternalPointer()->general->n_interactions_trajectory ; i++)
		channels_ptrs[i] = channels+i*xmimsim_input->GetInternalPointer()->detector->nchannels;


	if (options.custom_detector_response == NULL)
		xmi_detector_convolute_all(inputFPtr, channels_ptrs, channels_conv, options, escape_ratios, xmimsim_input->GetInternalPointer()->general->n_interactions_trajectory, zero_sum > 0.0 ? 1 : 0);
	else {
		XmiDetectorConvoluteAll xmi_detector_convolute_all_custom;
		if (!Glib::Module::get_supported()) {
			BAM::Exception("BAM::Job::XMSI::Start -> No module support on this platform: cannot use custom detector convolution routine");
		}
		Glib::Module module(std::string(options.custom_detector_response));
		if (!module) {
			BAM::Exception(std::string("BAM::Job::XMSI::Start -> Could not open ") + options.custom_detector_response + ": " + Glib::Module::get_last_error());
		}
		if (!module.get_symbol("xmi_detector_convolute_all_custom", (void *&) xmi_detector_convolute_all_custom)) {
			BAM::Exception(std::string("Error retrieving xmi_detector_convolute_all_custom in ") + options.custom_detector_response + ": " + Glib::Module::get_last_error());
		}
		else if (options.verbose)
			std::cout << "xmi_detector_convolute_all_custom loaded from " << options.custom_detector_response << std::endl;
		xmi_detector_convolute_all_custom(inputFPtr, channels_ptrs, channels_conv, options, escape_ratios, xmimsim_input->GetInternalPointer()->general->n_interactions_trajectory, zero_sum > 0.0 ? 1 : 0);
	}

	free(channels_ptrs);
	
	xmimsim_output = new BAM::File::XMSO(xmi_output_raw2struct(xmimsim_input->GetInternalPointer(), brute_history, options.use_variance_reduction == 1 ? var_red_history : NULL, channels_conv, channels, (char *) xmsi_filename.c_str(), zero_sum > 0.0 ? 1 : 0));
	for (int i=(zero_sum > 0.0 ? 0 : 1) ; i <= xmimsim_input->GetInternalPointer()->general->n_interactions_trajectory ; i++) {
		xmi_deallocate(channels_conv[i] );
	}
	free(channels_conv);
	xmi_deallocate(channels);
	xmi_deallocate(brute_history);
	if (options.use_variance_reduction)
		xmi_deallocate(var_red_history);
}

