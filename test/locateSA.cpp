#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include "../shared/patterns.hpp"
#include "../shared/timer.hpp"
#include "../sa.hpp"

using namespace std;
using namespace shared;
using namespace sa;

ChronoStopWatch timer;

void saStd(const char *textFileName, unsigned int queriesNum, unsigned int m);
void saDbl(const char *textFileName, unsigned int queriesNum, unsigned int m);
void saLut2Std(const char *textFileName, unsigned int queriesNum, unsigned int m);
void saLut2Dbl(const char *textFileName, unsigned int queriesNum, unsigned int m);
void saStdHash(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void saDblHash(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void saStdHashDense(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void saDblHashDense(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);

void getUsage(char **argv) {
	cout << "Select index you want to test (locate):" << endl;
	cout << "SA: " << argv[0] << " std fileName patternNum patternLen" << endl;
	cout << "SA-dbl: " << argv[0] << " dbl fileName patternNum patternLen" << endl;
	cout << "SA-LUT2: " << argv[0] << " lut2 fileName patternNum patternLen" << endl;
	cout << "SA-LUT2-dbl: " << argv[0] << " lut2-dbl fileName patternNum patternLen" << endl;
	cout << "SA-hash: " << argv[0] << " std-hash k loadFactor fileName patternNum patternLen" << endl;
	cout << "SA-dbl-hash: " << argv[0] << " dbl-hash k loadFactor fileName patternNum patternLen" << endl;
	cout << "SA-hash-dense: " << argv[0] << " std-hash-dense k loadFactor fileName patternNum patternLen" << endl;
	cout << "SA-dbl-hash-dense: " << argv[0] << " dbl-hash-dense k loadFactor fileName patternNum patternLen" << endl;
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
	if (string(argv[1]) == "std") saStd(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "dbl") saDbl(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "lut2") saLut2Std(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "lut2-dbl") saLut2Dbl(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "std-hash") saStdHash(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "dbl-hash") saDblHash(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "std-hash-dense") saStdHashDense(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "dbl-hash-dense") saDblHashDense(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	getUsage(argv);
	exit(1);
}

void saStd(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	SA<SA_STANDARD> *sa = new SA<SA_STANDARD>();
	string indexFileNameString = "SA-std-" + (string)textFileName + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		sa->load(indexFileName);
	} else {
		sa->build(textFileName);
		sa->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		sa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_SA.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)sa->getIndexSize() / (double)sa->getTextSize();
	cout << "locate SA-std " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " std " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete sa;
	delete P;
    exit(0);
}

void saDbl(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	SA<SA_DBL> *sa = new SA<SA_DBL>();
	string indexFileNameString = "SA-dbl-" + (string)textFileName + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		sa->load(indexFileName);
	} else {
		sa->build(textFileName);
		sa->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		sa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_SA.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)sa->getIndexSize() / (double)sa->getTextSize();
	cout << "locate SA-dbl " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " dbl " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete sa;
	delete P;
    exit(0);
}

void saLut2Std(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	SALut2<SA_STANDARD> *saLut2 = new SALut2<SA_STANDARD>();
	string indexFileNameString = "SALut2-std-" + (string)textFileName + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		saLut2->load(indexFileName);
	} else {
		saLut2->build(textFileName);
		saLut2->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		saLut2->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_SALut2.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)saLut2->getIndexSize() / (double)saLut2->getTextSize();
	cout << "locate SALut2-std " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " std " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete saLut2;
	delete P;
	exit(0);
}

void saLut2Dbl(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	SALut2<SA_DBL> *saLut2 = new SALut2<SA_DBL>();
	string indexFileNameString = "SALut2-dbl-" + (string)textFileName + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		saLut2->load(indexFileName);
	} else {
		saLut2->build(textFileName);
		saLut2->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		saLut2->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_SALut2.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)saLut2->getIndexSize() / (double)saLut2->getTextSize();
	cout << "locate SALut2-dbl " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " dbl " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete saLut2;
	delete P;
	exit(0);
}

void saStdHash(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	SAHash<SA_STANDARD, HT_STANDARD> *sa = new SAHash<SA_STANDARD, HT_STANDARD>(atoi(k.c_str()), atof(loadFactor.c_str()));
    string indexFileNameString = "SA-std-hash-" + (string)textFileName + "-" +  k + "-" + loadFactor + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		sa->load(indexFileName);
	} else {
		sa->build(textFileName);
		sa->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		sa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_SA-hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)sa->getIndexSize() / (double)sa->getTextSize();
	cout << "locate SA-std-hash-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " std " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete sa;
	delete P;
	exit(0);
}

void saDblHash(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	SAHash<SA_DBL, HT_STANDARD> *sa = new SAHash<SA_DBL, HT_STANDARD>(atoi(k.c_str()), atof(loadFactor.c_str()));
    string indexFileNameString = "SA-dbl-hash-" + (string)textFileName + "-" +  k + "-" + loadFactor + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		sa->load(indexFileName);
	} else {
		sa->build(textFileName);
		sa->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		sa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_SA-hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)sa->getIndexSize() / (double)sa->getTextSize();
	cout << "locate SA-dbl-hash-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " dbl " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete sa;
	delete P;
	exit(0);
}

void saStdHashDense(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	SAHash<SA_STANDARD, HT_DENSE> *sa = new SAHash<SA_STANDARD, HT_DENSE>(atoi(k.c_str()), atof(loadFactor.c_str()));
    string indexFileNameString = "SA-std-hash-dense-" + (string)textFileName + "-" +  k + "-" + loadFactor + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		sa->load(indexFileName);
	} else {
		sa->build(textFileName);
		sa->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		sa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_SA-hash-dense.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)sa->getIndexSize() / (double)sa->getTextSize();
	cout << "locate SA-std-hash-dense-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " std " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete sa;
	delete P;
	exit(0);
}

void saDblHashDense(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	SAHash<SA_DBL, HT_DENSE> *sa = new SAHash<SA_DBL, HT_DENSE>(atoi(k.c_str()), atof(loadFactor.c_str()));
    string indexFileNameString = "SA-dbl-hash-dense-" + (string)textFileName + "-" +  k + "-" + loadFactor + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		sa->load(indexFileName);
	} else {
		sa->build(textFileName);
		sa->save(indexFileName);
	}

	Patterns32 *P = new Patterns32(textFileName, queriesNum, m);
	//NegativePatterns32 *P = new NegativePatterns32(textFileName, queriesNum, m);
	/*MaliciousPatterns32 *P = new MaliciousPatterns32(textFileName, m);
	queriesNum = P->getQueriesNum();
	if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	vector<unsigned int> *indexLocates = new vector<unsigned int>[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		sa->locate(patterns[i], m, indexLocates[i]);
	}
	timer.stopTimer();

	string resultFileName = "results/" + string(textFileName) + "_locate_SA-hash-dense.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)sa->getIndexSize() / (double)sa->getTextSize();
	cout << "locate SA-dbl-hash-dense-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " dbl " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorLocatesNumber(indexLocates);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	delete[] indexLocates;
	delete sa;
	delete P;
	exit(0);
}