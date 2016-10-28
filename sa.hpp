#ifndef SA_HPP
#define	SA_HPP

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <sstream>
#include <cstdio>
#include "libs/asmlib.h"
#include "shared/sais.h"
#include "shared/common.hpp"
#include "shared/hash.hpp"

using namespace std;
using namespace shared;

namespace sa {

enum SAType {
    SA_STANDARD = 1,
    SA_DBL = 2
};

void binarySearchDblAStrcmp(unsigned int *sa, unsigned char *text, unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end);
void binarySearchDblStrncmp(unsigned int *sa, unsigned char *text, unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end);
void binarySearchDbl(unsigned int *sa, unsigned char *text, unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end);
    
template<SAType T> class SA {
protected:
	unsigned int *sa;
        unsigned int *alignedSa;
        unsigned int saLen;
        unsigned char *text;
        unsigned char *alignedText;
        unsigned int textLen;

	void freeMemory() {
            if (this->sa != NULL) delete[] this->sa;
            if (this->text != NULL) delete[] this->text;
        }
        
	void initialize() {
            this->sa = NULL;
            this->alignedSa = NULL;
            this->saLen = 0;
            this->text = NULL;
            this->alignedText = NULL;
            this->textLen = 0;
        }
        
        void getSA(const char *textFileName) {
            stringstream ss;
            ss << "SA32-" << textFileName << ".dat";
            string s = ss.str();
            char *saFileName = (char *)(s.c_str());
            this->saLen = this->textLen + 1;
            this->sa = new unsigned int[this->saLen + 32];
            this->alignedSa = this->sa;
            while ((unsigned long long)this->alignedSa % 128) ++this->alignedSa;
            if (!fileExists(saFileName)) {
                cout << "Building SA32 ... " << flush;
                this->alignedSa[0] = this->textLen;
                ++this->alignedSa;
                sais(this->alignedText, this->alignedSa, this->textLen);
                --this->alignedSa;
                cout << "Done" << endl;
                cout << "Saving SA32 in " << saFileName << " ... " << flush;
                FILE *outFile;
                outFile = fopen(saFileName, "w");
                fwrite(this->alignedSa, (size_t)(sizeof(unsigned int)), (size_t)this->saLen, outFile);
                fclose(outFile);
            } 
            else {
                cout << "Loading SA32 from " << saFileName << " ... " << flush;
                FILE *inFile;
                inFile = fopen(saFileName, "rb");
                size_t result = fread(this->alignedSa, (size_t)sizeof(unsigned int), (size_t)this->saLen, inFile);
                if (result != this->saLen) {
                        cout << "Error loading SA32 from " << textFileName << endl;
                        exit(1);
                }
                fclose(inFile);
            }
            cout << "Done" << endl;	
        }
        
        void loadText(const char *textFileName) {
            cout << "Loading text ... " << flush;
            this->textLen = getFileSize(textFileName, sizeof(unsigned char));
            this->text = new unsigned char[this->textLen + 128 + 1];
            this->alignedText = this->text;
            while ((unsigned long long)this->alignedText % 128) ++this->alignedText;
            FILE *inFile;
            inFile = fopen(textFileName, "rb");
            size_t result = fread(this->alignedText, (size_t)sizeof(unsigned char), (size_t)this->textLen, inFile);
            this->alignedText[this->textLen] = '\0';
            if (result != this->textLen) {
                    cout << "Error loading text from " << textFileName << endl;
                    exit(1);
            }
            fclose(inFile);
            checkNullChar(this->alignedText, this->textLen);
            cout << "Done" << endl;
        }
        
public:
	SA() {
		this->initialize();
	}

	~SA() {
		this->free();
	}

	void build(const char *textFileName) {
            this->free();
            this->loadText(textFileName);
            this->getSA(textFileName);
        }
        
        void save(FILE *outFile) {
            fwrite(&this->saLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
            if (this->saLen > 0) fwrite(this->alignedSa, (size_t)sizeof(unsigned int), (size_t)this->saLen, outFile);
            fwrite(&this->textLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
            if (this->textLen > 0) fwrite(this->alignedText, (size_t)sizeof(unsigned char), (size_t)this->textLen, outFile);
        }
        
	void save(const char *fileName) {
            cout << "Saving index in " << fileName << " ... " << flush;
            FILE *outFile = fopen(fileName, "w");
            this->save(outFile);
            fclose(outFile);
            cout << "Done" << endl;
        }
        
	void load(FILE *inFile) {
            this->free();
            size_t result = fread(&this->saLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
            if (result != 1) {
                    cout << "Error loading index" << endl;
                    exit(1);
            }
            if (this->saLen > 0) {
                    this->sa = new unsigned int[this->saLen + 32];
                    this->alignedSa = this->sa;
                    while ((unsigned long long)this->alignedSa % 128) ++this->alignedSa;
                    result = fread(this->alignedSa, (size_t)sizeof(unsigned int), (size_t)this->saLen, inFile);
                    if (result != this->saLen) {
                            cout << "Error loading index" << endl;
                            exit(1);
                    }
            }
            result = fread(&this->textLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
            if (result != 1) {
                    cout << "Error loading index" << endl;
                    exit(1);
            }
            if (this->textLen > 0) {
                    this->text = new unsigned char[this->textLen + 128 + 1];
                    this->alignedText = this->text;
                    while ((unsigned long long)this->alignedText % 128) ++this->alignedText;
                    result = fread(this->alignedText, (size_t)sizeof(unsigned char), (size_t)this->textLen, inFile);
                    this->alignedText[this->textLen] = '\0';
                    if (result != this->textLen) {
                            cout << "Error loading index" << endl;
                            exit(1);
                    }
            }
        }
        
        void load(const char *fileName) {
            FILE *inFile = fopen(fileName, "rb");
            cout << "Loading index from " << fileName << " ... " << flush;
            this->load(inFile);
            fclose(inFile);
            cout << "Done" << endl;
        }
        
	void free() {
            this->freeMemory();
            this->initialize();
        }
        
	unsigned int getIndexSize() {
            unsigned int size = sizeof(this->saLen) + sizeof(this->textLen);
            if (this->saLen > 0) size += (this->saLen + 32) * sizeof(unsigned int);
            if (this->textLen > 0) size += (this->textLen + 128 + 1) * sizeof(unsigned char);
            return size;
        }
        
	unsigned int getTextSize() {
            return this->textLen * sizeof(unsigned char);
        }
        
        string getParamsString() {
            ostringstream oss;
            oss << "SA";
            switch(T) {
                    case SAType::SA_DBL:
                            oss << " dbl";
                            break;
                    default:
                            oss << " standard";
                            break;
            }
            return oss.str();
        }

	unsigned int count(unsigned char *pattern, unsigned int patternLen) {
            unsigned int beg, end;
            switch(T) {
                    case SAType::SA_DBL:
                            binarySearchDbl(this->alignedSa, this->alignedText, 0, this->saLen, pattern, patternLen, beg, end);
                            break;
                    default:
                            binarySearch(this->alignedSa, this->alignedText, 0U, this->saLen, pattern, patternLen, beg, end);
                            break;
            }
            return end - beg;
        }
        
	void locate(unsigned char* pattern, unsigned int patternLen, vector<unsigned int>& res) {
            unsigned int beg, end;
            switch(T) {
                    case SAType::SA_DBL:
                            binarySearchDbl(this->alignedSa, this->alignedText, 0, this->saLen, pattern, patternLen, beg, end);
                            break;
                    default:
                            binarySearch(this->alignedSa, this->alignedText, 0U, this->saLen, pattern, patternLen, beg, end);
                            break;
            }
            res.insert(res.end(), this->alignedSa + beg, this->alignedSa + end);
        }
};

template<SAType T, HTType HASHTYPE> class SAHash : public SA<T> {
protected:
	HT32<HASHTYPE> *ht = NULL;

	void freeMemory() {
            SA<T>::freeMemory();
            if (this->ht != NULL) this->ht->free();
        }
              
public:
	SAHash(unsigned int k, double loadFactor) {
		this->initialize();
                this->ht = new HT32<HASHTYPE>(k, loadFactor);
	}

	~SAHash() {
                this->free();
                delete this->ht;
	}

	void build(const char *textFileName) {
            SA<T>::build(textFileName);
            cout << "Building hash table ... " << flush;
            this->ht->build(this->alignedText, this->textLen, this->alignedSa, this->saLen);
            cout << "Done" << endl;
        }
        
        void save(FILE *outFile) {
            SA<T>::save(outFile);
            this->ht->save(outFile);
        }
        
	void save(const char *fileName) {
            cout << "Saving index in " << fileName << " ... " << flush;
            FILE *outFile = fopen(fileName, "w");
            this->save(outFile);
            fclose(outFile);
            cout << "Done" << endl;
        }
        
	void load(FILE *inFile) {
            SA<T>::load(inFile);
            delete this->ht;
            this->ht = new HT32<HASHTYPE>();
            this->ht->load(inFile);
        }
        
        void load(const char *fileName) {
            FILE *inFile = fopen(fileName, "rb");
            cout << "Loading index from " << fileName << " ... " << flush;
            this->load(inFile);
            fclose(inFile);
            cout << "Done" << endl;
        }
        
	void free() {
            this->freeMemory();
            this->initialize();
        }
        
	unsigned int getIndexSize() {
            return SA<T>::getIndexSize() + sizeof(this->ht) + this->ht->getHTSize();
        }
        
        string getParamsString() {
            ostringstream oss;
            oss << "SA";
            switch(T) {
                case SAType::SA_DBL:
                    oss << " dbl";
                    break;
                default:
                    oss << " standard";
                    break;
            }
            switch(HASHTYPE) {
                case HTType::HT_DENSE:
                    oss << " hash-dense";
                    break;
                default:
                    oss << " hash";
                    break;
            }
            oss << " " << this->ht->k << " " << this->ht->loadFactor;
            return oss.str();
        }
        
	unsigned int count(unsigned char *pattern, unsigned int patternLen) {
            if (patternLen < this->ht->k) return SA<T>::count(pattern, patternLen);
            unsigned int leftBoundary, rightBoundary, beg, end;
            this->ht->getBoundaries(pattern, this->alignedText, this->alignedSa, leftBoundary, rightBoundary);
            switch(T) {
                case SAType::SA_DBL:
                    binarySearchDbl(this->alignedSa, this->alignedText, leftBoundary, rightBoundary, pattern, patternLen, beg, end);
                    break;
                default:
                    binarySearch(this->alignedSa, this->alignedText, leftBoundary, rightBoundary, pattern, patternLen, beg, end);
                    break;
            }
            return end - beg;
        }
        
	void locate(unsigned char* pattern, unsigned int patternLen, vector<unsigned int>& res) {
            if (patternLen < this->ht->k) SA<T>::locate(pattern, patternLen, res);
            else {
                unsigned int leftBoundary, rightBoundary, beg, end;
                this->ht->getBoundaries(pattern, this->alignedText, this->alignedSa, leftBoundary, rightBoundary);
                switch(T) {
                    case SAType::SA_DBL:
                        binarySearchDbl(this->alignedSa, this->alignedText, leftBoundary, rightBoundary, pattern, patternLen, beg, end);
                        break;
                    default:
                        binarySearch(this->alignedSa, this->alignedText, leftBoundary, rightBoundary, pattern, patternLen, beg, end);
                        break;
                }
                res.insert(res.end(), this->alignedSa + beg, this->alignedSa + end);
            }
        }
};

template<SAType T> class SALut2 : public SA<T> {
private:
        alignas(128) unsigned int lut2[256][256][2];
        
public:
	SALut2() {
		this->initialize();
	}
        
        void build(const char *textFileName) {
            SA<T>::build(textFileName);
            fillLUT2(this->lut2, this->alignedText, this->alignedSa, this->saLen);
        }
        
        void save(FILE *outFile) {
            SA<T>::save(outFile);
            fwrite(&this->lut2, (size_t)sizeof(unsigned int), (size_t)(256 * 256 * 2), outFile);
        }
        
        void save(const char *fileName) {
            cout << "Saving index in " << fileName << " ... " << flush;
            FILE *outFile = fopen(fileName, "w");
            this->save(outFile);
            fclose(outFile);
            cout << "Done" << endl;
        }
        
	void load(FILE *inFile) {
            SA<T>::load(inFile);
            size_t result = fread(this->lut2, (size_t)sizeof(unsigned int), (size_t)(256 * 256 * 2), inFile);
            if (result != (256 * 256 * 2)) {
                    cout << "Error loading index" << endl;
                    exit(1);
            }
        }
        
        void load(const char *fileName) {
            FILE *inFile = fopen(fileName, "rb");
            cout << "Loading index from " << fileName << " ... " << flush;
            this->load(inFile);
            fclose(inFile);
            cout << "Done" << endl;
        }
        
        unsigned int getIndexSize() {
            unsigned int size = SA<T>::getIndexSize();
            size += 256 * 256 * 2 * sizeof(unsigned int);
            return size;
        }
        
        string getParamsString() {
            ostringstream oss;
            oss << "SALut2";
            switch(T) {
                case SAType::SA_DBL:
                    oss << " dbl";
                    break;
                default:
                    oss << " standard";
                    break;
            }
            return oss.str();
        }
        
        unsigned int count(unsigned char *pattern, unsigned int patternLen) {
            if (patternLen < 2) return SA<T>::count(pattern, patternLen);
            unsigned int leftBoundaryLUT2 = this->lut2[pattern[0]][pattern[1]][0];
            unsigned int rightBoundaryLUT2 = this->lut2[pattern[0]][pattern[1]][1];
            if (leftBoundaryLUT2 < rightBoundaryLUT2) {
                unsigned int beg, end;
                switch(T) {
                    case SAType::SA_DBL:
                        binarySearchDbl(this->alignedSa, this->alignedText, leftBoundaryLUT2, rightBoundaryLUT2, pattern, patternLen, beg, end);
                        break;
                    default:
                        binarySearch(this->alignedSa, this->alignedText, leftBoundaryLUT2, rightBoundaryLUT2, pattern, patternLen, beg, end);
                        break;
                }
                return end - beg;
            } else return 0;
        }
        
	void locate(unsigned char* pattern, unsigned int patternLen, vector<unsigned int>& res) {
            if (patternLen < 2) {
		SA<T>::locate(pattern, patternLen, res);
            } else {
                    unsigned int leftBoundaryLUT2 = this->lut2[pattern[0]][pattern[1]][0];
                    unsigned int rightBoundaryLUT2 = this->lut2[pattern[0]][pattern[1]][1];
                    if (leftBoundaryLUT2 < rightBoundaryLUT2) {
                        unsigned int beg, end;
                        switch(T) {
                            case SAType::SA_DBL:
                                binarySearchDbl(this->alignedSa, this->alignedText, leftBoundaryLUT2, rightBoundaryLUT2, pattern, patternLen, beg, end);
                                break;
                            default:
                                binarySearch(this->alignedSa, this->alignedText, leftBoundaryLUT2, rightBoundaryLUT2, pattern, patternLen, beg, end);
                                break;
                        }
                        res.insert(res.end(), this->alignedSa + beg, this->alignedSa + end);
                    }
            }
        }
};

void binarySearchDblAStrcmp(unsigned int *sa, unsigned char *text, unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end) {
    unsigned int l = lStart;
    unsigned int r = rStart;
    unsigned int mid;
    while (l < r) {
            mid = (l + r) / 2;
            if (A_strcmp((const char*)pattern, (const char*)(text + sa[mid])) > 0) {
                    l = mid + 1;
            }
            else {
                    r = mid;
            }
    }
    beg = l;
    unsigned int maxGap = rStart - beg;
    ++pattern[patternLength - 1];
    for (unsigned int i = 1; ; i <<= 1) {
            if (i >= maxGap) {
                    r = rStart;
                    break;
            }
            r = l + i;
            if (A_strcmp((const char*)pattern, (const char*)(text + sa[r])) <= 0) {
                    l += (i >> 1);
                    break;
            }
    }
    while (l < r) {
            mid = (l + r) / 2;
            if (A_strcmp((const char*)pattern, (const char*)(text + sa[mid])) <= 0) {
                    r = mid;
            }
            else {
                    l = mid + 1;
            }
    }
    --pattern[patternLength - 1];
    end = r;
}

void binarySearchDblStrncmp(unsigned int *sa, unsigned char *text, unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end) {
    unsigned int l = lStart;
    unsigned int r = rStart;
    unsigned int mid;
    while (l < r) {
            mid = (l + r) / 2;
            if (strncmp((const char*)pattern, (const char*)(text + sa[mid]), patternLength) > 0) {
                    l = mid + 1;
            }
            else {
                    r = mid;
            }
    }
    beg = l;
    unsigned int maxGap = rStart - beg;
    for (unsigned int i = 1; ; i <<= 1) {
            if (i >= maxGap) {
                    r = rStart;
                    break;
            }
            r = l + i;
            if (strncmp((const char*)pattern, (const char*)(text + sa[r]), patternLength) < 0) {
                    l += (i >> 1);
                    break;
            }
    }
    while (l < r) {
            mid = (l + r) / 2;
            if (strncmp((const char*)pattern, (const char*)(text + sa[mid]), patternLength) < 0) {
                    r = mid;
            }
            else {
                    l = mid + 1;
            }
    }
    end = r;
}

void binarySearchDbl(unsigned int *sa, unsigned char *text, unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end) {
    if (pattern[patternLength - 1] == 255) binarySearchDblStrncmp(sa, text, lStart, rStart, pattern, patternLength, beg, end);
    else binarySearchDblAStrcmp(sa, text, lStart, rStart, pattern, patternLength, beg, end);
}

}

#endif	/* SA_HPP */

