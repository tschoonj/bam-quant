#ifndef APP2_PURESGRID_H
#define APP2_PURESGRID_H


#include <gtkmm/grid.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/button.h>
#include <gtkmm/treerowreference.h>
#include <string>
#include "bam_file_asr.h"
//
namespace App2 {
	//forward class declaration
	class Assistant;
	class EnergiesGrid;
	class SimulateGrid;
	class ConfirmationLabel;

	class PuresGrid: public Gtk::Grid {
	public:
		PuresGrid(Assistant *assistant, App2::EnergiesGrid *energies_grid, Gtk::TreeRowReference ref);
		virtual ~PuresGrid() {};
		Gtk::TreeModel::Path GetEnergiesGridPath() {
			return ref_energy.get_path();
		}
		double GetEnergy() {
			return energy;
		}
	private:
		Assistant *assistant;
		App2::EnergiesGrid *energies_grid;
		Gtk::TreeRowReference ref_energy;
		double energy;
		Glib::RefPtr<Gtk::ListStore> model;
		Gtk::TreeView tv;
		Gtk::ScrolledWindow sw;
		class Columns : public Gtk::TreeModel::ColumnRecord {
		public:
			Columns() {
				add(col_element);
				add(col_filename);
				add(col_filename_full);
				add(col_atomic_number);
				add(col_bam_file_asr);
				add(col_linetype);
			}
			Gtk::TreeModelColumn<Glib::ustring> col_element;
			Gtk::TreeModelColumn<std::string> col_filename;
			Gtk::TreeModelColumn<std::string> col_filename_full;
			Gtk::TreeModelColumn<int> col_atomic_number;
			Gtk::TreeModelColumn<BAM::File::ASR> col_bam_file_asr;
			Gtk::TreeModelColumn<Glib::ustring> col_linetype;
		};
		Columns columns;	
		Gtk::Button open_button;
		void on_open_button_clicked();
		bool on_backspace_clicked(GdkEventKey *key);

		Glib::RefPtr<Gtk::TreeModel> model_ref_energy;
		friend class SimulateGrid;
		friend class ConfirmationLabel;
	};
}
#endif

