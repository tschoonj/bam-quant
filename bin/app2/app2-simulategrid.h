#ifndef APP2_SIMULATEGRID_H
#define APP2_SIMULATEGRID_H

#include <gtkmm/grid.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <string>
#include "bam_file_xmsi.h"
#include "bam_file_xmso.h"
#include "bam_file_asr.h"
#include <vector>
#include <glibmm/timer.h>
#include <glibmm/spawn.h>
#include <glibmm/main.h>
#include <iomanip>
#include <iostream>

namespace App2 {
	//forward class declaration
	class Assistant;
	class ConfirmationLabel;

	class SimulateGrid: public Gtk::Grid {
	public:
		SimulateGrid(Assistant *assistant);
		virtual ~SimulateGrid() {
			if (columns)
				delete columns;
			if (diff_elements)
				delete[] diff_elements;
			if (union_elements)
				delete[] union_elements;
		};
		void Prepare();

	private:
		Assistant *assistant;
		Glib::RefPtr<Gtk::ListStore> model;
		Gtk::TreeView tv;
		Gtk::ScrolledWindow sw;
	
		Gtk::Button play_button;
		Gtk::Button stop_button;
		Gtk::Button pause_button;
		Gtk::ButtonBox buttons;
	
		class Columns : public Gtk::TreeModel::ColumnRecord {
		public:
			Columns(unsigned int n_energies) : 
				col_status(n_energies),
				col_progress(n_energies),
				col_xmsi_file(n_energies),
				col_xmso_file(n_energies),
				col_xmsi_filename(n_energies),
				col_xmso_filename(n_energies),
				col_xmso_counts_KA(n_energies),
				col_xmso_counts_LA(n_energies),
				col_bam_file_asr(n_energies),
				col_simulate_active(n_energies),
				col_simulate_sensitive(n_energies),
				col_linetype(n_energies) {
				add(col_element);
				add(col_atomic_number);
				for (unsigned int i = 0 ; i < n_energies ; i++) {
					add(col_status[i]);
					add(col_progress[i]);
					add(col_xmsi_file[i]);
					add(col_xmso_file[i]);
					add(col_xmsi_filename[i]);
					add(col_xmso_filename[i]);
					add(col_xmso_counts_KA[i]);
					add(col_xmso_counts_LA[i]);
					add(col_bam_file_asr[i]);
					add(col_simulate_active[i]);
					add(col_simulate_sensitive[i]);
					add(col_linetype[i]);
				}
			}
			Gtk::TreeModelColumn<Glib::ustring> col_element;
			Gtk::TreeModelColumn<int> col_atomic_number;
			std::vector<Gtk::TreeModelColumn<Glib::ustring> > col_status;
			std::vector<Gtk::TreeModelColumn<int> > col_progress;
			std::vector<Gtk::TreeModelColumn<BAM::File::XMSI> > col_xmsi_file;
                	std::vector<Gtk::TreeModelColumn<BAM::File::XMSO> > col_xmso_file;
			std::vector<Gtk::TreeModelColumn<std::string> > col_xmsi_filename;
			std::vector<Gtk::TreeModelColumn<std::string> > col_xmso_filename;
			std::vector<Gtk::TreeModelColumn<double> > col_xmso_counts_KA;
			std::vector<Gtk::TreeModelColumn<double> > col_xmso_counts_LA;
			std::vector<Gtk::TreeModelColumn<BAM::File::ASR> > col_bam_file_asr;
			std::vector<Gtk::TreeModelColumn<bool> > col_simulate_active;
			std::vector<Gtk::TreeModelColumn<bool> > col_simulate_sensitive;
			std::vector<Gtk::TreeModelColumn<Glib::ustring> > col_linetype;
		};
		Columns *columns;	
		std::vector<int> *diff_elements;
		std::vector<int> *union_elements;
		std::vector<int> elements_int;
	
		//XMI-MSIM related stuff
		Glib::RefPtr<Glib::IOChannel> xmimsim_stderr;
		Glib::RefPtr<Glib::IOChannel> xmimsim_stdout;
		bool xmimsim_paused;
		GPid xmimsim_pid;
		Glib::Timer *timer;
		std::vector<std::string> argv;
		void xmimsim_child_watcher(GPid pid, int child_status, unsigned int &current_row, unsigned int &current_column);
		bool xmimsim_stdout_watcher(Glib::IOCondition cond, unsigned int &current_row, unsigned int &current_column);
		bool xmimsim_stderr_watcher(Glib::IOCondition cond, unsigned int &current_row, unsigned int &current_column);
		bool xmimsim_iochannel_watcher(Glib::IOCondition cond, Glib::RefPtr<Glib::IOChannel> iochannel, unsigned int &current_row, unsigned int &current_column);
		void xmimsim_start_recursive(unsigned int &current_row, unsigned int &current_column);
		std::string get_elapsed_time() {
			if (!timer)
				return std::string("timer error");

			long time_elapsed = (long) timer->elapsed();
			long hours = time_elapsed / 3600;
			time_elapsed = time_elapsed % 3600;
			long minutes = time_elapsed / 60;
			long seconds = time_elapsed % 60;
			std::stringstream ss;
			ss.fill('0');
			ss << std::setw(2) << hours << ":" << std::setw(2) << minutes << ":" << std::setw(2) << seconds << " ";
			std::string rv = ss.str();
			return rv; 
		}
		void update_console(std::string line, std::string tag="");

		void on_play_clicked();
		void on_pause_clicked();
		void on_stop_clicked();
		void on_simulate_active_toggled(const Glib::ustring &path, unsigned int energy_index);
		bool on_query_tooltip(int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip);
		friend class ConfirmationLabel;
	};
}
#endif
