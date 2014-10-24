#include <gtkmm/application.h>
#include <glibmm/miscutils.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "application.h"



int main(int argc, char **argv) {
	//auto application = Gtk::Application::create(argc, argv, "de.bam.quant.app1", Gio::APPLICATION_FLAGS_NONE);
	//App1Window app1window;
	Glib::set_application_name("app1");
#if defined(G_OS_WIN32)
        setlocale(LC_ALL,"English_United States");
        //g_setenv("LANG","en_US",TRUE);
#else
        Glib::setenv("LANG","en_US",TRUE);
#endif
        gtk_disable_setlocale();
        setbuf(stdout,NULL);
	return Application::create()->run(argc, argv);
}
