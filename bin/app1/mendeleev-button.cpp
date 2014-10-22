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
}

MendeleevButton::MendeleevButton(int Z) : Z(Z), element(AtomicNumberToSymbol(Z)) {
	set_label(Glib::ustring(element));
	signal_clicked().connect( sigc::mem_fun(*this,
	              &MendeleevButton::on_button_clicked) );
	asr_file = 0;
}

MendeleevButton::~MendeleevButton() {
	xrlFree(element);
	if (asr_file)
		delete asr_file;
}

void MendeleevButton::reset_button() {
	if (asr_file) {
		delete asr_file;
		asr_file = 0;
	}

	Glib::RefPtr<Gtk::StyleContext> csscontext = get_style_context();
	Glib::RefPtr<Gtk::CssProvider> cssprovider = (dynamic_cast<Window*>(get_toplevel()))->cssprovider;
	csscontext->remove_provider(cssprovider);
}

