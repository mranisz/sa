#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <map>
#include "../shared/patterns.h"
#include "../shared/timer.h"
#include "../sa.h"

using namespace std;
using namespace shared;
using namespace sa;

ChronoStopWatch timer;

map<string, SA::IndexType> SAIndexTypesMap = {{"std", SA::STANDARD}, {"dbl", SA::DBL}};
map<string, HT::HTType> hashTypesMap = {{"hash", HT::STANDARD}, {"hash-dense", HT::DENSE}};

void saNoLut2(string indexType, const char *textFileName, unsigned int queriesNum, unsigned int m);
void saLut2(string indexType, const char *textFileName, unsigned int queriesNum, unsigned int m);
void saHash(string indexType, string hTType, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);

void getUsage(char **argv) {
	cout << "Select index you want to test (count):" << endl;
	cout << "SA: " << argv[0] << " std|dbl fileName patternNum patternLen" << endl;
        cout << "SA-LUT2: " << argv[0] << " lut2 std|dbl fileName patternNum patternLen" << endl;
        cout << "SA-hash: " << argv[0] << " std|plus hash|hash-dense k loadFactor fileName patternNum patternLen" << endl;
        cout << "where:" << endl;
	cout << "fileName - name of text file" << endl;
	cout << "patternNum - number of patterns (queries)" << endl;
	cout << "patternLen - pattern length" << endl;
	cout << "k - suffix length to be hashed (k > 0)" << endl;
	cout << "loadFactor - load factor of hash table (range: (0.0, 1.0))" << endl << endl;
}

int main(int argc, char *argv[]) {
	if (argc < 5) {
		getUsage(argv);
		exit(1);
	}
        if (SAIndexTypesMap.find(string(argv[1])) != SAIndexTypesMap.end()) {
                if (argc == 5) saNoLut2(string(argv[1]), argv[2], atoi(argv[3]), atoi(argv[4]));
                else if (argc == 8 && hashTypesMap.find(string(argv[2])) != hashTypesMap.end()) saHash(string(argv[1]), string(argv[2]), string(argv[3]), string(argv[4]), argv[5], atoi(argv[6]), atoi(argv[7]));
        }
        else if ((string)argv[1] == "lut2" && SAIndexTypesMap.find(string(argv[2])) != SAIndexTypesMap.end() && argc == 6) {
                saLut2(string(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]));
	}
        getUsage(argv);
        exit(1);
}

void saNoLut2(string indexType, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	SA *sa;
        string indexFileNameString = "SA-" + indexType + "-" + (string)textFileName + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		sa = new SA();
		sa->load(indexFileName);
	} else {
		sa = new SA(SAIndexTypesMap[indexType]);
		sa->setVerbose(true);
		sa->build(textFileName);
		sa->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m);
        //NegativePatterns *P = new NegativePatterns(textFileName, queriesNum, m);
        /*MaliciousPatterns *P = new MaliciousPatterns(textFileName, m);
        queriesNum = P->getQueriesNum();
        if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = sa->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/sa/" + string(textFileName) + "_count_SA.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)sa->getIndexSize() / (double)sa->getTextSize();
	cout << "count SA-" << indexType << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " " << indexType << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorCountsNumber(indexCounts);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexCounts;
	delete sa;
	delete P;
        exit(0);
}

void saLut2(string indexType, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	SALut2 *saLut2;
        string indexFileNameString = "SALut2-" + indexType + "-" + (string)textFileName + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		saLut2 = new SALut2();
		saLut2->load(indexFileName);
	} else {
		saLut2 = new SALut2(SAIndexTypesMap[indexType]);
		saLut2->setVerbose(true);
		saLut2->build(textFileName);
		saLut2->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m);
        //NegativePatterns *P = new NegativePatterns(textFileName, queriesNum, m);
        /*MaliciousPatterns *P = new MaliciousPatterns(textFileName, m);
        queriesNum = P->getQueriesNum();
        if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = saLut2->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/sa/" + string(textFileName) + "_count_SALut2.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)saLut2->getIndexSize() / (double)saLut2->getTextSize();
	cout << "count SALut2-" << indexType << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " " << indexType << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorCountsNumber(indexCounts);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexCounts;
	delete saLut2;
	delete P;
        exit(0);
}

void saHash(string indexType, string hTType, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	SA *sa;
        string indexFileNameString = "SA-" + indexType + "-" + hTType + "-" + (string)textFileName + "-" +  k + "-" + loadFactor + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		sa = new SA();
		sa->load(indexFileName);
	} else {
		sa = new SA(SAIndexTypesMap[indexType], hashTypesMap[hTType], atoi(k.c_str()), atof(loadFactor.c_str()));
		sa->setVerbose(true);
		sa->build(textFileName);
		sa->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m);
        //NegativePatterns *P = new NegativePatterns(textFileName, queriesNum, m);
        /*MaliciousPatterns *P = new MaliciousPatterns(textFileName, m);
        queriesNum = P->getQueriesNum();
        if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = sa->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/sa/" + string(textFileName) + "_count_SA-" + hTType + ".txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)sa->getIndexSize() / (double)sa->getTextSize();
	cout << "count SA-" << hTType << "-" << indexType << "-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " " << indexType << " " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorCountsNumber(indexCounts);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexCounts;
	delete sa;
	delete P;
        exit(0);
}