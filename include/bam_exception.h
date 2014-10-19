#ifndef BAM_EXCEPTION_H 
#define BAM_EXCEPTION_H 

#include <string>
#include <exception>

using namespace std;

namespace BAM {
	class Exception: public exception {
		private:
			const char *Message;  
		public:
			// constructors
			Exception(const char *ch)  {Message=ch;}
			Exception(string s)  {Message=s.c_str();}
			// throw method
			virtual const char* what() const throw() {
				return Message;
			}
	};
}
#endif
