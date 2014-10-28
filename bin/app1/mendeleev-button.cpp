#include "mendeleev-button.h"
#include <glibmm/ustring.h>
#include <iostream>
#include <gtkmm/cssprovider.h>
#include <xraylib.h>
#include <window.h>

void MendeleevButton::on_button_clicked() {
	std::cout << "Button with element: " << element << " clicked" << std::endl;
	//trial code to reset color
	/*Glib::RefPtr<Gtk::StyleContext> csscontext = get_style_context();
	Glib::RefPtr<Gtk::CssProvider> cssprovider = (dynamic_cast<Window*>(get_toplevel()))->cssprovider;
	csscontext->remove_provider(cssprovider);*/
	if (popover.get_visible()) {
		popover.hide();
	}
	else {
		if (popover.get_child())
			popover.remove();
		//Gtk::Label *label = Gtk::manage(new Gtk::Label("Label", Gtk::ALIGN_START, Gtk::ALIGN_START));
		//popover.add(*label);
		//label->show();
		Gtk::Box *box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
		Gtk::Label *label = Gtk::manage(new Gtk::Label("", Gtk::ALIGN_START));
		label->set_markup("<big>ASR counts</big>");
		box->pack_start(*label, false, false, 5);
		if (asr_file) {
			BAM::Data::ASR asr_data = asr_file->GetData(0);
			stringstream ss;
			if (asr_data.GetLine() == KA_LINE) {
				ss << "Kα: ";
			}
			else {
				ss << "Lα: ";
			}
			ss << asr_data.GetCounts(); 
			string label_text;
			ss >> label_text;
			label = Gtk::manage(new Gtk::Label(ss.str(), Gtk::ALIGN_START));
		}
		else {
			label = Gtk::manage(new Gtk::Label("No data", Gtk::ALIGN_START));
		}
		box->pack_start(*label, false, false, 5);
		popover.add(*box);
		box->show_all();
		popover.show_all();
	}
}

MendeleevButton::MendeleevButton(int Z) : Z(Z), element(AtomicNumberToSymbol(Z)), popover(*this) {
	set_label(Glib::ustring(element));
	signal_clicked().connect( sigc::mem_fun(*this,
	              &MendeleevButton::on_button_clicked) );
	asr_file = 0;
	xmsi_file = 0;
	xmso_file = 0;
	asr_counts_KA = asr_counts_LA = 0.0;
	xmso_counts_KA = xmso_counts_LA = 0.0;
	//Gtk::Label *label = Gtk::manage(new Gtk::Label("Label", Gtk::ALIGN_START, Gtk::ALIGN_START));
	//popover.add(*label);
	//label->show();
	popover.set_border_width(10);
	//popover.show_all_children();
}

MendeleevButton::~MendeleevButton() {
	xrlFree(element);
	if (asr_file)
		delete asr_file;
	if (xmsi_file)
		delete xmsi_file;
	if (xmso_file)
		delete xmso_file;
}

void MendeleevButton::reset_button() {
	if (asr_file) {
		delete asr_file;
		asr_file = 0;
	}
	if (xmsi_file) {
		delete xmsi_file;
		xmsi_file = 0;
	}
	if (xmso_file) {
		delete xmso_file;
		xmso_file = 0;
	}

	xmso_counts_KA = xmso_counts_LA = 0.0;
	asr_counts_KA = asr_counts_LA = 0.0;

	Glib::RefPtr<Gtk::StyleContext> csscontext = get_style_context();
	Glib::RefPtr<Gtk::CssProvider> cssprovider = (dynamic_cast<Window*>(get_toplevel()))->cssprovider;
	csscontext->remove_provider(cssprovider);
}

