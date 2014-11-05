#ifndef WINDOW_H
#define WINDOW_H
#include <gtkmm/applicationwindow.h>
#include "mendeleev-button.h"
#include <iostream>
#include <gtkmm/grid.h>
#include <gtkmm/box.h>
#include <gtkmm/cssprovider.h>
#include <giomm/simpleaction.h>
#include "xmi-msim-dialog.h"

#define ASR_SCALE_FACTOR 1E9

class XmiMsimDialog;

class Window: public Gtk::ApplicationWindow {

	public:
		Window();
		~Window() {
			for (std::map<int, MendeleevButton*>::iterator it = buttonMap.begin(); it != buttonMap.end(); ++it)
	   			delete it->second;
		}
		//MendeleevButton *refButton;
		double GetPhi() {return phi;}
		XmiMsimDialog *xmi_msim_dialog;
		std::vector<MendeleevButton*> buttonVectorASR; //only those buttons with an ASR file
		std::vector<MendeleevButton*> buttonVectorXMSO; //only those buttons with just an XMSO file
		Glib::RefPtr<Gio::SimpleAction> launch_action;

	private:
		//MendeleevButton test_button;
		static bool bam_catalog_loaded;
		Gtk::Box big_box;
		Gtk::Grid buttonGrid;
		std::map<int, MendeleevButton*> buttonMap; //all buttons!
		void new_project();
		void save_project();
		void open_project();
		void launch_simulations();
		//void settings();
		void reset_project();
		void update_phis();
		//Glib::RefPtr<Gio::SimpleAction> settings_action;
		Glib::RefPtr<Gio::SimpleAction> save_action;
		double phi; //average value

};

#endif
