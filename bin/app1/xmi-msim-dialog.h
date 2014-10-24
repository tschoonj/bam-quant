#ifndef XMI_MSIM_DIALOG_H
#define XMI_MSIM_DIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/liststore.h>
#include "window.h"

class XmiMsimDialog : public Gtk::Dialog {
	public:
		std::map<int, MendeleevButton*> buttonMap;
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
		Gtk::ScrolledWindow sw;
		Gtk::TreeView tv;
		Glib::RefPtr<Gtk::ListStore> model;


		XmiMsimDialog(Window &window, bool modal, std::map<int, MendeleevButton*> &buttonMap) : 
			Gtk::Dialog(Glib::ustring("XMI-MSIM control panel"), window, modal),
			buttonMap(buttonMap),
			buttons(Gtk::ORIENTATION_VERTICAL),
			hbox(Gtk::ORIENTATION_HORIZONTAL)
			 {

			add_button("_Cancel", Gtk::RESPONSE_CANCEL);
			add_button("Select", Gtk::RESPONSE_OK);
			pauseButton.set_image_from_icon_name("gtk-media-pause", Gtk::ICON_SIZE_DIALOG);	
			stopButton.set_image_from_icon_name("gtk-media-stop", Gtk::ICON_SIZE_DIALOG);	
			playButton.set_image_from_icon_name("gtk-media-play", Gtk::ICON_SIZE_DIALOG);	
			buttons.pack_start(playButton);
			buttons.pack_start(pauseButton);
			buttons.pack_start(stopButton);

			hbox.pack_start(buttons, false, false, 5);
			set_default_size(300, 300);
		
			model = Gtk::ListStore::create(mycolumns);
			tv.set_model(model);

			for (std::map<int, MendeleevButton*>::iterator it = buttonMap.begin(); it != buttonMap.end(); ++it) {
				if (it->second->asr_file) {
					Gtk::TreeModel::Row row = *(model->append());
					row[mycolumns.col_element] = it->second->GetElement();
					row[mycolumns.col_status] = Glib::ustring("not started");
				}
			}
			tv.append_column("Element", mycolumns.col_element);
			tv.append_column("Status", mycolumns.col_status);
			sw.add(tv);
			sw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
			hbox.pack_start(sw, true, true, 5);
			get_content_area()->pack_start(hbox, true, true, 3);
			show_all_children();
		}
};


#endif
