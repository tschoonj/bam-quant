#include "bam_job_xmsi.h"
#include <iostream>


using namespace BAM;
using namespace BAM::Job;

bool XMSI::random_acquisition_started = false;

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
		hdf5_file_c = (gchar *) hdf5_file.c_str();

	if (xmi_get_hdf5_data_file(&hdf5_file_c) == 0) {
		BAM::Exception("BAM::Job::XMSI::Initialize -> Error in xmi_get_hdf5_data_file");
	}

	if (options.use_variance_reduction && !xmimsim_hdf5_solid_angles.empty()) {
		xmimsim_hdf5_solid_angles_c = (gchar *) xmimsim_hdf5_solid_angles.c_str();

		if (xmi_get_solid_angle_file(&xmimsim_hdf5_solid_angles_c, 1) == 0)
			BAM::Exception("BAM::Job::XMSI::Initialize -> Error in xmi_get_solid_angle_file");
	}

	if (options.use_escape_peaks && !xmimsim_hdf5_escape_ratios.empty()) {
		xmimsim_hdf5_escape_ratios_c = (gchar *) xmimsim_hdf5_escape_ratios.c_str();

		if (xmi_get_escape_ratios_file(&xmimsim_hdf5_escape_ratios_c, 1) == 0)
			BAM::Exception("BAM::Job::XMSI::Initialize -> Error in xmi_get_escape_ratios_file");
	}

	//get Fortran counterpart of input
	xmi_input_C2F(xmimsim_input.GetInternalPointer(), &inputFPtr);

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

