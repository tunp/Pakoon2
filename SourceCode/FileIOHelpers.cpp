//
// File IO helpers
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include <fstream>
#include <iostream>
#include <map>

using namespace std;

#include "FileIOHelpers.h"

map<string, map<string, string> > FileHelpers::readMapFromFile(string filename) {
	map<string, map<string, string> > filemap;
	map<string, string> *current_section = 0;
	
	ifstream ini_file(filename.c_str());
	if (ini_file.is_open()) {
		string line;
		//bool in_right_section = false;
		while (!ini_file.eof()) {
			getline(ini_file, line);
			
			//clean up
			while (line[0] == ' ') {
				line.erase(line.begin());
			}
			
			while (!line.empty() && (line[line.length()-1] == '\n' || line[line.length()-1] == '\r' || line[line.length()-1] == ' ' || line[line.length()-1] == '\t')) {
				line.erase(line.end()-1);
			}
			
			//parsing
			if (!line.empty()) {
				if (line[0] == '[') {
					line.erase(line.begin());
					while (line.length() > 0 && line[line.length()-1] != ']') {
						line.erase(line.end()-1);
					}
					line.erase(line.end()-1);
					//in_right_section = (line.compare(sSection) == 0);
					filemap.insert(pair<string, map<string, string> >(line, map<string, string>()));
					current_section = &filemap[line];
				} else {
					//if (in_right_section) {
						//section_found = true;
						short find_pos = line.find('=');
						if (find_pos != string::npos) {
							short first_end = find_pos;
							short second_start = find_pos + 1;
							while (first_end > 0 && line[first_end-1] == ' ') {
								first_end--;
							}
							while (second_start > 0 && line[second_start] == ' ') {
								second_start++;
							}
							
							string first = line.substr(0, first_end);
							string second = line.substr(second_start);
							
							/*if (first.compare(sKeyName) == 0) {
								sReturnedString = second;
								nRet = second.length();
								//cout << second << endl;
								return nRet;
							}*/
							
							current_section->insert(pair<string, string>(first, second));
						}
					//}
				}
			}
		}
	} else {
		cout << "Cannot open file " << filename.c_str() << endl;
	}
	ini_file.close();
	
	return filemap;
}

bool FileHelpers::WriteMapToFile(map<string, map<string, string> > filemap, string filename) {
	ofstream ini_file(filename.c_str(), ios_base::trunc);
	
	if (ini_file.is_open()) {
		for (map<string, map<string, string> >::iterator filemap_it = filemap.begin(); filemap_it != filemap.end(); filemap_it++) {
			ini_file << "[" << filemap_it->first << "]" << endl;;
			map<string, string> section = filemap_it->second;
			for (map<string, string>::iterator section_it = section.begin(); section_it != section.end(); section_it++) {
				ini_file << section_it->first << " = " << section_it->second << endl;
			}
		}
	} else {
		return false;
	}
	ini_file.close();
	return true;
}

int FileHelpers::WriteKeyStringToINIFile(string  sSection,        // section name
                                         string  sKeyName,        // key name
                                         string  sValue,          // value to write
                                         string  sFilename) {     // initialization file name

	if (!sSection.empty() && !sFilename.empty()) {
		//FIXME if section or key not yet found
		map<string, map<string, string> > filemap = readMapFromFile(sFilename);
		filemap[sSection][sKeyName] = sValue;
		WriteMapToFile(filemap, sFilename);
	}
	
	return 0;
}



int FileHelpers::GetKeyStringFromINIFile(string  sSection,        // section name
                                         string  sKeyName,        // key name
                                         string  sDefault,        // default string
                                         string &sReturnedString, // destination buffer
                                         string  sFilename) {     // initialization file name
  int nRet = 0;
  
	//FIXME mitä jos key name null
	//cout << "GetKeyStringFromINIFile: " << sSection << " " << sKeyName << " " << sFilename << endl;
	
	bool section_found = false;
	
	if (!sSection.empty() && !sFilename.empty()) {
		map<string, map<string, string> > filemap = readMapFromFile(sFilename);
		map<string, map<string, string> >::iterator filemap_it = filemap.find(sSection);
		
		if (filemap_it != filemap.end()) {
			section_found = true;
			map<string, string> section = filemap_it->second;
			map<string, string>::iterator section_it = section.find(sKeyName);
			
			if (section_it != section.end()) {
				sReturnedString = section_it->second;
				nRet = section_it->second.length();
				//cout << section_it->second << endl;
				return nRet;
			}
		}
		
		//coutterii varte
		if (sKeyName.empty() && section_found) {
			sReturnedString = "hack";
			return 4;
		}
	}
  
  //cout << "Not found" << endl;
  sReturnedString = sDefault;
  nRet = sDefault.length();
  return nRet;
}


int FileHelpers::GetKeyDoubleFromINIFile(string  sSection,        // section name
                                         string  sKeyName,        // key name
                                         double   dDefault,        // default
                                         double  &dReturnedDouble, // destination buffer
                                         string  sFilename) {     // initialization file name
//cout << "GetKeyDoubleFromINIFile" << endl;
  string sValue, sDefault;
  sDefault = "default";
  int nRet = GetKeyStringFromINIFile(sSection, sKeyName, sDefault, sValue, sFilename);
  if(sValue.compare(sDefault) == 0) {
    dReturnedDouble = dDefault;
  } else {
    sscanf(sValue.c_str(), "%lf", &dReturnedDouble);
  }
  return nRet;
}

int FileHelpers::GetKeyIntFromINIFile(string  sSection,        // section name
                                      string  sKeyName,        // key name
                                      int      nDefault,        // default
                                      int     &nReturnedInt,    // destination buffer
                                      string  sFilename) {     // initialization file name
  double dValue;
  int nRet = GetKeyDoubleFromINIFile(sSection, sKeyName, double(nDefault), dValue, sFilename);
  nReturnedInt = int(dValue);
  return nRet;
} 


int FileHelpers::GetKeyVectorFromINIFile(string  sSection,        // section name
                                         string  sKeyName,        // key name
                                         BVector  vDefault,        // default
                                         BVector &vReturnedVector, // destination buffer
                                         string  sFilename) {     // initialization file name
  string sValue, sDefault;
  sDefault = "default";
  int nRet = GetKeyStringFromINIFile(sSection, sKeyName, sDefault, sValue, sFilename);
  if(sValue.compare(sDefault) == 0) {
    vReturnedVector = vDefault;
  } else {
    sscanf(sValue.c_str(), 
           "%lf, %lf, %lf", 
           &(vReturnedVector.m_dX), 
           &(vReturnedVector.m_dY), 
           &(vReturnedVector.m_dZ));
  }
  return nRet;
}



// PAKOON! Game, Source Code and Developer Package Copyright
// =========================================================
// 
// Restrictions related to PAKOON! Game and it's use
// -------------------------------------------------
// 
// You may download and play the PAKOON! game for free. You may also copy it freely to your friends and relatives as long as you 
// provide the original setup package (downloaded from www.nic.fi/~moxide) and the copyright included in it is also given. You 
// may also use the PAKOON! game for educational purposes, as long as the origin of the PAKOON! game (i.e. www.nic.fi/~moxide) 
// is mentioned and this copyright is also provided and the creator of the game (i.e. Mikko Oksalahti, email: 
// mikko.oksalahti@nic.fi) is notified of the use in advance.
// You may not sell or otherwise accept any payment for giving or offering the game to someone else. You may not offer the 
// PAKOON! game for free on any webpage, CD, DVD or other media without a written permission from the creator of the PAKOON! 
// game (i.e. Mikko Oksalahti, email: mikko.oksalahti@nic.fi).
// You may freely include a link to PAKOON! homepage (i.e. www.nic.fi/~moxide) from your own site.
// 
// 
// Restrictions related to PAKOON! Game
// Source Code and Developer Package and their use
// -----------------------------------------------
// 
// You may download and use the PAKOON! game source code for personal use. You may not use any part of the source code or the 
// developer package on any commercial or free game or other computer program intended for public distribution without a written 
// permission from the creator of the PAKOON! game (i.e. Mikko Oksalahti, email: mikko.oksalahti@nic.fi). You may use the 
// PAKOON! game source code or developer package for educational purposes, as long as the origin of the PAKOON! game (i.e. 
// www.nic.fi/~moxide) is mentioned and this copyright is also provided and the creator of the game (i.e. Mikko Oksalahti, 
// email: mikko.oksalahti@nic.fi) is notified of the use in advance.
// You may not sell or otherwise accept any payment for giving or offering the PAKOON! game source code or developer package to 
// someone else. You may not offer the PAKOON! game source code or developer package for free on any webpage, CD, DVD or other 
// media without a written permission from the creator of the PAKOON! game (i.e. Mikko Oksalahti, email: 
// mikko.oksalahti@nic.fi).
// 
// 
// Mikko Oksalahti
// Helsinki, FINLAND
// 7.10.2002
