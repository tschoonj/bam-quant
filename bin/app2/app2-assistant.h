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
#include <gtkmm/entry.h>
#include <gtkmm/buttonbox.h>
#include <glibmm/timer.h>
#include "bam_file_asr.h"
#include "bam_file_xmsi.h"
#include "bam_file_xmso.h"
#include "bam_data_asr.h"
#include <iomanip>
#include <iostream>
#include <gsl/gsl_multifit_nlin.h>

class App2Assistant : public Gtk::Assistant {
public:
	App2Assistant();
	virtual ~App2Assistant();
	
	

private:
	void on_assistant_cancel();
	void on_assistant_close();
	void on_assistant_prepare(Gtk::Widget *page);
	bool on_delete_event(GdkEventAny* event);

	//first page: introduction
	Gtk::Label first_page;

	//second page: ASR files of standards
	Gtk::ScrolledWindow second_page_sw;
	Gtk::TreeView second_page_tv;
	Glib::RefPtr<Gtk::ListStore> second_page_model;
	Gtk::Grid second_page;
	class SecondPageColumns : public Gtk::TreeModel::ColumnRecord {
		public:
			SecondPageColumns() {
				add(col_element);
				add(col_filename);
				add(col_filename_full);
				add(col_atomic_number);
				add(col_bam_file_asr);
				add(col_linetype);
			}
			Gtk::TreeModelColumn<Glib::ustring> col_element;
			Gtk::TreeModelColumn<std::string> col_filename;
			Gtk::TreeModelColumn<std::string> col_filename_full;
			Gtk::TreeModelColumn<int> col_atomic_number;
			Gtk::TreeModelColumn<BAM::File::ASR*> col_bam_file_asr;
			Gtk::TreeModelColumn<Glib::ustring> col_linetype;
	};
	SecondPageColumns second_page_columns;	
	Gtk::Button second_page_open;
	void on_second_page_open_clicked();
	bool on_second_page_backspace_clicked(GdkEventKey *key);

	//third page: ASR files of samples 
	Gtk::ScrolledWindow third_page_sw;
	Gtk::TreeView third_page_tv;
	Glib::RefPtr<Gtk::ListStore> third_page_model;
	Gtk::Grid third_page;
	class ThirdPageColumns : public Gtk::TreeModel::ColumnRecord {
		public:
			ThirdPageColumns() {
				add(col_elements);
				add(col_filename);
				add(col_bam_file_asr);
				add(col_elements_int);
				add(col_filename_full);
				add(col_density);
				add(col_thickness);
			}
			Gtk::TreeModelColumn<Glib::ustring> col_elements;
			Gtk::TreeModelColumn<Glib::ustring> col_filename;
			Gtk::TreeModelColumn<BAM::File::ASR*> col_bam_file_asr;
			Gtk::TreeModelColumn<std::vector<int>* > col_elements_int;
			Gtk::TreeModelColumn<std::string> col_filename_full;
			Gtk::TreeModelColumn<double> col_density;
			Gtk::TreeModelColumn<double> col_thickness;
	};
	Gtk::ButtonBox third_page_buttons;
	ThirdPageColumns third_page_columns;	
	Gtk::Button third_page_open_button;
	void on_third_page_open_clicked();
	bool on_third_page_backspace_clicked(GdkEventKey *key);
	void on_third_page_edited(const Glib::ustring & path, const Glib::ustring &new_text, bool is_it_density);
	Gtk::Button third_page_density_button;
	Gtk::Button third_page_thickness_button;
	void on_third_page_open_rho_or_T_clicked(bool is_it_density);
	void on_third_page_selection_changed();

	//fourth page: XMSI file 
	BAM::File::XMSI *fourth_page_xmsi_file;
	Gtk::Grid fourth_page;
	Gtk::Entry fourth_page_xmsi_entry;
	void on_fourth_page_open_clicked(Gtk::EntryIconPosition icon_position, const GdkEventButton* event);

	//fifth page: run MC simulations if necessary
	Gtk::Button fifth_page_play_button;
	Gtk::Button fifth_page_stop_button;
	Gtk::Button fifth_page_pause_button;
	Gtk::ButtonBox fifth_page_buttons;
	class FifthPageColumns : public Gtk::TreeModel::ColumnRecord {
		public:
			FifthPageColumns() {
				add(col_element);
				add(col_atomic_number);
				add(col_status);
				add(col_progress);
				add(col_xmsi_file);
				add(col_xmso_file);
				add(col_xmsi_filename);
				add(col_xmso_filename);
				add(col_xmso_counts_KA);
				add(col_xmso_counts_LA);
				add(col_bam_file_asr);
				add(col_simulate_active);
				add(col_simulate_sensitive);
			}
			Gtk::TreeModelColumn<Glib::ustring> col_element;
			Gtk::TreeModelColumn<int> col_atomic_number;
			Gtk::TreeModelColumn<Glib::ustring> col_status;
			Gtk::TreeModelColumn<int> col_progress;
			Gtk::TreeModelColumn<BAM::File::XMSI *> col_xmsi_file;
                	Gtk::TreeModelColumn<BAM::File::XMSO *> col_xmso_file;
			Gtk::TreeModelColumn<Glib::ustring> col_xmsi_filename;
			Gtk::TreeModelColumn<Glib::ustring> col_xmso_filename;
			Gtk::TreeModelColumn<double> col_xmso_counts_KA;
			Gtk::TreeModelColumn<double> col_xmso_counts_LA;
			Gtk::TreeModelColumn<BAM::File::ASR *> col_bam_file_asr;
			Gtk::TreeModelColumn<bool> col_simulate_active;
			Gtk::TreeModelColumn<bool> col_simulate_sensitive;
	};
	FifthPageColumns fifth_page_columns;	
	Gtk::Grid fifth_page;
	Gtk::ScrolledWindow fifth_page_sw;
	Gtk::TreeView fifth_page_tv;
	Glib::RefPtr<Gtk::ListStore> fifth_page_model;
	Gtk::TreeModel::Children::iterator fifth_page_iter;
	std::vector<int> fifth_page_diff_elements;
	std::vector<int> fifth_page_union_elements;
	void on_fifth_page_play_clicked();
	void on_fifth_page_pause_clicked();
	void on_fifth_page_stop_clicked();
	void on_fifth_page_simulate_active_toggled(const Glib::ustring &path);

	//XMI-MSIM related stuff
	Glib::RefPtr<Glib::IOChannel> xmimsim_stderr;
	Glib::RefPtr<Glib::IOChannel> xmimsim_stdout;
	bool xmimsim_paused;
	GPid xmimsim_pid;
	Glib::Timer *timer;
	std::vector<std::string> argv;
	void xmimsim_child_watcher(GPid pid, int child_status);
	bool xmimsim_stdout_watcher(Glib::IOCondition cond);
	bool xmimsim_stderr_watcher(Glib::IOCondition cond);
	bool xmimsim_iochannel_watcher(Glib::IOCondition cond, Glib::RefPtr<Glib::IOChannel> iochannel);
	void xmimsim_start_recursive();
	std::string get_elapsed_time() {
		if (!timer)
			return std::string("timer error");

		long time_elapsed = (long) timer->elapsed();
		long hours = time_elapsed / 3600;
		time_elapsed = time_elapsed % 3600;
		long minutes = time_elapsed / 60;
		long seconds = time_elapsed % 60;
		std::stringstream ss;
		ss.fill('0');
		ss << std::setw(2) << hours << ":" << std::setw(2) << minutes << ":" << std::setw(2) << seconds << " ";
		std::string rv = ss.str();
		return rv; 
	}
	void update_console(std::string line, std::string tag="");

	//sixth page -> save the results
	Gtk::Grid sixth_page;
	Gtk::Entry sixth_page_bpq2_entry;
	Gtk::Button sixth_page_save;
	void on_sixth_page_open_clicked();

	//seventh page -> last page!
	Gtk::Label seventh_page;

	static int phi_lqfit_f(const gsl_vector *x, void *data, gsl_vector *f);
	static int phi_lqfit_df(const gsl_vector *x, void *data, gsl_matrix *J);
	static int phi_lqfit_fdf(const gsl_vector *x, void *data, gsl_vector *f, gsl_matrix *J) {
		phi_lqfit_f(x, data, f);
		phi_lqfit_df(x, data, J);
		return GSL_SUCCESS;
	}
	

};

struct phi_lqfit_data {
	std::vector<double> x;
	std::vector<double> y;
	std::vector<double> sigma;
};



#endif

