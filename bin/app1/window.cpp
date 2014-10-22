#include <window.h>
#include <gtkmm/filechooser.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stylecontext.h>


Window::Window()  {
	//menu signals
	add_action("new", sigc::mem_fun(*this, &Window::new_project));

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
	add(buttonGrid);

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
	Gtk::FileChooserDialog dialog(*this, "Please select a number of AXIL results files", Gtk::FILE_CHOOSER_ACTION_OPEN);
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
			delete asr_file;
			//reset everything in window!
			reset_project();
			return;
		}
		catch (BAM::Exception &e) {
			Gtk::MessageDialog dialog(*this, "Error reading in "+*it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			dialog.set_secondary_text(Glib::ustring("Parser error: ")+ e.what());
  			dialog.run();
			delete asr_file;
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
		//change color of element
		//buttonMap[Z]->override_background_color(Gdk::RGBA("Chartreuse"));
		Glib::RefPtr<Gtk::StyleContext> csscontext = buttonMap[Z]->get_style_context();
		csscontext->add_provider(cssprovider, 600);
			

	}

	//ask to load an XMI-MSIM input-file

}

void Window::reset_project() {
	for (std::map<int, MendeleevButton*>::iterator it = buttonMap.begin(); it != buttonMap.end(); ++it) {
		(it->second)->reset_button();
	}
}
