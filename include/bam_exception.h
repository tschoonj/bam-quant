#ifndef BAM_EXCEPTION_H 
#define BAM_EXCEPTION_H 

#include <string>
#include <exception>

using namespace std;

namespace BAM {
	class bam_exception: public exception {
		private:
			const char *Message;  
		public:
			// constructors
			bam_exception(const char *ch)  {Message=ch;}
			bam_exception(string s)  {Message=s.c_str();}
			// throw method
			virtual const char* what() const throw() {
				return Message;
			}
	};
}
#endif
