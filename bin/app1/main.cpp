#include <gtkmm.h>
#include "application.h"



int main(int argc, char **argv) {
	//auto application = Gtk::Application::create(argc, argv, "de.bam.quant.app1", Gio::APPLICATION_FLAGS_NONE);
	//App1Window app1window;
	return Application::create()->run(argc, argv);
}
