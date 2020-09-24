#pragma once
#include <iostream>
class yc_str
{
public:
	static void copy(std::string s, char* buf)
	{
		s.copy(buf, s.length());
		buf[s.length()] = '\0';
	}
	static void copy(std::wstring s, wchar_t* buf)
	{
		s.copy(buf, s.length());
		buf[s.length()] = '\0';
	}
};