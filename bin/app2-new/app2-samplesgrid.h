#ifndef APP2_SAMPLESGRID_H
#define APP2_SAMPLESGRID_H


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
	class SamplesSummaryGrid;

	class SamplesGrid: public Gtk::Grid {
	public:
		SamplesGrid(Assistant *assistant, App2::EnergiesGrid *energies_grid, Gtk::TreeRowReference ref);
		virtual ~SamplesGrid() {};
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
				add(col_index);
				add(col_elements);
				add(col_filename);
				add(col_filename_full);
				add(col_bam_file_asr);
				add(col_elements_int);
			}
			Gtk::TreeModelColumn<unsigned int> col_index;
			Gtk::TreeModelColumn<Glib::ustring> col_elements;
			Gtk::TreeModelColumn<std::string> col_filename;
			Gtk::TreeModelColumn<std::string> col_filename_full;
			Gtk::TreeModelColumn<BAM::File::ASR> col_bam_file_asr;
			Gtk::TreeModelColumn<std::vector<int> > col_elements_int;
		};

		Columns columns;	
		Gtk::Button open_button;
		void on_open_button_clicked();
		bool on_backspace_clicked(GdkEventKey *key);
		void on_row_deleted_or_inserted();

		sigc::connection signal_row_deleted_handler;
		sigc::connection signal_row_inserted_handler;

		Glib::RefPtr<Gtk::TreeModel> model_ref_energy;
		friend SamplesSummaryGrid;
	};
}
#endif
