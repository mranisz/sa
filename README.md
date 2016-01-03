# SA (suffix array) variants

##What is it?
The library contains ...

The current version of SA variants handles only the count query (i.e., returns the number of occurrences of the given pattern).

##Requirements
The SA indexes require:
- C++11 ready compiler such as g++ version 4.7 or higher
- a 64-bit operating system
- text size is limited to 4GB

##Installation
To download and build the library use the following commands:
```
git clone https://github.com/mranisz/sa.git
cd sa
make
```

##Usage
To use the SA variants library:
- include "sa/sa.h" to your project
- compile it with "-std=c++11 -O3" options and link it with libraries:
  - sa/libsa.a
  - sa/libs/libaelf64.a (linux) or fbcsa/libs/libacof64.lib (windows)
- use "sa" namespace

##API
There are several functions you can call on each of the suffix array text index:
- **build** the index using the text:
```
void build(unsigned char* text, unsigned int textLen);
```
- **save** the index to file called fileName:
```
void save(const char *fileName);
```
- **load** the index from file called fileName:
```
void load(const char *fileName);
```
- **free** memory occupied by index:
```
void free();
```
- get the **index size** in bytes (size in memory):
```
unsigned int getIndexSize();
```
- get the size in bytes of the text used to build the index:
```
unsigned int getTextSize();
```
- get the result of **count** query:
```
unsigned int count(unsigned char *pattern, unsigned int patternLen);
```
- set **verbose** mode:
```
void setVerbose(bool verbose);
```

##SA

Parameters:
- indexType:
      - SA::STANDARD (default)
      - SA::PLUS - efficient for longer patterns (longer than 20)

Constructors:
```
SA();
SA(SA::IndexType indexType);
```

##SA-hash
SA-hash is suffix array with hashed k-symbol prefixes of suffix array suffixes to speed up searches (k ≥ 2). This variant is particularly efficient in speed for short patterns (not much longer than k).

Parameters:
- indexType:
      - SA::STANDARD (default)
      - SA::PLUS - efficient for longer patterns (longer than 20)
- hash type:
      - HT::STANDARD - using 8 bytes for each hashed entry: 4 bytes for left boundary + 4 bytes for right boundary
      - HT::DENSE - using 6 bytes for each hashed entry: 4 bytes for left boundary + 2 bytes for right boundary
- k - length of prefixes of suffixes from suffix array (k ≥ 2)
- loadFactor - hash table load factor (0.0 < loadFactor < 1.0)

Limitations: 
- pattern length >= k (patterns shorter than k are handled by standard variant of SA index)

Constructors:
```
SA(SA::IndexType indexType, HT::HTType hTType, unsigned int k, double loadFactor);
```

##SA-LUT2
To speed up searches, SA stores lookup table over all 2-symbol strings (LUT2), whose entries are the suffix intervals.

Parameters:
- indexType:
      - SA::STANDARD (default)
      - SA::PLUS - efficient for longer patterns (longer than 20)

Limitations: 
- pattern length ≥ 2 (patterns shorter than 2 are handled by standard variant of SA index)

Constructors:
```
SALut2();
SALut2(SA::IndexType indexType);
```

##SA usage example
```
#include <iostream>
#include <stdlib.h>
#include "sa/shared/common.h"
#include "sa/shared/patterns.h"
#include "sa/sa.h"

using namespace std;
using namespace sa;

int main(int argc, char *argv[]) {

	unsigned int queriesNum = 1000000;
	unsigned int patternLen = 30;
	unsigned char* text = NULL;
	unsigned int textLen;
	SA *sa;
	const char *textFileName = "english";
	const char *indexFileName = "english-sa.idx";

	if (fileExists(indexFileName)) {
		sa = new SA();
		sa->load(indexFileName);
	} else {
		sa = new SA(SA::PLUS);
		sa->setVerbose(true);
		text = readText(textFileName, textLen, 0);
		sa->build(text, textLen);
		sa->save(indexFileName);
	}

	double indexSize = (double)sa->getIndexSize();
	cout << "Index size: " << indexSize << "B (" << (indexSize / (double)sa->getTextSize()) << "n)" << endl << endl;

	Patterns *P = new Patterns(textFileName, queriesNum, patternLen);
	unsigned char **patterns = P->getPatterns();

	for (unsigned int i = 0; i < queriesNum; ++i) {
		cout << "Pattern |" << patterns[i] << "| occurs " << sa->count(patterns[i], patternLen) << " times." << endl;
	}

	if (text != NULL) delete[] text;
	delete sa;
	delete P;
}
```
Using other variants of SA index is analogous.

##External resources used in SA project
- Suffix array building by Yuta Mori (sais)
- A multi-platform library of highly optimized functions for C and C++ by Agner Fog (asmlib)
- A very fast hash function by Yann Collet (xxHash)

##Authors
- Szymon Grabowski
- [Marcin Raniszewski](https://github.com/mranisz)
