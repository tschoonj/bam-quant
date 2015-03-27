#include "app2-matrixdialog.h"
#include "bam_data_xraylib.h"
#include <gtkmm/radiobuttongroup.h>
#include <gtkmm/separator.h>

void App2MatrixDialog::BuildDialog() {
	
	add_button("Ok", Gtk::RESPONSE_OK);
	add_button("Cancel", Gtk::RESPONSE_CANCEL);
	set_response_sensitive(Gtk::RESPONSE_OK, false);

	grid.set_row_homogeneous(false);
	grid.set_row_spacing(5);
	Gtk::Label *label;
	Gtk::RadioButton::Group group;

	label = Gtk::manage(new Gtk::Label("<b>Chemical formula</b>")); 
	label->set_use_markup();
	chemical_compound_radio.add(*label);
	chemical_compound_radio.set_group(group);
	
	grid.attach(chemical_compound_radio, 0, 0, 1, 1);
	grid.attach(chemical_compound_entry, 0, 1, 1, 1);
	grid.attach(*Gtk::manage(new Gtk::Separator()), 0, 2, 1, 1);
	
	label = Gtk::manage(new Gtk::Label("<b>NIST compound</b>")); 
	label->set_use_markup();
	nist_compound_radio.add(*label);
	nist_compound_radio.set_group(group);

	std::vector<std::string> nist_compounds = BAM::Data::Xraylib::CompoundNIST::GetList();
	for (std::vector<std::string>::iterator it = nist_compounds.begin() ; it != nist_compounds.end() ; ++it) {
		nist_compound_combobox.append(*it);
	}
	grid.attach(nist_compound_radio, 0, 3, 1, 1);
	grid.attach(nist_compound_combobox, 0, 4, 1, 1);
	nist_compound_combobox.set_sensitive(false);
	chemical_compound_radio.set_active();
	nist_compound_combobox.set_active(0);

	grid.attach(*Gtk::manage(new Gtk::Separator()), 0, 5, 1, 1);

	label = Gtk::manage(new Gtk::Label("<b>No matrix</b>")); 
	label->set_use_markup();
	no_compound_radio.add(*label);
	no_compound_radio.set_group(group);
	grid.attach(no_compound_radio, 0, 6, 1, 1);

	grid.attach(*Gtk::manage(new Gtk::Separator()), 0, 7, 1, 1);

	status_label.set_use_markup();
	status_label.set_text("Select a valid matrix");
	grid.attach(status_label, 0, 8, 1, 1);

	grid.set_border_width(10);
	grid.show_all();

	//signals
	chemical_compound_radio.signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &App2MatrixDialog::on_radio_toggled), &chemical_compound_radio));
	nist_compound_radio.signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &App2MatrixDialog::on_radio_toggled), &nist_compound_radio));
	no_compound_radio.signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &App2MatrixDialog::on_radio_toggled), &no_compound_radio));
	nist_compound_combobox.signal_changed().connect(sigc::mem_fun(*this, &App2MatrixDialog::on_combobox_changed));
	chemical_compound_entry.signal_changed().connect(sigc::mem_fun(*this, &App2MatrixDialog::on_entry_changed));
	
	
	get_content_area()->pack_start(grid, true, true, 0);
	
}

void App2MatrixDialog::on_radio_toggled(Gtk::RadioButton *radio) {
	if (radio->get_active())
		on_radio_toggled_main();

}

void App2MatrixDialog::on_radio_toggled_main() {
	if (chemical_compound_radio.get_active()) {
		chemical_compound_entry.set_sensitive(true);
		nist_compound_combobox.set_sensitive(false);
		//check if compound is valid
		CheckCompound(chemical_compound_entry.get_text());
	}
	else if (nist_compound_radio.get_active()) {
		chemical_compound_entry.set_sensitive(false);
		nist_compound_combobox.set_sensitive(true);
		//check if compound is valid
		CheckCompound(nist_compound_combobox.get_active_text());
	}
	else {
		chemical_compound_entry.set_sensitive(false);
		nist_compound_combobox.set_sensitive(false);
		status_label.set_markup("<span foreground=\"green\">Matrix Ok!</span>");
		get_widget_for_response(Gtk::RESPONSE_OK)->set_sensitive();
	}

}

void App2MatrixDialog::CheckCompound(std::string compound_string) {
	try {
		BAM::Data::Base::Composition *compound = BAM::Data::Xraylib::Parse(compound_string);
		if (compound->MatchesAnyFrom(sample_elements)) {
			delete compound;
			throw BAM::Exception("match found");
		}
		get_widget_for_response(Gtk::RESPONSE_OK)->set_sensitive();
		delete compound;
		status_label.set_markup("<span foreground=\"green\">Matrix Ok!</span>");
	}
	catch (BAM::Exception &e){
		get_widget_for_response(Gtk::RESPONSE_OK)->set_sensitive(false);
		if (std::string(e.what()) == "match found")
			status_label.set_markup("<span foreground=\"red\">Matrix cannot contain elements from ASR file</span>");
		else 
			status_label.set_markup("<span foreground=\"red\">Invalid chemical formula</span>");
	}
	catch (...){
		get_widget_for_response(Gtk::RESPONSE_OK)->set_sensitive(false);
		std::cerr << "Some weird exception caught in App2MatrixDialog::UpdateOkButton" << std::endl;
		status_label.set_markup("<span foreground=\"red\">Unknown exception caught</span>");
	}

}

void App2MatrixDialog::on_combobox_changed() {
	if (!nist_compound_radio.get_active()) {
		nist_compound_radio.set_active(); //this will call on_radio_toggled
		return;
	}
	CheckCompound(nist_compound_combobox.get_active_text());
}

void App2MatrixDialog::on_entry_changed() {
	if (!chemical_compound_radio.get_active()) {
		chemical_compound_radio.set_active(); //this will call on_radio_toggled
		return;
	}
	CheckCompound(chemical_compound_entry.get_text());
}
