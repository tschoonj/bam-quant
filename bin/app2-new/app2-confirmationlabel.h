#ifndef APP2_CONFIRMATIONLABEL_H
#define APP2_CONFIRMATIONLABEL_H

#include <gtkmm/label.h>

namespace App2 {
	class Assistant;
	class ConfirmationLabel: public Gtk::Label {
	public:
		ConfirmationLabel(Assistant *assistant);
		virtual ~ConfirmationLabel() {}
		void WriteRXIs();

	private:
		Assistant *assistant;
	};
}


#endif
