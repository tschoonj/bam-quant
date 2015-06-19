#include "app2-assistant.h"
#include <gtkmm/messagedialog.h>


App2::Assistant::Assistant() :  intro_page("Welcome!\n\nIn this wizard you will produce a file containing\n"
							"relative X-ray intensities based on the net-line intensities\n"
							"obtained from pure elemental standards and samples."),
				energies_grid(this),
				samples_summary_grid(this),
				simulate_grid(this),
				output_file_grid(this),
				confirmation_label(this)
				{

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

	append_page(samples_summary_grid);
	set_page_type(samples_summary_grid, Gtk::ASSISTANT_PAGE_CONTENT);
	set_page_title(samples_summary_grid, "Samples summary");
	set_page_complete(samples_summary_grid, false);

	append_page(simulate_grid);
	set_page_type(simulate_grid, Gtk::ASSISTANT_PAGE_CONTENT);
	set_page_title(simulate_grid, "Simulate missing elements");
	set_page_complete(simulate_grid, false);

	append_page(output_file_grid);
	set_page_type(output_file_grid, Gtk::ASSISTANT_PAGE_CONTENT);
	set_page_title(output_file_grid, "Save the results");
	set_page_complete(output_file_grid, false);


	append_page(confirmation_label);
	set_page_type(confirmation_label, Gtk::ASSISTANT_PAGE_CONFIRM);
	set_page_title(confirmation_label, "The end...");
	set_page_complete(confirmation_label, true);

	//these two signals do not need to be connected since I have implemented my own methods of these virtual functions
	//signal_delete_event().connect(sigc::mem_fun(*this, &App2::Assistant::on_delete_event));
	//signal_prepare().connect(sigc::mem_fun(*this, &App2::Assistant::on_prepare));

	show_all_children();
}

void App2::Assistant::on_prepare(Gtk::Widget *page) {
	//these functions may throw exceptions
	try {
		if (page == &samples_summary_grid)
			samples_summary_grid.Prepare();	
		else if (page == &simulate_grid)
			simulate_grid.Prepare();	
		else if (page == &output_file_grid)
			commit();
	}
	catch (BAM::Exception &e) {
		Gtk::MessageDialog dialog(*this, "BAM::Exception caught!", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  		dialog.set_secondary_text(Glib::ustring("error message: ")+e.what());
  		dialog.run();
		get_application()->remove_window(*this);
	}
}
