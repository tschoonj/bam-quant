#ifndef APP2_SAMPLESSUMMARYGRID_H
#define APP2_SAMPLESSUMMARYGRID_H

#include <gtkmm/grid.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <string>


namespace App2 {
	//forward class declaration
	class Assistant;

	class SamplesSummaryGrid: public Gtk::Grid {
	public:
		SamplesSummaryGrid(Assistant *assistant);
		virtual ~SamplesSummaryGrid() {}
		void prepare();
	private:
		Assistant *assistant;
		Glib::RefPtr<Gtk::ListStore> model;
		Gtk::TreeView tv;
		Gtk::ScrolledWindow sw;
		class Columns : public Gtk::TreeModel::ColumnRecord {
		public:
			Columns() {
				add(col_index);
				add(col_density);
				add(col_thickness);
				add(col_fix_thickness_density);
				add(col_matrix);
			}	
			Gtk::TreeModelColumn<unsigned int> col_index;
			Gtk::TreeModelColumn<double> col_density;
			Gtk::TreeModelColumn<double> col_thickness;
			Gtk::TreeModelColumn<bool> col_fix_thickness_density;
			Gtk::TreeModelColumn<std::string> col_matrix;
		};
		Columns columns;
		Gtk::ButtonBox buttons;
		Gtk::Button density_button;
		Gtk::Button thickness_button;
		Gtk::Button fix_thickness_density_button;
		Gtk::Button matrix_button;
		Gtk::TreeViewColumn *matrix_column;

		void on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
		bool on_backspace_clicked(GdkEventKey *key);
		void on_edited(const Glib::ustring & path, const Glib::ustring &new_text, bool is_it_density);
		void on_open_rho_or_T_clicked(bool is_it_density);
		void on_open_fix_thickness_density_clicked();
		void on_open_matrix_clicked();
		void on_selection_changed();
		void on_fix_thickness_density_toggled(const Glib::ustring &path);
	};
}
#endif
