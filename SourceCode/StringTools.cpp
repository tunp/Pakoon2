#include "StringTools.h"

void StringTools::makeUpper(string &s) {
	for (int x = 0; x < s.length(); x++) {
		if (s[x] >= 'a' && s[x] <= 'z') {
			s[x] -= 0x20;
		}
	}
}

bool StringTools::compareNoCase(string s1, string s2) {
	makeUpper(s1);
	makeUpper(s2);
	return s1.compare(s2);
}
