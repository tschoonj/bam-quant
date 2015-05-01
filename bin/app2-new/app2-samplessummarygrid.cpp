#include "app2-samplessummarygrid.h"
#include "app2-assistant.h"
#include "app2-matrixdialog.h"
#include <gtkmm/messagedialog.h>
#include <gtkmm/switch.h>
#include <iostream>
#include <algorithm>

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
	density_renderer->set_alignment(0.5, 0.5);

	Gtk::CellRendererText *thickness_renderer = static_cast<Gtk::CellRendererText*> (tv.get_column_cell_renderer(2));
	thickness_renderer->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_edited), false));
	thickness_renderer->set_alignment(0.5, 0.5);

	
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
		matrix_column->set_alignment(0.5);
		cell->property_ellipsize_set() = true;
		cell->property_ellipsize() = Pango::ELLIPSIZE_END;
		cell->set_alignment(0.5, 0.5);
	}
	tv.signal_row_activated().connect(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_row_activated) );	

	sw.add(tv);
	sw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	attach(sw, 0, 1, 1, 1);
	sw.set_vexpand();
	sw.set_hexpand();

	tv.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
	tv.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_selection_changed));

	density_button.signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_open_rho_or_T_clicked), true));
	thickness_button.signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_open_rho_or_T_clicked), false));
	fix_thickness_density_button.signal_clicked().connect(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_open_fix_thickness_density_clicked));
	matrix_button.signal_clicked().connect(sigc::mem_fun(*this, &App2::SamplesSummaryGrid::on_open_matrix_clicked));

	show_all_children();
}

void App2::SamplesSummaryGrid::on_edited(const Glib::ustring & path, const Glib::ustring &new_text, bool is_it_density) {
	//the is_it_density is not used, since append_column_numeric_editable already appears to update the model anyway, but it's a nice reminder on how sigc::bind works
	if (model->children().size() >= 1) {
		//check density and thickness
		bool zero_found = false;
		Gtk::TreeModel::Children kids = model->children();
		for (Gtk::TreeModel::Children::iterator iter = kids.begin() ; iter != kids.end() ; ++iter) {
			Gtk::TreeModel::Row row = *iter;
			if (row[columns.col_density] <= 0.0 || 
			    row[columns.col_thickness] <= 0.0) {
				zero_found = true;
				break;
			}
		}
		if (zero_found)
			assistant->set_page_complete(*this, false);	
		else
			assistant->set_page_complete(*this, true);	
	}
	else {
		assistant->set_page_complete(*this, false);	
	}
}

void App2::SamplesSummaryGrid::on_fix_thickness_density_toggled(const Glib::ustring &path) {
	//get current status and invert it
	Gtk::TreeModel::Row row = *(model->get_iter(path));
	row[columns.col_fix_thickness_density] = !row[columns.col_fix_thickness_density];
}

void App2::SamplesSummaryGrid::on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column) {

	if (column != matrix_column) {
		return;
	}
	
	std::vector<int> elements_int;

	for (std::vector<SamplesGrid*>::iterator iter = assistant->samples_grid_vec.begin() ; iter != assistant->samples_grid_vec.end() ; ++iter) {
		Gtk::TreeModel::Row row = *((*iter)->model->get_iter(path));
		std::vector<int> elements_int_local = row[(*iter)->columns.col_elements_int];
		elements_int.insert(elements_int.end(), elements_int_local.begin(), elements_int_local.end());
	}
	std::sort(elements_int.begin(), elements_int.end() );
	elements_int.erase(std::unique(elements_int.begin(), elements_int.end()), elements_int.end());


	App2::MatrixDialog matrix_dialog("Set the matrix", *assistant, true, elements_int);
	int result = matrix_dialog.run();

	if (result == Gtk::RESPONSE_OK) {
		Gtk::TreeModel::Row row = *(model->get_iter(path));
		row[columns.col_matrix] = matrix_dialog.GetCompound();
	}
}

void App2::SamplesSummaryGrid::on_selection_changed() {
	if (model->children().size() >= 1) {
		density_button.set_sensitive();
		thickness_button.set_sensitive();
		fix_thickness_density_button.set_sensitive();
		matrix_button.set_sensitive();
	}
	else {
		density_button.set_sensitive(false);
		thickness_button.set_sensitive(false);
		fix_thickness_density_button.set_sensitive(false);
		matrix_button.set_sensitive(false);
	}
}

void App2::SamplesSummaryGrid::on_open_rho_or_T_clicked(bool is_it_density) {
	//prepare a dialog with a label and an entry
	Gtk::Dialog dialog(is_it_density ? "Set the density" : "Set the thickness", *assistant, true);
	dialog.add_button("Ok", Gtk::RESPONSE_OK);
	dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
	Gtk::Grid grid;
	Gtk::Label label;
	if (is_it_density)
		label.set_markup("Density (g/cm<sup>3</sup>)");
	else
		label.set_text("Thickness (cm)");
	grid.attach(label, 0, 0, 1, 1);

	Gtk::Entry entry;
	grid.attach(entry, 1, 0, 1, 1);
	grid.set_column_spacing(5);
	grid.set_border_width(5);
	entry.set_hexpand();
	grid.show_all();
	dialog.get_content_area()->pack_start(grid, true, false, 0);

	if (dialog.run() != Gtk::RESPONSE_OK) {
		return;
	}

	//now check if the value was acceptable
	std::istringstream ss(entry.get_text());
	double value;
	ss >> value;
	if (ss.fail() || value <= 0.0) {
		Gtk::MessageDialog mdialog(dialog, "Only positive numbers are accepted!");
		mdialog.run();
		return;
	}

	//assign value	
	Glib::RefPtr<Gtk::TreeSelection> selection = tv.get_selection();
	std::vector<Gtk::TreeModel::Path> paths = selection->get_selected_rows();
	for (std::vector<Gtk::TreeModel::Path>::iterator rit = paths.begin() ; rit != paths.end() ; ++rit) {
		Gtk::TreeModel::Row row = *(model->get_iter(*rit));
		if (is_it_density)
			row[columns.col_density] = value;
		else
			row[columns.col_thickness] = value;
	}

	//check density and thickness
	bool zero_found = false;
	Gtk::TreeModel::Children kids = model->children();
	for (Gtk::TreeModel::Children::iterator iter = kids.begin() ; iter != kids.end() ; ++iter) {
		Gtk::TreeModel::Row row = *iter;
		if (row[columns.col_density] <= 0.0 || 
		    row[columns.col_thickness] <= 0.0) {
			zero_found = true;
			break;
		}
	}
	if (zero_found)
		assistant->set_page_complete(*this, false);	
	else
		assistant->set_page_complete(*this, true);	

}

void App2::SamplesSummaryGrid::on_open_fix_thickness_density_clicked() {
	//prepare a dialog with a label and an entry
	Gtk::Dialog dialog("Fix the density and the thickness", *assistant, true);
	dialog.add_button("Ok", Gtk::RESPONSE_OK);
	dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
	Gtk::Grid grid;
	Gtk::Label label;
	label.set_text("Fix?");
	grid.attach(label, 0, 0, 1, 1);

	Gtk::Switch my_switch;
	grid.attach(my_switch, 1, 0, 1, 1);
	grid.set_column_spacing(5);
	grid.set_border_width(5);
	my_switch.set_hexpand();
	my_switch.set_active();
	grid.show_all();
	dialog.get_content_area()->pack_start(grid, true, false, 0);

	if (dialog.run() != Gtk::RESPONSE_OK) {
		return;
	}
	
	//assign value	
	Glib::RefPtr<Gtk::TreeSelection> selection = tv.get_selection();
	std::vector<Gtk::TreeModel::Path> paths = selection->get_selected_rows();
	for (std::vector<Gtk::TreeModel::Path>::iterator rit = paths.begin() ; rit != paths.end() ; ++rit) {
		Gtk::TreeModel::Row row = *(model->get_iter(*rit));
		row[columns.col_fix_thickness_density] = my_switch.get_active();
	}

}

void App2::SamplesSummaryGrid::on_open_matrix_clicked() {
	//prepare vector with all elements
	std::vector<int> elements_all;
	Glib::RefPtr<Gtk::TreeSelection> selection = tv.get_selection();
	std::vector<Gtk::TreeModel::Path> paths = selection->get_selected_rows();

	for (std::vector<Gtk::TreeModel::Path>::iterator rit = paths.begin() ; rit != paths.end() ; ++rit) {
		for (std::vector<SamplesGrid*>::iterator iter = assistant->samples_grid_vec.begin() ; iter != assistant->samples_grid_vec.end() ; ++iter) {
			Gtk::TreeModel::Row row = *((*iter)->model->get_iter(*rit));
			std::vector<int> elements_int_local = row[(*iter)->columns.col_elements_int];
			elements_all.insert(elements_all.end(), elements_all.begin(), elements_all.end());
		}
	}

	//get uniques and sort
	std::sort(elements_all.begin(), elements_all.end() );
	elements_all.erase(std::unique(elements_all.begin(), elements_all.end()), elements_all.end());

	App2::MatrixDialog matrix_dialog("Set the matrix", *assistant, true, elements_all);
	int result = matrix_dialog.run();

	if (result == Gtk::RESPONSE_OK) {
		for (std::vector<Gtk::TreeModel::Path>::iterator rit = paths.begin() ; rit != paths.end() ; ++rit) {
			Gtk::TreeModel::Row row = *(model->get_iter(*rit));
			row[columns.col_matrix] = matrix_dialog.GetCompound();
		}
	}
}

void App2::SamplesSummaryGrid::Prepare() {
	std::cout << "Entering App2::SamplesSummaryGrid::Prepare" << std::endl;
	/* check if there is already data in the model
	 *	-> if true
	 *		-> check if number of rows still corresponds to number of samples
	 *			-> if true
	 *				-> check if all parameters are valid
	 *					-> if true
	 *						-> page complete
	 *					-> else
	 *						-> page not complete
	 *			-> else
	 *				-> clear model
	 *				-> page not complete
	 *				-> error message popup
	 *	-> else
	 *		-> check if number of samples is the same for each energy
	 *			-> if true
	 *				-> fill up the model
	 *				-> page not complete
	 *			-> else
	 *				-> page not complete
	 *				-> error message popup
	 */
	unsigned int model_current_size = model->children().size();

	if (model_current_size > 0) {
		bool no_match_found = false;
		for (std::vector<SamplesGrid*>::iterator iter = assistant->samples_grid_vec.begin() ;
		     iter != assistant->samples_grid_vec.end() ; 
		     ++iter) {
			if ((*iter)->model->children().size() != model_current_size) {
				no_match_found = true;
				break;
			}
		}
		if (!no_match_found) {
			bool zero_found = false;
			Gtk::TreeModel::Children kids = model->children();
			for (Gtk::TreeModel::Children::iterator iter = kids.begin() ; iter != kids.end() ; ++iter) {
				Gtk::TreeModel::Row row = *iter;
				if (row[columns.col_density] <= 0.0 || 
			    	    row[columns.col_thickness] <= 0.0) {
					zero_found = true;
					break;
				}
			}
			if (zero_found)
				assistant->set_page_complete(*this, false);	
			else
				assistant->set_page_complete(*this, true);	

		}
		else {
			model->clear();
			assistant->set_page_complete(*this, false);	
			Gtk::MessageDialog mdialog(*assistant, "Number of samples must be the same for each energy!", false, Gtk::MESSAGE_ERROR);
			mdialog.run();
		}
	}
	else {
		bool no_match_found = false;
		unsigned int size;
		for (std::vector<SamplesGrid*>::iterator iter = assistant->samples_grid_vec.begin() ;
		     iter != assistant->samples_grid_vec.end() ; 
		     ++iter) {
			if (iter == assistant->samples_grid_vec.begin()) {
				size = (*iter)->model->children().size();
				continue;
			}
			if ((*iter)->model->children().size() != size) {
				no_match_found = true;
				break;
			}
		}
		if (!no_match_found) {
			for (unsigned int i = 0 ; i < size ; i++) {
				Gtk::TreeModel::Row row = *(model->append());
				row[columns.col_index] = i;
				row[columns.col_density] = 0.0;
				row[columns.col_thickness] = 0.0;
				row[columns.col_fix_thickness_density] = true;
				row[columns.col_matrix] = "none";
			}
		}
		else {
			Gtk::MessageDialog mdialog(*assistant, "Number of samples must be the same for each energy!", false, Gtk::MESSAGE_ERROR);
			mdialog.run();
		}
		assistant->set_page_complete(*this, false);	
	}
}
