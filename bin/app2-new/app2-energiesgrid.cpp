#include "app2-energiesgrid.h"
#include "app2-assistant.h"
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <glibmm/miscutils.h>
#include <sstream>

App2::EnergiesGrid::EnergiesGrid(App2::Assistant *assistant) :
	assistant(assistant) {

	Gtk::Label *label = new Gtk::Label("Select at least one XMI-MSIM input-file that describes the excitation conditions that the samples and pure element standards were exposed to. All input-files require non-matching excitation energies.");
	attach(*label, 0, 0, 2, 1);
	set_column_spacing(5);
	set_row_spacing(5);
	set_row_homogeneous(false);
	set_column_homogeneous(false);
	label->set_hexpand();
	label->set_margin_bottom(10);
	label->set_margin_top(10);
	label->set_line_wrap();
	label->set_justify(Gtk::JUSTIFY_LEFT);
	
	open_button.set_image_from_icon_name("document-open");
	open_button.set_vexpand(false);
	open_button.set_hexpand(false);
	open_button.set_valign(Gtk::ALIGN_CENTER);
	attach(open_button, 0, 1, 1, 1);
	
	model = Gtk::ListStore::create(columns);
	model->set_sort_column(3, Gtk::SORT_ASCENDING);
	tv.set_model(model);
	tv.append_column("Filename", columns.col_filename_base);
	tv.append_column_numeric("Energy (keV)", columns.col_bam_file_xmsi_energy, "%g");
	//alignment
	tv.get_column_cell_renderer(1)->set_alignment(0.5, 0.5); 
	tv.get_column(0)->set_expand();
	sw.add(tv);
	sw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	attach(sw, 1, 1, 1, 1);
	sw.set_vexpand();
	sw.set_hexpand();
	open_button.signal_clicked().connect(sigc::mem_fun(*this, &App2::EnergiesGrid::on_open_button_clicked));
	tv.signal_key_press_event().connect(sigc::mem_fun(*this, &App2::EnergiesGrid::on_backspace_clicked));
	tv.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);

	show_all_children();
}

void App2::EnergiesGrid::on_open_button_clicked() {
	Gtk::FileChooserDialog dialog(*assistant, "Please select a number of XMI-MSIM input-files", Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog.set_select_multiple();
	Glib::RefPtr<Gtk::FileFilter> filter_xmsi = Gtk::FileFilter::create();
	filter_xmsi->set_name("XMSI files");
	filter_xmsi->add_mime_type("application/x-xmimsim-xmsi");
	dialog.add_filter(filter_xmsi);
	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("Select", Gtk::RESPONSE_OK);
	
	int result = dialog.run();
	std::vector<std::string> filenames;

	switch(result) {
		case(Gtk::RESPONSE_OK):
			filenames = dialog.get_filenames();
      			break;
		case(Gtk::RESPONSE_CANCEL):
		default:
			return;
	}

	dialog.hide();

	for (std::vector<std::string>::iterator it = filenames.begin() ; it != filenames.end() ; ++it)  {
		std::cout << "Selected file: " << *it << std::endl;
		BAM::File::XMSI *xmsi_file(0);
		double energy(0);
		try {
			//read in the file
			xmsi_file = new BAM::File::XMSI(*it);
			//ensure the excitation is strictly monochromatic
			xmsi_file->GetExcitation().EnsureMonochromaticExcitation();
			//check this energy is not yet present
			energy = xmsi_file->GetExcitation().GetDiscreteEnergy().GetEnergy();
			Gtk::TreeModel::Children kids = model->children();
			bool match = false;
			for (Gtk::TreeModel::Children::iterator iter = kids.begin() ; iter != kids.end() ; ++iter) {
				Gtk::TreeModel::Row row = *iter;
				if (*it == std::string(row[columns.col_filename_full]) || fabs(row[columns.col_bam_file_xmsi_energy] - energy) < 1E-5) {
					match = true;
					break;
				}
			}
			if (match) {
				throw BAM::Exception("Energy already present in table");
			}
		}
		catch (BAM::Exception &e) {
			Gtk::MessageDialog message_dialog(*assistant, "Error reading in " + *it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			message_dialog.set_secondary_text(e.what());
  			message_dialog.run();
			if (xmsi_file)
				delete xmsi_file;
			continue;
		}
		catch (...) {
			Gtk::MessageDialog message_dialog(*assistant, "Error reading in " + *it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			message_dialog.set_secondary_text("Unknown exception caught: this should not happen!!!");
  			message_dialog.run();
			if (xmsi_file)
				delete xmsi_file;
			continue;
		}
		//if everything is well -> add to the model
		Gtk::TreeModel::Row row = *(model->append());
		row[columns.col_filename_base] = Glib::path_get_basename(*it);
		row[columns.col_filename_full] = *it;
		row[columns.col_bam_file_xmsi] = *xmsi_file;
		row[columns.col_bam_file_xmsi_energy] = energy;
		delete xmsi_file;


	}

	//create PuresGrid
	//loop over all rows
	unsigned int counter = 0;
	for (Gtk::TreeModel::Children::iterator iter = model->children().begin() ; iter != model->children().end() ; ++iter) {
		//comparison with assistant's pures_grid_vec will happen by comparing paths
		bool match = false;
		for (std::vector<App2::PuresGrid*>::iterator iter_vec = assistant->pures_grid_vec.begin() ; iter_vec != assistant->pures_grid_vec.end() ; ++iter_vec) {
			if ((*iter_vec)->GetEnergiesGridPath() == model->get_path(*iter)) {
				match = true;
				break;
			}
		}
		Gtk::TreeModel::Row row = *iter;
		row[columns.col_pures_grid_page_index] = counter + 2;

		//if matched -> do nothing
		//if not matched -> add a new page to assistant
		if (match) {
			counter++;
			continue;
		}
	
		//PuresGrid	
		App2::PuresGrid *pures_grid = *assistant->pures_grid_vec.insert(assistant->pures_grid_vec.begin()+counter, Gtk::manage(new App2::PuresGrid(assistant, this, Gtk::TreeRowReference(model, model->get_path(*iter)))));
		assistant->insert_page(*pures_grid, row[columns.col_pures_grid_page_index]);
		assistant->set_page_type(*pures_grid, Gtk::ASSISTANT_PAGE_CONTENT);
		assistant->set_page_complete(*pures_grid, false);
		{
			std::ostringstream oss;
			oss << "Pure elements at " 
	    		<< row[columns.col_bam_file_xmsi_energy] 
	    		<< "keV";
			assistant->set_page_title(*pures_grid, oss.str());
		}
		counter++;
	}


	//create SamplesGrid
	//loop over all rows
	counter = 0;
	for (Gtk::TreeModel::Children::iterator iter = model->children().begin() ; iter != model->children().end() ; ++iter) {
		//comparison with assistant's samples_grid_vec will happen by comparing paths
		bool match = false;
		for (std::vector<App2::SamplesGrid*>::iterator iter_vec = assistant->samples_grid_vec.begin() ; iter_vec != assistant->samples_grid_vec.end() ; ++iter_vec) {
			if ((*iter_vec)->GetEnergiesGridPath() == model->get_path(*iter)) {
				match = true;
				break;
			}
		}
		Gtk::TreeModel::Row row = *iter;
		row[columns.col_samples_grid_page_index] = model->children().size() + counter + 2;

		//if matched -> do nothing
		//if not matched -> add a new page to assistant
		if (match) {
			counter++;
			continue;
		}
		//SamplesGrid	
		App2::SamplesGrid *samples_grid = *assistant->samples_grid_vec.insert(assistant->samples_grid_vec.begin()+counter, Gtk::manage(new App2::SamplesGrid(assistant, this, Gtk::TreeRowReference(model, model->get_path(*iter)))));
		assistant->insert_page(*samples_grid, row[columns.col_samples_grid_page_index]);
		assistant->set_page_type(*samples_grid, Gtk::ASSISTANT_PAGE_CONTENT);
		assistant->set_page_complete(*samples_grid, false);
		{
			std::ostringstream oss;
			oss << "Samples at " 
	    		<< row[columns.col_bam_file_xmsi_energy] 
	    		<< "keV";
			assistant->set_page_title(*samples_grid, oss.str());
		}
		counter++;
	}

	assistant->show_all_children();


	//update assistant buttons
	if (model->children().size())
		assistant->set_page_complete(*this, true);
	else
		assistant->set_page_complete(*this, false);

	//In case the samples_summary_grid was complete, undo this
	assistant->set_page_complete(assistant->samples_summary_grid, false);

	assistant->update_buttons_state();
}

bool App2::EnergiesGrid::on_backspace_clicked(GdkEventKey *event) {
	if (event->keyval == gdk_keyval_from_name("BackSpace") ||
	    event->keyval == gdk_keyval_from_name("Delete")) {
		Glib::RefPtr<Gtk::TreeSelection> selection = tv.get_selection();
		std::vector<Gtk::TreeModel::Path> paths = selection->get_selected_rows();
		for (std::vector<Gtk::TreeModel::Path>::reverse_iterator rit = paths.rbegin() ; rit != paths.rend() ; ++rit) {
			Gtk::TreeModel::Row row = *(model->get_iter(*rit));
			//remove samples_grid page
			assistant->remove_page(row[columns.col_samples_grid_page_index]);
			assistant->samples_grid_vec.erase(assistant->samples_grid_vec.begin()+row[columns.col_pures_grid_page_index]-2);
			//remove pures_grid page
			assistant->remove_page(row[columns.col_pures_grid_page_index]);
			assistant->pures_grid_vec.erase(assistant->pures_grid_vec.begin()+row[columns.col_pures_grid_page_index]-2);
			model->erase(row);
		}
		//update col_pures_grid_page_index and col_samples_grid_page_index for all
		unsigned int counter = 0;
		for (Gtk::TreeModel::Children::iterator iter = model->children().begin() ; iter != model->children().end() ; ++iter) {
			Gtk::TreeModel::Row row = *iter;
			row[columns.col_pures_grid_page_index] = counter + 2;
			row[columns.col_samples_grid_page_index] = model->children().size() + counter + 2;
		}
		if (model->children().size() >= 1) {
			assistant->set_page_complete(*this, true);
		}
		else {
			assistant->set_page_complete(*this, false);
		}
		
		//In case the samples_summary_grid was complete, undo this
		assistant->set_page_complete(assistant->samples_summary_grid, false);

		assistant->show_all_children();
		assistant->update_buttons_state();
		
               	return true;
        }

        return false;
}
