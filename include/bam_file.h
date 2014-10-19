#ifndef BAM_FILE_H
#define BAM_FILE_H

#include <fstream>

using namespace std;

namespace BAM {
	class File {
		private:
		protected:
			ifstream fs;
			string filename;
		public:
		//constructor
		File(string);
		//destructor
		virtual ~File();
		void Open();
		void Close();
		virtual void Parse() = 0;
	};
}
#endif
