#ifndef BAM_EXCEPTION_H 
#define BAM_EXCEPTION_H 

#include <string>
#include <exception>

namespace BAM {
	class Exception: public std::exception {
	private:
		std::string Message;  
	public:
		// constructors
		//explicit Exception(const char *ch) : Message(ch) {}
		explicit Exception(const std::string &s) : Message(s) {}
		virtual ~Exception() throw() {};
		// throw method
		virtual const char* what() const throw() {
			return Message.c_str();
		}
	};
}
#endif
