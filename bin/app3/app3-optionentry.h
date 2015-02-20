#ifndef APP3_OPTION_ENTRY
#define APP3_OPTION_ENTRY

#include <glibmm/ustring.h>
#include <glibmm/optionentry.h>

class App3OptionEntry : public Glib::OptionEntry {
public:
	App3OptionEntry(const Glib::ustring& long_name, gchar short_name= '\0', const Glib::ustring& description=Glib::ustring(), const Glib::ustring& arg_description=Glib::ustring(), int flags=0) {
		set_long_name(long_name);
		set_short_name(short_name);
		set_description(description);
		set_arg_description(arg_description);
		set_flags(flags);
	}
	

};

#endif
