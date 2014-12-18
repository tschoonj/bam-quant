#include "app2-assistant.h"
#include <iostream>
#include <gtkmm/application.h>
#include <gtkmm/main.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <xraylib.h>
#include <glibmm/miscutils.h>



App2Assistant::App2Assistant() : first_page("Welcome!\n\nIn this wizard you will produce a file containing\n"
							"relative X-ray intensities based on the net-line intensities\n"
							"obtained from pure elemental standards and samples.")
	{

	set_border_width(12);
	set_default_size(400, 300);

	//first page
	append_page(first_page);
	set_page_type(first_page, Gtk::ASSISTANT_PAGE_INTRO);
	set_page_title(first_page, "Introduction...");
	set_page_complete(first_page, true);

	//second page
	Gtk::Label *label = Gtk::manage(new Gtk::Label("Select at least two AXIL result files of\npure element samples..."));
	second_page.attach(*label, 0, 0, 2, 1);
	second_page.set_column_spacing(5);
	second_page.set_row_spacing(5);
	second_page.set_row_homogeneous(false);
	second_page.set_column_homogeneous(false);
	//label->set_vexpand();
	label->set_hexpand();
	label->set_margin_bottom(10);
	label->set_margin_top(10);
	
	second_page_open.set_image_from_icon_name("document-open");
	second_page_open.set_vexpand(false);
	second_page_open.set_hexpand(false);
	second_page_open.set_valign(Gtk::ALIGN_CENTER);
	second_page.attach(second_page_open, 0, 1, 1, 1);
	
	second_page_model = Gtk::ListStore::create(second_page_columns);
	second_page_tv.set_model(second_page_model);
	second_page_tv.append_column("Element", second_page_columns.col_element);
	second_page_tv.append_column("Filename", second_page_columns.col_filename);
	second_page_sw.add(second_page_tv);
	second_page_sw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	second_page.attach(second_page_sw, 1, 1, 1, 1);
	second_page_sw.set_vexpand();
	second_page_sw.set_hexpand();
	append_page(second_page);
	set_page_type(second_page, Gtk::ASSISTANT_PAGE_CONTENT);
	set_page_title(second_page, "Select pure element ASR files");
	second_page_open.signal_clicked().connect(sigc::mem_fun(*this, &App2Assistant::on_second_page_open_clicked));
	second_page_tv.signal_key_press_event().connect(sigc::mem_fun(*this, &App2Assistant::on_second_page_backspace_clicked));
	second_page_tv.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);


	signal_cancel().connect(sigc::mem_fun(*this, &App2Assistant::on_assistant_cancel));
	signal_delete_event().connect(sigc::mem_fun(*this, &App2Assistant::on_delete_event));
	

	show_all_children();
}


App2Assistant::~App2Assistant() {
	//nada!
}

void App2Assistant::on_assistant_cancel() {
	std::cout << "Cancel button clicked" << std::endl;
	//Gtk::Main::quit();
	//get_application()->quit();
	get_application()->remove_window(*this);
}
bool App2Assistant::on_delete_event(GdkEventAny* event) {
	std::cout << "Window delete event" << std::endl;
	//Gtk::Main::quit();
	//get_application()->quit();
	get_application()->remove_window(*this);
	return true;
}

void App2Assistant::on_second_page_open_clicked() {
	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*this, "Please select a number of AXIL results files", Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog->set_select_multiple();
	Glib::RefPtr<Gtk::FileFilter> filter_asr = Gtk::FileFilter::create();
	//perhaps a custom filter could be useful here, one that actually allows only single peak asr files?
	//could be slow though
	filter_asr->set_name("ASR files");
	filter_asr->add_pattern("*.asr");
	filter_asr->add_pattern("*.ASR");
	dialog->add_filter(filter_asr);
	dialog->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog->add_button("Select", Gtk::RESPONSE_OK);
	
	int result = dialog->run();
	std::vector<std::string> filenames;

	switch(result) {
		case(Gtk::RESPONSE_OK):
			std::cout << "Open clicked." << std::endl;
			filenames = dialog->get_filenames();

      			break;
		case(Gtk::RESPONSE_CANCEL):
		default:
			delete dialog;
			return;
	}

	delete dialog;

	for (std::vector<std::string>::iterator it = filenames.begin() ; it != filenames.end() ; ++it)  {
		std::cout << "Selected file: " << *it << std::endl;

		BAM::File::ASR *asr_file;
		try {
			asr_file = new BAM::File::ASR(*it);
		}
		catch (ifstream::failure &e) {
			Gtk::MessageDialog dialog(*this, "Error reading in "+*it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			dialog.set_secondary_text(Glib::ustring("I/O error: ")+e.what());
  			dialog.run();
			continue;
		}
		catch (BAM::Exception &e) {
			Gtk::MessageDialog dialog(*this, "Error reading in "+*it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			dialog.set_secondary_text(e.what());
  			dialog.run();
			continue;
		}
		//count number of elements -> must be 1!
		if (asr_file->GetNPeaks() > 1) {
			Gtk::MessageDialog dialog(*this, "Error reading in "+*it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			dialog.set_secondary_text(Glib::ustring("BAM::Data::ASR error: only one peak permitted per file!"));
  			dialog.run();
			delete asr_file;
			continue;
		}
		BAM::Data::ASR asr_data = asr_file->GetData(0);
		int Z = asr_data.GetZ();
		//check if already present
		Gtk::TreeModel::Children kids = second_page_model->children();
		bool match = false;
		for (Gtk::TreeModel::Children::iterator iter = kids.begin() ; iter != kids.end() ; ++iter) {
			Gtk::TreeModel::Row row = *iter;
			if (row[second_page_columns.col_atomic_number] == Z) {
				match = true;
				break;
			}
		}
		if (match) {
			Gtk::MessageDialog dialog(*this, "Error reading in "+*it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			dialog.set_secondary_text(Glib::ustring("Element already present in table. Delete it first before adding it again!"));
  			dialog.run();
			delete asr_file;
			continue;
		}

		char *element = AtomicNumberToSymbol(Z);
		Gtk::TreeModel::Row row = *(second_page_model->append());
		row[second_page_columns.col_element] = Glib::ustring(element);
		xrlFree(element);
		row[second_page_columns.col_filename] = Glib::path_get_basename(*it);
		row[second_page_columns.col_atomic_number] = Z;
		//this is causing a memory leak when the assistant is killed!!!
		row[second_page_columns.col_bam_file_asr] = asr_file;
	}
	if (second_page_model->children().size() >= 2) {
		set_page_complete(second_page, true);	
	}
	else {
		set_page_complete(second_page, false);	
	}
}

bool App2Assistant::on_second_page_backspace_clicked(GdkEventKey *event) {
	if (event->keyval == gdk_keyval_from_name("BackSpace") ||
	    event->keyval == gdk_keyval_from_name("Delete")) {
		Glib::RefPtr<Gtk::TreeSelection> selection = second_page_tv.get_selection();
		std::vector<Gtk::TreeModel::Path> paths = selection->get_selected_rows();
		for (std::vector<Gtk::TreeModel::Path>::reverse_iterator rit = paths.rbegin() ; rit != paths.rend() ; ++rit) {
			second_page_model->erase(second_page_model->get_iter(*rit));
		}
		if (second_page_model->children().size() >= 2) {
			set_page_complete(second_page, true);	
		}
		else {
			set_page_complete(second_page, false);	
		}
		
               	return TRUE;
        }

        return FALSE;

}
