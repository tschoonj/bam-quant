#include <gtkmm/applicationwindow.h>
#include <gtkmm/label.h>
#include "mendeleev-button.h"
#include <iostream>

class Window: public Gtk::ApplicationWindow {

	public:
		Window()  {
			set_title("app1");
			set_size_request(400, 200);
			set_border_width(10);

			//initialize grid
			buttonGrid.set_row_spacing(10);
			buttonGrid.set_column_spacing(10);
			add(buttonGrid);

			int row = 0, column = 0;

			for (int i = 1 /* H */; i <= 94 /* Pu */ ; i++) {
				MendeleevButton *button = new MendeleevButton(i);
				buttonMap[i] = button;

				buttonGrid.attach(*button, column, row, 1, 1);

				switch(i) {
					case 1:
						column = 17;
						break;
					case 2:
					case 10:
					case 18:
					case 36:
					case 54:
					case 86:
						column = 0;
						row++;
						break;
					case 4:
					case 12:
						column = 12;
						break;
					case 56: /* lanthanides */
						column = 2;
						row = 8;
						break;
					case 71: /* exit lanthanides */
						column = 3;
						row = 5;
						break;
					case 88: /* actinides */
						column = 2;
						row = 9;
						break;
					case 103: /* exit lanthanides */
						column = 3;
						row = 6;
						break;
					default:
						column++;
				}
			}

			std::cout << "Before show_all_children" << std::endl;
			show_all_children();
		};

	private:
		//MendeleevButton test_button;
		Gtk::Grid buttonGrid;
		std::map<int, MendeleevButton*> buttonMap;

};
