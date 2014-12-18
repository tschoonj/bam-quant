#ifndef BAM_DATA_ASR_H
#define BAM_DATA_ASR_H


using namespace std;

namespace BAM {
	namespace Data {
		class ASR {
			private:
				int Z;
				int line;
				double counts;
				double stddev;
				double chi;
				//double bg;
			public:
				ASR() : Z(0), line(0), counts(0.0), stddev(0), chi(0) {}
				ASR(int Z, int line, double counts, double stddev, double chi/*, double bg*/) :
					Z(Z), line(line), counts(counts), stddev(stddev), chi(chi)/*, bg(bg)*/ {}
				int GetZ() {
					return Z;
				}
				int GetLine() {
					return line;
				}
				double GetCounts() {
					return counts;
				}
				double GetStdDev() {
					return stddev;
				}
				double GetChi() {
					return chi;
				}
				/*double GetBG() {
					return bg;
				}*/
		};
	}
}
#endif
