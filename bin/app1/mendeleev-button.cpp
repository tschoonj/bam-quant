#include "mendeleev-button.h"
#include <glibmm/ustring.h>
#include <iostream>
#include <gtkmm/cssprovider.h>
#include <xraylib.h>
#include <window.h>

using namespace std;

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
		if ((dynamic_cast<Window*>(get_toplevel()))->buttonVectorASR.size()) {
			Gtk::Label *label = Gtk::manage(new Gtk::Label("", Gtk::ALIGN_START));
			label->set_markup("<b><big>ASR counts</big></b>");
			box->pack_start(*label, false, false, 5);
			if (asr_file) {
				stringstream ss;
				if (asr_counts_KA > 0) {
					ss << "Kα: ";
					ss << asr_counts_KA * asr_file->GetNormfactor() / ASR_SCALE_FACTOR;
				}
				else {
					ss << "Lα: ";
					ss << asr_counts_LA * asr_file->GetNormfactor() / ASR_SCALE_FACTOR;
				}
				string label_text;
				ss >> label_text;
				label = Gtk::manage(new Gtk::Label(ss.str(), Gtk::ALIGN_START));
			}
			else {
				label = Gtk::manage(new Gtk::Label("Not available", Gtk::ALIGN_START));
			}
			box->pack_start(*label, false, false, 5);

			label = Gtk::manage(new Gtk::Label("", Gtk::ALIGN_START));
			label->set_markup("<b><big>XMSO counts</big></b>");
			box->pack_start(*label, false, false, 5);
			if (xmso_file) {
				stringstream ss;
				if (xmso_counts_KA > 0) {
					ss << "Kα: ";
					ss << xmso_counts_KA * (dynamic_cast<Window*>(get_toplevel()))->GetPhi();
				}
				else {
					ss << "Lα: ";
					ss << xmso_counts_LA * (dynamic_cast<Window*>(get_toplevel()))->GetPhi();
				}
				string label_text;
				ss >> label_text;
				label = Gtk::manage(new Gtk::Label(ss.str(), Gtk::ALIGN_START));
				box->pack_start(*label, false, false, 5);
			}
			else {
				//label = Gtk::manage(new Gtk::Label("Not available", Gtk::ALIGN_START));
				//add button
				bool match = false;
				for (std::vector<MendeleevButton*>::iterator it = (dynamic_cast<Window*>(get_toplevel()))->buttonVectorXMSO.begin() ; it != (dynamic_cast<Window*>(get_toplevel()))->buttonVectorXMSO.end() ; ++it) {
					if (*it == this) {
						match = true;
						break;
					}	
				}
				if (match) {
					label = Gtk::manage(new Gtk::Label("", Gtk::ALIGN_START));
					label->set_markup("Already added");
					box->pack_start(*label, false, false, 5);
				
				}
				else {
					Gtk::Button *button = Gtk::manage(new Gtk::Button("Add to list"));
					box->pack_start(*button, false, false, 5);
					button->signal_clicked().connect( sigc::mem_fun(*this, &MendeleevButton::on_add_button_clicked));
				}
			}
		}
		else {
			//minimal info mode
			Gtk::Label *label = Gtk::manage(new Gtk::Label("", Gtk::ALIGN_START));
			label->set_markup("<b><big>No project active</big></b>");
			box->pack_start(*label, false, false, 5);
		}

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
	popover.set_border_width(10);
	cssprovider_red = Gtk::CssProvider::create();
	Glib::ustring cssdata = "GtkButton {color: red}";
	if (not cssprovider_red->load_from_data(cssdata)) {
		std::cerr << "Failed to load css" << std::endl;
	}
	cssprovider_blue = Gtk::CssProvider::create();
	cssdata = "GtkButton {color: blue}";
	if (not cssprovider_blue->load_from_data(cssdata)) {
		std::cerr << "Failed to load css" << std::endl;
	}
	cssprovider_green= Gtk::CssProvider::create();
	cssdata = "GtkButton {color: green}";
	if (not cssprovider_green->load_from_data(cssdata)) {
		std::cerr << "Failed to load css" << std::endl;
	}
	cssprovider_current = Glib::RefPtr<Gtk::CssProvider>();
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

	if (cssprovider_current) {
		Glib::RefPtr<Gtk::StyleContext> csscontext = get_style_context();
		csscontext->remove_provider(cssprovider_current);
	}
	cssprovider_current = Glib::RefPtr<Gtk::CssProvider>();
}

void MendeleevButton::on_add_button_clicked() {
	cout << "Adding element " << element << " to list" << endl;
	(dynamic_cast<Window*>(get_toplevel()))->buttonVectorXMSO.push_back(this);
	popover.hide();
	SetGreen();

	//activate launch simulations button
	(dynamic_cast<Window*>(get_toplevel()))->launch_action->set_enabled(true);

}
