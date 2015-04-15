#ifndef APP2_ENERGIESGRID_H
#define APP2_ENERGIESGRID_H

#include <gtkmm/grid.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/button.h>
#include <string>
#include "bam_file_xmsi.h"

namespace App2 {
	//forward class declaration
	class Assistant;
	class PuresGrid;
	class SimulateGrid;

	class EnergiesGrid: public Gtk::Grid {
	public:
		EnergiesGrid(Assistant *assistant);
		virtual ~EnergiesGrid() {};

	private:
		Assistant *assistant;
		Glib::RefPtr<Gtk::ListStore> model;
		Gtk::TreeView tv;
		Gtk::ScrolledWindow sw;
		class Columns : public Gtk::TreeModel::ColumnRecord {
		public:
			Columns() {
				add(col_filename_base);
				add(col_filename_full);
				add(col_bam_file_xmsi);
				add(col_bam_file_xmsi_energy);
				add(col_pures_grid_page_index);
				add(col_samples_grid_page_index);
			}
			Gtk::TreeModelColumn<std::string> col_filename_base;
			Gtk::TreeModelColumn<std::string> col_filename_full;
			Gtk::TreeModelColumn<BAM::File::XMSI> col_bam_file_xmsi;
			Gtk::TreeModelColumn<double> col_bam_file_xmsi_energy;
			Gtk::TreeModelColumn<unsigned int> col_pures_grid_page_index;
			Gtk::TreeModelColumn<unsigned int> col_samples_grid_page_index;
		};
		Columns columns;	
		Gtk::Button open_button;
		void on_open_button_clicked();
		bool on_backspace_clicked(GdkEventKey *key);
		friend class PuresGrid;
		friend class SamplesGrid;
		friend class SimulateGrid;
	};
}
#endif
