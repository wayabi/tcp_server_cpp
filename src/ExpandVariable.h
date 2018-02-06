#ifndef __EXPAND_VARIABLE_H__
#define __EXPAND_VARIABLE_H__

#include <string>

class ExpandVariable {
public:
	static std::string expandVariable(const std::string& s, const std::string& path_file_variable);
};

#endif
