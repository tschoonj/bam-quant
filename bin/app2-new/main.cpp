#include <gtkmm/application.h>
#include <glibmm/miscutils.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "app2-assistant.h"
#include <xmi_msim.h>



int main(int argc, char **argv) {
	Glib::set_application_name("app2");
#if defined(G_OS_WIN32)
        setlocale(LC_ALL,"English_United States");
        //g_setenv("LANG","en_US",TRUE);
#else
        Glib::setenv("LANG","en_US",TRUE);
#endif
        gtk_disable_setlocale();
        setbuf(stdout,NULL);
	Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "de.bam.app2-new");

	App2::Assistant window;

	if (xmi_xmlLoadCatalog() == 0) {
		return 1;
	}

	return app->run(window);
}
