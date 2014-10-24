#ifndef WINDOW_H
#define WINDOW_H
#include <gtkmm/applicationwindow.h>
#include <gtkmm/cssprovider.h>
#include "mendeleev-button.h"
#include <iostream>
#include <gtkmm/grid.h>
#include <gtkmm/box.h>
#include <gtkmm/cssprovider.h>

class Window: public Gtk::ApplicationWindow {

	public:
		Window();
		~Window() {
			for (std::map<int, MendeleevButton*>::iterator it = buttonMap.begin(); it != buttonMap.end(); ++it)
	   			delete it->second;
		}
		Glib::RefPtr<Gtk::CssProvider> cssprovider;

	private:
		//MendeleevButton test_button;
		Gtk::Box big_box;
		Gtk::Grid buttonGrid;
		std::map<int, MendeleevButton*> buttonMap;
		void new_project();
		void reset_project();

};

#endif
