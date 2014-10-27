#ifndef XMI_MSIM_DIALOG_H
#define XMI_MSIM_DIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/liststore.h>
#include <gtkmm/textview.h>
#include <glibmm/timer.h>
#include <glibmm/iochannel.h>
#include <iomanip>
#include <iostream>
#include "window.h"
#include <glib.h>
#ifdef G_OS_UNIX
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <signal.h>
#elif defined(G_OS_WIN32)
	#include <windows.h>
#endif

class XmiMsimDialog : public Gtk::Dialog {
	private:
		bool xmimsim_paused;
		GPid xmimsim_pid;
		Glib::Timer *timer;
		vector<std::string> argv;
		int buttonIndex;

	public:
		std::vector<MendeleevButton*> buttonVector;
		Gtk::Button playButton;
		Gtk::Button stopButton;
		Gtk::Button pauseButton;
		Gtk::ButtonBox buttons;
		class MyColumns : public Gtk::TreeModel::ColumnRecord {
			public:
				MyColumns() {
					add(col_element);
					add(col_status);
				}
				Gtk::TreeModelColumn<Glib::ustring> col_element;
				Gtk::TreeModelColumn<Glib::ustring> col_status;
		};
		MyColumns mycolumns;	
		Gtk::Box hbox;
		Gtk::ScrolledWindow sw_tree;
		Gtk::ScrolledWindow sw_text;
		Gtk::TreeView tv;
		Glib::RefPtr<Gtk::ListStore> model;
		Glib::RefPtr<Gtk::TextBuffer> console_buffer;
		Gtk::TextView console_view;
		Glib::RefPtr<Glib::IOChannel> xmimsim_stderr;
		Glib::RefPtr<Glib::IOChannel> xmimsim_stdout;



		XmiMsimDialog(Window &window, bool modal, std::vector<MendeleevButton*> &buttonVector);
		~XmiMsimDialog() {
			if (timer)
				delete timer;
			if (xmimsim_pid) {
				//a process is still running!
#ifdef G_OS_UNIX
				int kill_rv;
				kill_rv = kill((pid_t) xmimsim_pid, SIGTERM);
#if !GLIB_CHECK_VERSION (2, 35, 0)
				waitpid(xmimsim_pid, NULL, WNOHANG);
#endif
#elif defined(G_OS_WIN32)
				BOOL terminate_rv;
				terminate_rv = TerminateProcess((HANDLE) xmimsim_pid, (UINT) 1);
#endif

			}
		}
		void on_play_clicked();
		void on_pause_clicked();
		void on_stop_clicked();
		void xmimsim_child_watcher(GPid pid, int child_status);
		bool xmimsim_stdout_watcher(Glib::IOCondition cond);
		bool xmimsim_stderr_watcher(Glib::IOCondition cond);
		bool xmimsim_iochannel_watcher(Glib::IOCondition cond, Glib::RefPtr<Glib::IOChannel> iochannel);
		void xmimsim_start_recursive();
		string get_elapsed_time() {
			if (!timer)
				return string("timer error");

			long time_elapsed = (long) timer->elapsed();
			long hours = time_elapsed / 3600;
			time_elapsed = time_elapsed % 3600;
			long minutes = time_elapsed / 60;
			long seconds = time_elapsed % 60;
			stringstream ss;
			ss.fill('0');
			ss << setw(2) << hours << ":" << setw(2) << minutes << ":" << setw(2) << seconds << " ";
			string rv = ss.str();
			return rv; 
		}
		void update_console(string line, string tag="");
};


#endif
