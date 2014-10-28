#ifndef MENDELEEV_BUTTON_H
#define MENDELEEV_BUTTON_H

#include <gtkmm/button.h>
#include <gtkmm/popover.h>
#include "bam_file_asr.h"
#include "bam_file_xmsi.h"
#include "bam_file_xmso.h"
#include <gtkmm/label.h>

class MendeleevButton : public Gtk::Button {
	private:
		int Z;
		char *element;
		void on_button_clicked();
	public:
		MendeleevButton(int Z);
		~MendeleevButton();
		BAM::File::ASR *asr_file;
		BAM::File::XMSI *xmsi_file;
		BAM::File::XMSO *xmso_file;
		double asr_counts_KA; //raw
		double asr_counts_LA; //raw
		double xmso_counts_KA; //raw
		double xmso_counts_LA; //raw
		double phi;
		Gtk::Popover popover;
		string temp_xmsi_filename;
		void reset_button();
		int GetZ() {
			return Z;
		}
		std::string GetElement() {
			return std::string(element);
		}

};



#endif
