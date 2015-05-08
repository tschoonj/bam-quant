#ifndef APP2_CONFIRMATIONLABEL_H
#define APP2_CONFIRMATIONLABEL_H

#include <gtkmm/label.h>
#include <gsl/gsl_multifit_nlin.h>

namespace App2 {
	class Assistant;
	class ConfirmationLabel: public Gtk::Label {
	public:
		ConfirmationLabel(Assistant *assistant);
		virtual ~ConfirmationLabel() {}
		void WriteRXIs();

	private:
		Assistant *assistant;
		static int phi_lqfit_f(const gsl_vector *x, void *data, gsl_vector *f);
		static int phi_lqfit_df(const gsl_vector *x, void *data, gsl_matrix *J);
		static int phi_lqfit_fdf(const gsl_vector *x, void *data, gsl_vector *f, gsl_matrix *J) {
			phi_lqfit_f(x, data, f);
			phi_lqfit_df(x, data, J);
			return GSL_SUCCESS;
		}

	};
}

struct phi_lqfit_data {
	std::vector<double> x;
	std::vector<double> y;
	std::vector<double> sigma;
};


#endif
