#ifndef SA_H_
#define SA_H_

#include "shared/common.h"
#include "shared/hash.h"
#include <cstdio>
#include <vector>

using namespace std;
using namespace shared;

namespace sa {
    
/*SA*/
    
class SA : public Index {
protected:
	unsigned int *sa;
        unsigned int *alignedSa;
        unsigned int saLen;
        unsigned char *text;
        unsigned char *alignedText;
        unsigned int textLen;
	HT *ht = NULL;
        
        int type;
        
        void (*binarySearchOperation)(unsigned int *, unsigned char *, unsigned int, unsigned int, unsigned char *, int, unsigned int &, unsigned int &) = NULL;
        unsigned int (SA::*countOperation)(unsigned char *, unsigned int) = NULL;
        void (SA::*locateOperation)(unsigned char *, unsigned int, vector<unsigned int> &) = NULL;

	void freeMemory();
	void initialize();
        void setType(int indexType);
	void setFunctions();
        void getSA(const char *textFileName);
        void loadText(const char *textFileName);
        unsigned int count_no_hash(unsigned char *pattern, unsigned int patternLen);
        unsigned int count_hash(unsigned char *pattern, unsigned int patternLen);
        void locate_no_hash(unsigned char* pattern, unsigned int patternLen, vector<unsigned int>& res);
        void locate_hash(unsigned char* pattern, unsigned int patternLen, vector<unsigned int>& res);
        
public:
        enum IndexType {
		STANDARD = 1,
                DBL = 3
	};
        
	SA() {
		this->initialize();
                this->setType(SA::STANDARD);
		this->setFunctions();
	}

	SA(SA::IndexType indexType) {
		this->initialize();
                this->setType(indexType);
		this->setFunctions();
	}

	SA(SA::IndexType indexType, HT::HTType hTType, unsigned int k, double loadFactor) {
		this->initialize();
                this->setType(indexType);
                this->ht = new HT(hTType, k, loadFactor);
		this->setFunctions();
	}

	~SA() {
		this->freeMemory();
                if (this->ht != NULL) delete this->ht;
	}

	void build(const char *textFileName);
	void save(const char *fileName);
	void load(const char *fileName);
	void free();
	unsigned int getIndexSize();
	unsigned int getTextSize();
        string getParamsString();

	unsigned int count(unsigned char *pattern, unsigned int patternLen);
	void locate(unsigned char* pattern, unsigned int patternLen, vector<unsigned int>& res);
};

/*SALut2*/

class SALut2 : public SA {
private:
        alignas(128) unsigned int lut2[256][256][2];
        
public:
	SALut2() {
		this->initialize();
                this->setType(SA::STANDARD);
		this->setFunctions();
	}

	SALut2(SA::IndexType indexType) {
		this->initialize();
                this->setType(indexType);
		this->setFunctions();
	}
        
        void build(const char *textFileName);
        void save(const char *fileName);
	void load(const char *fileName);
        unsigned int getIndexSize();
        string getParamsString();
        
        unsigned int count(unsigned char *pattern, unsigned int patternLen);
	void locate(unsigned char* pattern, unsigned int patternLen, vector<unsigned int>& res);
};

/*SHARED STUFF*/

void binarySearchDbl(unsigned int *sa, unsigned char *text, unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end);
void binarySearchDblAStrcmp(unsigned int *sa, unsigned char *text, unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end);
void binarySearchDblStrncmp(unsigned int *sa, unsigned char *text, unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end);

}

#endif /* SA_H_ */
