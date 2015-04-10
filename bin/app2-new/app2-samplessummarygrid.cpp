#include "app2-samplessummarygrid.h"
#include "app2-assistant.h"

App2::SamplesSummaryGrid::SamplesSummaryGrid(App2::Assistant *assistant_arg) : 
	assistant(assistant_arg),
	buttons(Gtk::ORIENTATION_HORIZONTAL),
	density_button("Set density"),
	thickness_button("Set thickness"),
	fix_thickness_density_button("Set fixed thickness"),
	matrix_button("Set matrix") {

	Gtk::Label *label = new Gtk::Label("Set the following parameters for each of the selected samples on the previous pages. Remember: this will only work if the Sample pages all have the same amount of selected ASR files!");
	attach(*label, 0, 0, 1, 1);
	set_column_spacing(5);
	set_row_spacing(5);
	set_row_homogeneous(false);
	set_column_homogeneous(false);
	label->set_hexpand();
	label->set_margin_bottom(10);
	label->set_margin_top(10);
	label->set_line_wrap();
	label->set_justify(Gtk::JUSTIFY_LEFT);

	attach(buttons, 0, 2, 1, 1);
	density_button.set_sensitive(false);
	thickness_button.set_sensitive(false);
	fix_thickness_density_button.set_sensitive(false);
	matrix_button.set_sensitive(false);
	buttons.pack_start(density_button);
	buttons.pack_start(thickness_button);
	buttons.pack_start(fix_thickness_density_button);
	buttons.pack_start(matrix_button);
	buttons.set_layout(Gtk::BUTTONBOX_CENTER);
	buttons.set_spacing(10);
	buttons.set_vexpand(false);
	buttons.set_hexpand(false);

	
	model = Gtk::ListStore::create(columns);
	tv.set_model(model);
	tv.append_column("#", columns.col_index);
	tv.append_column_numeric_editable("Density (g/cm<sup>3</sup>)", columns.col_density, "%g");
	tv.append_column_numeric_editable("Thickness (cm)", columns.col_thickness, "%g");

	label = Gtk::manage(new Gtk::Label("Density (g/cm<sup>3</sup>)")); 
	label->set_use_markup();
	label->show();
	tv.get_column(1)->set_widget(*label);
	Gtk::CellRendererText *density_renderer = static_cast<Gtk::CellRendererText*> (tv.get_column_cell_renderer(1)); 
	density_renderer->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_edited), true));

	Gtk::CellRendererText *thickness_renderer = static_cast<Gtk::CellRendererText*> (tv.get_column_cell_renderer(2));
	thickness_renderer->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_edited), false));

	
	{
		Gtk::CellRendererToggle* cell = Gtk::manage(new Gtk::CellRendererToggle);
		int cols_count = tv.append_column("Fixed density and thickness", *cell);
		Gtk::TreeViewColumn* temp_column = tv.get_column(cols_count-1);
		temp_column->add_attribute(cell->property_active(), columns.col_fix_thickness_density);
		cell->signal_toggled().connect(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_fix_thickness_density_toggled));
	}
	
	{
		Gtk::CellRendererText* cell = Gtk::manage(new Gtk::CellRendererText);
		int cols_count = tv.append_column("Matrix", *cell);
		matrix_column = tv.get_column(cols_count-1);
		matrix_column->add_attribute(cell->property_text(), columns.col_matrix);
		cell->property_ellipsize_set() = true;
		cell->property_ellipsize() = Pango::ELLIPSIZE_END;
	}
	tv.signal_row_activated().connect(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_row_activated) );	

	sw.add(tv);
	sw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	attach(sw, 0, 1, 1, 1);
	sw.set_vexpand();
	sw.set_hexpand();

	tv.signal_key_press_event().connect(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_backspace_clicked));
	tv.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
	tv.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_selection_changed));

	density_button.signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_open_rho_or_T_clicked), true));
	thickness_button.signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_open_rho_or_T_clicked), false));
	fix_thickness_density_button.signal_clicked().connect(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_open_fix_thickness_density_clicked));
	matrix_button.signal_clicked().connect(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_open_matrix_clicked));

	show_all_children();
}
