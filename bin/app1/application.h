#ifndef APPLICATION_H
#define APPLICATION_H

#include <gtkmm/application.h>
#include <gtkmm/aboutdialog.h>
#include "window.h"

class Application : public Gtk::Application {
	public:
		virtual ~Application();
		static Glib::RefPtr<Application> create();
	protected:
		Application();
		virtual void on_startup();
	private:
		void load_ui();
		void show_about_dialog();
		void hide_all_windows();
		void about_clicked(int buttonid);

		Window *window;
		Gtk::AboutDialog *about_dialog;
};


#endif
