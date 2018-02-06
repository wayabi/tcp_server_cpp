#include "ExpandVariable.h"
#include "Util.h"

#include <stdio.h>
#include <vector>
#include <utility>
#include <iostream>
#include <functional>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/program_options.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <boost/algorithm/string/classification.hpp> // is_any_of
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace std;
using namespace boost::program_options;

namespace {
	enum {
		eVARIABLE = 0,
		eBASE64_FILE,
		eHEX_STRING,
	};
	std::string base64_decode(const std::string& s) {
		namespace bai = boost::archive::iterators;

		std::stringstream os;

		typedef bai::transform_width<bai::binary_from_base64<const char *>, 8, 6> base64_dec;

		unsigned int size = s.size();

		string temp = s;
		std::replace(temp.begin(), temp.end(), '=', 'A');

		if (size == 0) return std::string();

		std::copy(base64_dec(temp.data()), base64_dec(temp.data() + size), std::ostream_iterator<char>(os));

		return os.str();
	}

	std::string base64_encode(const std::string& s) {
		namespace bai = boost::archive::iterators;

		const std::string base64_padding[] = {"", "==","="};
		std::stringstream os;

		// convert binary values to base64 characters
		typedef bai::base64_from_binary
		// retrieve 6 bit integers from a sequence of 8 bit bytes
		<bai::transform_width<const char *, 6, 8> > base64_enc; // compose all the above operations in to a new iterator

		std::copy(base64_enc(s.c_str()), base64_enc(s.c_str() + s.size()),
							std::ostream_iterator<char>(os));

		os << base64_padding[s.size() % 3];
		return os.str();
	}

	string expandByNameValueFormat(const string& name_variable, const string& path_file_variable)
	{
		FILE* f;
		if ((f = fopen(path_file_variable.c_str(), "r")) == NULL) {
			cout << "variable file open error" << endl;
			return string("");
		}

		const int size_buf = 1000;
		char buf[size_buf];

		while (fgets(buf, size_buf, f) != NULL) {
			vector<string> list;
			string temp_split(buf);
			boost::algorithm::split(list, temp_split, boost::is_any_of(","));
			if (list.size() < (size_t)2) {
				continue;
			}
			if (list.at(0).compare(name_variable) == 0) {
				fclose(f);
				string ret;
				for (int i=1;(size_t)i<list.size();++i) {
					if (i >= 2) ret += ",";
					ret += list.at(i);
				}
				if (*(ret.c_str()+ret.length()-1) == '\n') {
					ret = ret.substr(0, ret.length()-1);
				}
				return ret;
			}
		}
		fclose(f);
		return string("");
	}

	string expandFileToBase64(const string& path_file)
	{
		stringstream ss;
		std::ifstream ifs(path_file, std::ifstream::in);
		char c = ifs.get();
		while (ifs.good()) {
			ss << c;
			c = ifs.get();
		}
		ifs.close();
		return base64_encode(ss.str());
	}

	std::pair<string, bool> expandVariableSub(const string& source, int type_resolve, const string& path_file_variable)
	{
		string head = "\"";
		string foot = "\"";
		if (type_resolve == eVARIABLE) {
			head = "${";
			foot = "}";
		} else if (type_resolve == eBASE64_FILE) {
			head = "$base64{";
			foot = "}";
		} else if (type_resolve == eHEX_STRING) {
			head = "$hex{";
			foot = "}";
		}

		bool flag_changed = false;
		stringstream ret;
		int index_resolved = 0;

		int index_head = -1;
		for(int i=0;i<(int)source.length();++i){
			if (index_head == -1 && memcmp(source.c_str()+i, head.c_str(), head.length()) == 0) {
				index_head = i+head.length();
				//cout << "index_head1 = " << index_head << endl;
				i += head.length()-1;
			}else if (index_head >= 0 && memcmp(source.c_str()+i, foot.c_str(), foot.length()) == 0) {
				string name_variable = source.substr(index_head, i-index_head);
				ret << source.substr(index_resolved, index_head-head.length()-index_resolved);
				if(type_resolve == eVARIABLE){
					ret << expandByNameValueFormat(name_variable, path_file_variable);
				}else if(type_resolve == eBASE64_FILE){
					ret << expandFileToBase64(name_variable);
				}else if(type_resolve == eHEX_STRING){
					vector<unsigned char> b = Util::hexToBinary(name_variable);
					string s((char*)(&b[0]), b.size());
					ret << s;
				}

				index_resolved = i+foot.length();
				index_head = -1;
				flag_changed = true;
			}
		}
		ret << source.substr(index_resolved, source.length()-index_resolved);
		return std::make_pair(ret.str(), flag_changed);
	}
}

string ExpandVariable::expandVariable(const string& s, const string& path_file_variable)
{
	string temp = s;
	while (true) {
		pair<string, bool> ret = expandVariableSub(temp, eVARIABLE, path_file_variable);
		if (ret.second) {
			temp = ret.first;
		} else {
			break;
		}
	}
	pair<string, bool> ret = expandVariableSub(temp, eBASE64_FILE, path_file_variable);
	ret = expandVariableSub(ret.first, eHEX_STRING, path_file_variable);
	return ret.first;
}
