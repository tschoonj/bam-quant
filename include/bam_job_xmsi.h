#ifndef BAM_JOB_XMSI_H
#define BAM_JOB_XMSI_H

#include "bam_file_xmsi.h"
#include "bam_file_xmso.h"
#include <xmi_msim.h>
#include <string>
#include <glib.h>
#include <cstdio>

namespace BAM {
	namespace Job {
		class XMSI {
		private:
			BAM::File::XMSI xmimsim_input;
			BAM::File::XMSO *xmimsim_output;
			struct xmi_main_options options;
			std::string hdf5_file;
			std::string xmimsim_hdf5_solid_angles;
			std::string xmimsim_hdf5_escape_ratios;
			gchar *hdf5_file_c;
			gchar *xmimsim_hdf5_solid_angles_c;
			gchar *xmimsim_hdf5_escape_ratios_c;
			xmi_inputFPtr inputFPtr;
			xmi_hdf5FPtr hdf5FPtr;

			static bool random_acquisition_started;
			void Initialize();
			void Start();
		public:
			XMSI(	std::string xmsi_filename,
				struct xmi_main_options options,
				std::string hdf5_file = "",
				std::string xmimsim_hdf5_solid_angles = "",
				std::string xmimsim_hdf5_escape_ratios = "") :
				xmimsim_input(xmsi_filename),
				options(options),
				hdf5_file(hdf5_file),
				xmimsim_hdf5_solid_angles(xmimsim_hdf5_solid_angles),
				xmimsim_hdf5_escape_ratios(xmimsim_hdf5_escape_ratios),
				hdf5_file_c(0),
				xmimsim_hdf5_solid_angles_c(0),
				xmimsim_hdf5_escape_ratios_c(0),
				inputFPtr(0),
				hdf5FPtr(0) {

				if (options.verbose)
					std::cout << "Inputfile " << xmsi_filename << "successfully parsed" << std::endl;

				if (options.extra_verbose)
					xmi_print_input(stdout, xmimsim_input.GetInternalPointer());

				Initialize();
			}
		};
	}
}




#endif

