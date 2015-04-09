#include "app2-assistant.h"


App2::Assistant::Assistant() :  intro_page("Welcome!\n\nIn this wizard you will produce a file containing\n"
							"relative X-ray intensities based on the net-line intensities\n"
							"obtained from pure elemental standards and samples."),
				energies_grid(this),
				confirm_page("Just one more thing to do!\n\nClick the apply button and the file will be saved...") {

	//initialize assistant
	set_border_width(12);
	set_default_size(650, 450);

	//populate pages
	append_page(intro_page);
	set_page_type(intro_page, Gtk::ASSISTANT_PAGE_INTRO);
	set_page_title(intro_page, "Introduction...");
	set_page_complete(intro_page, true);

	append_page(energies_grid);
	set_page_type(energies_grid, Gtk::ASSISTANT_PAGE_CONTENT);
	set_page_title(energies_grid, "Select XMI-MSIM input-files");
	set_page_complete(energies_grid, false);

	append_page(confirm_page);
	set_page_type(confirm_page, Gtk::ASSISTANT_PAGE_CONFIRM);
	set_page_title(confirm_page, "The end...");
	set_page_complete(confirm_page, true);

	signal_delete_event().connect(sigc::mem_fun(*this, &App2::Assistant::on_delete_event));

	show_all_children();
}
