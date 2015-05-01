#ifndef APP2_OUTPUTFILEGRID_H
#define APP2_OUTPUTFILEGRID_H

#include <gtkmm/grid.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <string>

namespace App2 {
	class Assistant;
	class OutputFileGrid: public Gtk::Grid {
	public:
		OutputFileGrid(Assistant *assistant);
		virtual ~OutputFileGrid() {}
		std::string GetOutputFile() {
			return std::string(entry.get_text().c_str()); 
		}

	private:
		Assistant *assistant;
		Gtk::Label label;
		Gtk::Entry entry;
		Gtk::Button save_button;
		void on_save_clicked();
	};
}



#endif
