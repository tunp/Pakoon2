#ifndef STRINGTOOLS_H
#define STRINGTOOLS_H

#include <string>

using namespace std;

class StringTools {
public:
	static void makeUpper(string &s);
	static bool compareNoCase(string s1, string s2);
};

#endif
