#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <sstream>
#include "libs/asmlib.h"
#include "shared/sais.h"
#include "sa.h"

using namespace std;

namespace sa {

/*SA*/
    
void SA::setType(int indexType) {
	if (indexType != SA::STANDARD && indexType != SA::DBL) {
		cout << "Error: not valid index type" << endl;
		exit(1);
	}
	this->type = indexType;
}

void SA::setFunctions() {
        switch (this->type) {
		case SA::STANDARD:
                        this->binarySearchOperation = &binarySearch;
			break;
                case SA::DBL:
                        this->binarySearchOperation = &binarySearchDbl;
			break;
		default:
			cout << "Error: not valid index type" << endl;
			exit(1);
        }
        if (this->ht != NULL) {
            this->countOperation = &SA::count_hash;
            this->locateOperation = &SA::locate_hash;
	}
        else {
            this->countOperation = &SA::count_no_hash;
            this->locateOperation = &SA::locate_no_hash;
        }
}

void SA::free() {
	this->freeMemory();
	this->initialize();
}

void SA::initialize() {
	this->sa = NULL;
	this->alignedSa = NULL;
	this->saLen = 0;
        this->text = NULL;
	this->alignedText = NULL;
        this->textLen = 0;
}

void SA::freeMemory() {
	if (this->sa != NULL) delete[] this->sa;
        if (this->text != NULL) delete[] this->text;
	if (this->ht != NULL) this->ht->free();
}

void SA::getSA(const char *textFileName) {
        stringstream ss;
	ss << "SA-" << textFileName << ".dat";
	string s = ss.str();
	char *saFileName = (char *)(s.c_str());
        this->saLen = this->textLen + 1;
        this->sa = new unsigned int[this->saLen + 32];
        this->alignedSa = this->sa;
        while ((unsigned long long)this->alignedSa % 128) ++this->alignedSa;
        if (!fileExists(saFileName)) {
                if (this->verbose) cout << "Building SA ... " << flush;
                this->alignedSa[0] = this->textLen;
                ++this->alignedSa;
                sais(this->alignedText, (int *)this->alignedSa, this->textLen);
                --this->alignedSa;
                if (verbose) cout << "Done" << endl;
                if (verbose) cout << "Saving SA in " << saFileName << " ... " << flush;
                FILE *outFile;
                outFile = fopen(saFileName, "w");
                fwrite(this->alignedSa, (size_t)(sizeof(unsigned int)), (size_t)this->saLen, outFile);
                fclose(outFile);
        } 
        else {
                if (verbose) cout << "Loading SA from " << saFileName << " ... " << flush;
                FILE *inFile;
                inFile = fopen(saFileName, "rb");
                size_t result = fread(this->alignedSa, (size_t)sizeof(unsigned int), (size_t)this->saLen, inFile);
                if (result != this->saLen) {
                        cout << "Error loading SA from " << textFileName << endl;
                        exit(1);
                }
                fclose(inFile);
        }
        if (verbose) cout << "Done" << endl;	
}

void SA::loadText(const char *textFileName) {
        if (this->verbose) cout << "Loading text ... " << flush;
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
        if (this->verbose) cout << "Done" << endl;
}

void SA::build(const char *textFileName) {
	this->free();
        this->loadText(textFileName);
        this->getSA(textFileName);
        
        if (this->ht != NULL) {
                if (this->verbose) cout << "Building hash table ... " << flush;
		this->ht->build(this->alignedText, this->textLen, this->alignedSa, this->saLen);
		if (this->verbose) cout << "Done" << endl;
	}
}

unsigned int SA::getIndexSize() {
	unsigned int size = sizeof(this->type) + sizeof(this->saLen) + sizeof(this->textLen) + sizeof(this->ht);
        if (this->saLen > 0) size += (this->saLen + 32) * sizeof(unsigned int);
        if (this->textLen > 0) size += (this->textLen + 128 + 1) * sizeof(unsigned char);
        if (this->ht != NULL) size += this->ht->getHTSize();
	return size;
}

unsigned int SA::getTextSize() {
	return this->textLen * sizeof(unsigned char);
}

string SA::getParamsString() {
        ostringstream oss;
        oss << "SA";
        switch(this->type) {
            case SA::STANDARD:
                oss << " standard";
                break;
            case SA::DBL:
                oss << " dbl";
                break;
        }
        if (this->ht != NULL) {
            switch(this->ht->type) {
                case HT::STANDARD:
                    oss << " hash";
                    break;
                case HT::DENSE:
                    oss << " hash-dense";
                    break;
            }
            oss << " " << this->ht->k << " " << this->ht->loadFactor;
        }
        return oss.str();
}

unsigned int SA::count_no_hash(unsigned char *pattern, unsigned int patternLen) {
        unsigned int beg, end;
	(this->binarySearchOperation)(this->alignedSa, this->alignedText, 0, this->saLen, pattern, patternLen, beg, end);
	return end - beg;
}

void SA::locate_no_hash(unsigned char* pattern, unsigned int patternLen, vector<unsigned int>& res) {
        unsigned int beg, end;
	(this->binarySearchOperation)(this->alignedSa, this->alignedText, 0, this->saLen, pattern, patternLen, beg, end);
        res.insert(res.end(), this->alignedSa + beg, this->alignedSa + end);
}

unsigned int SA::count_hash(unsigned char *pattern, unsigned int patternLen) {
        if (patternLen < this->ht->k) return this->count_no_hash(pattern, patternLen);
        unsigned int leftBoundary, rightBoundary, beg, end;
        this->ht->getBoundaries(pattern, this->alignedText, this->alignedSa, leftBoundary, rightBoundary);
        (this->binarySearchOperation)(this->alignedSa, this->alignedText, leftBoundary, rightBoundary, pattern, patternLen, beg, end);
        return end - beg;
}

void SA::locate_hash(unsigned char* pattern, unsigned int patternLen, vector<unsigned int>& res) {
        if (patternLen < this->ht->k) this->locate_no_hash(pattern, patternLen, res);
        else {
            unsigned int leftBoundary, rightBoundary, beg, end;
            this->ht->getBoundaries(pattern, this->alignedText, this->alignedSa, leftBoundary, rightBoundary);
            (this->binarySearchOperation)(this->alignedSa, this->alignedText, leftBoundary, rightBoundary, pattern, patternLen, beg, end);
            res.insert(res.end(), this->alignedSa + beg, this->alignedSa + end);
        }
}

unsigned int SA::count(unsigned char *pattern, unsigned int patternLen) {
	return (this->*countOperation)(pattern, patternLen);
}

void SA::locate(unsigned char* pattern, unsigned int patternLen, vector<unsigned int>& res) {
	(this->*locateOperation)(pattern, patternLen, res);
}

void SA::save(const char *fileName) {
	if (this->verbose) cout << "Saving index in " << fileName << " ... " << flush;
	bool nullPointer = false;
	bool notNullPointer = true;
	FILE *outFile;
	outFile = fopen(fileName, "w");
	fwrite(&this->verbose, (size_t)sizeof(bool), (size_t)1, outFile);
        fwrite(&this->type, (size_t)sizeof(int), (size_t)1, outFile);
	fwrite(&this->saLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	if (this->saLen > 0) fwrite(this->alignedSa, (size_t)sizeof(unsigned int), (size_t)this->saLen, outFile);
        fwrite(&this->textLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	if (this->textLen > 0) fwrite(this->alignedText, (size_t)sizeof(unsigned char), (size_t)this->textLen, outFile);
        if (this->ht == NULL) fwrite(&nullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
        else {
                fwrite(&notNullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
                this->ht->save(outFile);
        }
	fclose(outFile);
	if (this->verbose) cout << "Done" << endl;
}

void SA::load(const char *fileName) {
	this->free();
        if (this->ht != NULL) {
                delete this->ht;
                this->ht = NULL;
        }
	bool isNotNullPointer;
	FILE *inFile;
	inFile = fopen(fileName, "rb");
	size_t result;
	result = fread(&this->verbose, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->verbose) cout << "Loading index from " << fileName << " ... " << flush;
        result = fread(&this->type, (size_t)sizeof(int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->saLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->saLen > 0) {
		this->sa = new unsigned int[this->saLen + 32];
		this->alignedSa = this->sa;
		while ((unsigned long long)this->alignedSa % 128) ++this->alignedSa;
		result = fread(this->alignedSa, (size_t)sizeof(unsigned int), (size_t)this->saLen, inFile);
		if (result != this->saLen) {
			cout << "Error loading index from " << fileName << endl;
			exit(1);
		}
	}
        result = fread(&this->textLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->textLen > 0) {
		this->text = new unsigned char[this->textLen + 128 + 1];
		this->alignedText = this->text;
		while ((unsigned long long)this->alignedText % 128) ++this->alignedText;
		result = fread(this->alignedText, (size_t)sizeof(unsigned char), (size_t)this->textLen, inFile);
                this->alignedText[this->textLen] = '\0';
		if (result != this->textLen) {
			cout << "Error loading index from " << fileName << endl;
			exit(1);
		}
	}
	result = fread(&isNotNullPointer, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (isNotNullPointer) {
                this->ht = new HT();
                this->ht->load(inFile);
	}
	fclose(inFile);
	this->setFunctions();
	if (this->verbose) cout << "Done" << endl;
}

/*SALut2*/

void SALut2::build(const char *textFileName) {
        SA::build(textFileName);
        fillLUT2(this->lut2, this->alignedText, this->alignedSa, this->saLen);
}

unsigned int SALut2::getIndexSize() {
	unsigned int size = SA::getIndexSize();
        size += 256 * 256 * 2 * sizeof(unsigned int);
        return size;
}

string SALut2::getParamsString() {
        ostringstream oss;
        oss << "SALut2";
        switch(this->type) {
            case SA::STANDARD:
                oss << " standard";
                break;
            case SA::DBL:
                oss << " dbl";
                break;
        }
        return oss.str();
}

void SALut2::save(const char *fileName) {
        if (this->verbose) cout << "Saving index in " << fileName << " ... " << flush;
	FILE *outFile;
	outFile = fopen(fileName, "w");
	fwrite(&this->verbose, (size_t)sizeof(bool), (size_t)1, outFile);
        fwrite(&this->type, (size_t)sizeof(int), (size_t)1, outFile);
	fwrite(&this->saLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	if (this->saLen > 0) fwrite(this->alignedSa, (size_t)sizeof(unsigned int), (size_t)this->saLen, outFile);
        fwrite(&this->textLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	if (this->textLen > 0) fwrite(this->alignedText, (size_t)sizeof(unsigned char), (size_t)this->textLen, outFile);
        fwrite(&this->lut2, (size_t)sizeof(unsigned int), (size_t)(256 * 256 * 2), outFile);
	fclose(outFile);
	if (this->verbose) cout << "Done" << endl;
}

void SALut2::load(const char *fileName) {
        this->free();
        if (this->ht != NULL) {
                delete this->ht;
                this->ht = NULL;
        }
	FILE *inFile;
	inFile = fopen(fileName, "rb");
	size_t result;
	result = fread(&this->verbose, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->verbose) cout << "Loading index from " << fileName << " ... " << flush;
        result = fread(&this->type, (size_t)sizeof(int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->saLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->saLen > 0) {
		this->sa = new unsigned int[this->saLen + 32];
		this->alignedSa = this->sa;
		while ((unsigned long long)this->alignedSa % 128) ++this->alignedSa;
		result = fread(this->alignedSa, (size_t)sizeof(unsigned int), (size_t)this->saLen, inFile);
		if (result != this->saLen) {
			cout << "Error loading index from " << fileName << endl;
			exit(1);
		}
	}
        result = fread(&this->textLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->textLen > 0) {
		this->text = new unsigned char[this->textLen + 128 + 1];
		this->alignedText = this->text;
		while ((unsigned long long)this->alignedText % 128) ++this->alignedText;
		result = fread(this->alignedText, (size_t)sizeof(unsigned char), (size_t)this->textLen, inFile);
                this->alignedText[this->textLen] = '\0';
		if (result != this->textLen) {
			cout << "Error loading index from " << fileName << endl;
			exit(1);
		}
	}
	result = fread(this->lut2, (size_t)sizeof(unsigned int), (size_t)(256 * 256 * 2), inFile);
	if (result != (256 * 256 * 2)) {
		cout << "Error loading index" << endl;
		exit(1);
	}
	fclose(inFile);
	this->setFunctions();
	if (this->verbose) cout << "Done" << endl;
}

unsigned int SALut2::count(unsigned char *pattern, unsigned int patternLen) {
        if (patternLen < 2) return this->count_no_hash(pattern, patternLen);
        unsigned int leftBoundaryLUT2 = this->lut2[pattern[0]][pattern[1]][0];
	unsigned int rightBoundaryLUT2 = this->lut2[pattern[0]][pattern[1]][1];
	if (leftBoundaryLUT2 < rightBoundaryLUT2) {
                unsigned int beg, end;
                (this->binarySearchOperation)(this->alignedSa, this->alignedText, leftBoundaryLUT2, rightBoundaryLUT2, pattern, patternLen, beg, end);
                return end - beg;
	} else return 0;
}

void SALut2::locate(unsigned char* pattern, unsigned int patternLen, vector<unsigned int>& res) {
	if (patternLen < 2) {
            this->locate_no_hash(pattern, patternLen, res);
        } else {
            unsigned int leftBoundaryLUT2 = this->lut2[pattern[0]][pattern[1]][0];
            unsigned int rightBoundaryLUT2 = this->lut2[pattern[0]][pattern[1]][1];
            if (leftBoundaryLUT2 < rightBoundaryLUT2) {
                    unsigned int beg, end;
                    (this->binarySearchOperation)(this->alignedSa, this->alignedText, leftBoundaryLUT2, rightBoundaryLUT2, pattern, patternLen, beg, end);
                    res.insert(res.end(), this->alignedSa + beg, this->alignedSa + end);
            }
        }
}

/*SHARED STUFF*/

void binarySearchDbl(unsigned int *sa, unsigned char *text, unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end) {
	if (pattern[patternLength - 1] == 255) binarySearchDblStrncmp(sa, text, lStart, rStart, pattern, patternLength, beg, end);
	else binarySearchDblAStrcmp(sa, text, lStart, rStart, pattern, patternLength, beg, end);
}

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

}