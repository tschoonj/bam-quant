#ifndef BAM_JOB_XMSI_H
#define BAM_JOB_XMSI_H

#include "bam_file_xmsi.h"
#include "bam_file_xmso.h"
#include <xmi_msim.h>
#include <string>
#include <cstdio>
#include <cstdlib>

namespace BAM {
	namespace Job {
		class XMSI {
		private:
			BAM::File::XMSI *xmimsim_input;
			BAM::File::XMSO *xmimsim_output;
			std::string xmsi_filename;
			struct xmi_main_options options;
			std::string hdf5_file;
			std::string xmimsim_hdf5_solid_angles;
			std::string xmimsim_hdf5_escape_ratios;
			char *hdf5_file_c;
			char *xmimsim_hdf5_solid_angles_c;
			char *xmimsim_hdf5_escape_ratios_c;
			xmi_inputFPtr inputFPtr;
			xmi_hdf5FPtr hdf5FPtr;
			struct xmi_solid_angle *solid_angles;
			struct xmi_escape_ratios *escape_ratios;

			static bool random_acquisition_started;
			void Initialize();
		public:
			static void RandomNumberAcquisitionStart() {
				//start random number acquisition
				if (!random_acquisition_started) {
					if (xmi_start_random_acquisition() == 0) {
						throw BAM::Exception("BAM::Job::XMSI::RandomNumberAcquisitionStart -> Could not start random number acquisition");
					}
					random_acquisition_started = true;
				}
			}
			static void RandomNumberAcquisitionStop() {
				if (random_acquisition_started) {
					if (xmi_end_random_acquisition() == 0) {
						throw BAM::Exception("BAM::Job::XMSI::RandomNumberAcquisitionStop -> Could not stop random number acquisition");
					}
					random_acquisition_started = false;
				}
			}
			XMSI(	std::string xmsi_filename,
				struct xmi_main_options options = xmi_get_default_main_options(),
				std::string hdf5_file = "",
				std::string xmimsim_hdf5_solid_angles = "",
				std::string xmimsim_hdf5_escape_ratios = "") :
				xmimsim_input(0),
				xmimsim_output(0),
				xmsi_filename(xmsi_filename),
				options(options),
				hdf5_file(hdf5_file),
				xmimsim_hdf5_solid_angles(xmimsim_hdf5_solid_angles),
				xmimsim_hdf5_escape_ratios(xmimsim_hdf5_escape_ratios),
				hdf5_file_c(0),
				xmimsim_hdf5_solid_angles_c(0),
				xmimsim_hdf5_escape_ratios_c(0),
				inputFPtr(0),
				hdf5FPtr(0),
				solid_angles(0),
				escape_ratios(0) {

				xmimsim_input = new BAM::File::XMSI(xmsi_filename);

				if (options.verbose)
					std::cout << "Inputfile " << xmsi_filename << " successfully parsed" << std::endl;

				if (options.extra_verbose)
					xmi_print_input(stdout, xmimsim_input->GetInternalPointer());

				Initialize();
			}
			XMSI(	BAM::File::XMSI xmimsim_input_new,
				struct xmi_main_options options = xmi_get_default_main_options(),
				std::string hdf5_file = "",
				std::string xmimsim_hdf5_solid_angles = "",
				std::string xmimsim_hdf5_escape_ratios = "") :
				xmimsim_input(0),
				xmimsim_output(0),
				xmsi_filename(""),
				options(options),
				hdf5_file(hdf5_file),
				xmimsim_hdf5_solid_angles(xmimsim_hdf5_solid_angles),
				xmimsim_hdf5_escape_ratios(xmimsim_hdf5_escape_ratios),
				hdf5_file_c(0),
				xmimsim_hdf5_solid_angles_c(0),
				xmimsim_hdf5_escape_ratios_c(0),
				inputFPtr(0),
				hdf5FPtr(0),
				solid_angles(0),
				escape_ratios(0) {


				xmimsim_input = new BAM::File::XMSI(xmimsim_input_new);

				if (options.verbose)
					std::cout << "Inputfile " << xmsi_filename << " successfully parsed" << std::endl;

				if (options.extra_verbose)
					xmi_print_input(stdout, xmimsim_input->GetInternalPointer());

				Initialize();
			}
			~XMSI() {
				if (hdf5_file_c)
					free(hdf5_file_c);
				if (xmimsim_hdf5_solid_angles_c)
					free(xmimsim_hdf5_solid_angles_c);
				if (xmimsim_hdf5_escape_ratios_c)
					free(xmimsim_hdf5_escape_ratios_c);
				if (solid_angles)
					xmi_free_solid_angle(solid_angles);
				if (escape_ratios)
					xmi_free_escape_ratios(escape_ratios);
				if (hdf5FPtr)
					xmi_free_hdf5_F(&hdf5FPtr);
				if (inputFPtr)
					xmi_free_input_F(&inputFPtr);
				if (xmimsim_output)
					delete xmimsim_output;
				if (xmimsim_input)
					delete xmimsim_input;

			}
			void Start();
			void Write() {
				if (!xmimsim_output)
					throw BAM::Exception("BAM::Job::XMSI::Write -> No output found that can be written to file");
				xmimsim_output->Write();
			}
			BAM::File::XMSO GetFileXMSO() {
				if (!xmimsim_output)
					throw BAM::Exception("BAM::Job::XMSI::GetFileXMSO -> No output found that can be returned");
				return *xmimsim_output;	
			}
		};
	}
}




#endif

