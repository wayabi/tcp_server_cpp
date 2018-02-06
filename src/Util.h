#ifndef __RS_UTIL__
#define __RS_UTIL__

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <math.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

using namespace std;

class Util {
public:
	static string urlEncode(const char *source);
	static string getHostFromURL(const string &url);
	static string getMACAddress(void);
	static vector<string> getListFile(string path_dir);
	static string getExtension(string name_file);
	static vector<string> split(string source, char delimiter);
	static bool existFile(const char *filename);
	static size_t getSizeFile(const char *filename);
	static time_t getTimeFromSeperatedFormat(const char *seperated);
	static string getTimeString(time_t t);

	static string hex_texted_ucs2_to_utf8(const char *hex_texted_ucs2);
	static string trim(const string &s, const char *list_char_trim = " \r\t\v\n");
	static string getIPAddress(const string &name_interface);

	static time_t getTime(int year, int month, int date, int hour, int minute, int second);
	static void lowess(double span, double* source, double* out, int num_value);
	static string getCRLFString(const char* c);
	static bool hasTheHead(const char* source, const char* head);
	static bool hasTheHeadFoot(const char* source, const char* head, const char* foot);
	static int getIndexText(const char* source, const char* text);
	static string getHexString(void* buf, int size, bool flag_capital = true);
	static int atoiHeading(const char* c);
	static ostream& hexdump(ostream& ost, const void* bin, size_t length);
	static vector<unsigned char> hexToBinary(const string& hex);

private:
	static bool sort_string(const string& a, const string& b);
};

#endif
