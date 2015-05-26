#include "app2-puresgrid.h"
#include "app2-assistant.h"
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <glibmm/miscutils.h>
#include <sstream>
#include <xraylib.h>
#include "bam_exception.h"

App2::PuresGrid::PuresGrid(App2::Assistant *assistant_arg, App2::EnergiesGrid *grid_arg, Gtk::TreeRowReference ref_arg) :
	assistant(assistant_arg), energies_grid(grid_arg), ref_energy(ref_arg) {

	//extract energy from ref_energy
	if (!ref_energy)
		throw BAM::Exception("Invalid TreeRowReference passed to PuresGrid constructor");

	model_ref_energy = ref_energy.get_model();
	Gtk::TreeModel::Row row = *(model_ref_energy->get_iter(ref_energy.get_path()));
	energy = row[energies_grid->columns.col_bam_file_xmsi_energy];

	std::ostringstream oss;
	oss << "Select at least two AXIL results files of pure element samples that were recorded at " 
	    << energy
	    << "keV";
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
	model->set_sort_column(3, Gtk::SORT_ASCENDING);
	tv.set_model(model);
	tv.append_column("Element", columns.col_element);
	tv.append_column("Filename", columns.col_filename);
	tv.append_column("Linetype", columns.col_linetype);
	tv.get_column(1)->set_expand();
	sw.add(tv);
	sw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	attach(sw, 1, 1, 1, 1);
	sw.set_vexpand();
	sw.set_hexpand();

	open_button.signal_clicked().connect(sigc::mem_fun(*this, &App2::PuresGrid::on_open_button_clicked));
	tv.signal_key_press_event().connect(sigc::mem_fun(*this, &App2::PuresGrid::on_backspace_clicked));
	tv.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);

	show_all_children();
}

void App2::PuresGrid::on_open_button_clicked() {
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

	for (std::vector<std::string>::iterator it = filenames.begin() ; it != filenames.end() ; ++it)  {
		std::cout << "Selected file: " << *it << std::endl;

		BAM::File::ASR *asr_file(0);
		int Z;
		try {
			asr_file = new BAM::File::ASR(*it);
		
			//count number of elements -> must be 1!
			if (asr_file->GetNPeaks() > 1) {
  				throw BAM::Exception("Only one peak permitted per file!");
			}
			BAM::Data::ASR asr_data = asr_file->GetData(0);
			Z = asr_data.GetZ();
			//check if already present
			Gtk::TreeModel::Children kids = model->children();
			bool match = false;
			for (Gtk::TreeModel::Children::iterator iter = kids.begin() ; iter != kids.end() ; ++iter) {
				Gtk::TreeModel::Row row = *iter;
				if (row[columns.col_atomic_number] == Z) {
					match = true;
					break;
				}
			}
			if (match) {
  				throw BAM::Exception("Element already present in table. Delete it first before adding it again!");
			}
			if (asr_data.GetLine() != KA_LINE && asr_data.GetLine() != LA_LINE)
  				throw BAM::Exception("Unknown linetype detected");
				
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

		char *element = AtomicNumberToSymbol(Z);
		Gtk::TreeModel::Row row = *(model->append());
		row[columns.col_element] = Glib::ustring(element);
		xrlFree(element);
		row[columns.col_filename] = Glib::path_get_basename(*it);
		row[columns.col_filename_full] = *it;
		row[columns.col_atomic_number] = Z;
		row[columns.col_bam_file_asr] = *asr_file;
		BAM::Data::ASR asr_data = asr_file->GetData(0);
		int linetype = asr_data.GetLine();
		if (linetype == KA_LINE) {
			row[columns.col_linetype] = Glib::ustring("Kα");
		}
		else if (linetype == LA_LINE) {
			row[columns.col_linetype] = Glib::ustring("Lα");
		}
	}
	if (model->children().size() >= 2) {
		assistant->set_page_complete(*this, true);
	}
	else {
		assistant->set_page_complete(*this, false);
	}
}

bool App2::PuresGrid::on_backspace_clicked(GdkEventKey *event) {
	if (event->keyval == gdk_keyval_from_name("BackSpace") ||
	    event->keyval == gdk_keyval_from_name("Delete")) {
		Glib::RefPtr<Gtk::TreeSelection> selection = tv.get_selection();
		std::vector<Gtk::TreeModel::Path> paths = selection->get_selected_rows();
		for (std::vector<Gtk::TreeModel::Path>::reverse_iterator rit = paths.rbegin() ; rit != paths.rend() ; ++rit) {
			Gtk::TreeModel::Row row = *(model->get_iter(*rit));
			model->erase(row);
		}
		if (model->children().size() >= 2) {
			assistant->set_page_complete(*this, true);	
		}
		else {
			assistant->set_page_complete(*this, false);	
		}
               	return true;
        }
        return false;
}
