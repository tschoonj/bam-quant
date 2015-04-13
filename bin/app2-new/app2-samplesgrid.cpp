#include "app2-samplesgrid.h"
#include "app2-energiesgrid.h"
#include "app2-assistant.h"
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <glibmm/miscutils.h>
#include <sstream>
#include <xraylib.h>
#include "bam_exception.h"


App2::SamplesGrid::SamplesGrid(App2::Assistant *assistant_arg, App2::EnergiesGrid *grid_arg, Gtk::TreeRowReference ref_arg) :
	assistant(assistant_arg), energies_grid(grid_arg), ref_energy(ref_arg) {

	//extract energy from ref_energy
	if (!ref_energy)
		throw BAM::Exception("Invalid TreeRowReference passed to PuresGrid constructor");

	model_ref_energy = ref_energy.get_model();
	Gtk::TreeModel::Row row = *(model_ref_energy->get_iter(ref_energy.get_path()));
	energy = row[energies_grid->columns.col_bam_file_xmsi_energy];

	std::ostringstream oss;
	oss << "Select at least one AXIL results file of a sample with unknown composition that was recorded at " 
	    << energy
	    << "keV. The files that are selected on the other sample pages should correspond to the samples selected here, while respecting the order.";
	Gtk::Label *label = Gtk::manage(new Gtk::Label(oss.str()));
	attach(*label, 0, 0, 2, 1);
	set_column_spacing(5);
	set_row_spacing(5);
	set_row_homogeneous(false);
	set_column_homogeneous(false);
	//label->set_vexpand();
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
	


	//no sorting required here
	//model->set_sort_column(3, Gtk::SORT_ASCENDING);
	tv.set_model(model);
	tv.append_column("#", columns.col_index);
	tv.append_column("Elements", columns.col_elements);
	tv.append_column("Filename", columns.col_filename);
	tv.get_column(2)->set_expand();
	tv.set_reorderable(true);
	sw.add(tv);
	sw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	attach(sw, 1, 1, 1, 1);
	sw.set_vexpand();
	sw.set_hexpand();

	open_button.signal_clicked().connect(sigc::mem_fun(*this, &App2::SamplesGrid::on_open_button_clicked));
	tv.signal_key_press_event().connect(sigc::mem_fun(*this, &App2::SamplesGrid::on_backspace_clicked));
	tv.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
	signal_row_deleted_handler = model->signal_row_deleted().connect(sigc::hide(sigc::mem_fun(*this, &App2::SamplesGrid::on_row_deleted_or_inserted)));
	signal_row_inserted_handler = model->signal_row_inserted().connect(sigc::hide(sigc::hide(sigc::mem_fun(*this, &App2::SamplesGrid::on_row_deleted_or_inserted))));

	show_all_children();
}

void App2::SamplesGrid::on_open_button_clicked() {
	Gtk::FileChooserDialog dialog(*assistant, "Please select a number of AXIL results files", Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog.set_select_multiple();
	Glib::RefPtr<Gtk::FileFilter> filter_asr = Gtk::FileFilter::create();
	filter_asr->set_name("ASR files");
	filter_asr->add_pattern("*.asr");
	filter_asr->add_pattern("*.ASR");
	dialog.add_filter(filter_asr);
	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("Select", Gtk::RESPONSE_OK);
	
	int result = dialog.run();
	std::vector<std::string> filenames;

	switch(result) {
		case(Gtk::RESPONSE_OK):
			std::cout << "Open clicked." << std::endl;
			filenames = dialog.get_filenames();

      			break;
		case(Gtk::RESPONSE_CANCEL):
		default:
			return;
	}

	dialog.hide();

	signal_row_inserted_handler.block();

	for (std::vector<std::string>::iterator it = filenames.begin() ; it != filenames.end() ; ++it)  {
		std::cout << "Selected file: " << *it << std::endl;

		BAM::File::ASR *asr_file(0);
		try {
			//check if the file is already present in the model
			//ideally I should check all models in assistant->samples_grid_vec
			for (Gtk::TreeModel::Children::iterator iter = model->children().begin() ; iter != model->children().end() ; ++iter) {
				Gtk::TreeModel::Row row = *iter;
				if (*it == std::string(row[columns.col_filename_full]))
					throw BAM::Exception("File already present in model!");
			}


			asr_file = new BAM::File::ASR(*it);
		}
		catch (BAM::Exception &e) {
			Gtk::MessageDialog message_dialog(*assistant, "Error reading in " + *it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			message_dialog.set_secondary_text(e.what());
  			message_dialog.run();
			if (asr_file)
				delete asr_file;
			continue;
		}
		catch (...) {
			Gtk::MessageDialog message_dialog(*assistant, "Error reading in " + *it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			message_dialog.set_secondary_text("Unknown exception caught: this should not happen!!!");
  			message_dialog.run();
			if (asr_file)
				delete asr_file;
			continue;
		}
		Glib::ustring elements;
		std::vector<int> elements_int;
		for (int i = 0 ; i < asr_file->GetNPeaks() ; i++) {
			BAM::Data::ASR asr_data = asr_file->GetData(i);
			int Z = asr_data.GetZ();
			char *element = AtomicNumberToSymbol(Z);
			elements += element;
			if (i != asr_file->GetNPeaks()-1)
				elements += ", "; 	
			xrlFree(element);
			elements_int.push_back(Z);
		}
		Gtk::TreeModel::Row row = *(model->append());
		row[columns.col_elements] = elements;
		row[columns.col_filename] = Glib::path_get_basename(*it);
		row[columns.col_filename_full] = *it;
		row[columns.col_bam_file_asr] = *asr_file;
		row[columns.col_elements_int] = elements_int;
		delete asr_file;
	}

	//update all the indices
	on_row_deleted_or_inserted();

	signal_row_inserted_handler.unblock();
	
	//In case the samples_summary_grid was complete, undo this
	assistant->set_page_complete(assistant->samples_summary_grid, false);
}

bool App2::SamplesGrid::on_backspace_clicked(GdkEventKey *event) {
	if (event->keyval == gdk_keyval_from_name("BackSpace") ||
	    event->keyval == gdk_keyval_from_name("Delete")) {
		Glib::RefPtr<Gtk::TreeSelection> selection = tv.get_selection();
		std::vector<Gtk::TreeModel::Path> paths = selection->get_selected_rows();
		signal_row_deleted_handler.block();
		for (std::vector<Gtk::TreeModel::Path>::reverse_iterator rit = paths.rbegin() ; rit != paths.rend() ; ++rit) {
			Gtk::TreeModel::Row row = *(model->get_iter(*rit));
			model->erase(row);
		}
		on_row_deleted_or_inserted();
		signal_row_deleted_handler.unblock();
		
		//In case the samples_summary_grid was complete, undo this
		assistant->set_page_complete(assistant->samples_summary_grid, false);

               	return true;
        }
        return false;
}

void App2::SamplesGrid::on_row_deleted_or_inserted() {
	//all this function has to do is update the indices
	unsigned int counter(0);	
	for (Gtk::TreeModel::Children::iterator iter = model->children().begin() ; iter != model->children().end() ; ++iter) {
		Gtk::TreeModel::Row row = *iter;
		row[columns.col_index] = counter++;
	}

	if (model->children().size() >= 1) {
		assistant->set_page_complete(*this, true);
	}
	else {
		assistant->set_page_complete(*this, false);
	}
}

