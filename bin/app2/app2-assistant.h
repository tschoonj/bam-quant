#ifndef APP2_ASSISTANT_H
#define APP2_ASSISTANT_H

#include <gtkmm/assistant.h>
#include <gtkmm/label.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/liststore.h>
#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/grid.h>
#include "bam_file_asr.h"
#include "bam_data_asr.h"

class App2Assistant : public Gtk::Assistant {
public:
	App2Assistant();
	virtual ~App2Assistant();
	
	

private:
	void on_assistant_apply();
	void on_assistant_cancel();
	void on_assistant_close();
	bool on_delete_event(GdkEventAny* event);
	Gtk::Label first_page;
	Gtk::ScrolledWindow second_page_sw;
	Gtk::TreeView second_page_tv;
	Glib::RefPtr<Gtk::ListStore> second_page_model;
	Gtk::Grid second_page;
	class SecondPageColumns : public Gtk::TreeModel::ColumnRecord {
		public:
			SecondPageColumns() {
				add(col_element);
				add(col_filename);
				add(col_atomic_number);
				add(col_bam_file_asr);
			}
			Gtk::TreeModelColumn<Glib::ustring> col_element;
			Gtk::TreeModelColumn<Glib::ustring> col_filename;
			Gtk::TreeModelColumn<int> col_atomic_number;
			Gtk::TreeModelColumn<BAM::File::ASR*> col_bam_file_asr;
	};
	SecondPageColumns second_page_columns;	
	Gtk::Button second_page_open;
	void on_second_page_open_clicked();
	bool on_second_page_backspace_clicked(GdkEventKey *key);

};


#endif

