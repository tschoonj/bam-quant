#include "app2-confirmationlabel.h"
#include "app2-assistant.h"
#include "app2-samplesgrid.h"
#include "bam_file_rxi.h"
#include <gsl/gsl_multifit.h>
#include <set>
#include <gtkmm/messagedialog.h>


App2::ConfirmationLabel::ConfirmationLabel(App2::Assistant *assistant_arg) : Gtk::Label("Just one more thing to do!\n\nClick the apply button and the file will be saved..."), assistant(assistant_arg) {

	//really: this is it...
}


void App2::ConfirmationLabel::WriteRXIs() {

	//let's try saving our data
	//if it works we close the program
	try {

		//start by calculating phi for every energy
		std::vector<double> phi(assistant->pures_grid_vec.size(), 0.0);
		std::vector<double> a(assistant->pures_grid_vec.size(), 0.0);
		std::vector<double> b(assistant->pures_grid_vec.size(), 0.0);
		std::vector<double> c(assistant->pures_grid_vec.size(), 0.0);
		std::vector<unsigned int> pld_size(assistant->pures_grid_vec.size(), 0);
		
		for (unsigned int index = 0 ; index < assistant->pures_grid_vec.size() ; index++) {
			std::string message;

			//ignore if no simulations were available to start with
			if (assistant->simulate_grid.diff_elements[index].size() == 0) 
				continue;
		
			//run over all elements
			App2::SimulateGrid::Columns *columns = assistant->simulate_grid.columns;	
			std::vector<double> fit_x;
			std::vector<double> fit_y;
			for (Gtk::TreeModel::Children::iterator iter = assistant->simulate_grid.model->children().begin() ;
			     iter != assistant->simulate_grid.model->children().end() ;
			     ++iter) {
				Gtk::TreeModel::Row row = *iter;

				//skip those elements that had no pure counterpart and those whose pure counterpart was skipped explicitly by the user
				BAM::File::ASR col_bam_file_asr = row[columns->col_bam_file_asr[index]];

				if (col_bam_file_asr == 0 ||
				    row[columns->col_simulate_active[index]] == false)
					continue;

				double local_phi = 0.0;
				std::cout << "Element for phi: " << col_bam_file_asr.GetData(0).GetZ() << std::endl;


				if (col_bam_file_asr.GetData(0).GetLine() == KA_LINE && row[columns->col_xmso_counts_KA[index]] > 0.0) {
					local_phi = col_bam_file_asr.GetData(0).GetCounts() * col_bam_file_asr.GetNormfactor() / row[columns->col_xmso_counts_KA[index]]; 
				}
				else if (col_bam_file_asr.GetData(0).GetLine() == LA_LINE && row[columns->col_xmso_counts_LA[index]] > 0.0) {
					local_phi = col_bam_file_asr.GetData(0).GetCounts() * col_bam_file_asr.GetNormfactor() / row[columns->col_xmso_counts_LA[index]];
				}
				else {
					throw BAM::Exception(std::string("Mismatch found: counts in ASR file and corresponding simulation result must both be positive values! Problem detected for element:")+row[columns->col_element]);
				}
				std::cout << "local_phi: " << local_phi << std::endl;
				fit_x.push_back(LineEnergy(col_bam_file_asr.GetData(0).GetZ(), col_bam_file_asr.GetData(0).GetLine()));
				fit_y.push_back(local_phi);
			}
			//simple phi -> calculate mean
			phi[index] = std::accumulate(fit_y.begin(), fit_y.end(), 0)/fit_y.size();
			std::cout << "average phi simple: " << phi[index] << std::endl;
			pld_size[index] = fit_x.size();
			//pld_size[index] = 0;

			if (fit_x.size() > 3) {
				gsl_matrix *X_fit, *cov_fit;
				gsl_vector *y_fit, *c_fit;

				X_fit = gsl_matrix_alloc(fit_x.size(), 3);
				y_fit = gsl_vector_alloc(fit_x.size());
				c_fit = gsl_vector_alloc(3);
				cov_fit = gsl_matrix_alloc(3, 3);

				for (unsigned int i = 0 ; i < fit_x.size() ; i++) {
					gsl_matrix_set(X_fit, i, 0, 1.0);
					gsl_matrix_set(X_fit, i, 1, fit_x[i]);
					gsl_matrix_set(X_fit, i, 2, fit_x[i]*fit_x[i]);
					gsl_vector_set(y_fit, i, fit_y[i]);
				}
				gsl_multifit_linear_workspace *work = gsl_multifit_linear_alloc(fit_x.size(), 3);
				double chisq_fit;
				gsl_multifit_linear(X_fit, y_fit, c_fit, cov_fit, &chisq_fit, work);
				gsl_multifit_linear_free(work);


				std::cout << "fit results: " << std::endl;
				std::cout << "a: " << gsl_vector_get(c_fit, 2) << std::endl;
				std::cout << "b: " << gsl_vector_get(c_fit, 1) << std::endl;
				std::cout << "c: " << gsl_vector_get(c_fit, 0) << std::endl;
				a[index] = gsl_vector_get(c_fit, 2);
				b[index] = gsl_vector_get(c_fit, 1);
				c[index] = gsl_vector_get(c_fit, 0);
			} //for loop over all MC simulated elements
		} // for loop over all energies

		//create object that will be written to file	
		std::map<double,BAM::File::XMSI> energies_grid_map(assistant->energies_grid.GetMap());
		BAM::File::RXI::Multi rxi_multi(energies_grid_map);
		
		//loop over all samples from sample summary page
		unsigned int index;
		Gtk::TreeModel::Children::iterator summary_iter;
		for(summary_iter =  assistant->samples_summary_grid.model->children().begin(), index = 0 ;
		    summary_iter != assistant->samples_summary_grid.model->children().end() ; 
		    ++summary_iter, index++) {
			Gtk::TreeModel::Row row = *summary_iter;

			//get the asrfiles by looping over all samples_grids
			std::map<std::string,double> asrfiles;
			for (std::vector<App2::SamplesGrid*>::iterator samples_iter = assistant->samples_grid_vec.begin() ;
			     samples_iter != assistant->samples_grid_vec.end() ;
			     ++samples_iter) {
				asrfiles[((*samples_iter)->model->children()[index])[(*samples_iter)->columns.col_filename_full]] = (*samples_iter)->GetEnergy();
			}

			BAM::Data::RXI::Sample sample(asrfiles, row[assistant->samples_summary_grid.columns.col_density], row[assistant->samples_summary_grid.columns.col_thickness], row[assistant->samples_summary_grid.columns.col_fix_thickness_density], row[assistant->samples_summary_grid.columns.col_matrix]);

			//loop over all elements in the sample
			//by going over all energies
			std::set<int> col_elements_int;
			for (std::vector<App2::SamplesGrid*>::iterator samples_iter = assistant->samples_grid_vec.begin() ;
			     samples_iter != assistant->samples_grid_vec.end() ;
			     ++samples_iter) {
				std::vector<int> col_elements_int_slice = ((*samples_iter)->model->children()[index])[(*samples_iter)->columns.col_elements_int];
				col_elements_int.insert(col_elements_int_slice.begin(), col_elements_int_slice.end());
			}
			//std::sort(col_elements_int.begin(), col_elements_int.end());
			//col_elements_int.erase(std::unique(col_elements_int.begin(), col_elements_int.end()), col_elements_int.end());

			for (std::set<int>::iterator elements_iter = col_elements_int.begin() ;
			     elements_iter != col_elements_int.end() ;
			     ++elements_iter) {
				//first problem: get the correct col_bam_file_asr for the sample
				std::vector<App2::SamplesGrid*>::iterator samples_iter_good = assistant->samples_grid_vec.end();
				int line = -1000;
				double max_counts = 0.0;
				BAM::File::ASR file_asr_sample;
				BAM::Data::ASR data_asr_sample;
				for (std::vector<App2::SamplesGrid*>::iterator samples_iter = assistant->samples_grid_vec.begin() ;
				     samples_iter != assistant->samples_grid_vec.end() ;
				     ++samples_iter) {
					BAM::File::ASR col_bam_file_asr = ((*samples_iter)->model->children()[index])[(*samples_iter)->columns.col_bam_file_asr];
					try {
						BAM::Data::ASR data_asr = col_bam_file_asr(*elements_iter, KA_LINE);
						if (data_asr.GetCounts() > max_counts) {
							line = KA_LINE;
							max_counts = data_asr.GetCounts();
							samples_iter_good = samples_iter;
							file_asr_sample = col_bam_file_asr;
							data_asr_sample = data_asr;
						}
					}
					catch (BAM::Exception &e) {
						try {
							BAM::Data::ASR data_asr = col_bam_file_asr(*elements_iter, LA_LINE);
							if (data_asr.GetCounts() > max_counts) {
								line = LA_LINE;
								max_counts = data_asr.GetCounts();
								samples_iter_good = samples_iter;
								file_asr_sample = col_bam_file_asr;
								data_asr_sample = data_asr;
							}
						}
						catch (BAM::Exception &e) {
							//move on to the next one
						}
					}
				} //for loop over all samples
				if (max_counts == 0.0 || line == -1000 || samples_iter_good == assistant->samples_grid_vec.end()) {
					throw BAM::Exception("Could not find BAM::File::ASR for sample");
				}
				double norm_factor_sample = file_asr_sample.GetNormfactor();
				int Z = *elements_iter;
				std::string element(AtomicNumberToSymbol(Z));
				std::string linetype;
				std::string datatype;
				double rxi;
				double excitation_energy = (*samples_iter_good)->GetEnergy();
			
				//next step: see if the element is present in the pures database
				bool energy_match(false);
				unsigned int energy_index;
				for (energy_index = 0 ; energy_index < assistant->pures_grid_vec.size() ; energy_index++) {
					if (assistant->pures_grid_vec[energy_index]->GetEnergy() == excitation_energy) {
						energy_match = true;
						break;
					}
				}
				if (!energy_match) {
					//this should never happen
					throw BAM::Exception("Excitation energy not found in pures_grid_vec");
				}

				//pure_grid has been identified -> now look for our element
				bool element_found(false);
				Gtk::TreeModel::Row pures_grid_row;
				for (Gtk::TreeModel::Children::iterator pures_grid_iter = assistant->pures_grid_vec[energy_index]->model->children().begin() ;
				     pures_grid_iter != assistant->pures_grid_vec[energy_index]->model->children().end() ;
				     ++pures_grid_iter) {
					pures_grid_row = *pures_grid_iter;
					if (pures_grid_row[assistant->pures_grid_vec[energy_index]->columns.col_atomic_number] == Z) {
						element_found = true;
						break;
					}
				}

				if (element_found) {
					//pure element asr found
					BAM::File::ASR file_asr_pure = pures_grid_row[assistant->pures_grid_vec[energy_index]->columns.col_bam_file_asr];
					BAM::Data::ASR data_asr_pure = file_asr_pure.GetData(0);
					
					//error checking -> linetypes must match
					//would be weird if this wouldnt be the case though
					//strictly speaking one could add both KA_LINE and LA_LINE to the axfit model. If so this needs to be corrected
					if (data_asr_pure.GetLine() != line)
						throw BAM::Exception("Linetype in sample and pure not matching!");
					double norm_factor_pure = file_asr_pure.GetNormfactor();
					rxi = data_asr_sample.GetCounts() * norm_factor_sample /
					(data_asr_pure.GetCounts() * norm_factor_pure);

					if (data_asr_sample.GetLine() == KA_LINE) {
						linetype = "KA_LINE";
					}
					else if (data_asr_sample.GetLine() == LA_LINE) {
						linetype = "LA_LINE";
					}
					else {
						throw BAM::Exception("Invalid linetype detected");
					}
					datatype = "experimental";
				}
				else {
					//interpolate from MC generated data
					//good thing is that we can use energy_index here
					App2::SimulateGrid::Columns *columns = assistant->simulate_grid.columns;	
					Gtk::TreeModel::Row simulate_row;
					Gtk::TreeModel::Children::iterator simulate_iter;
					for (simulate_iter =  assistant->simulate_grid.model->children().begin() ;
					     simulate_iter != assistant->simulate_grid.model->children().end() ;
					     ++simulate_iter) {
						simulate_row = *simulate_iter;
						if (simulate_row[columns->col_atomic_number] == Z) {
							break;
						}
					}
					if (simulate_iter == assistant->simulate_grid.model->children().end()) 
						throw BAM::Exception("Element not found in simulate_grid");
					if (!BAM::File::XMSO(simulate_row[columns->col_xmso_file[energy_index]]))
						throw BAM::Exception("XMSO file empty");

					if (simulate_row[columns->col_xmso_counts_KA[energy_index]] > 0) {
						double phi_local;
						if (pld_size[energy_index] > 3) {
							double lE = LineEnergy(Z, KA_LINE);
							phi_local = a[energy_index]*lE*lE + 
							      	    b[energy_index]*lE+
							      	    c[energy_index];
							std::cout << "average phi fit: " << phi_local << " for " << Z << std::endl;
						}
						else {
							phi_local = phi[energy_index];
						}
						rxi = data_asr_sample.GetCounts() * norm_factor_sample /
						(simulate_row[columns->col_xmso_counts_KA[energy_index]]*phi_local);
						linetype = "KA_LINE";
					}
					else if (simulate_row[columns->col_xmso_counts_LA[energy_index]] > 0) {
						double phi_local;
						if (pld_size[energy_index] > 3) {
							double lE = LineEnergy(Z, LA_LINE);
							phi_local = a[energy_index]*lE*lE + 
							      	    b[energy_index]*lE+
							      	    c[energy_index];
							std::cout << "average phi fit: " << phi_local << " for " << Z << std::endl;
						}
						else {
							phi_local = phi[energy_index];
						}
						rxi = data_asr_sample.GetCounts() * norm_factor_sample /
						(simulate_row[columns->col_xmso_counts_LA[energy_index]]*phi_local);
						linetype = "LA_LINE";
					}
					else
						throw BAM::Exception("An element in a sample without corresponding pure measurement has no positive counts in the simulation results.");
					datatype = "interpolated";
				}

				sample.AddSingleElement(BAM::Data::RXI::SingleElement(element, linetype, datatype, rxi, excitation_energy));
			} //for loop over all elements in the sample

			rxi_multi.AddSample(sample);

		} //for loop over all samples from samples_summary_grid
		rxi_multi.Write(assistant->output_file_grid.GetOutputFile());
	}
	catch (BAM::Exception &e) {
		//produce a message dialog telling the user to change the filename
		Gtk::MessageDialog dialog(*assistant, std::string("BAM::Exception detected while writing to ")+assistant->output_file_grid.GetOutputFile(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  		dialog.set_secondary_text(Glib::ustring("BAM error: ")+e.what());
  		dialog.run();
		assistant->previous_page();	
		assistant->set_page_complete(assistant->output_file_grid, false);
		return;
	}
	catch (std::exception& e) {
		//produce a message dialog telling the user to change the filename
		Gtk::MessageDialog dialog(*assistant, std::string("std::exception detected while writing to ")+assistant->output_file_grid.GetOutputFile(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  		dialog.set_secondary_text(Glib::ustring("error: ")+e.what());
  		dialog.run();
		assistant->previous_page();
		assistant->set_page_complete(assistant->output_file_grid, false);
		return;
	}
	catch (...) {
		Gtk::MessageDialog dialog(*assistant, std::string("Some weird exception detected while writing to ")+assistant->output_file_grid.GetOutputFile(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  		dialog.run();
		assistant->previous_page();
		assistant->set_page_complete(assistant->output_file_grid, false);
		return;
	}

	assistant->get_application()->remove_window(*assistant);
}
