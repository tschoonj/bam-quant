#include "window.h"
#include <gtkmm/filechooser.h>
#include <gtkmm/label.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stylecontext.h>
#include <unistd.h>
#include <glibmm/miscutils.h>
#include "xmi-msim-dialog.h"




Window::Window() : big_box(Gtk::ORIENTATION_VERTICAL, 5) {
	xmi_msim_dialog = 0;

	//menu signals
	add_action("new", sigc::mem_fun(*this, &Window::new_project));
	/*settings_action = add_action("settings", sigc::mem_fun(*this, &Window::settings));
	settings_action->set_enabled(false);
	*/

	set_title("app1");
	set_size_request(400, 200);
	set_border_width(10);

	//cssprovider stuff
	cssprovider = Gtk::CssProvider::create();
	Glib::ustring cssdata = "GtkButton {color: red}";
	if (not cssprovider->load_from_data(cssdata)) {
		std::cerr << "Failed to load css" << std::endl;
	}

	//initialize grid
	buttonGrid.set_row_spacing(10);
	buttonGrid.set_column_spacing(10);
	big_box.pack_start(buttonGrid, false, false, 10);
	add(big_box);

	int row = 0, column = 0;

	for (int i = 1 /* H */; i <= 94 /* Pu */ ; i++) {
		MendeleevButton *button = new MendeleevButton(i);
		buttonMap[i] = button;

		buttonGrid.attach(*button, column, row, 1, 1);

		switch(i) {
			case 1:
				column = 17;
				break;
			case 2:
			case 10:
			case 18:
			case 36:
			case 54:
			case 86:
				column = 0;
				row++;
				break;
			case 4:
			case 12:
				column = 12;
				break;
			case 56: /* lanthanides */
				column = 2;
				row = 8;
				break;
			case 71: /* exit lanthanides */
				column = 3;
				row = 5;
				break;
			case 88: /* actinides */
				column = 2;
				row = 9;
				break;
			case 103: /* exit lanthanides */
				column = 3;
				row = 6;
				break;
			default:
				column++;
		}
	}

	show_all_children();
}

void Window::new_project() {
	reset_project();


	//prepare filedialog
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
			//reset everything in window!
			reset_project();
			return;
		}
		catch (BAM::Exception &e) {
			Gtk::MessageDialog dialog(*this, "Error reading in "+*it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			dialog.set_secondary_text(e.what());
  			dialog.run();
			//reset everything in window!
			reset_project();
			return;
		}
		//count number of elements -> must be 1!
		if (asr_file->GetNPeaks() > 1) {
			Gtk::MessageDialog dialog(*this, "Error reading in "+*it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			dialog.set_secondary_text(Glib::ustring("BAM::Data::ASR error: only one peak permitted per file!"));
  			dialog.run();
			delete asr_file;
			//reset everything in window!
			reset_project();
			return;
		}
		BAM::Data::ASR asr_data = asr_file->GetData(0);
		int Z = asr_data.GetZ();
		if (buttonMap[Z]->asr_file) {
			Gtk::MessageDialog dialog(*this, "Error reading in "+*it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			dialog.set_secondary_text(Glib::ustring("BAM::Data::ASR error: element already present in map!"));
  			dialog.run();
			delete asr_file;
			//reset everything in window!
			reset_project();
			return;
		}
		buttonMap[Z]->asr_file = asr_file;
		if (asr_data.GetLine() == KA_LINE) {
			buttonMap[Z]->asr_counts_KA = asr_data.GetCounts();
		}
		else if (asr_data.GetLine() == LA_LINE) {
			buttonMap[Z]->asr_counts_LA = asr_data.GetCounts();
		}
		buttonVector.push_back(buttonMap[Z]);

		//change color of element
		//buttonMap[Z]->override_background_color(Gdk::RGBA("Chartreuse"));
		Glib::RefPtr<Gtk::StyleContext> csscontext = buttonMap[Z]->get_style_context();
		csscontext->add_provider(cssprovider, 600);
			
		/*if (it == filenames.begin())
			refButton = buttonMap[Z];*/


	}

	//ask to load an XMI-MSIM input-file
	dialog = new Gtk::FileChooserDialog(*this, "Please select a XMI-MSIM input-file", Gtk::FILE_CHOOSER_ACTION_OPEN);
	Glib::RefPtr<Gtk::FileFilter> filter_xmsi = Gtk::FileFilter::create();
	filter_xmsi->set_name("XMSI files");
	filter_xmsi->add_pattern("*.xmsi");
	filter_xmsi->add_pattern("*.XMSI");
	dialog->add_filter(filter_xmsi);
	dialog->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog->add_button("Select", Gtk::RESPONSE_OK);
	
	result = dialog->run();
	string filename;
	switch(result) {
		case(Gtk::RESPONSE_OK):
			filename = dialog->get_filename();
			std::cout << "Open clicked: " << filename << std::endl;
      			break;
		case(Gtk::RESPONSE_CANCEL):
		default:
			delete dialog;
			return;
	}

	delete dialog;

	BAM::File::XMSI *xmsi_file;

	try {
		xmsi_file = new BAM::File::XMSI(filename);
	}
	catch (BAM::Exception &e) {
		Gtk::MessageDialog dialog(*this, "Error reading in "+filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
		Glib::ustring error_message = Glib::ustring(e.what());
		std::cout << e.what() << std::endl;
  		dialog.set_secondary_text(error_message);
  		dialog.run();
		//reset everything in window!
		reset_project();
		return;
	
	}

	//for all elements -> establish XMSI files with the correct composition
	for (int i = 1 ; i <= 94 ; i++) {
		buttonMap[i]->temp_xmsi_filename = Glib::build_filename(Glib::get_tmp_dir(), "bam-quant-" + Glib::get_user_name() + "-" + static_cast<ostringstream*>( &(ostringstream() << getpid()))->str() + buttonMap[i]->GetElement()+ ".xmsi");
		buttonMap[i]->xmsi_file = new BAM::File::XMSI(*xmsi_file);
		
		//now change its composition
		BAM::Data::XMSI::Composition composition;

		BAM::Data::XMSI::Layer layer1("Air, Dry (near sea level)", 5.0);
		composition.AddLayer(layer1);

		BAM::Data::XMSI::Layer layer2(ElementDensity(buttonMap[i]->GetZ()), 5.0);
		layer2.AddElement(buttonMap[i]->GetZ(), 1.0);
		composition.AddLayer(layer2);

		composition.SetReferenceLayer(2);
		buttonMap[i]->xmsi_file->ReplaceComposition(composition);
		string temp_xmso_filename = buttonMap[i]->temp_xmsi_filename;
		temp_xmso_filename.replace(temp_xmso_filename.end()-1,temp_xmso_filename.end(), "o");
		buttonMap[i]->xmsi_file->SetOutputFile(temp_xmso_filename);
		buttonMap[i]->xmsi_file->SetFilename(buttonMap[i]->temp_xmsi_filename);


			
	}
	delete xmsi_file;

	//now launch the XMI-MSIM dialog
	xmi_msim_dialog = new XmiMsimDialog(*this, true, buttonVector);
	try {
		result = xmi_msim_dialog->run();
	}
	catch (BAM::Exception &e) {
		delete xmi_msim_dialog;
		Gtk::MessageDialog dialog(*this, "Error occurred in XMI-MSIM dialog", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
		Glib::ustring error_message = Glib::ustring(e.what());
		std::cout << e.what() << std::endl;
  		dialog.set_secondary_text(error_message);
  		dialog.run();
		//reset everything in window!
		xmi_msim_dialog = 0;
		reset_project();
		return;
	}
	if (result == Gtk::RESPONSE_DELETE_EVENT) {
		//delete event caught
		delete xmi_msim_dialog;
		xmi_msim_dialog = 0;
		reset_project();
		return;
	}
	else {
		//check if simulation was not stopped
		for (std::vector<MendeleevButton *>::iterator it = buttonVector.begin() ; it != buttonVector.end(); ++it) {
			if ((*it)->asr_file && !(*it)->xmso_file) {
				delete xmi_msim_dialog;
				reset_project();
				return;
			}
		}
	}
	delete xmi_msim_dialog;

	update_phis();

	//update menu
	//settings_action->set_enabled();
	
}

void Window::update_phis() {
	//let's calculate the normalization factor
	for (std::vector<MendeleevButton *>::iterator it = buttonVector.begin() ; it != buttonVector.end(); ++it) {
		cout << "asr_counts_KA: " << (*it)->asr_counts_KA << endl;
		cout << "asr_counts_LA: " << (*it)->asr_counts_LA << endl;
		cout << "xmso_counts_KA: " << (*it)->xmso_counts_KA << endl;
		cout << "xmso_counts_LA: " << (*it)->xmso_counts_LA << endl;
		if ((*it)->asr_counts_KA > 0 && (*it)->xmso_counts_KA > 0) {
			(*it)->phi = (*it)->asr_counts_KA * (*it)->asr_file->GetNormfactor() / ASR_SCALE_FACTOR / (*it)->xmso_counts_KA;
		}
		else if((*it)->asr_counts_LA > 0 && (*it)->xmso_counts_LA > 0) {
			(*it)->phi = (*it)->asr_counts_LA * (*it)->asr_file->GetNormfactor() / ASR_SCALE_FACTOR / (*it)->xmso_counts_LA;
		}
		else {
			Gtk::MessageDialog dialog(*this, string("Error detected while calculating phi for ")+(*it)->GetElement(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			dialog.run();
			dialog.hide();
			//reset everything in window!
			reset_project();
			return;
		}
		phi += (*it)->phi;
		cout << "Phi for element " << (*it)->GetElement() << ": " << (*it)->phi << endl;
	}
	phi /= buttonVector.size();
	cout << "Average phi: " << phi << endl;

}


void Window::reset_project() {
	for (std::map<int, MendeleevButton*>::iterator it = buttonMap.begin(); it != buttonMap.end(); ++it) {
		(it->second)->reset_button();
	}
	phi = 0;
	//settings_action->set_enabled(false);
	buttonVector.clear();
}

/*
void Window::settings() {
	cout << "Settings activated" << endl;

	Gtk::Dialog dialog("Settings panel", *this, true);
	dialog.add_button("Apply", Gtk::RESPONSE_OK);

	Gtk::Box hbox(Gtk::ORIENTATION_HORIZONTAL);
	Gtk::Label label("Reference element");
	hbox.pack_start(label, false, false, 3);
	Gtk::ComboBoxText combo;
	
	int counter = 0;
	for (int Z = 1 ; Z <= 94 ; Z++) {
		if (buttonMap[Z]->asr_file) {
			combo.append(buttonMap[Z]->GetElement());
			if (buttonMap[Z] == refButton) {
				combo.set_active(counter);
			}
			counter++;
		}
	}
	hbox.pack_end(combo, false, false, 3);

	dialog.get_content_area()->pack_start(hbox, true, true, 3);
	dialog.show_all_children();

	int result = dialog.run();
	if (result == Gtk::RESPONSE_OK) {
		//update refButton
		Glib::ustring selected = combo.get_active_text();
		for (int Z = 1 ; Z <= 94 ; Z++) {
			if (buttonMap[Z]->asr_file && buttonMap[Z]->GetElement() == selected) {
				refButton = buttonMap[Z];
				break;
			}
		}
	}
	dialog.hide();
	update_phis();
}*/
