#include "config.h"
#include "application.h"
#include <gtkmm/builder.h>

#include <iostream>


Application::~Application() {
	delete window;
}

Glib::RefPtr<Application> Application::create() {
	return Glib::RefPtr<Application>(new Application());
}

Application::Application() : Gtk::Application("tschoonj.de.bam.quant.app1",
                       	     Gio::APPLICATION_FLAGS_NONE) {}

void Application::on_startup() {
	Gtk::Application::on_startup();
	add_action("about", sigc::mem_fun(*this, &Application::show_about_dialog));
	add_action("quit", sigc::mem_fun(*this, &Application::hide_all_windows));
	try {
		load_ui();
	} catch (const Glib::Error& error) {
		std::cerr << "Error loading UI: " << error.what() << std::endl;
		exit(EXIT_FAILURE);
	}

}

void Application::about_clicked(int buttonid) {
	about_dialog->hide();
}

void Application::show_about_dialog() {
  if (!about_dialog) {
    about_dialog = new Gtk::AboutDialog();
    about_dialog->set_program_name(PACKAGE_NAME);
    about_dialog->set_version(PACKAGE_VERSION);
    about_dialog->set_website(PACKAGE_URL);
    about_dialog->set_copyright("Copyright © 2014 Tom Schoonjans");
    about_dialog->set_license_type(Gtk::LICENSE_GPL_3_0);
    about_dialog->set_modal();
    // Destroy the dialog when the close button is clicked.
    about_dialog->signal_response().connect(sigc::mem_fun(*this, &Application::about_clicked));
  }
  about_dialog->set_transient_for(*get_active_window());
  about_dialog->present();
}

void Application::hide_all_windows() {
  //for (Gtk::Window* window : get_windows())
  //  window->hide();
  std::vector<Gtk::Window *> windows = get_windows();
  for (std::vector<Gtk::Window *>::iterator it = windows.begin() ; it != windows.end() ; it++)
  	(*it)->hide();
}

void Application::load_ui() {
	Glib::ustring app_menu_string = 
		"<interface>"
		"  <menu id='app-menu'>"
		"    <section>"
		"      <item>"
		"        <attribute name='label' translatable='yes'>_About</attribute>"
		"        <attribute name='action'>app.about</attribute>"
		"      </item>"
		"      <item>"
		"        <attribute name='label' translatable='yes'>_Quit</attribute>"
		"        <attribute name='action'>app.quit</attribute>"
		"        <attribute name='accel'>&lt;Primary&gt;q</attribute>"
		"      </item>"
		"    </section>"
		"  </menu>"
		"</interface>";
	Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create();
	builder->add_from_string(app_menu_string);
	set_app_menu(Glib::RefPtr<Gio::Menu>::cast_dynamic(builder->get_object("app-menu")));

	//construct window here
	window = new Window();
	add_window(*window);
	window->show();
}
