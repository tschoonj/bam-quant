#ifndef MENDELEEV_BUTTON_H
#define MENDELEEV_BUTTON_H

#include <gtkmm/button.h>
#include "bam_file_asr.h"

class MendeleevButton : public Gtk::Button {
	private:
		int Z;
		char *element;
		void on_button_clicked();
	public:
		MendeleevButton(int Z);
		~MendeleevButton();
		BAM::File::ASR *asr_file;
		void reset_button();

};



#endif
