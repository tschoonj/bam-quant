#include "app2-outputfilegrid.h"
#include "app2-assistant.h"
#include <gtkmm/filechooserdialog.h>
#include <string>

App2::OutputFileGrid::OutputFileGrid(App2::Assistant *assistant) : assistant(assistant), label("Select a file that will be created to store\nthe relative X-ray intensities") {

	attach(label, 0, 0, 2, 1);
	set_column_spacing(5);
	set_row_spacing(5);
	set_row_homogeneous(false);
	set_column_homogeneous(false);
	label.set_hexpand();
	label.set_margin_bottom(10);
	label.set_margin_top(10);
	save_button.set_image_from_icon_name("document-save");
	save_button.set_vexpand(false);
	save_button.set_hexpand(false);
	save_button.set_valign(Gtk::ALIGN_CENTER);
	attach(save_button, 0, 1, 1, 1);
	attach(entry, 1, 1, 1, 1);
	entry.set_editable(false);
	entry.set_hexpand();
	save_button.signal_clicked().connect(sigc::mem_fun(*this, &App2::OutputFileGrid::on_save_clicked));

	show_all_children();
}


void App2::OutputFileGrid::on_save_clicked() {
	//fire up a filechooserdialog
	Gtk::FileChooserDialog dialog(*assistant, "Please select a BAM-QUANT relative X-ray intensities file", Gtk::FILE_CHOOSER_ACTION_SAVE);
	Glib::RefPtr<Gtk::FileFilter> filter_bqp1 = Gtk::FileFilter::create();
	filter_bqp1->set_name("BAM-QUANT RXI files");
	filter_bqp1->add_pattern("*.rxi");
	filter_bqp1->add_pattern("*.RXI");
	dialog.add_filter(filter_bqp1);
	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("Select", Gtk::RESPONSE_OK);
	
	int result = dialog.run();
	std::string filename;
	switch(result) {
		case(Gtk::RESPONSE_OK):
			filename = dialog.get_filename();
			if (filename.compare(filename.length()-4, std::string::npos, ".rxi") != 0)
				filename += ".rxi";
			std::cout << "Open clicked: " << filename << std::endl;
      			break;
		case(Gtk::RESPONSE_CANCEL):
		default:
			return;
	}
	assistant->set_page_complete(*this, true);	
	entry.set_text(filename);
	return;
}
