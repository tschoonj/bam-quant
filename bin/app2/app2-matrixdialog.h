#ifndef APP2_MATRIXDIALOG_H
#define APP2_MATRIXDIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/grid.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/entry.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/separator.h>
#include <gtkmm/label.h>
#include <vector>


class App2MatrixDialog : public Gtk::Dialog {
private:
	//member variables
	Gtk::Grid grid;
	Gtk::ComboBoxText nist_compound_combobox;
	Gtk::Entry chemical_compound_entry;
	Gtk::RadioButton chemical_compound_radio;
	Gtk::RadioButton nist_compound_radio;
	Gtk::RadioButton no_compound_radio;
	std::vector<int> sample_elements;
	Gtk::Label status_label;

	//signals
	void on_radio_toggled(Gtk::RadioButton* radio);
	void on_radio_toggled_main();
	void on_combobox_changed();
	void on_entry_changed();

	//where the dialog is built
	void BuildDialog();
	void CheckCompound(std::string compound);

public:
	Glib::ustring GetCompound() {
		//this function should only be called if the Ok button was clicked
		Glib::ustring matrix;
		if (chemical_compound_radio.get_active())
			matrix = chemical_compound_entry.get_text();	
		else if (nist_compound_radio.get_active())
			matrix = nist_compound_combobox.get_active_text();	
		else
			matrix = "none";

		return matrix;
	}
	App2MatrixDialog(const Glib::ustring& title, Gtk::Window& parent, bool modal=false) :
		Gtk::Dialog(title, parent, modal) {
		BuildDialog();
	} 
	App2MatrixDialog(const Glib::ustring& title, Gtk::Window& parent, bool modal, std::vector<int> sample_elements) :
		Gtk::Dialog(title, parent, modal),
		sample_elements(sample_elements) {
		BuildDialog();
	} 


};


#endif
