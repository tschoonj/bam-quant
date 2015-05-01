#ifndef APP2_ASSISTANT_H
#define APP2_ASSISTANT_H

#include <gtkmm/assistant.h>
#include <gtkmm/label.h>
#include "app2-energiesgrid.h"
#include "app2-puresgrid.h"
#include "app2-samplesgrid.h"
#include "app2-samplessummarygrid.h"
#include "app2-simulategrid.h"
#include "app2-outputfilegrid.h"
#include <vector>


namespace App2 {
	class Assistant : public Gtk::Assistant {
	public:
		Assistant();
		virtual ~Assistant() {};
		//vector of pures_grid's
		std::vector<PuresGrid*> pures_grid_vec;
		std::vector<SamplesGrid*> samples_grid_vec;

		//first page: introduction
		Gtk::Label intro_page;

		//second page: energies grid (XMSI files)
		EnergiesGrid energies_grid;	

		//sample summary page -> will get filled up by prepare event
		SamplesSummaryGrid samples_summary_grid;

		//simulate page: perform the Monte Carlo simulations necessary to approximate RXI's, if necessary
		SimulateGrid simulate_grid;

		//outputfile page: select a filename for the outputfile with RXI values
		OutputFileGrid output_file_grid;

		//last page: confirmation
		Gtk::Label confirm_page;
	private:
		//void on_assistant_cancel();
		//void on_assistant_close();
		void on_prepare(Gtk::Widget *page);
		bool on_delete_event(GdkEventAny* event) {
			get_application()->remove_window(*this);
			return true;
		}


	};
}
#endif

