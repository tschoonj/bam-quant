#include <gtkmm/applicationwindow.h>
#include <gtkmm/label.h>
#include "mendeleev-button.h"
#include <iostream>

class Window: public Gtk::ApplicationWindow {

	public:
		Window() : test_label("This is a test") {
			set_title("This is a test indeed");
			set_default_size(400, 200);
			set_border_width(10);
			add(test_label);
			std::cout << "Before show_all_children" << std::endl;
			show_all_children();
		};

	private:
		Gtk::Label test_label;

};
