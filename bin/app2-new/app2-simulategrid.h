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

namespace App2 {
	//forward class declaration
	class Assistant;

	class SimulateGrid: public Gtk::Grid {
	public:
		SimulateGrid(Assistant *assistant);
		virtual ~SimulateGrid() {
			if (columns)
				delete columns;
			if (diff_elements)
				delete diff_elements;
			if (union_elements)
				delete union_elements;
		};
		void prepare();

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
			Columns(unsigned int n_energies) {
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
				}
			}
			Gtk::TreeModelColumn<Glib::ustring> col_element;
			Gtk::TreeModelColumn<int> col_atomic_number;
			std::vector<Gtk::TreeModelColumn<Glib::ustring> > col_status;
			std::vector<Gtk::TreeModelColumn<int> > col_progress;
			std::vector<Gtk::TreeModelColumn<BAM::File::XMSI *> > col_xmsi_file;
                	std::vector<Gtk::TreeModelColumn<BAM::File::XMSO *> > col_xmso_file;
			std::vector<Gtk::TreeModelColumn<std::string> > col_xmsi_filename;
			std::vector<Gtk::TreeModelColumn<std::string> > col_xmso_filename;
			std::vector<Gtk::TreeModelColumn<double> > col_xmso_counts_KA;
			std::vector<Gtk::TreeModelColumn<double> > col_xmso_counts_LA;
			std::vector<Gtk::TreeModelColumn<BAM::File::ASR> > col_bam_file_asr;
			std::vector<Gtk::TreeModelColumn<bool> > col_simulate_active;
			std::vector<Gtk::TreeModelColumn<bool> > col_simulate_sensitive;
		};
		Columns *columns;	

		std::vector<int> *diff_elements;
		std::vector<int> *union_elements;
		std::vector<int> elements_int;
		void on_play_clicked();
		void on_pause_clicked();
		void on_stop_clicked();
		void on_simulate_active_toggled(const Glib::ustring &path, unsigned int energy_index);
	};
}
#endif
