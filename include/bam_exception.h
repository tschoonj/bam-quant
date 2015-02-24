#ifndef BAM_EXCEPTION_H 
#define BAM_EXCEPTION_H 

#include <string>
#include <exception>

namespace BAM {
	class Exception: public std::exception {
	private:
		const char *Message;  
	public:
		// constructors
		Exception(const char *ch)  {Message=ch;}
		Exception(std::string s)  {Message=s.c_str();}
		// throw method
		virtual const char* what() const throw() {
			return Message;
		}
	};
}
#endif
