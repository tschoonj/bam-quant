#include "window.h"
#include <gtkmm/filechooser.h>
#include <gtkmm/label.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stylecontext.h>
#include <unistd.h>
#include <glibmm/miscutils.h>
#include "xmi-msim-dialog.h"
#include <libxml++/libxml++.h>
#include <libxml/xmlwriter.h>
#include <libxml/catalog.h>


bool Window::bam_catalog_loaded = false;


Window::Window() : big_box(Gtk::ORIENTATION_VERTICAL, 5) {
	xmi_msim_dialog = 0;
	phi = 0;

	//menu signals
	add_action("new", sigc::mem_fun(*this, &Window::new_project));
	add_action("open", sigc::mem_fun(*this, &Window::open_project));
	save_action = add_action("save", sigc::mem_fun(*this, &Window::save_project));
	/*settings_action = add_action("settings", sigc::mem_fun(*this, &Window::settings));
	settings_action->set_enabled(false);
	*/
	launch_action = add_action("start", sigc::mem_fun(*this, &Window::launch_simulations));
	launch_action->set_enabled(false);

	multiple_add_action = add_action("multiple_add", sigc::mem_fun(*this, &Window::multiple_add));
	multiple_add_action->set_enabled(false);

	save_action->set_enabled(false);

	set_title("app1");
	set_size_request(400, 200);
	set_border_width(10);

	//initialize grid
	buttonGrid.set_row_spacing(10);
	buttonGrid.set_column_spacing(10);
	big_box.pack_start(buttonGrid, false, false, 10);
	add(big_box);

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

	show_all_children();
}

void Window::new_project() {
	reset_project();


	//prepare filedialog
	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*this, "Please select a number of AXIL results files", Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog->set_select_multiple();
	Glib::RefPtr<Gtk::FileFilter> filter_asr = Gtk::FileFilter::create();
	filter_asr->set_name("ASR files");
	filter_asr->add_pattern("*.asr");
	filter_asr->add_pattern("*.ASR");
	dialog->add_filter(filter_asr);
	dialog->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog->add_button("Select", Gtk::RESPONSE_OK);
	
	int result = dialog->run();
	std::vector<std::string> filenames;

	switch(result) {
		case(Gtk::RESPONSE_OK):
			std::cout << "Open clicked." << std::endl;
			filenames = dialog->get_filenames();

      			break;
		case(Gtk::RESPONSE_CANCEL):
		default:
			delete dialog;
			return;
	}

	delete dialog;


	for (std::vector<std::string>::iterator it = filenames.begin() ; it != filenames.end() ; ++it)  {
		std::cout << "Selected file: " << *it << std::endl;

		BAM::File::ASR *asr_file;
		try {
			asr_file = new BAM::File::ASR(*it);
		}
		catch (ifstream::failure &e) {
			Gtk::MessageDialog dialog(*this, "Error reading in "+*it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			dialog.set_secondary_text(Glib::ustring("I/O error: ")+e.what());
  			dialog.run();
			//reset everything in window!
			reset_project();
			return;
		}
		catch (BAM::Exception &e) {
			Gtk::MessageDialog dialog(*this, "Error reading in "+*it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			dialog.set_secondary_text(e.what());
  			dialog.run();
			//reset everything in window!
			reset_project();
			return;
		}
		//count number of elements -> must be 1!
		if (asr_file->GetNPeaks() > 1) {
			Gtk::MessageDialog dialog(*this, "Error reading in "+*it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			dialog.set_secondary_text(Glib::ustring("BAM::Data::ASR error: only one peak permitted per file!"));
  			dialog.run();
			delete asr_file;
			//reset everything in window!
			reset_project();
			return;
		}
		BAM::Data::ASR asr_data = asr_file->GetData(0);
		int Z = asr_data.GetZ();
		if (buttonMap[Z]->asr_file) {
			Gtk::MessageDialog dialog(*this, "Error reading in "+*it, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			dialog.set_secondary_text(Glib::ustring("BAM::Data::ASR error: element already present in map!"));
  			dialog.run();
			delete asr_file;
			//reset everything in window!
			reset_project();
			return;
		}
		buttonMap[Z]->asr_file = asr_file;
		if (asr_data.GetLine() == KA_LINE) {
			buttonMap[Z]->asr_counts_KA = asr_data.GetCounts();
		}
		else if (asr_data.GetLine() == LA_LINE) {
			buttonMap[Z]->asr_counts_LA = asr_data.GetCounts();
		}
		buttonVectorASR.push_back(buttonMap[Z]);

		//change color of element
		//buttonMap[Z]->override_background_color(Gdk::RGBA("Chartreuse"));
		buttonMap[Z]->SetRed();
			
		/*if (it == filenames.begin())
			refButton = buttonMap[Z];*/


	}

	//ask to load an XMI-MSIM input-file
	dialog = new Gtk::FileChooserDialog(*this, "Please select a XMI-MSIM input-file", Gtk::FILE_CHOOSER_ACTION_OPEN);
	Glib::RefPtr<Gtk::FileFilter> filter_xmsi = Gtk::FileFilter::create();
	filter_xmsi->set_name("XMSI files");
	filter_xmsi->add_pattern("*.xmsi");
	filter_xmsi->add_pattern("*.XMSI");
	dialog->add_filter(filter_xmsi);
	dialog->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog->add_button("Select", Gtk::RESPONSE_OK);
	
	result = dialog->run();
	string filename;
	switch(result) {
		case(Gtk::RESPONSE_OK):
			filename = dialog->get_filename();
			std::cout << "Open clicked: " << filename << std::endl;
      			break;
		case(Gtk::RESPONSE_CANCEL):
		default:
			delete dialog;
			reset_project();
			return;
	}

	delete dialog;

	BAM::File::XMSI *xmsi_file;

	try {
		xmsi_file = new BAM::File::XMSI(filename);
	}
	catch (BAM::Exception &e) {
		Gtk::MessageDialog dialog(*this, "Error reading in "+filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
		Glib::ustring error_message = Glib::ustring(e.what());
		std::cout << e.what() << std::endl;
  		dialog.set_secondary_text(error_message);
  		dialog.run();
		//reset everything in window!
		reset_project();
		return;
	
	}

	//for all elements -> establish XMSI files with the correct composition
	for (int i = 1 ; i <= 94 ; i++) {
		buttonMap[i]->temp_xmsi_filename = Glib::build_filename(Glib::get_tmp_dir(), "bam-quant-" + Glib::get_user_name() + "-" + static_cast<ostringstream*>( &(ostringstream() << getpid()))->str() + buttonMap[i]->GetElement()+ ".xmsi");
		buttonMap[i]->xmsi_file = new BAM::File::XMSI(*xmsi_file);
		
		//now change its composition
		BAM::Data::XMSI::Composition composition;

		BAM::Data::XMSI::Layer layer1("Air, Dry (near sea level)", 5.0);
		composition.AddLayer(layer1);

		BAM::Data::XMSI::Layer layer2(ElementDensity(buttonMap[i]->GetZ()), 5.0);
		layer2.AddElement(buttonMap[i]->GetZ(), 1.0);
		composition.AddLayer(layer2);

		composition.SetReferenceLayer(2);
		buttonMap[i]->xmsi_file->ReplaceComposition(composition);
		string temp_xmso_filename = buttonMap[i]->temp_xmsi_filename;
		temp_xmso_filename.replace(temp_xmso_filename.end()-1,temp_xmso_filename.end(), "o");
		buttonMap[i]->xmsi_file->SetOutputFile(temp_xmso_filename);
		buttonMap[i]->xmsi_file->SetFilename(buttonMap[i]->temp_xmsi_filename);
	}
	delete xmsi_file;

	//now launch the XMI-MSIM dialog
	xmi_msim_dialog = new XmiMsimDialog(*this, true, buttonVectorASR);
	try {
		result = xmi_msim_dialog->run();
	}
	catch (BAM::Exception &e) {
		delete xmi_msim_dialog;
		Gtk::MessageDialog dialog(*this, "Error occurred in XMI-MSIM dialog", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
		Glib::ustring error_message = Glib::ustring(e.what());
		std::cout << e.what() << std::endl;
  		dialog.set_secondary_text(error_message);
  		dialog.run();
		//reset everything in window!
		xmi_msim_dialog = 0;
		reset_project();
		return;
	}
	if (result == Gtk::RESPONSE_DELETE_EVENT) {
		//delete event caught
		delete xmi_msim_dialog;
		xmi_msim_dialog = 0;
		reset_project();
		return;
	}
	else {
		//check if simulation was not stopped
		for (std::vector<MendeleevButton *>::iterator it = buttonVectorASR.begin() ; it != buttonVectorASR.end(); ++it) {
			if ((*it)->asr_file && !(*it)->xmso_file) {
				delete xmi_msim_dialog;
				reset_project();
				return;
			}
		}
	}
	delete xmi_msim_dialog;

	update_phis();

	//update menu
	//settings_action->set_enabled();
	save_action->set_enabled();
	multiple_add_action->set_enabled();
	
}

void Window::update_phis() {
	//let's calculate the normalization factor
	for (std::vector<MendeleevButton *>::iterator it = buttonVectorASR.begin() ; it != buttonVectorASR.end(); ++it) {
		cout << "asr_counts_KA: " << (*it)->asr_counts_KA << endl;
		cout << "asr_counts_LA: " << (*it)->asr_counts_LA << endl;
		cout << "xmso_counts_KA: " << (*it)->xmso_counts_KA << endl;
		cout << "xmso_counts_LA: " << (*it)->xmso_counts_LA << endl;
		if ((*it)->asr_counts_KA > 0 && (*it)->xmso_counts_KA > 0) {
			(*it)->phi = (*it)->asr_counts_KA * (*it)->asr_file->GetNormfactor() / ASR_SCALE_FACTOR / (*it)->xmso_counts_KA;
		}
		else if((*it)->asr_counts_LA > 0 && (*it)->xmso_counts_LA > 0) {
			(*it)->phi = (*it)->asr_counts_LA * (*it)->asr_file->GetNormfactor() / ASR_SCALE_FACTOR / (*it)->xmso_counts_LA;
		}
		else {
			Gtk::MessageDialog dialog(*this, string("Error detected while calculating phi for ")+(*it)->GetElement(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
  			dialog.run();
			dialog.hide();
			//reset everything in window!
			reset_project();
			return;
		}
		phi += (*it)->phi;
		cout << "Phi for element " << (*it)->GetElement() << ": " << (*it)->phi << endl;
	}
	phi /= buttonVectorASR.size();
	cout << "Average phi: " << phi << endl;

}


void Window::reset_project() {
	for (std::map<int, MendeleevButton*>::iterator it = buttonMap.begin(); it != buttonMap.end(); ++it) {
		(it->second)->reset_button();
	}
	phi = 0;
	//settings_action->set_enabled(false);
	save_action->set_enabled(false);
	launch_action->set_enabled(false);
	multiple_add_action->set_enabled(false);
	buttonVectorASR.clear();
	buttonVectorXMSO.clear();
}

/*
void Window::settings() {
	cout << "Settings activated" << endl;

	Gtk::Dialog dialog("Settings panel", *this, true);
	dialog.add_button("Apply", Gtk::RESPONSE_OK);

	Gtk::Box hbox(Gtk::ORIENTATION_HORIZONTAL);
	Gtk::Label label("Reference element");
	hbox.pack_start(label, false, false, 3);
	Gtk::ComboBoxText combo;
	
	int counter = 0;
	for (int Z = 1 ; Z <= 94 ; Z++) {
		if (buttonMap[Z]->asr_file) {
			combo.append(buttonMap[Z]->GetElement());
			if (buttonMap[Z] == refButton) {
				combo.set_active(counter);
			}
			counter++;
		}
	}
	hbox.pack_end(combo, false, false, 3);

	dialog.get_content_area()->pack_start(hbox, true, true, 3);
	dialog.show_all_children();

	int result = dialog.run();
	if (result == Gtk::RESPONSE_OK) {
		//update refButton
		Glib::ustring selected = combo.get_active_text();
		for (int Z = 1 ; Z <= 94 ; Z++) {
			if (buttonMap[Z]->asr_file && buttonMap[Z]->GetElement() == selected) {
				refButton = buttonMap[Z];
				break;
			}
		}
	}
	dialog.hide();
	update_phis();
}*/


void Window::save_project() {
	//fire up a filechooserdialog
	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*this, "Please select a BAM-QUANT project 1 file", Gtk::FILE_CHOOSER_ACTION_SAVE);
	Glib::RefPtr<Gtk::FileFilter> filter_bqp1 = Gtk::FileFilter::create();
	filter_bqp1->set_name("BAM-QUANT project 1 files");
	filter_bqp1->add_pattern("*.bqp1");
	filter_bqp1->add_pattern("*.BQP1");
	dialog->add_filter(filter_bqp1);
	dialog->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog->add_button("Select", Gtk::RESPONSE_OK);
	
	int result = dialog->run();
	string filename;
	switch(result) {
		case(Gtk::RESPONSE_OK):
			filename = dialog->get_filename();
			if (filename.compare(filename.length()-5, string::npos, ".bqp1") != 0)
				filename += ".bqp1";
			std::cout << "Open clicked: " << filename << std::endl;
      			break;
		case(Gtk::RESPONSE_CANCEL):
		default:
			delete dialog;
			return;
	}
	delete dialog;
	
	try {
		xmlpp::Document document;

		document.set_internal_subset("bam-quant-app1", "", "http://www.bam.de/xml/bam-quant-app1.dtd");
		//document.add_comment("this is a comment");
		xmlDocPtr doc = document.cobj();
		xmlTextWriterPtr writer = xmlNewTextWriterTree(doc, 0, 0);
		if (xmi_write_default_comments(writer) == 0) {
			throw BAM::Exception("Could not write XMI-MSIM default comments");
		}

		//
		xmlTextWriterFlush(writer);
		xmlFreeTextWriter(writer);
		xmlpp::Element *rootnode = document.create_root_node("bam-quant-app1");
		for (std::map<int, MendeleevButton*>::iterator it = buttonMap.begin(); it != buttonMap.end(); ++it) {
			if (it->second->asr_file) {
				xmlpp::Element *element_data = rootnode->add_child("element_data");
				element_data->set_attribute("datatype", "experimental");
				element_data->set_attribute("element", it->second->GetElement());
				element_data->set_attribute("linetype", it->second->asr_counts_KA > 0.0 ? "KA_LINE": "LA_LINE");
				xmlpp::Element *asrfile = element_data->add_child("asrfile");
				stringstream ss;
				if (it->second->asr_counts_KA > 0.0)
					ss << it->second->asr_counts_KA;
				else
					ss << it->second->asr_counts_LA;
				xmlpp::Element *counts = asrfile->add_child("axil_counts");
				counts->add_child_text(ss.str());
				ss.str("");
				ss.clear();
				ss << it->second->asr_file->GetNormfactor();
				xmlpp::Element *normfactor = asrfile->add_child("normfactor");
				normfactor->add_child_text(ss.str());
				
				xmlpp::Node *nodepp = dynamic_cast<xmlpp::Node *>(element_data);
				xmlNodePtr node = nodepp->cobj();
				writer = xmlNewTextWriterTree(doc, node, 0);
				struct xmi_output *xmso_raw = it->second->xmso_file->GetInternalCopy();
				if (xmi_write_output_xml_body(writer, xmso_raw, -1, -1, 0) == 0) {
					throw BAM::Exception("Could not write XMI-MSIM output body");
				}
				xmlTextWriterFlush(writer);
				xmlFreeTextWriter(writer);
				xmi_free_output(xmso_raw);
			}
			else if (it->second->xmso_file) {
				xmlpp::Element *element_data = rootnode->add_child("element_data");
				element_data->set_attribute("datatype", "interpolated");
				element_data->set_attribute("element", it->second->GetElement());
				element_data->set_attribute("linetype", it->second->xmso_counts_KA > 0.0 ? "KA_LINE": "LA_LINE");
				xmlpp::Node *nodepp = dynamic_cast<xmlpp::Node *>(element_data);
				xmlNodePtr node = nodepp->cobj();
				writer = xmlNewTextWriterTree(doc, node, 0);
				struct xmi_output *xmso_raw = it->second->xmso_file->GetInternalCopy();
				if (xmi_write_output_xml_body(writer, xmso_raw, -1, -1, 0) == 0) {
					throw BAM::Exception("Could not write XMI-MSIM output body");
				}
				xmlTextWriterFlush(writer);
				xmlFreeTextWriter(writer);
				xmi_free_output(xmso_raw);
			}
		}

		document.write_to_file_formatted(filename);
	}
	catch (std::exception& ex) {
		//error handling
		std::cerr << "Exception occurred: " << ex.what() << std::endl;
		return;
	}
	catch (...) {
		std::cerr << "some other exception occurred" << std::endl;
	}
	cout << "end of save_project reached" << endl;
	return;
}

void Window::open_project() {
	//fire up a filechooserdialog
	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*this, "Please select a BAM-QUANT project 1 file", Gtk::FILE_CHOOSER_ACTION_OPEN);
	Glib::RefPtr<Gtk::FileFilter> filter_bqp1 = Gtk::FileFilter::create();
	filter_bqp1->set_name("BAM-QUANT project 1 files");
	filter_bqp1->add_pattern("*.bqp1");
	filter_bqp1->add_pattern("*.BQP1");
	dialog->add_filter(filter_bqp1);
	dialog->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog->add_button("Select", Gtk::RESPONSE_OK);
	
	int result = dialog->run();
	string filename;
	switch(result) {
		case(Gtk::RESPONSE_OK):
			filename = dialog->get_filename();
			std::cout << "Open clicked: " << filename << std::endl;
      			break;
		case(Gtk::RESPONSE_CANCEL):
		default:
			delete dialog;
			return;
	}
	delete dialog;

	reset_project();

	//load our catalog file
	if (!bam_catalog_loaded) {
		if (xmlLoadCatalog(BAM_CATALOG) != 0) {
			std::cerr << "Could not load catalog: " << BAM_CATALOG << std::endl;
			return;
		}
		else
			bam_catalog_loaded = true;
	}
	if (xmi_xmlLoadCatalog() == 0) {
		std::cerr << "Could not load XMI-MSIM XML catalog" << std::endl;
	}
	try {
		xmlpp::DomParser *parser = new xmlpp::DomParser;
		parser->set_validate();
		parser->parse_file(filename);
		xmlpp::Document *document = parser->get_document();
		xmlpp::Element *root = document->get_root_node();
		xmlpp::Node::NodeList list = root->get_children("element_data");
		BAM::File::XMSI *xmsi_file;
		for (xmlpp::Node::NodeList::iterator it = list.begin() ; it != list.end() ; ++it) {
			//get attributes first
			Glib::ustring element = dynamic_cast<xmlpp::Element*>(*it)->get_attribute_value("element");
			Glib::ustring datatype= dynamic_cast<xmlpp::Element*>(*it)->get_attribute_value("datatype");
			Glib::ustring linetype = dynamic_cast<xmlpp::Element*>(*it)->get_attribute_value("linetype");
			int Z = SymbolToAtomicNumber((char *) element.c_str());
			if (datatype == "experimental") {
				//read asrfile
				xmlpp::Node *asrfile = (*it)->get_first_child("asrfile");
				if (asrfile == 0) {
					cerr << "experimental datatype requires asrfile child!!!" << endl;
					reset_project();
					return;
				}
				Glib::ustring axil_counts_str = dynamic_cast<xmlpp::Element *>(asrfile->get_first_child("axil_counts"))->get_child_text()->get_content();
				Glib::ustring normfactor_str = dynamic_cast<xmlpp::Element *>(asrfile->get_first_child("normfactor"))->get_child_text()->get_content();
				buttonMap[Z]->SetRed();
				stringstream ss;
				if (linetype == "KA_LINE") {
					ss << axil_counts_str;
					ss >> buttonMap[Z]->asr_counts_KA;
					buttonMap[Z]->asr_counts_LA = 0.0;
				}
				else if (linetype == "LA_LINE") {
					ss << axil_counts_str;
					ss >> buttonMap[Z]->asr_counts_LA;
					buttonMap[Z]->asr_counts_KA = 0.0;
				}
				else {
					cerr << "Unknown linetype detected!!" << endl;
					reset_project();
					return;
				}
				ss.str("");
				ss.clear();
				ss << normfactor_str;
				double normfactor;
				ss >> normfactor;
				BAM::File::ASR *asr_file = new BAM::File::ASR(normfactor);
				buttonMap[Z]->asr_file = asr_file;
				buttonVectorASR.push_back(buttonMap[Z]);
			}
			else {
				buttonMap[Z]->SetGreen();
				buttonVectorXMSO.push_back(buttonMap[Z]);
			}
			
			xmlpp::Node *xmimsim_results = (*it)->get_first_child("xmimsim-results");
			struct xmi_output *output = (struct xmi_output*) malloc(sizeof(struct xmi_output));
			if (xmi_read_output_xml_body(document->cobj(), xmimsim_results->cobj(), output, 0, 0) == 0) {
				cerr << "Error in xmi_read_output_xml_body" << endl;
				reset_project();
				return;
			}
			if (it == list.begin()) {
				//first file only
				cout << "Before constructing xmsi_file" << endl;
				xmsi_file = new BAM::File::XMSI(output->input);
				cout << "After constructing xmsi_file" << endl;
			}


			BAM::File::XMSO *xmso_file;
			xmso_file = new BAM::File::XMSO(output);
			xmi_free_output(output);
			buttonMap[Z]->xmso_file = xmso_file;
			try {
				buttonMap[Z]->xmso_counts_KA += xmso_file->GetCountsForElementForLine(Z, "KL2");
			}
			catch (BAM::Exception &e) { /*ignore*/}
			try {
				buttonMap[Z]->xmso_counts_KA += xmso_file->GetCountsForElementForLine(Z, "KL2");
			}
			catch (BAM::Exception &e) { /*ignore*/}
			try {
				buttonMap[Z]->xmso_counts_LA += xmso_file->GetCountsForElementForLine(Z, "L3M4");
			}
			catch (BAM::Exception &e) { /*ignore*/}
			try {
				buttonMap[Z]->xmso_counts_LA += xmso_file->GetCountsForElementForLine(Z, "L3M5");
			}
			catch (BAM::Exception &e) { /*ignore*/}


			
		}
		delete parser;

		for (int i = 1 ; i <= 94 ; i++) {
			buttonMap[i]->temp_xmsi_filename = Glib::build_filename(Glib::get_tmp_dir(), "bam-quant-" + Glib::get_user_name() + "-" + static_cast<ostringstream*>( &(ostringstream() << getpid()))->str() + buttonMap[i]->GetElement()+ ".xmsi");
			buttonMap[i]->xmsi_file = new BAM::File::XMSI(*xmsi_file);
			
			//now change its composition
			BAM::Data::XMSI::Composition composition;

			BAM::Data::XMSI::Layer layer1("Air, Dry (near sea level)", 5.0);
			composition.AddLayer(layer1);

			BAM::Data::XMSI::Layer layer2(ElementDensity(buttonMap[i]->GetZ()), 5.0);
			layer2.AddElement(buttonMap[i]->GetZ(), 1.0);
			composition.AddLayer(layer2);

			composition.SetReferenceLayer(2);
			buttonMap[i]->xmsi_file->ReplaceComposition(composition);
			string temp_xmso_filename = buttonMap[i]->temp_xmsi_filename;
			temp_xmso_filename.replace(temp_xmso_filename.end()-1,temp_xmso_filename.end(), "o");
			buttonMap[i]->xmsi_file->SetOutputFile(temp_xmso_filename);
			buttonMap[i]->xmsi_file->SetFilename(buttonMap[i]->temp_xmsi_filename);
		}


		delete xmsi_file;
	}
	catch (xmlpp::validity_error &e) {
		std::cerr << "Error message while checking document validity: " << e.what() << endl;
		return;
	} 
	catch (const std::exception &e) {
		std::cerr << "Error message while parsing: " << e.what() << endl;
		return;
	} 
	catch (...) {
		std::cerr << "Some other exception caught" << endl;
		return;
	}
	//ok, so now everything is processed, add it to the view
	update_phis();

	save_action->set_enabled();
	multiple_add_action->set_enabled(true);
	
}

void Window::multiple_add() {

	Gtk::Dialog *dialog = new Gtk::Dialog("Select elements to add", *this, true);

	dialog->add_button("Ok", Gtk::RESPONSE_OK);
	dialog->add_button("Cancel", Gtk::RESPONSE_CANCEL);
	Gtk::Box *vbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
	Gtk::Box *hbox; 
	Gtk::Label *label;

	hbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
	label = Gtk::manage(new Gtk::Label("From"));
	hbox->pack_start(*label, false, false, 3);

	combo_from = Gtk::manage(new Gtk::ComboBoxText);

	combo_to= Gtk::manage(new Gtk::ComboBoxText);

	for (int Z = 1 ; Z <= 93 ; Z++) {
		char *element = AtomicNumberToSymbol(Z);
		combo_from->append(element);
		xrlFree(element);
	}
	combo_from->set_active(0);
	hbox->pack_end(*combo_from, false, false, 3);
	vbox->pack_start(*hbox, false, false, 5);

	hbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
	label = Gtk::manage(new Gtk::Label("To"));
	hbox->pack_start(*label, false, false, 3);

	for (int Z = 2 ; Z <= 94 ; Z++) {
		char *element = AtomicNumberToSymbol(Z);
		combo_to->append(element);
		xrlFree(element);
	}
	combo_to->set_active(92);
	hbox->pack_end(*combo_to, false, false, 3);
	vbox->pack_start(*hbox, false, false, 5);
	dialog->get_content_area()->pack_start(*vbox, true, false, 5);
	dialog->show_all_children();
	combo_from->signal_changed().connect(sigc::mem_fun(*this, &Window::on_combo_from_changed));

	int result = dialog->run();
	if (result == Gtk::RESPONSE_OK) {
		//read values from comboboxes
		Glib::ustring element_from = combo_from->get_active_text();
		Glib::ustring element_to = combo_to->get_active_text();
		std::vector<MendeleevButton*> buttonVectorAll;
		buttonVectorAll.reserve(buttonVectorXMSO.size() + buttonVectorASR.size());
		buttonVectorAll.insert(buttonVectorAll.end(), buttonVectorXMSO.begin(), buttonVectorXMSO.end());
		buttonVectorAll.insert(buttonVectorAll.end(), buttonVectorASR.begin(), buttonVectorASR.end());
		for (int Z = SymbolToAtomicNumber((char*) element_from.c_str()) ; Z <= SymbolToAtomicNumber((char*) element_to.c_str()) ; Z++) {
			bool match = false;
			//check if it's not already in buttonVectorXMSO
			for (std::vector<MendeleevButton*>::iterator it = buttonVectorAll.begin() ; it != buttonVectorAll.end() ; ++it) {
				if ((*it)->GetZ() == Z) {
					match = true;
					break;
				}
			}
			if (!match) {
				buttonVectorXMSO.push_back(buttonMap[Z]);
				buttonMap[Z]->SetGreen();
			}
		}
		launch_action->set_enabled();
	}
	delete dialog;

}

void Window::launch_simulations() {
	std::vector<MendeleevButton*> buttonVectorXMSO_added;
	
	for (std::vector<MendeleevButton*>::iterator it = buttonVectorXMSO.begin() ; it != buttonVectorXMSO.end() ; ++it) {
		if (!(*it)->xmso_file)
			buttonVectorXMSO_added.push_back(*it);
	}

	xmi_msim_dialog = new XmiMsimDialog(*this, true, buttonVectorXMSO_added);
	int result;
	try {
		result = xmi_msim_dialog->run();
	}
	catch (BAM::Exception &e) {
		delete xmi_msim_dialog;
		Gtk::MessageDialog dialog(*this, "Error occurred in XMI-MSIM dialog", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
		Glib::ustring error_message = Glib::ustring(e.what());
		std::cout << e.what() << std::endl;
  		dialog.set_secondary_text(error_message);
  		dialog.run();
		//reset everything in window!
		xmi_msim_dialog = 0;
		//reset only those that were added as XMSO
		//reset_project();
		return;
	}
	if (result == Gtk::RESPONSE_DELETE_EVENT) {
		//delete event caught
		delete xmi_msim_dialog;
		xmi_msim_dialog = 0;
		//reset_project();
		return;
	}
	else {
		//check if simulation was not stopped
		for (std::vector<MendeleevButton *>::iterator it = buttonVectorXMSO.begin() ; it != buttonVectorXMSO.end(); ++it) {
			if (!(*it)->xmso_file) {
				delete xmi_msim_dialog;
				//reset_project();
				return;
			}
		}
	}
	delete xmi_msim_dialog;
	launch_action->set_enabled(false);
	multiple_add_action->set_enabled(true);

}

void Window::on_combo_from_changed() {
	cout << "on_combo_from_changed" << endl;
	//when combo_from is changed, combo_to may need to be updated
	const char *element_from = combo_from->get_active_text().c_str();
	int Z_from = SymbolToAtomicNumber((char *) element_from);
	const char *element_to = combo_to->get_active_text().c_str();
	int Z_to = SymbolToAtomicNumber((char *) element_to);

	combo_to->remove_all();
	for (int Z = Z_from+1 ; Z <= 94 ; Z++) {
		char *element = AtomicNumberToSymbol(Z);
		combo_to->append(element);
		xrlFree(element);
	}
	if (Z_to <= Z_from) {
		combo_to->set_active_text("Pu");
	}
	else {
		char *element_to2 = AtomicNumberToSymbol(Z_to);
		combo_to->set_active_text(element_to2);
		xrlFree((void*) element_to2);
	}
}
