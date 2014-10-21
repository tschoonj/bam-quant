#ifndef MENDELEEV_BUTTON_H
#define MENDELEEV_BUTTON_H

#include <gtkmm/button.h>
#include <xraylib.h>
#include <glibmm/ustring.h>
#include <iostream>

class MendeleevButton : public Gtk::Button {
	private:
		int Z;
		char *element;
		void on_button_clicked() {
			std::cout << "Button with element: " << element << "clicked" << std::endl;
		}
	public:
		MendeleevButton(int Z) : Z(Z), element(AtomicNumberToSymbol(Z)) {
			set_label(Glib::ustring(element));
			signal_clicked().connect( sigc::mem_fun(*this,
			              &MendeleevButton::on_button_clicked) );
		}
		~MendeleevButton() {
			xrlFree(element);
		}

};



#endif
