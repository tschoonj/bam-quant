#include "app2-simulategrid.h"
#include "app2-assistant.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <xraylib.h>


App2::SimulateGrid::SimulateGrid(App2::Assistant *assistant_arg) : assistant(assistant_arg), buttons(Gtk::ORIENTATION_VERTICAL), columns(0), diff_elements(0), union_elements(0) {
	
	pause_button.set_image_from_icon_name("media-playback-pause", Gtk::ICON_SIZE_DIALOG);	
	stop_button.set_image_from_icon_name("media-playback-stop", Gtk::ICON_SIZE_DIALOG);	
	play_button.set_image_from_icon_name("media-playback-start", Gtk::ICON_SIZE_DIALOG);	
	buttons.pack_start(play_button);
	buttons.pack_start(pause_button);
	buttons.pack_start(stop_button);
	buttons.set_layout(Gtk::BUTTONBOX_CENTER);
	buttons.set_spacing(10);
	buttons.set_vexpand(false);
	buttons.set_hexpand(false);

	sw.add(tv);
	sw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	sw.set_vexpand();
	sw.set_hexpand();
	Gtk::Label *label = Gtk::manage(new Gtk::Label("Simulate elements from the list of samples\nthat do not have matching pure standard data."));
	attach(*label, 0, 0, 2, 1);
	set_column_spacing(5);
	set_row_spacing(5);
	set_row_homogeneous(false);
	set_column_homogeneous(false);
	set_vexpand();
	set_hexpand();
	label->set_hexpand();
	label->set_margin_bottom(10);
	label->set_margin_top(10);
	attach(buttons, 0, 1, 1, 1);
	attach(sw, 1, 1, 1, 1);

	//pause_button.signal_clicked().connect(sigc::mem_fun(*this, &App2::SimulateGrid::on_pause_clicked));
	//play_button.signal_clicked().connect(sigc::mem_fun(*this, &App2Assistant::on_play_clicked));
	//stop_button.signal_clicked().connect(sigc::mem_fun(*this, &App2Assistant::on_stop_clicked));

	show_all_children();
}



void App2::SimulateGrid::prepare() {
	//no way back! although I could probably with some work remove this constraint...
	assistant->commit();

	//first thing to do: set up the columns!	
	unsigned int n_energies = assistant->pures_grid_vec.size();
	columns = new Columns(n_energies);
	model = Gtk::ListStore::create(*columns);
	tv.set_model(model);
	tv.append_column("Element", columns->col_element);

	for (unsigned int i = 0 ; i < n_energies ; i++) {
		Gtk::CellRendererToggle* cell = Gtk::manage(new Gtk::CellRendererToggle);
		std::stringstream ss;
		ss << assistant->pures_grid_vec[i]->GetEnergy();
		ss << " keV";
		int cols_count = tv.append_column(ss.str(), *cell);
		Gtk::TreeViewColumn* temp_column = tv.get_column(cols_count-1);
		temp_column->set_expand();
		temp_column->add_attribute(cell->property_active(), columns->col_simulate_active[i]);
		temp_column->add_attribute(cell->property_sensitive(), columns->col_simulate_sensitive[i]);
		cell->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &App2::SimulateGrid::on_simulate_active_toggled), i));
	} 

	//next: throw the data into the model
	//start with the zero-th column: all elements
	elements_int.clear();

	for (unsigned int i = 0 ; i < n_energies ; i++) {
		PuresGrid *pures_grid = assistant->pures_grid_vec[i];
		for (Gtk::TreeModel::Children::iterator iter = pures_grid->model->children().begin() ; 
		     iter != pures_grid->model->children().end() ;
		     ++iter) {
			Gtk::TreeModel::Row row = *iter;
			elements_int.push_back(row[pures_grid->columns.col_atomic_number]);	
		}		
		SamplesGrid *samples_grid = assistant->samples_grid_vec[i];
		for (Gtk::TreeModel::Children::iterator iter = samples_grid->model->children().begin() ; 
		     iter != samples_grid->model->children().end() ;
		     ++iter) {
			Gtk::TreeModel::Row row = *iter;
			std::vector<int> elements_int_local = row[samples_grid->columns.col_elements_int];
			elements_int.insert(elements_int.end(), elements_int_local.begin(), elements_int_local.end());
		}		
	}
	std::sort(elements_int.begin(), elements_int.end() );
	elements_int.erase(std::unique(elements_int.begin(), elements_int.end()), elements_int.end());


	//now the rest: pretty complicated...
	for (unsigned int i = 0 ; i < elements_int.size() ; i++) {
		Gtk::TreeModel::Row row = *(model->append());
		char *element = AtomicNumberToSymbol(elements_int[i]);
		row[columns->col_element] = element;
		xrlFree(element);
		row[columns->col_atomic_number] = elements_int[i];
	}

	unsigned int diff_elements_sum = 0;

	diff_elements = new std::vector<int>[n_energies]; 
	union_elements = new std::vector<int>[n_energies]; 

	for (unsigned int i = 0 ; i < n_energies ; i++) {
		PuresGrid *pures_grid = assistant->pures_grid_vec[i];
		std::vector<int> pure_elements;
		Gtk::TreeModel::Children kids = pures_grid->model->children();
		for (Gtk::TreeModel::Children::iterator iter = kids.begin() ; iter != kids.end() ; ++iter) {
			Gtk::TreeModel::Row row = *iter;
			pure_elements.push_back(row[pures_grid->columns.col_atomic_number]);
		}
		//this next line is probably superfluous
		std::sort(pure_elements.begin(), pure_elements.end());

		SamplesGrid *samples_grid = assistant->samples_grid_vec[i];
		std::vector<int> sample_elements;
		kids = samples_grid->model->children();
		for (Gtk::TreeModel::Children::iterator iter = kids.begin() ; iter != kids.end() ; ++iter) {
			Gtk::TreeModel::Row row = *iter;
			std::vector<int> col_elements_int = row[samples_grid->columns.col_elements_int];
			sample_elements.insert(sample_elements.end(), col_elements_int.begin(), col_elements_int.end());
		}
		std::sort(sample_elements.begin(), sample_elements.end());
		std::vector<int>::iterator it = std::unique(sample_elements.begin(), sample_elements.end());
		sample_elements.resize(it - sample_elements.begin());

		diff_elements[i].resize(pure_elements.size()+sample_elements.size());
	
		it = std::set_difference(sample_elements.begin(), sample_elements.end(), pure_elements.begin(), pure_elements.end(), diff_elements[i].begin());
		diff_elements[i].resize(it - diff_elements[i].begin());

		union_elements[i].resize(pure_elements.size()+sample_elements.size());
		it = std::set_union(sample_elements.begin(), sample_elements.end(), pure_elements.begin(), pure_elements.end(), union_elements[i].begin());
		union_elements[i].resize(it - union_elements[i].begin());

		diff_elements_sum += diff_elements[i].size();

		
		for (Gtk::TreeModel::Children::iterator iter = model->children().begin() ; 
		     iter != model->children().end(); 
		     ++iter) {
			//default is to assume that the element is not used at all
			Gtk::TreeModel::Row row = *iter;
			row[columns->col_status[i]] = Glib::ustring("Not started");
			row[columns->col_progress[i]] = 0;
			row[columns->col_simulate_sensitive[i]] = false;
			row[columns->col_simulate_active[i]] = false;

			//check if element is present in union_elements[i]
			it = std::find(union_elements[i].begin(), union_elements[i].end(), row[columns->col_atomic_number]);
			if (it != union_elements[i].end()) {
				row[columns->col_simulate_active[i]] = true;

				//check if present in pure_elements
				it = std::find(pure_elements.begin(), pure_elements.end(), row[columns->col_atomic_number]);
				if (it != pure_elements.end()) {
					Gtk::TreeModel::Row pure_row = pures_grid->model->children()[it-pure_elements.begin()];
					row[columns->col_simulate_sensitive[i]] = true;
					row[columns->col_bam_file_asr[i]] = BAM::File::ASR(pure_row[pures_grid->columns.col_bam_file_asr]);
				}
			}

		}
	}

	if (diff_elements_sum == 0) {
		//no XMI-MSIM necessary -> check if this is really an ok solution
		play_button.set_sensitive(false);
		pause_button.set_sensitive(false);
		stop_button.set_sensitive(false);
		assistant->set_page_complete(*this, true);	
		return;
	}

}

void App2::SimulateGrid::on_simulate_active_toggled(const Glib::ustring &path, unsigned int energy_index) {
	//get current status and invert it
	Gtk::TreeModel::Row row = *(model->get_iter(path));
	row[columns->col_simulate_active[energy_index]] = !row[columns->col_simulate_active[energy_index]];
}

