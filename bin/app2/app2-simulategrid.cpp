#include "app2-simulategrid.h"
#include "app2-assistant.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <xraylib.h>
#include <glibmm/miscutils.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glibmm/spawn.h>
#include <glibmm/convert.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/cellrenderertoggle.h>
#include <gtkmm/cellrendererprogress.h>


#ifdef G_OS_UNIX
  #include <sys/types.h>
  #include <sys/wait.h>
  #include <signal.h>
  #define real_xmimsim_pid ((int) xmimsim_pid)
#elif defined(G_OS_WIN32)
  #include <windows.h>
  #define real_xmimsim_pid ((int) GetProcessId((HANDLE) xmimsim_pid))
  typedef LONG (NTAPI *pNtSuspendProcess )(IN HANDLE ProcessHandle );
  typedef LONG (NTAPI *pNtResumeProcess )(IN HANDLE ProcessHandle );
  static pNtSuspendProcess NtSuspendProcess = NULL;
  static pNtResumeProcess NtResumeProcess = NULL;
#endif



App2::SimulateGrid::SimulateGrid(App2::Assistant *assistant_arg) : 
	assistant(assistant_arg),
	buttons(Gtk::ORIENTATION_VERTICAL),
	columns(0),
	diff_elements(0),
	union_elements(0),
	xmimsim_paused(false),
	xmimsim_pid(0),
	timer(0) {
	
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

	tv.set_has_tooltip(true);
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

	tv.signal_query_tooltip().connect(sigc::mem_fun(*this, &App2::SimulateGrid::on_query_tooltip));
	pause_button.signal_clicked().connect(sigc::mem_fun(*this, &App2::SimulateGrid::on_pause_clicked));
	play_button.signal_clicked().connect(sigc::mem_fun(*this, &App2::SimulateGrid::on_play_clicked));
	stop_button.signal_clicked().connect(sigc::mem_fun(*this, &App2::SimulateGrid::on_stop_clicked));

	show_all_children();
}



void App2::SimulateGrid::Prepare() {
	//no way back! although I could probably with some work remove this constraint...
	assistant->commit();

	//first thing to do: set up the columns!	
	unsigned int n_energies = assistant->pures_grid_vec.size();
	columns = new Columns(n_energies);
	model = Gtk::ListStore::create(*columns);
	tv.set_model(model);
	tv.append_column("Element", columns->col_element);
	tv.get_column(0)->set_alignment(0.5);
	tv.get_column_cell_renderer(0)->set_alignment(0.5, 0.5);

	for (unsigned int i = 0 ; i < n_energies ; i++) {
		//add first cellrenderer
		Gtk::CellRendererToggle* cell = Gtk::manage(new Gtk::CellRendererToggle);
		std::stringstream ss;
		ss << assistant->pures_grid_vec[i]->GetEnergy();
		ss << " keV";
		int cols_count = tv.append_column(ss.str(), *cell);
		Gtk::TreeViewColumn* temp_column = tv.get_column(cols_count-1);
		temp_column->set_expand();
		temp_column->set_alignment(0.5);
		temp_column->add_attribute(cell->property_active(), columns->col_simulate_active[i]);
		temp_column->add_attribute(cell->property_sensitive(), columns->col_simulate_sensitive[i]);
		cell->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &App2::SimulateGrid::on_simulate_active_toggled), i));
		//add second cellrenderer
		Gtk::CellRendererProgress *cell2 = Gtk::manage(new Gtk::CellRendererProgress);
		cell2->set_visible(false);
		temp_column->pack_end(*cell2);
		temp_column->add_attribute(cell2->property_value(), columns->col_progress[i]);
		temp_column->add_attribute(cell2->property_text(), columns->col_status[i]);
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

			//no point in doing anything for those elements not present in union_elements
			if (it == union_elements[i].end())
				continue;

			row[columns->col_simulate_active[i]] = true;

			if (pures_grid->GetEnergy() > EdgeEnergy(row[columns->col_atomic_number], K_SHELL))
				row[columns->col_linetype[i]] = Glib::ustring("Kα");
			else if (pures_grid->GetEnergy() > EdgeEnergy(row[columns->col_atomic_number], L3_SHELL))
				row[columns->col_linetype[i]] = Glib::ustring("Lα");
			else
				throw BAM::Exception("Excitation energy lower than L3 edge!");

			//check if present in pure_elements
			it = std::find(pure_elements.begin(), pure_elements.end(), row[columns->col_atomic_number]);
			if (it != pure_elements.end()) {
				Gtk::TreeModel::Row pure_row = pures_grid->model->children()[it-pure_elements.begin()];
				row[columns->col_simulate_sensitive[i]] = true;
				row[columns->col_bam_file_asr[i]] = BAM::File::ASR(pure_row[pures_grid->columns.col_bam_file_asr]);
			}
		
			//create xmsi files
			row[columns->col_xmsi_filename[i]] = Glib::build_filename(Glib::get_tmp_dir(), "bam-quant-" + Glib::get_user_name() + "-" + static_cast<std::ostringstream*>( &(std::ostringstream() << getpid()))->str() + "-" + row[columns->col_element] + "-" + static_cast<std::ostringstream*>( &(std::ostringstream() << pures_grid->GetEnergy()))->str() + "keV.xmsi");
			//row[columns->col_xmso_file[i]] = 0;
			row[columns->col_xmso_counts_KA[i]] = 0.0;
			row[columns->col_xmso_counts_LA[i]] = 0.0;
			std::string temp_xmsi_filename(row[columns->col_xmsi_filename[i]]);
		
			Gtk::TreeModel::Row energies_row = assistant->energies_grid.model->children()[i];
			BAM::File::XMSI temp_xmsi_file = energies_row[assistant->energies_grid.columns.col_bam_file_xmsi];

			//now change its composition
			BAM::Data::XMSI::Composition composition;

			BAM::Data::XMSI::Layer layer1("Air, Dry (near sea level)", 5.0);
			composition.AddLayer(layer1);

			BAM::Data::XMSI::Layer layer2(ElementDensity(row[columns->col_atomic_number]), 5.0);
			layer2.AddElement(row[columns->col_atomic_number], 1.0);
			composition.AddLayer(layer2);

			composition.SetReferenceLayer(2);
			temp_xmsi_file.ReplaceComposition(composition);
			std::string temp_xmso_filename = temp_xmsi_filename;
			temp_xmso_filename.replace(temp_xmso_filename.end()-1,temp_xmso_filename.end(), "o");
			temp_xmsi_file.SetOutputFile(temp_xmso_filename);
			temp_xmsi_file.SetFilename(temp_xmsi_filename);
			row[columns->col_xmso_filename[i]] = temp_xmso_filename;
			row[columns->col_xmsi_file[i]] = temp_xmsi_file;
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

bool App2::SimulateGrid::on_query_tooltip(int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip) {
	int nx, ny, cellx, celly;
	Gtk::TreeModel::Path path;
	Gtk::TreeView::Column* column;
	tv.convert_widget_to_bin_window_coords(x, y, nx, ny);
	bool isRow = tv.get_path_at_pos(nx, ny, path, column, cellx, celly);

	if (!isRow) // Cursor is not hovering over a row
		return false;

	tv.set_tooltip_cell(tooltip, &path, column, 0);
	Gtk::TreeModel::Children::iterator  iter = model->get_iter(path);

	//loop over all columns and see if it matches column
	for (unsigned int i = 1 ; i < tv.get_n_columns() ; i++) {
		if (tv.get_column(i) == column) {
			Glib::ustring text = (*iter)[columns->col_linetype[i-1]];
			if (text.length() > 0) {
				tooltip->set_text(text);
				return true;
			}
			else {
				return false;
			}
		}
	}
	return false;
}

void App2::SimulateGrid::on_play_clicked() {

	if (xmimsim_paused) {
		/* Simulations have been paused! */
		int kill_rv;

		play_button.set_sensitive(false);	

		//glibmm doesn't have the continue method so let's use glib for this
		g_timer_continue(timer->gobj());
#ifdef G_OS_UNIX
		kill_rv = kill((pid_t) xmimsim_pid, SIGCONT);
#elif defined(G_OS_WIN32)
		kill_rv = (int) NtResumeProcess((HANDLE) xmimsim_pid);
#else
		#error "Neither G_OS_UNIX nor G_OS_WIN32 is defined!"
#endif

		if (kill_rv == 0) {
			std::stringstream ss;
			ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " was successfully resumed" << std::endl;
			update_console(ss.str(), "pause-continue-stopped");
			xmimsim_paused = false;
			pause_button.set_sensitive(true);
		}
		else {
			//if this happens, we're in serious trouble!
			std::stringstream ss;
			ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " could not be resumed" << std::endl;
			update_console(ss.str(), "pause-continue-stopped");
			play_button.set_sensitive(true);
			xmimsim_pid = 0;
		}
		return;
	}

	//normal case -> check if input is valid
	unsigned int n_energies = assistant->pures_grid_vec.size();
	
	std::stringstream ss;

	for (unsigned int i = 0 ; i < n_energies ; i++) {
		int n_to_be_interpolated = 0;
		int n_used_for_interpolation = 0;
		for (Gtk::TreeModel::Children::iterator iter = model->children().begin() ;
		     iter != model->children().end() ;
		     ++iter) {
			//count how many elements need to be interpolated
			//count how many elements will be used for the interpolation
			Gtk::TreeModel::Row row = *iter;
			if (!row[columns->col_simulate_active[i]])
				continue;
			if (row[columns->col_simulate_sensitive[i]])
				n_used_for_interpolation++;
			else
				n_to_be_interpolated++;
		}
		if (n_to_be_interpolated > 0 && n_used_for_interpolation < 2) 
			ss << tv.get_column(i+1)->get_title() << std::endl;
	}
	if (ss.str().length() > 0) {
		Gtk::MessageDialog dialog(*assistant, "At least two elements have to be selected for the interpolation to work in columns", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  		dialog.set_secondary_text(ss.str());
  		dialog.run();
		return;
	}

	//start the preparations
	xmimsim_paused = false;
	play_button.set_sensitive(false);

	//switch to the second cellrenderer
	for (unsigned int i = 0 ; i < n_energies ; i++) {
		std::vector<Gtk::CellRenderer *> cells = tv.get_column(i+1)->get_cells();
		cells[0]->set_visible(false);
		cells[1]->set_visible(true);

		Gtk::TreeModel::Children kids = model->children();
		for (Gtk::TreeModel::Children::iterator iter = kids.begin() ; iter != kids.end() ; ++iter) {
			Gtk::TreeModel::Row row = *iter;
			if (row[columns->col_simulate_active[i]])
				row[columns->col_status[i]] = Glib::ustring("Waiting");
			else
				row[columns->col_status[i]] = Glib::ustring("Ignored");
		}
	}
	
	tv.queue_draw();

	//let's start the clock!
	timer = new Glib::Timer();

	//create argv vector
	//at some point these could be read from preferences or so...
	argv.push_back("xmimsim");
	argv.push_back("--enable-M-lines");
	argv.push_back("--enable-radiative-cascade");
	argv.push_back("--enable-auger-cascade");
	argv.push_back("--enable-variance-reduction");
	argv.push_back("--disable-pile-up");
	argv.push_back("--disable-poisson");
	argv.push_back("--enable-escape-peaks");
	argv.push_back("--disable-advanced-compton");
	argv.push_back("--enable-opencl");
	argv.push_back("--verbose");
	//on windows; dont forget to set the solid angles and escape ratios hdf5 files



	try {
		unsigned int row = 0;
		unsigned int column = 0;
		xmimsim_start_recursive(row, column);
	}
	catch (BAM::Exception &e) {
		Gtk::MessageDialog dialog(*assistant, "Error occurred in XMI-MSIM dialog", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
		Glib::ustring error_message = Glib::ustring(e.what());
		std::cout << e.what() << std::endl;
  		dialog.set_secondary_text(error_message);
  		dialog.run();
	}


}

void App2::SimulateGrid::update_console(std::string line, std::string tag) {
	//this function doesnt do anything for the moment, but this may change in the future
	//if I would decide to add a log window

	/*
	Glib::RefPtr<Gtk::TextBuffer::Mark> cur_mark = console_buffer->create_mark(console_buffer->end());
	if (tag == "") {
		console_buffer->insert(console_buffer->end(), line);
	}
	else {
		console_buffer->insert_with_tag(console_buffer->end(), line, tag);
	}
	console_buffer->move_mark(cur_mark, console_buffer->end());
	console_view.scroll_to(cur_mark, 0.0);*/
}

void App2::SimulateGrid::xmimsim_start_recursive(unsigned int &current_row, unsigned int &current_column) {
	int out_fh, err_fh;

	//let's start work with a nap!
	g_usleep(G_USEC_PER_SEC/2);

	Gtk::TreeModel::Row row = model->children()[current_row];

	if (row[columns->col_simulate_active[current_column]] == false) {
		//skip this one
		current_row++;
		if (current_row == model->children().size()) {
			current_column++;
			current_row = 0;
		}
		if (current_column == tv.get_n_columns()-1) {
			//this was the last one
			play_button.set_sensitive(false);
			stop_button.set_sensitive(false);
			pause_button.set_sensitive(false);
			assistant->set_page_complete(*this, true);
			xmimsim_pid = 0;
			return;
		}
		else {
			//and the beat goes on...
			xmimsim_start_recursive(current_row, current_column);
			return;
		}
	}

	//write inputfile
	BAM::File::XMSI temp_xmsi_file = row[columns->col_xmsi_file[current_column]];
	temp_xmsi_file.Write();
	

	//std::cout << "Processing " << temp_xmsi_file->GetFilename() << std::endl;
	//std::cout << "Outputfile will be " << temp_xmsi_file->GetOutputFile() << std::endl;
	argv.push_back(temp_xmsi_file.GetFilename());


	//spawn process
	try {
		Glib::spawn_async_with_pipes(std::string(""), argv, Glib::SPAWN_SEARCH_PATH | Glib::SPAWN_DO_NOT_REAP_CHILD, sigc::slot<void>(), &xmimsim_pid, 0, &out_fh, &err_fh);	
	}
	catch (Glib::SpawnError &e) {
		throw BAM::Exception(std::string("App2::SimulateGrid::xmimsim_start_recursive -> ") + e.what());
	}

	//update treemodel
	row[columns->col_status[current_column]] = Glib::ustring("0 %");
	

	std::stringstream ss;
	ss << get_elapsed_time() << argv.back() << " was started with process id " << real_xmimsim_pid << std::endl;
	update_console(ss.str());

	xmimsim_paused = false;
	pause_button.set_sensitive(true);
	stop_button.set_sensitive(true);
		
	xmimsim_stderr = Glib::IOChannel::create_from_fd(err_fh);
	xmimsim_stdout = Glib::IOChannel::create_from_fd(out_fh);

	xmimsim_stderr->set_close_on_unref(true);
	xmimsim_stdout->set_close_on_unref(true);

	//add watchers
	Glib::signal_child_watch().connect(sigc::bind(sigc::mem_fun(*this, &App2::SimulateGrid::xmimsim_child_watcher), current_row, current_column), xmimsim_pid);
	Glib::signal_io().connect(sigc::bind(sigc::mem_fun(*this, &App2::SimulateGrid::xmimsim_stdout_watcher), current_row, current_column), xmimsim_stdout, Glib::IO_IN | Glib::IO_PRI | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL, Glib::PRIORITY_HIGH); 
	Glib::signal_io().connect(sigc::bind(sigc::mem_fun(*this, &App2::SimulateGrid::xmimsim_stderr_watcher), current_row, current_column), xmimsim_stderr, Glib::IO_IN | Glib::IO_PRI | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL, Glib::PRIORITY_HIGH); 
	//Glib::signal_io().connect(sigc::mem_fun(*this, &App2::SimulateGrid::xmimsim_stderr_watcher), xmimsim_stderr,Glib::IO_IN | Glib::IO_PRI | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL, Glib::PRIORITY_HIGH); 

	//remove xmsi file from argv
	argv.pop_back();
}

void App2::SimulateGrid::xmimsim_child_watcher(GPid pid, int status, unsigned int &current_row, unsigned int &current_column) {
	int success;
	//end of process
	stop_button.set_sensitive(false);
	pause_button.set_sensitive(false);

	//windows <-> unix issues here
	//unix allows to obtain more info about the way the process was terminated, windows will just have the exit code (status)
	//conditional compilation here
#ifdef G_OS_UNIX
	if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == 0) { /* child was terminated due to a call to exit */
			std::stringstream ss;
			ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " exited normally without errors" << std::endl;
			update_console(ss.str(), "success");
			success = 1;
		}
		else {
			std::stringstream ss;
			ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " exited with an error (code " << WEXITSTATUS(status) << ")" << std::endl;
			update_console(ss.str(), "error");
			success = 0;
			xmimsim_pid = 0;
		}
	}
	else if (WIFSIGNALED(status)) { /* child was terminated due to a signal */
		std::stringstream ss;
		ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " was terminated by signal " << WTERMSIG(status) << std::endl;
		update_console(ss.str(), "error");
		xmimsim_pid = 0;
		success = 0;
	}
	else {
		std::stringstream ss;
		ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " was terminated in some special way" << std::endl;
		update_console(ss.str(), "error");
		xmimsim_pid = 0;
		success = 0;
	}

#elif defined(G_OS_WIN32)
	if (status == 0) {
		std::stringstream ss;
		ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " exited normally without errors" << std::endl;
		update_console(ss.str(), "success");
		success = 1;
	}
	else {
		ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " exited with an error (code " << status << ")" << std::endl;
		update_console(ss.str(), "error");
		success = 0;
		xmimsim_pid = 0;
	}
#endif

	Glib::spawn_close_pid(xmimsim_pid);
	
	if (success == 0) {
		//something went badly wrong
		Gtk::TreeModel::Row row = model->children()[current_row];
		row[columns->col_status[current_column]] = Glib::ustring("FAILED");
		play_button.set_sensitive(false);
		stop_button.set_sensitive(false);
		pause_button.set_sensitive(false);
		return;
	}

	Gtk::TreeModel::Row row = model->children()[current_row];
	BAM::File::XMSO *xmso_file;
	std::string xmso_filename = row[columns->col_xmso_filename[current_column]];
	
	try {
		xmso_file = new BAM::File::XMSO(xmso_filename);
	}
	catch (BAM::Exception &e) {
		throw BAM::Exception("App2::SimulateGrid::xmimsim_start_recursive -> Could not read XMSO file");
	}
	row[columns->col_xmso_file[current_column]] = *xmso_file;
	int Z = row[columns->col_atomic_number];
	double col_xmso_counts_KA = 0;
	double col_xmso_counts_LA = 0;

	if (Z == 0)
		throw BAM::Exception("App2::SimulateGrid::xmimsim_start_recursive -> Invalid col_element");
			
	try {
		col_xmso_counts_KA += xmso_file->GetCountsForElementForLine(Z, "KL2");
	}
	catch (BAM::Exception &e) {}
	try {
		col_xmso_counts_KA += xmso_file->GetCountsForElementForLine(Z, "KL3");
	}
	catch (BAM::Exception &e) {}
	try {
		col_xmso_counts_LA += xmso_file->GetCountsForElementForLine(Z, "L3M4");
	}
	catch (BAM::Exception &e) {}
	try {
		col_xmso_counts_LA += xmso_file->GetCountsForElementForLine(Z, "L3M5");
	}
	catch (BAM::Exception &e) {}

	//both values should not be zero
	if (col_xmso_counts_KA == 0.0 && col_xmso_counts_LA == 0.0)
		throw BAM::Exception("App2::SimulateGrid::xmimsim_start_recursive -> Ka and La counts cannot both be 0");

	row[columns->col_xmso_counts_KA[current_column]] = col_xmso_counts_KA;
	row[columns->col_xmso_counts_LA[current_column]] = col_xmso_counts_LA;

	delete xmso_file;
	
	row[columns->col_status[current_column]] = Glib::ustring("Completed");

	//delete temparary XMI-MSIM files
	g_unlink(std::string(row[columns->col_xmsi_filename[current_column]]).c_str());
	g_unlink(std::string(row[columns->col_xmso_filename[current_column]]).c_str());

	//update current_row and current_column
	current_row++;
	if (current_row == model->children().size()) {
		current_column++;
		current_row = 0;
	}
	if (current_column == tv.get_n_columns()-1) {
		//this was the last one
		play_button.set_sensitive(false);
		stop_button.set_sensitive(false);
		pause_button.set_sensitive(false);
		assistant->set_page_complete(*this, true);
		xmimsim_pid = 0;
	}
	else {
		//and the beat goes on...
		xmimsim_start_recursive(current_row, current_column);
	}
}

bool App2::SimulateGrid::xmimsim_stdout_watcher(Glib::IOCondition cond, unsigned int &current_row, unsigned int &current_column) {
	return xmimsim_iochannel_watcher(cond, xmimsim_stdout, current_row, current_column);
}

bool App2::SimulateGrid::xmimsim_stderr_watcher(Glib::IOCondition cond, unsigned int &current_row, unsigned int &current_column) {
	return xmimsim_iochannel_watcher(cond, xmimsim_stderr, current_row, current_column);
}

bool App2::SimulateGrid::xmimsim_iochannel_watcher(Glib::IOCondition condition, Glib::RefPtr<Glib::IOChannel> iochannel, unsigned int &current_row, unsigned int &current_column) {
	Glib::IOStatus pipe_status;
	Glib::ustring pipe_string;
	int progress;

	if (condition & (Glib::IO_IN | Glib::IO_PRI)) {
		try {
			pipe_status = iochannel->read_line(pipe_string);	
			if (pipe_status == Glib::IO_STATUS_NORMAL) {
				if (sscanf(pipe_string.c_str(), "Simulating interactions at %i",&progress) == 1) {
					Gtk::TreeModel::Row row = model->children()[current_row];
					std::stringstream ss;
					ss << progress << " %";
					row[columns->col_status[current_column]] = ss.str();
					row[columns->col_progress[current_column]] = progress;
					
				}
				else {
					std::stringstream ss;
					ss << get_elapsed_time() << pipe_string;
					update_console(ss.str());
					std::cout << pipe_string;
				}
			}
			else
				return false;
		}
		catch (Glib::IOChannelError &e) {
			std::stringstream ss;
			ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " had an I/O channel error: " << e.what() << std::endl;
			update_console(ss.str(), "error");
			xmimsim_pid = 0;
			return false;
		}
		catch (Glib::ConvertError &e) {
			std::stringstream ss;
			ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " had a convert error: " << e.what() << std::endl;
			update_console(ss.str(), "error");
			xmimsim_pid = 0;
			return false;
		}
	}
	else if (condition & (Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL)) {
		//hung up...
		return false;
	}

	return true;
}

void App2::SimulateGrid::on_stop_clicked() {

	stop_button.set_sensitive(false);
	pause_button.set_sensitive(false);
	play_button.set_sensitive(false);

	xmimsim_paused = false;

#ifdef G_OS_UNIX
	int kill_rv;
	
	kill_rv = kill((pid_t) xmimsim_pid, SIGTERM);
#if !GLIB_CHECK_VERSION (2, 35, 0)
	//starting with 2.36.0 (and some unstable versions before),
	//waitpid is called from within the main loop
	//causing all kinds of trouble if I would call wait here
	//wait(NULL);
	waitpid(xmimsim_pid, NULL, WNOHANG);
#endif
	if (kill_rv == 0) {
		std::stringstream ss;
		ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " was successfully terminated before completion" << std::endl;
		update_console(ss.str(), "pause-continue-stopped");
	}
	else {
		std::stringstream ss;
		ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " could not be terminated with the SIGTERM signal" << std::endl;
		update_console(ss.str(), "error");
		xmimsim_pid = 0;
	}
#elif defined(G_OS_WIN32)
	BOOL terminate_rv;

	terminate_rv = TerminateProcess((HANDLE) xmimsim_pid, (UINT) 1);

	if (terminate_rv == TRUE) {
		std::stringstream ss;
		ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " was successfully terminated before completion" << std::endl;
		update_console(ss.str(), "pause-continue-stopped");
	}
	else {
		std::stringstream ss;
		ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " could not be terminated with the TerminateProcess call" << std::endl;
		update_console(ss.str(), "error");
		xmimsim_pid = 0;
	}
#endif

	return;
}

void App2::SimulateGrid::on_pause_clicked() {
	timer->stop();

	pause_button.set_sensitive(false);
	stop_button.set_sensitive(false);
	int kill_rv;
#ifdef G_OS_UNIX
	kill_rv = kill((pid_t) xmimsim_pid, SIGSTOP);
#elif defined(G_OS_WIN32)
	kill_rv = (int) NtSuspendProcess((HANDLE) xmimsim_pid);
#endif
	if (kill_rv == 0) {
		std::stringstream ss;
		ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " was successfully paused. Press the Play button to continue or Stop to kill the process" << std::endl;
		update_console(ss.str(), "pause-continue-stopped");
		xmimsim_paused = true;
		stop_button.set_sensitive(true);
		play_button.set_sensitive(true);
	}
}

