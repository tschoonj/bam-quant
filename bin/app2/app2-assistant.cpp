#include "app2-assistant.h"
#include <iostream>
#include <gtkmm/application.h>
#include <gtkmm/main.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <xraylib.h>
#include <glibmm/miscutils.h>
#include <glib.h>
#include <glibmm/spawn.h>
#include <glibmm/main.h>
#include <glibmm/convert.h>
#include <algorithm>

#ifdef G_OS_UNIX
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <signal.h>
	#define real_xmimsim_pid ((int) xmimsim_pid)
#elif defined(G_OS_WIN32)
	#include <windows.h>
	#define real_xmimsim_pid ((int) GetProcessId((HANDLE) xmimsim_pid))
	typedef LONG (NTAPI *pNtSuspendProcess )(IN HANDLE ProcessHandle );
	typedef LONG (NTAPI *pNtResumeProcess )(IN HANDLE ProcessHandle );
	static pNtSuspendProcess NtSuspendProcess = NULL;
	static pNtResumeProcess NtResumeProcess = NULL;
#endif



App2Assistant::App2Assistant() : first_page("Welcome!\n\nIn this wizard you will produce a file containing\n"
							"relative X-ray intensities based on the net-line intensities\n"
							"obtained from pure elemental standards and samples."),
		xmimsim_paused(false),
		xmimsim_pid(0),
		timer(0),
		fifth_page_buttons(Gtk::ORIENTATION_VERTICAL)
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

	//third page
	label = Gtk::manage(new Gtk::Label("Select at least one ASR file\ncorresponding to measurements performed\non samples with unknown composition..."));
	third_page.attach(*label, 0, 0, 2, 1);
	third_page.set_column_spacing(5);
	third_page.set_row_spacing(5);
	third_page.set_row_homogeneous(false);
	third_page.set_column_homogeneous(false);
	//label->set_vexpand();
	label->set_hexpand();
	label->set_margin_bottom(10);
	label->set_margin_top(10);
	
	third_page_open.set_image_from_icon_name("document-open");
	third_page_open.set_vexpand(false);
	third_page_open.set_hexpand(false);
	third_page_open.set_valign(Gtk::ALIGN_CENTER);
	third_page.attach(third_page_open, 0, 1, 1, 1);
	
	third_page_model = Gtk::ListStore::create(third_page_columns);
	third_page_tv.set_model(third_page_model);
	third_page_tv.append_column("Elements", third_page_columns.col_elements);
	third_page_tv.append_column("Filename", third_page_columns.col_filename);
	third_page_sw.add(third_page_tv);
	third_page_sw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	third_page.attach(third_page_sw, 1, 1, 1, 1);
	third_page_sw.set_vexpand();
	third_page_sw.set_hexpand();
	append_page(third_page);
	set_page_type(third_page, Gtk::ASSISTANT_PAGE_CONTENT);
	set_page_title(third_page, "Select sample ASR files");
	third_page_open.signal_clicked().connect(sigc::mem_fun(*this, &App2Assistant::on_third_page_open_clicked));
	third_page_tv.signal_key_press_event().connect(sigc::mem_fun(*this, &App2Assistant::on_third_page_backspace_clicked));
	third_page_tv.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);

	
	//fourth page
	fourth_page_xmsi_file = 0; //set to zero when initializing
	label = Gtk::manage(new Gtk::Label("Select an XMI-MSIM input-file\nthat properly describes the\nexcitation conditions as well as the\nexperimental geometry"));
	fourth_page.attach(*label, 0, 0, 1, 1);
	fourth_page.set_column_spacing(5);
	fourth_page.set_row_spacing(5);
	fourth_page.set_row_homogeneous(false);
	fourth_page.set_column_homogeneous(false);
	//label->set_vexpand();
	label->set_hexpand();
	label->set_margin_bottom(10);
	label->set_margin_top(10);
	
	fourth_page_xmsi_entry.set_icon_from_icon_name("Logo_xmi_msim");
	fourth_page.attach(fourth_page_xmsi_entry, 0, 1, 1, 1);
	fourth_page.set_vexpand();
	fourth_page.set_hexpand();
	append_page(fourth_page);
	set_page_type(fourth_page, Gtk::ASSISTANT_PAGE_CONTENT);
	set_page_title(fourth_page, "Select XMSI file");
	fourth_page_xmsi_entry.signal_icon_press().connect(sigc::mem_fun(*this, &App2Assistant::on_fourth_page_open_clicked));
	fourth_page_xmsi_entry.set_editable(false);

	//fifth page
	fifth_page_pause_button.set_image_from_icon_name("media-playback-pause", Gtk::ICON_SIZE_DIALOG);	
	fifth_page_stop_button.set_image_from_icon_name("media-playback-stop", Gtk::ICON_SIZE_DIALOG);	
	fifth_page_play_button.set_image_from_icon_name("media-playback-start", Gtk::ICON_SIZE_DIALOG);	
	fifth_page_buttons.pack_start(fifth_page_play_button);
	fifth_page_buttons.pack_start(fifth_page_pause_button);
	fifth_page_buttons.pack_start(fifth_page_stop_button);
	fifth_page_buttons.set_layout(Gtk::BUTTONBOX_CENTER);
	fifth_page_buttons.set_spacing(10);
	fifth_page_buttons.set_vexpand(false);
	fifth_page_buttons.set_hexpand(false);
	fifth_page_model = Gtk::ListStore::create(fifth_page_columns);
	fifth_page_tv.set_model(fifth_page_model);
	fifth_page_tv.append_column("Element", fifth_page_columns.col_element);
	fifth_page_tv.append_column("Status", fifth_page_columns.col_status);
	fifth_page_sw.add(fifth_page_tv);
	fifth_page_sw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	fifth_page_sw.set_vexpand();
	fifth_page_sw.set_hexpand();
	label = Gtk::manage(new Gtk::Label("Simulate elements from the list of samples\nthat do not have matching pure standard data."));
	fifth_page.attach(*label, 0, 0, 2, 1);
	fifth_page.set_column_spacing(5);
	fifth_page.set_row_spacing(5);
	fifth_page.set_row_homogeneous(false);
	fifth_page.set_column_homogeneous(false);
	fifth_page.set_vexpand();
	fifth_page.set_hexpand();
	//label->set_vexpand();
	label->set_hexpand();
	label->set_margin_bottom(10);
	label->set_margin_top(10);
	fifth_page.attach(fifth_page_buttons, 0, 1, 1, 1);
	fifth_page.attach(fifth_page_sw, 1, 1, 1, 1);
	append_page(fifth_page);
	set_page_type(fifth_page, Gtk::ASSISTANT_PAGE_CONTENT);
	set_page_title(fifth_page, "Simulate missing elements");
	fifth_page_pause_button.signal_clicked().connect(sigc::mem_fun(*this, &App2Assistant::on_fifth_page_pause_clicked));
	fifth_page_play_button.signal_clicked().connect(sigc::mem_fun(*this, &App2Assistant::on_fifth_page_play_clicked));
	fifth_page_stop_button.signal_clicked().connect(sigc::mem_fun(*this, &App2Assistant::on_fifth_page_stop_clicked));
		
	


	signal_cancel().connect(sigc::mem_fun(*this, &App2Assistant::on_assistant_cancel));
	signal_delete_event().connect(sigc::mem_fun(*this, &App2Assistant::on_delete_event));
	signal_prepare().connect(sigc::mem_fun(*this, &App2Assistant::on_assistant_prepare));
	

	show_all_children();
}


App2Assistant::~App2Assistant() {
	//destructor
	//delete all the ASR data
	//second page
	Gtk::TreeModel::Children kids = second_page_model->children();
	for (Gtk::TreeModel::Children::iterator iter = kids.begin() ; iter != kids.end() ; ++iter) {
		Gtk::TreeModel::Row row = *iter;
		delete row[second_page_columns.col_bam_file_asr];
	}
	kids = third_page_model->children();
	for (Gtk::TreeModel::Children::iterator iter = kids.begin() ; iter != kids.end() ; ++iter) {
		Gtk::TreeModel::Row row = *iter;
		delete row[third_page_columns.col_bam_file_asr];
		delete row[third_page_columns.col_elements_int];
	}
	kids = fifth_page_model->children();
	for (Gtk::TreeModel::Children::iterator iter = kids.begin() ; iter != kids.end() ; ++iter) {
		Gtk::TreeModel::Row row = *iter;
		if (row[fifth_page_columns.col_xmsi_file])
			delete row[fifth_page_columns.col_xmsi_file];
		if (row[fifth_page_columns.col_xmso_file])
			delete row[fifth_page_columns.col_xmso_file];
	}
	if (fourth_page_xmsi_file)
		delete fourth_page_xmsi_file;
	if (timer)
		delete timer;
	if (xmimsim_pid) {
		//a process is still running!
#ifdef G_OS_UNIX
		int kill_rv;
		kill_rv = kill((pid_t) xmimsim_pid, SIGTERM);
#if !GLIB_CHECK_VERSION (2, 35, 0)
		waitpid(xmimsim_pid, NULL, WNOHANG);
#endif
#elif defined(G_OS_WIN32)
		BOOL terminate_rv;
		terminate_rv = TerminateProcess((HANDLE) xmimsim_pid, (UINT) 1);
#endif
	}
}

void App2Assistant::on_assistant_cancel() {
	std::cout << "Cancel button clicked" << std::endl;
	//Gtk::Main::quit();
	//get_application()->quit();
	get_application()->remove_window(*this);
}

void App2Assistant::on_assistant_prepare(Gtk::Widget *page) {
	//when the 5th page has been reached, fill it up properly and commit
	if (get_current_page() != 4) {
		return;
	}
	commit();

	//check if elements need be simulated
	std::vector<int> pure_elements;
	Gtk::TreeModel::Children kids = second_page_model->children();
	for (Gtk::TreeModel::Children::iterator iter = kids.begin() ; iter != kids.end() ; ++iter) {
		Gtk::TreeModel::Row row = *iter;
		pure_elements.push_back(row[second_page_columns.col_atomic_number]);
	}
	std::sort(pure_elements.begin(), pure_elements.end());
	std::cout << "pure_elements contains:";
  	for (std::vector<int>::iterator it=pure_elements.begin(); it!=pure_elements.end(); ++it)
    		std::cout << ' ' << *it;
  	std::cout << std::endl;

	std::vector<int> sample_elements;
	kids = third_page_model->children();
	for (Gtk::TreeModel::Children::iterator iter = kids.begin() ; iter != kids.end() ; ++iter) {
		Gtk::TreeModel::Row row = *iter;
		std::vector<int> *col_elements_int = row[third_page_columns.col_elements_int];
		//std::cout << "col_elements: " << row[third_page_columns.col_elements] << std::endl;
		//std::cout << "col_elements_int size: " << col_elements_int->size() << std::endl;
		sample_elements.insert(sample_elements.end(), col_elements_int->begin(), col_elements_int->end());
	}
	std::sort(sample_elements.begin(), sample_elements.end());
	std::unique(sample_elements.begin(), sample_elements.end());
	/*std::cout << "sample_elements contains:";
  	for (std::vector<int>::iterator it=sample_elements.begin(); it!=sample_elements.end(); ++it)
    		std::cout << ' ' << *it;
  	std::cout << std::endl;*/

	std::vector<int> diff_elements(pure_elements.size()+sample_elements.size());
	std::vector<int>::iterator it = std::set_difference(sample_elements.begin(), sample_elements.end(), pure_elements.begin(), pure_elements.end(), diff_elements.begin());
	diff_elements.resize(it-diff_elements.begin());
	/*std::cout << "diff_elements contains:";
  	for (std::vector<int>::iterator it=diff_elements.begin(); it!=diff_elements.end(); ++it)
    		std::cout << ' ' << *it;
  	std::cout << std::endl;*/


	if (diff_elements.size() == 0) {
		//no simulations needed
		fifth_page_play_button.set_sensitive(false);
		fifth_page_pause_button.set_sensitive(false);
		fifth_page_stop_button.set_sensitive(false);
		set_page_complete(fifth_page, true);	
		return;
	}
	//otherwise make the necessary preparations
  	for (std::vector<int>::iterator it=diff_elements.begin(); it!=diff_elements.end(); ++it) {
		//update model
		char *element = AtomicNumberToSymbol(*it);
		Gtk::TreeModel::Row row = *(fifth_page_model->append());
		row[fifth_page_columns.col_element] = Glib::ustring(element);
		row[fifth_page_columns.col_status] = Glib::ustring("not started");
		xrlFree(element);
		
		//create xmsi files
		row[fifth_page_columns.col_xmsi_filename] = Glib::build_filename(Glib::get_tmp_dir(), "bam-quant-" + Glib::get_user_name() + "-" + static_cast<ostringstream*>( &(ostringstream() << getpid()))->str() + row[fifth_page_columns.col_element] + ".xmsi");
		row[fifth_page_columns.col_xmsi_file] = new BAM::File::XMSI(*fourth_page_xmsi_file);
		row[fifth_page_columns.col_xmso_file] = 0;
		BAM::File::XMSI *temp_xmsi_file = row[fifth_page_columns.col_xmsi_file];
		std::string temp_xmsi_filename = Glib::locale_from_utf8(row[fifth_page_columns.col_xmsi_filename]);
		
		//now change its composition
		BAM::Data::XMSI::Composition composition;

		BAM::Data::XMSI::Layer layer1("Air, Dry (near sea level)", 5.0);
		composition.AddLayer(layer1);

		BAM::Data::XMSI::Layer layer2(ElementDensity(*it), 5.0);
		layer2.AddElement(*it, 1.0);
		composition.AddLayer(layer2);

		composition.SetReferenceLayer(2);
		temp_xmsi_file->ReplaceComposition(composition);
		std::string temp_xmso_filename = temp_xmsi_filename;
		temp_xmso_filename.replace(temp_xmso_filename.end()-1,temp_xmso_filename.end(), "o");
		temp_xmsi_file->SetOutputFile(temp_xmso_filename);
		temp_xmsi_file->SetFilename(temp_xmsi_filename);
		row[fifth_page_columns.col_xmso_filename] = temp_xmso_filename;
	}

	fifth_page_play_button.set_sensitive(true);
	fifth_page_pause_button.set_sensitive(false);
	fifth_page_stop_button.set_sensitive(false);
	
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
			Gtk::TreeModel::Row row = *(second_page_model->get_iter(*rit));
			delete row[second_page_columns.col_bam_file_asr];
			second_page_model->erase(row);
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

void App2Assistant::on_third_page_open_clicked() {
	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*this, "Please select a number of AXIL results files", Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog->set_select_multiple();
	Glib::RefPtr<Gtk::FileFilter> filter_asr = Gtk::FileFilter::create();
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
		Glib::ustring elements;
		std::vector<int> *elements_int = new std::vector<int>;
		for (int i = 0 ; i < asr_file->GetNPeaks() ; i++) {
			BAM::Data::ASR asr_data = asr_file->GetData(i);
			int Z = asr_data.GetZ();
			char *element = AtomicNumberToSymbol(Z);
			elements += element;
			if (i != asr_file->GetNPeaks()-1)
				elements += ", "; 	
			xrlFree(element);
			elements_int->push_back(Z);
		}
		std::cout << "elements_int size: " << elements_int->size() << std::endl;
		std::cout << "elements: " << elements << std::endl;
		Gtk::TreeModel::Row row = *(third_page_model->append());
		row[third_page_columns.col_elements] = elements;
		row[third_page_columns.col_filename] = Glib::path_get_basename(*it);
		row[third_page_columns.col_bam_file_asr] = asr_file;
		row[third_page_columns.col_elements_int] = elements_int;
	}
	if (third_page_model->children().size() >= 1) {
		set_page_complete(third_page, true);	
	}
	else {
		set_page_complete(third_page, false);	
	}
}

bool App2Assistant::on_third_page_backspace_clicked(GdkEventKey *event) {
	if (event->keyval == gdk_keyval_from_name("BackSpace") ||
	    event->keyval == gdk_keyval_from_name("Delete")) {
		Glib::RefPtr<Gtk::TreeSelection> selection = third_page_tv.get_selection();
		std::vector<Gtk::TreeModel::Path> paths = selection->get_selected_rows();
		for (std::vector<Gtk::TreeModel::Path>::reverse_iterator rit = paths.rbegin() ; rit != paths.rend() ; ++rit) {
			Gtk::TreeModel::Row row = *(third_page_model->get_iter(*rit));
			delete row[third_page_columns.col_bam_file_asr];
			third_page_model->erase(row);
		}
		if (third_page_model->children().size() >= 1) {
			set_page_complete(third_page, true);	
		}
		else {
			set_page_complete(third_page, false);	
		}
               	return TRUE;
        }

        return FALSE;

}
void App2Assistant::on_fourth_page_open_clicked(Gtk::EntryIconPosition icon_position, const GdkEventButton* event) {
	//launch dialog
	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*this, "Please select an XMI-MSIM input-file", Gtk::FILE_CHOOSER_ACTION_OPEN);
	Glib::RefPtr<Gtk::FileFilter> filter_asr = Gtk::FileFilter::create();
	filter_asr->set_name("XMSI files");
	filter_asr->add_pattern("*.XMSI");
	filter_asr->add_pattern("*.xmsi");
	dialog->add_filter(filter_asr);
	dialog->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog->add_button("Select", Gtk::RESPONSE_OK);
	
	int result = dialog->run();
	Glib::ustring filename;
	switch(result) {
		case(Gtk::RESPONSE_OK):
			std::cout << "Open clicked." << std::endl;
			filename = dialog->get_filename();

      			break;
		case(Gtk::RESPONSE_CANCEL):
		default:
			delete dialog;
			return;
	}

	delete dialog;
	std::cout << "Selected file: " << filename << std::endl;
	BAM::File::XMSI *temp_xmsi_file;
	try {
		temp_xmsi_file = new BAM::File::XMSI(filename);
	}
	catch (ifstream::failure &e) {
		Gtk::MessageDialog dialog(*this, "Error reading in "+filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  		dialog.set_secondary_text(Glib::ustring("I/O error: ")+e.what());
  		dialog.run();
		return;
	}
	catch (BAM::Exception &e) {
		Gtk::MessageDialog dialog(*this, "Error reading in "+filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  		dialog.set_secondary_text(e.what());
  		dialog.run();
		return;
	}
	if (fourth_page_xmsi_file)
		delete fourth_page_xmsi_file;

	fourth_page_xmsi_file = temp_xmsi_file;

	set_page_complete(fourth_page, true);	
	fourth_page_xmsi_entry.set_text(filename);
}

void App2Assistant::on_fifth_page_play_clicked() {
	if (xmimsim_paused) {
		fifth_page_play_button.set_sensitive(false);	
		int kill_rv;

		//glibmm doesn't have the continue method so let's use glib for this
		g_timer_continue(timer->gobj());
#ifdef G_OS_UNIX
		kill_rv = kill((pid_t) xmimsim_pid, SIGCONT);
#elif defined(G_OS_WIN32)
		kill_rv = (int) NtResumeProcess((HANDLE) xmimsim_pid);
#else
		#error "Neither G_OS_UNIX nor G_OS_WIN32 is defined!"
#endif

		if (kill_rv == 0) {
			stringstream ss;
			ss << get_elapsed_time() <<"Process " << real_xmimsim_pid << " was successfully resumed" << endl;
			update_console(ss.str(), "pause-continue-stopped");
			xmimsim_paused = false;
			fifth_page_pause_button.set_sensitive(true);
		}
		else {
			//if this happens, we're in serious trouble!
			stringstream ss;
			ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " could not be resumed" << endl;
			update_console(ss.str(), "pause-continue-stopped");
			fifth_page_play_button.set_sensitive(true);
			xmimsim_pid = 0;
				
		}
		return;
	}

	xmimsim_paused = false;
	fifth_page_play_button.set_sensitive(false);

	//let's start the clock!
	timer = new Glib::Timer();

	//create argv vector
	//at some point these could be read from preferences or so...
	argv.push_back("xmimsim");
	argv.push_back("--enable-M-lines");
	argv.push_back("--enable-radiative-cascade");
	argv.push_back("--enable-auger-cascade");
	argv.push_back("--enable-variance-reduction");
	argv.push_back("--disable-pile-up");
	argv.push_back("--disable-poisson");
	argv.push_back("--enable-escape-peaks");
	argv.push_back("--disable-advanced-compton");
	argv.push_back("--enable-opencl");
	argv.push_back("--verbose");
	//on windows; dont forget to set the solid angles and escape ratios hdf5 files

	fifth_page_iter = fifth_page_model->children().begin(); 

	try {
		xmimsim_start_recursive();	
	}
	catch (BAM::Exception &e) {
		Gtk::MessageDialog dialog(*this, "Error occurred in XMI-MSIM dialog", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
		Glib::ustring error_message = Glib::ustring(e.what());
		std::cout << e.what() << std::endl;
  		dialog.set_secondary_text(error_message);
  		dialog.run();
	}

}

void App2Assistant::on_fifth_page_pause_clicked() {
	std::cout << "pause clicked" << std::endl;

	timer->stop();

	fifth_page_pause_button.set_sensitive(false);
	fifth_page_stop_button.set_sensitive(false);
	int kill_rv;
#ifdef G_OS_UNIX
	kill_rv = kill((pid_t) xmimsim_pid, SIGSTOP);
#elif defined(G_OS_WIN32)
	kill_rv = (int) NtSuspendProcess((HANDLE) xmimsim_pid);
#endif
	if (kill_rv == 0) {
		stringstream ss;
		ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " was successfully paused. Press the Play button to continue or Stop to kill the process" << std::endl;
		update_console(ss.str(), "pause-continue-stopped");
		xmimsim_paused = true;
		fifth_page_stop_button.set_sensitive(true);
		fifth_page_play_button.set_sensitive(true);
	}
}

void App2Assistant::on_fifth_page_stop_clicked() {
	std::cout << "stop clicked" << std::endl;

	fifth_page_stop_button.set_sensitive(false);
	fifth_page_pause_button.set_sensitive(false);
	fifth_page_play_button.set_sensitive(false);

	xmimsim_paused = false;

#ifdef G_OS_UNIX
	int kill_rv;
	
	kill_rv = kill((pid_t) xmimsim_pid, SIGTERM);
#if !GLIB_CHECK_VERSION (2, 35, 0)
	//starting with 2.36.0 (and some unstable versions before),
	//waitpid is called from within the main loop
	//causing all kinds of trouble if I would call wait here
	//wait(NULL);
	waitpid(xmimsim_pid, NULL, WNOHANG);
#endif
	if (kill_rv == 0) {
		stringstream ss;
		ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " was successfully terminated before completion" << std::endl;
		update_console(ss.str(), "pause-continue-stopped");
	}
	else {
		stringstream ss;
		ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " could not be terminated with the SIGTERM signal" << std::endl;
		update_console(ss.str(), "error");
		xmimsim_pid = 0;
	}
#elif defined(G_OS_WIN32)
	BOOL terminate_rv;

	terminate_rv = TerminateProcess((HANDLE) xmimsim_pid, (UINT) 1);

	if (terminate_rv == TRUE) {
		stringstream ss;
		ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " was successfully terminated before completion" << std::endl;
		update_console(ss.str(), "pause-continue-stopped");
	}
	else {
		stringstream ss;
		ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " could not be terminated with the TerminateProcess call" << std::endl;
		update_console(ss.str(), "error");
		xmimsim_pid = 0;
	}
#endif

	return;
}

void App2Assistant::xmimsim_start_recursive() {
	int out_fh, err_fh;


	//let's start work with a nap!
	g_usleep(G_USEC_PER_SEC/2);


	//write inputfile
	Gtk::TreeModel::Row row = *fifth_page_iter;
	BAM::File::XMSI *temp_xmsi_file = row[fifth_page_columns.col_xmsi_file];
	temp_xmsi_file->Write();
	

	std::cout << "Processing " << temp_xmsi_file->GetFilename() << endl;
	std::cout << "Outputfile will be " << temp_xmsi_file->GetOutputFile() << endl;
	argv.push_back(temp_xmsi_file->GetFilename());


	//spawn process
	try {
		Glib::spawn_async_with_pipes(std::string(""), argv, Glib::SPAWN_SEARCH_PATH | Glib::SPAWN_DO_NOT_REAP_CHILD, sigc::slot<void>(), &xmimsim_pid, 0, &out_fh, &err_fh);	
	}
	catch (Glib::SpawnError &e) {
		throw BAM::Exception(string("App2Assistant::on_fifth_page_play_clicked -> ") + e.what());
	}

	//update treemodel
	row[fifth_page_columns.col_status] = Glib::ustring("running");
	

	stringstream ss;
	ss << get_elapsed_time() << argv.back() << " was started with process id " << real_xmimsim_pid << std::endl;
	update_console(ss.str());

	xmimsim_paused = false;
	fifth_page_pause_button.set_sensitive(true);
	fifth_page_stop_button.set_sensitive(true);
		
	xmimsim_stderr = Glib::IOChannel::create_from_fd(err_fh);
	xmimsim_stdout = Glib::IOChannel::create_from_fd(out_fh);

	xmimsim_stderr->set_close_on_unref(true);
	xmimsim_stdout->set_close_on_unref(true);

	//add watchers
	Glib::signal_child_watch().connect(sigc::mem_fun(*this, &App2Assistant::xmimsim_child_watcher), xmimsim_pid);
	Glib::signal_io().connect(sigc::mem_fun(*this, &App2Assistant::xmimsim_stdout_watcher), xmimsim_stdout,Glib::IO_IN | Glib::IO_PRI | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL, Glib::PRIORITY_HIGH); 
	Glib::signal_io().connect(sigc::mem_fun(*this, &App2Assistant::xmimsim_stderr_watcher), xmimsim_stderr,Glib::IO_IN | Glib::IO_PRI | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL, Glib::PRIORITY_HIGH); 



	//remove xmsi file from argv
	argv.pop_back();
	
}

void App2Assistant::xmimsim_child_watcher(GPid pid, int status) {
	int success;
	//end of process
	fifth_page_stop_button.set_sensitive(false);
	fifth_page_pause_button.set_sensitive(false);

	//windows <-> unix issues here
	//unix allows to obtain more info about the way the process was terminated, windows will just have the exit code (status)
	//conditional compilation here
#ifdef G_OS_UNIX
	if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == 0) { /* child was terminated due to a call to exit */
			stringstream ss;
			ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " exited normally without errors" << std::endl;
			update_console(ss.str(), "success");
			success = 1;
		}
		else {
			stringstream ss;
			ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " exited with an error (code " << WEXITSTATUS(status) << ")" << std::endl;
			update_console(ss.str(), "error");
			success = 0;
			xmimsim_pid = 0;
		}
	}
	else if (WIFSIGNALED(status)) { /* child was terminated due to a signal */
		stringstream ss;
		ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " was terminated by signal " << WTERMSIG(status) << std::endl;
		update_console(ss.str(), "error");
		xmimsim_pid = 0;
		success = 0;
	}
	else {
		stringstream ss;
		ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " was terminated in some special way" << endl;
		update_console(ss.str(), "error");
		xmimsim_pid = 0;
		success = 0;
	}

#elif defined(G_OS_WIN32)
	if (status == 0) {
		stringstream ss;
		ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " exited normally without errors" << endl;
		update_console(ss.str(), "success");
		success = 1;
	}
	else {
		ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " exited with an error (code " << status << ")" << endl;
		update_console(ss.str(), "error");
		success = 0;
		xmimsim_pid = 0;
	}
#endif

	Glib::spawn_close_pid(xmimsim_pid);
	
	if (success == 0) {
		//something went badly wrong
		Gtk::TreeModel::Row row = *fifth_page_iter;
		row[fifth_page_columns.col_status] = Glib::ustring("failed");
		fifth_page_play_button.set_sensitive(false);
		fifth_page_stop_button.set_sensitive(false);
		fifth_page_pause_button.set_sensitive(false);
	}
	else if (++fifth_page_iter == fifth_page_model->children().end()) {
		fifth_page_iter--;
		//last simulation
		Gtk::TreeModel::Row row = *fifth_page_iter;
		BAM::File::XMSO *xmso_file;
		Glib::ustring xmso_filename = row[fifth_page_columns.col_xmso_filename];
		try {
			xmso_file = new BAM::File::XMSO(Glib::locale_from_utf8(xmso_filename));
		}
		catch (BAM::Exception &e) {
			throw BAM::Exception("App2Assistant::xmimsim_start_recursive -> Could not read XMSO file");
		}
		row[fifth_page_columns.col_xmso_file] = xmso_file;
		/*	
		try {
			buttonVector[buttonIndex-1]->xmso_counts_KA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "KL2");
		}
		catch (BAM::Exception &e) {}
		try {
			buttonVector[buttonIndex-1]->xmso_counts_KA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "KL3");
		}
		catch (BAM::Exception &e) {}
		try {
			buttonVector[buttonIndex-1]->xmso_counts_LA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "L3M4");
		}
		catch (BAM::Exception &e) {}
		try {
			buttonVector[buttonIndex-1]->xmso_counts_LA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "L3M5");
		}
		catch (BAM::Exception &e) {}
		*/
		row[fifth_page_columns.col_status] = Glib::ustring("completed");
		fifth_page_play_button.set_sensitive(false);
		fifth_page_stop_button.set_sensitive(false);
		fifth_page_pause_button.set_sensitive(false);
		set_page_complete(fifth_page, true);
		xmimsim_pid = 0;
		//delete temparary XMI-MSIM files
		//g_unlink(buttonVector[buttonIndex-1]->xmsi_file->GetOutputFile().c_str());
		//g_unlink(buttonVector[buttonIndex-1]->xmsi_file->GetFilename().c_str());
	}
	else {
		fifth_page_iter--;
		Gtk::TreeModel::Row row = *fifth_page_iter;
		BAM::File::XMSO *xmso_file;
		Glib::ustring xmso_filename = row[fifth_page_columns.col_xmso_filename];
		try {
			xmso_file = new BAM::File::XMSO(Glib::locale_from_utf8(xmso_filename));
		}
		catch (BAM::Exception &e) {
			throw BAM::Exception("App2Assistant::xmimsim_start_recursive -> Could not read XMSO file");
		}
		row[fifth_page_columns.col_xmso_file] = xmso_file;
		/*
		try {
			buttonVector[buttonIndex-1]->xmso_counts_KA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "KL2");
		}
		catch (BAM::Exception &e) {}
		try {
			buttonVector[buttonIndex-1]->xmso_counts_KA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "KL3");
		}
		catch (BAM::Exception &e) {}
		try {
			buttonVector[buttonIndex-1]->xmso_counts_LA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "L3M4");
		}
		catch (BAM::Exception &e) {}
		try {
			buttonVector[buttonIndex-1]->xmso_counts_LA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "L3M5");
		}
		catch (BAM::Exception &e) {}
		*/

		row[fifth_page_columns.col_status] = Glib::ustring("completed");
		//g_unlink(buttonVector[buttonIndex-1]->xmsi_file->GetOutputFile().c_str());
		//g_unlink(buttonVector[buttonIndex-1]->xmsi_file->GetFilename().c_str());
		//
		fifth_page_iter++;
		xmimsim_start_recursive();	
	}
}


bool App2Assistant::xmimsim_stdout_watcher(Glib::IOCondition cond) {
	return xmimsim_iochannel_watcher(cond, xmimsim_stdout);
}

bool App2Assistant::xmimsim_stderr_watcher(Glib::IOCondition cond) {
	return xmimsim_iochannel_watcher(cond, xmimsim_stderr);
}

bool App2Assistant::xmimsim_iochannel_watcher(Glib::IOCondition condition, Glib::RefPtr<Glib::IOChannel> iochannel) {
	Glib::IOStatus pipe_status;
	Glib::ustring pipe_string;

	if (condition & (Glib::IO_IN | Glib::IO_PRI)) {
		try {
			pipe_status = iochannel->read_line(pipe_string);	
			if (pipe_status == Glib::IO_STATUS_NORMAL) {
				stringstream ss;
				ss << get_elapsed_time() << pipe_string;
				update_console(ss.str());
				std::cout << pipe_string;
			}
			else
				return false;
		}
		catch (Glib::IOChannelError &e) {
			stringstream ss;
			ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " had an I/O channel error: " << e.what() << endl;
			update_console(ss.str(), "error");
			xmimsim_pid = 0;
			return false;
		}
		catch (Glib::ConvertError &e) {
			stringstream ss;
			ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " had a convert error: " << e.what() << endl;
			update_console(ss.str(), "error");
			xmimsim_pid = 0;
			return false;
		}
	}
	else if (condition & (Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL)) {
		//hung up...
		return false;
	}

	return true;
}

void App2Assistant::update_console(string line, string tag) {
	/*
	Glib::RefPtr<Gtk::TextBuffer::Mark> cur_mark = console_buffer->create_mark(console_buffer->end());
	if (tag == "") {
		console_buffer->insert(console_buffer->end(), line);
	}
	else {
		console_buffer->insert_with_tag(console_buffer->end(), line, tag);
	}
	console_buffer->move_mark(cur_mark, console_buffer->end());
	console_view.scroll_to(cur_mark, 0.0);*/
}
