#include "config.h"
#include "application.h"
#include <gtkmm/builder.h>
#include <giomm/menu.h>

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

	//menu bar -> with actual coding!
	Glib::RefPtr<Gio::Menu> win_menu = Gio::Menu::create();
	Glib::RefPtr<Gio::Menu> submenu_file = Gio::Menu::create();
	Glib::RefPtr<Gio::MenuItem> item = Gio::MenuItem::create("_New project", "win.new");
	item->set_attribute_value("accel", Glib::Variant<Glib::ustring>::create("<Primary>n"));
	submenu_file->append_item(item);
	win_menu->append_submenu("File", submenu_file);

	/*Glib::RefPtr<Gio::Menu> submenu_options = Gio::Menu::create();
	item = Gio::MenuItem::create("Settings", "win.settings");
	submenu_options->append_item(item);
	win_menu->append_submenu("Options", submenu_options);*/
	

	set_menubar(win_menu);

	//construct window here
	window = new Window();
	add_window(*window);
	window->show();

	//construct about dialog
	about_dialog.set_transient_for(*window);
    	about_dialog.set_program_name(PACKAGE_NAME);
   	about_dialog.set_version(PACKAGE_VERSION);
    	about_dialog.set_website(PACKAGE_URL);
    	about_dialog.set_copyright("Copyright © 2014 Tom Schoonjans");
    	about_dialog.set_license_type(Gtk::LICENSE_GPL_3_0);
    	about_dialog.set_modal();
    	// Destroy the dialog when the close button is clicked.
    	about_dialog.signal_response().connect(sigc::mem_fun(*this, &Application::on_about_dialog_response));
}

void Application::on_about_dialog_response(int response_id) {
	/*std::cout << response_id
	<< ", close=" << Gtk::RESPONSE_CLOSE
	<< ", cancel=" << Gtk::RESPONSE_CANCEL
	<< ", delete_event=" << Gtk::RESPONSE_DELETE_EVENT
	<< std::endl;*/

	//if((response_id == Gtk::RESPONSE_CLOSE) ||
	//(response_id == Gtk::RESPONSE_CANCEL) ) {
		//std::cout << "Hiding dialog" << std::endl;
		about_dialog.hide();
	//}
}

void Application::show_about_dialog() {
	about_dialog.show();

	//Bring it to the front, in case it was already shown:
	about_dialog.present();
}

