# SA (Suffix Array) variants

##What is it?
The library contains implementions of a few suffix array (SA) variants. The standard SA \[[1](#references)\] is a widely known full-text index, allowing to find all the occurrences of a pattern in the (indexed) text in O(m log n) time, where m is the pattern length and n is the text length.
Assuming n < 2^31 (characters or bytes), the standard SA occupies 4n bytes, on top of the 1n bytes for the text.

The other implemented variants, SA-hash \[[2](#references)\] and SA-LUT2 \[[1](#references)\], speed up the SA searches in practice, for the price of extra space.

##Requirements
The SA indexes require:
- C++11 ready compiler such as g++ version 4.7 or higher
- a 64-bit operating system
- text size is limited to 2GB

##Installation
To download and build the library use the following commands:
```
git clone https://github.com/mranisz/sa.git
cd sa
make
```

##Usage
To use the SA variants library:
- include "sa/sa.hpp" to your project
- compile it with "-std=c++11 -O3" options and link it with libraries:
  - sa/libsa.a
  - sa/libs/libaelf64.a (linux) or fbcsa/libs/libacof64.lib (windows)
- use "sa" and "shared" namespaces

##API
There are several functions you can call on each of the suffix array text index:
- **build** the index using text file called textFileName:
```
void build(const char *textFileName);
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
- get the result of **locate** query:
```
void locate(unsigned char *pattern, unsigned int patternLen, vector<unsigned int>& res);
```

##SA\<SAType T\>

Parameters:
- T:
      - SA_STANDARD
      - SA_DBL - ...

Constructors:
```
SA<SAType T>();
```

##SAHash\<SAType T, HTType HASHTYPE\>
SAHash is suffix array with hashed k-symbol prefixes of suffix array suffixes to speed up searches (k ≥ 2). This variant is particularly efficient in speed for short patterns (not much longer than k).

Parameters:
- T:
      - SA_STANDARD
      - SA_DBL - ...
- HASHTYPE:
      - HT_STANDARD - using 8 bytes for each hashed entry: 4 bytes for left boundary + 4 bytes for right boundary
      - HT_DENSE - using 6 bytes for each hashed entry: 4 bytes for left boundary + 2 bytes for right boundary
- k - length of prefixes of suffixes from suffix array
- loadFactor - hash table load factor

Limitations: 
- pattern length ≥ k (patterns shorter than k are handled by standard variant of SA index)
- k ≥ 2
- 0.0 < loadFactor < 1.0

Constructors:
```
SAHash<SAType T, HTType HASHTYPE>(unsigned int k, double loadFactor);
```

##SALut2\<SAType T\>
To speed up searches, SA stores lookup table over all 2-symbol strings (LUT2), whose entries are the suffix intervals.

Parameters:
- T:
      - SA_STANDARD
      - SA_DBL - ...

Limitations: 
- pattern length ≥ 2 (patterns shorter than 2 are handled by standard variant of SA index)

Constructors:
```
SALut2<SAType T>();
```

##SA usage example
```
#include <iostream>
#include <stdlib.h>
#include "sa/shared/patterns.hpp"
#include "sa/sa.hpp"

using namespace std;
using namespace shared;
using namespace sa;

int main(int argc, char *argv[]) {

	unsigned int queriesNum = 1000000;
	unsigned int patternLen = 20;
	SA<SA_DBL> *sa = new SA<SA_DBL>();
	const char *textFileName = "english";
	const char *indexFileName = "english-sa-dbl.idx";

	if (fileExists(indexFileName)) {
		sa->load(indexFileName);
	} else {
		sa->build(textFileName);
		sa->save(indexFileName);
	}

	double indexSize = (double)sa->getIndexSize();
	cout << "Index size: " << indexSize << "B (" << (indexSize / (double)sa->getTextSize()) << "n)" << endl << endl;

	Patterns32 *P = new Patterns32(textFileName, queriesNum, patternLen);
	unsigned char **patterns = P->getPatterns();

	for (unsigned int i = 0; i < queriesNum; ++i) {
		cout << "Pattern |" << patterns[i] << "| occurs " << sa->count(patterns[i], patternLen) << " times." << endl;
	}

	delete sa;
	delete P;
}
```
Using other variants of SA index is analogous.

##External resources used in SA project
- Suffix array building by Yuta Mori (sais)
- A multi-platform library of highly optimized functions for C and C++ by Agner Fog (asmlib)
- A very fast hash function by Yann Collet (xxHash)

##References
1. U. Manber, G. Myers. Suffix arrays: a new method for on-line string searches, in Proceedings of the 1st ACM-SIAM Annual Symposium on Discrete Algorithms, SIAM, 1990, pp. 319-327.
2. Sz. Grabowski, M. Raniszewski. Two simple full-text indexes based on the suffix array. arXiv:1405.5919, 2016.

##Authors
- Szymon Grabowski
- [Marcin Raniszewski](https://github.com/mranisz)
