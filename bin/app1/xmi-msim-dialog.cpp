#include "xmi-msim-dialog.h"
#include <glib.h>
#include <glibmm/spawn.h>
#include <glibmm/main.h>
#include <glibmm/convert.h>

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


XmiMsimDialog::XmiMsimDialog(Window &window, bool modal, std::vector<MendeleevButton*> &buttonVector) : 
	Gtk::Dialog(Glib::ustring("XMI-MSIM control panel"), window, modal),
	xmimsim_paused(false),
	xmimsim_pid(0),
	buttonVector(buttonVector),
	buttons(Gtk::ORIENTATION_VERTICAL),
	hbox(Gtk::ORIENTATION_HORIZONTAL)
	{

	timer = 0;
	add_button("Close", Gtk::RESPONSE_OK);
	set_response_sensitive(Gtk::RESPONSE_OK, false);
	pauseButton.set_image_from_icon_name("gtk-media-pause", Gtk::ICON_SIZE_DIALOG);	
	stopButton.set_image_from_icon_name("gtk-media-stop", Gtk::ICON_SIZE_DIALOG);	
	playButton.set_image_from_icon_name("gtk-media-play", Gtk::ICON_SIZE_DIALOG);	
	buttons.pack_start(playButton);
	buttons.pack_start(pauseButton);
	buttons.pack_start(stopButton);
	buttons.set_layout(Gtk::BUTTONBOX_CENTER);
	buttons.set_spacing(10);

	hbox.pack_start(buttons, false, false, 5);
	set_default_size(300, 600);
		
	model = Gtk::ListStore::create(mycolumns);
	tv.set_model(model);

	for (std::vector<MendeleevButton*>::iterator it = buttonVector.begin(); it != buttonVector.end(); ++it) {
		Gtk::TreeModel::Row row = *(model->append());
		row[mycolumns.col_element] = (*it)->GetElement();
		row[mycolumns.col_status] = Glib::ustring("not started");
	}
	tv.append_column("Element", mycolumns.col_element);
	tv.append_column("Status", mycolumns.col_status);
	sw_tree.add(tv);
	sw_tree.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	hbox.pack_start(sw_tree, true, true, 5);
	get_content_area()->pack_start(hbox, true, true, 3);

	sw_text.add(console_view);
	sw_text.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	console_view.set_wrap_mode(Gtk::WRAP_WORD);
	console_view.set_left_margin(3);
	console_view.set_editable(false);
	console_view.set_cursor_visible(false);
	get_content_area()->pack_start(sw_text, true, true, 3);

	Glib::RefPtr<Gtk::TextBuffer::TagTable> tag_table = Gtk::TextBuffer::TagTable::create();
	Glib::RefPtr<Gtk::TextBuffer::Tag> tag_error = Gtk::TextBuffer::Tag::create("error");
	tag_error->property_foreground() = "red";
	Glib::RefPtr<Gtk::TextBuffer::Tag> tag_success= Gtk::TextBuffer::Tag::create("success");
	tag_success->property_foreground() = "green";
	Glib::RefPtr<Gtk::TextBuffer::Tag> tag_pause = Gtk::TextBuffer::Tag::create("pause-continue-stopped");
	tag_pause->property_foreground() = "orange";
	tag_table->add(tag_error);
	tag_table->add(tag_success);
	tag_table->add(tag_pause);

	console_buffer = Gtk::TextBuffer::create(tag_table);
	console_view.set_buffer(console_buffer);

	pauseButton.set_sensitive(false);
	stopButton.set_sensitive(false);

	//connect signals
	pauseButton.signal_clicked().connect(sigc::mem_fun(*this, &XmiMsimDialog::on_pause_clicked));
	playButton.signal_clicked().connect(sigc::mem_fun(*this, &XmiMsimDialog::on_play_clicked));
	stopButton.signal_clicked().connect(sigc::mem_fun(*this, &XmiMsimDialog::on_stop_clicked));

	show_all_children();
}

void XmiMsimDialog::on_play_clicked() {
	if (xmimsim_paused) {
		playButton.set_sensitive(false);	
		int kill_rv;

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
			stringstream ss;
			ss << get_elapsed_time() <<"Process " << real_xmimsim_pid << " was successfully resumed" << endl;
			update_console(ss.str(), "pause-continue-stopped");
			xmimsim_paused = false;
			pauseButton.set_sensitive(true);
		}
		else {
			//if this happens, we're in serious trouble!
			stringstream ss;
			ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " could not be resumed" << endl;
			update_console(ss.str(), "pause-continue-stopped");
			playButton.set_sensitive(true);
			xmimsim_pid = 0;
				
		}
		return;
	}

	xmimsim_paused = false;
	playButton.set_sensitive(false);

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

	buttonIndex = 0;

	xmimsim_start_recursive();	

}

void XmiMsimDialog::on_pause_clicked() {
	std::cout << "pause clicked" << std::endl;

	timer->stop();

	pauseButton.set_sensitive(false);
	stopButton.set_sensitive(false);
	int kill_rv;
#ifdef G_OS_UNIX
	kill_rv = kill((pid_t) xmimsim_pid, SIGSTOP);
#elif defined(G_OS_WIN32)
	kill_rv = (int) NtSuspendProcess((HANDLE) xmimsim_pid);
#endif
	if (kill_rv == 0) {
		stringstream ss;
		ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " was successfully paused. Press the Play button to continue or Stop to kill the process" << std::endl;
		update_console(ss.str(), "pause-continue-stopped");
		xmimsim_paused = true;
		stopButton.set_sensitive(true);
		playButton.set_sensitive(true);
	}
}

void XmiMsimDialog::on_stop_clicked() {
	std::cout << "stop clicked" << std::endl;

	stopButton.set_sensitive(false);
	pauseButton.set_sensitive(false);
	playButton.set_sensitive(false);

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
		stringstream ss;
		ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " was successfully terminated before completion" << std::endl;
		update_console(ss.str(), "pause-continue-stopped");
	}
	else {
		stringstream ss;
		ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " could not be terminated with the SIGTERM signal" << std::endl;
		update_console(ss.str(), "error");
		xmimsim_pid = 0;
	}
#elif defined(G_OS_WIN32)
	BOOL terminate_rv;

	terminate_rv = TerminateProcess((HANDLE) xmimsim_pid, (UINT) 1);

	if (terminate_rv == TRUE) {
		stringstream ss;
		ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " was successfully terminated before completion" << std::endl;
		update_console(ss.str(), "pause-continue-stopped");
	}
	else {
		stringstream ss;
		ss << get_elapsed_time() << "Process " << real_xmimsim_pid << " could not be terminated with the TerminateProcess call" << std::endl;
		update_console(ss.str(), "error");
		xmimsim_pid = 0;
	}
#endif

	set_response_sensitive(Gtk::RESPONSE_OK, true);
	return;
}

void XmiMsimDialog::xmimsim_start_recursive() {
	int out_fh, err_fh;


	//let's start work with a nap!
	g_usleep(G_USEC_PER_SEC/2);


	MendeleevButton *myButton = buttonVector[buttonIndex];


	//write inputfile
	myButton->xmsi_file->Write();
	argv.push_back(myButton->xmsi_file->GetFilename());

	std::cout << "Processing " << myButton->xmsi_file->GetFilename() << endl;
	std::cout << "Outputfile will be " << myButton->xmsi_file->GetOutputFile() << endl;


	//spawn process
	try {
		Glib::spawn_async_with_pipes(std::string(""), argv, Glib::SPAWN_SEARCH_PATH | Glib::SPAWN_DO_NOT_REAP_CHILD, sigc::slot<void>(), &xmimsim_pid, 0, &out_fh, &err_fh);	
	}
	catch (Glib::SpawnError &e) {
		throw BAM::Exception(string("XmiMsimDialog::on_start_clicked -> ") + e.what());
	}

	//update treemodel
	stringstream ss;
	ss << buttonIndex;
	Gtk::TreeModel::iterator iter = model->get_iter(ss.str());
	Gtk::TreeModel::Row row = *iter;
	row[mycolumns.col_status] = Glib::ustring("running");
	

	ss.str("");
	ss << get_elapsed_time() << argv.back() << " was started with process id " << real_xmimsim_pid << std::endl;
	update_console(ss.str());

	xmimsim_paused = false;
	pauseButton.set_sensitive(true);
	stopButton.set_sensitive(true);
		
	xmimsim_stderr = Glib::IOChannel::create_from_fd(err_fh);
	xmimsim_stdout = Glib::IOChannel::create_from_fd(out_fh);

	xmimsim_stderr->set_close_on_unref(true);
	xmimsim_stdout->set_close_on_unref(true);

	//add watchers
	Glib::signal_child_watch().connect(sigc::mem_fun(*this, &XmiMsimDialog::xmimsim_child_watcher), xmimsim_pid);
	Glib::signal_io().connect(sigc::mem_fun(*this, &XmiMsimDialog::xmimsim_stdout_watcher), xmimsim_stdout,Glib::IO_IN | Glib::IO_PRI | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL, Glib::PRIORITY_HIGH); 
	Glib::signal_io().connect(sigc::mem_fun(*this, &XmiMsimDialog::xmimsim_stderr_watcher), xmimsim_stderr,Glib::IO_IN | Glib::IO_PRI | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL, Glib::PRIORITY_HIGH); 



	//remove xmsi file from argv
	argv.pop_back();
	
}

void XmiMsimDialog::xmimsim_child_watcher(GPid pid, int status) {
	int success;
	//end of process
	stopButton.set_sensitive(false);
	pauseButton.set_sensitive(false);

	//windows <-> unix issues here
	//unix allows to obtain more info about the way the process was terminated, windows will just have the exit code (status)
	//conditional compilation here
#ifdef G_OS_UNIX
	if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == 0) { /* child was terminated due to a call to exit */
			stringstream ss;
			ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " exited normally without errors" << std::endl;
			update_console(ss.str(), "success");
			success = 1;
		}
		else {
			stringstream ss;
			ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " exited with an error (code " << WEXITSTATUS(status) << ")" << std::endl;
			update_console(ss.str(), "error");
			success = 0;
			xmimsim_pid = 0;
		}
	}
	else if (WIFSIGNALED(status)) { /* child was terminated due to a signal */
		stringstream ss;
		ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " was terminated by signal " << WTERMSIG(status) << std::endl;
		update_console(ss.str(), "error");
		xmimsim_pid = 0;
		success = 0;
	}
	else {
		stringstream ss;
		ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " was terminated in some special way" << endl;
		update_console(ss.str(), "error");
		xmimsim_pid = 0;
		success = 0;
	}

#elif defined(G_OS_WIN32)
	if (status == 0) {
		stringstream ss;
		ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " exited normally without errors" << endl;
		update_console(ss.str(), "success");
		success = 1;
	}
	else {
		ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " exited with an error (code " << status << ")" << endl;
		update_console(ss.str(), "error");
		success = 0;
		xmimsim_pid = 0;
	}
#endif

	g_spawn_close_pid(xmimsim_pid);
	
	if (success == 0) {
		//something went badly wrong
		stringstream ss;
		ss << buttonIndex;
		Gtk::TreeModel::iterator iter = model->get_iter(ss.str());
		Gtk::TreeModel::Row row = *iter;
		row[mycolumns.col_status] = Glib::ustring("failed");
		playButton.set_sensitive(false);
		stopButton.set_sensitive(false);
		pauseButton.set_sensitive(false);
		set_response_sensitive(Gtk::RESPONSE_OK, true);
	}
	else if (++buttonIndex == buttonVector.size()) {
		//last simulation
		BAM::File::XMSO *xmso_file;
		try {
			xmso_file = new BAM::File::XMSO(buttonVector[buttonIndex-1]->xmsi_file->GetOutputFile());
		}
		catch (BAM::Exception &e) {
			throw BAM::Exception("XmiMsimDialog::xmimsim_start_recursive -> Could not read XMSO file");
		}
		buttonVector[buttonIndex-1]->xmso_file = xmso_file;

		try {
			buttonVector[buttonIndex-1]->xmso_counts_KA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "KL2");
		}
		catch (BAM::Exception &e) { /*ignore*/}
		try {
			buttonVector[buttonIndex-1]->xmso_counts_KA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "KL3");
		}
		catch (BAM::Exception &e) { /*ignore*/}
		try {
			buttonVector[buttonIndex-1]->xmso_counts_LA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "L3M4");
		}
		catch (BAM::Exception &e) { /*ignore*/}
		try {
			buttonVector[buttonIndex-1]->xmso_counts_LA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "L3M5");
		}
		catch (BAM::Exception &e) { /*ignore*/}

		stringstream ss;
		ss << buttonIndex-1;
		Gtk::TreeModel::iterator iter = model->get_iter(ss.str());
		Gtk::TreeModel::Row row = *iter;
		row[mycolumns.col_status] = Glib::ustring("completed");
		playButton.set_sensitive(false);
		stopButton.set_sensitive(false);
		pauseButton.set_sensitive(false);
		set_response_sensitive(Gtk::RESPONSE_OK, true);
		xmimsim_pid = 0;
		//delete temparary XMI-MSIM files
		//g_unlink(buttonVector[buttonIndex-1]->xmsi_file->GetOutputFile().c_str());
		//g_unlink(buttonVector[buttonIndex-1]->xmsi_file->GetFilename().c_str());
	}
	else {
		BAM::File::XMSO *xmso_file;
		try {
			xmso_file = new BAM::File::XMSO(buttonVector[buttonIndex-1]->xmsi_file->GetOutputFile());
		}
		catch (BAM::Exception &e) {
			throw BAM::Exception("XmiMsimDialog::xmimsim_child_watcher -> Could not read XMSO file");
		}
		buttonVector[buttonIndex-1]->xmso_file = xmso_file;
			
		try {
			buttonVector[buttonIndex-1]->xmso_counts_KA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "KL2");
		}
		catch (BAM::Exception &e) { /*ignore*/}
		try {
			buttonVector[buttonIndex-1]->xmso_counts_KA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "KL3");
		}
		catch (BAM::Exception &e) { /*ignore*/}
		try {
			buttonVector[buttonIndex-1]->xmso_counts_LA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "L3M4");
		}
		catch (BAM::Exception &e) { /*ignore*/}
		try {
			buttonVector[buttonIndex-1]->xmso_counts_LA += xmso_file->GetCountsForElementForLine(buttonVector[buttonIndex-1]->GetZ(), "L3M5");
		}
		catch (BAM::Exception &e) { /*ignore*/}

		stringstream ss;
		ss << buttonIndex-1;
		Gtk::TreeModel::iterator iter = model->get_iter(ss.str());
		Gtk::TreeModel::Row row = *iter;
		row[mycolumns.col_status] = Glib::ustring("completed");
		//g_unlink(buttonVector[buttonIndex-1]->xmsi_file->GetOutputFile().c_str());
		//g_unlink(buttonVector[buttonIndex-1]->xmsi_file->GetFilename().c_str());
		//
		xmimsim_start_recursive();	
	}


}


bool XmiMsimDialog::xmimsim_stdout_watcher(Glib::IOCondition cond) {
	return xmimsim_iochannel_watcher(cond, xmimsim_stdout);
}

bool XmiMsimDialog::xmimsim_stderr_watcher(Glib::IOCondition cond) {
	return xmimsim_iochannel_watcher(cond, xmimsim_stderr);
}

bool XmiMsimDialog::xmimsim_iochannel_watcher(Glib::IOCondition condition, Glib::RefPtr<Glib::IOChannel> iochannel) {
	Glib::IOStatus pipe_status;
	Glib::ustring pipe_string;

	if (condition & (Glib::IO_IN | Glib::IO_PRI)) {
		try {
			pipe_status = iochannel->read_line(pipe_string);	
			if (pipe_status == Glib::IO_STATUS_NORMAL) {
				stringstream ss;
				ss << get_elapsed_time() << pipe_string;
				update_console(ss.str());
				std::cout << pipe_string;
			}
			else
				return false;
		}
		catch (Glib::IOChannelError &e) {
			stringstream ss;
			ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " had an I/O channel error: " << e.what() << endl;
			update_console(ss.str(), "error");
			xmimsim_pid = 0;
			return false;
		}
		catch (Glib::ConvertError &e) {
			stringstream ss;
			ss << get_elapsed_time() << "xmimsim with process id " << real_xmimsim_pid << " had a convert error: " << e.what() << endl;
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

void XmiMsimDialog::update_console(string line, string tag) {
	Glib::RefPtr<Gtk::TextBuffer::Mark> cur_mark = console_buffer->create_mark(console_buffer->end());
	if (tag == "") {
		console_buffer->insert(console_buffer->end(), line);
	}
	else {
		console_buffer->insert_with_tag(console_buffer->end(), line, tag);
	}
	console_buffer->move_mark(cur_mark, console_buffer->end());
	console_view.scroll_to(cur_mark, 0.0);
}
