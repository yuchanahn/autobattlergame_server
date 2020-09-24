#pragma once
#include <fmt/format.h>

#include <functional>
#include <mutex>
#include <iostream>
#include <regex>

#define DB_PATH "C:/YCDB/UE4_Game"

class YCDB
{
public:
	static int register_id(bool bnew_acc, std::wstring id_str, std::wstring pass_str)
	{
		static std::wstring validate_id = L"([\\w*\\d*]{6,20})";
		static std::wstring validate_pass = L"((?=.*[A-Za-z])(?=.*\\d)(?=.*[$@$!%*#?&])[A-Za-z\\d$@$!%*#?&]{8,20})";
		static std::once_flag db_register_id_validate_f;
		static int idx = 0;
		static std::unordered_map<std::wstring, std::tuple<std::wstring, int>> users;
		std::call_once(db_register_id_validate_f, [&]() {
			std::ifstream file(fmt::format("{}/{}", DB_PATH, "register_data.txt"));
			std::ifstream validate(fmt::format("{}/{}", DB_PATH, "register_validate_data.txt"));
			if (!validate.is_open())
			{
				validate.close();
				std::ofstream o(fmt::format("{}/{}", DB_PATH, "register_validate_data.txt"));
				std::string s[2];

				s[0].assign(validate_id.cbegin(), validate_id.cend());
				s[0] += "\n";
				s[1].assign(validate_pass.cbegin(), validate_pass.cend());
				s[1] += "\n";

				o.write(s[0].c_str(), s[0].length());
				o.write(s[1].c_str(), s[1].length());
				o.close();
				validate.open(fmt::format("{}/{}", DB_PATH, "register_validate_data.txt"));
			}
			std::string s_row[2];
			int i = 0;
			std::getline(validate, s_row[0]);
			std::getline(validate, s_row[1]);

			validate_id.assign(s_row[0].cbegin(), s_row[0].cend());
			validate_pass.assign(s_row[1].cbegin(), s_row[1].cend());

			if (file.is_open())
			{
				std::wregex re(fmt::format(L"(\\d+),{},{}", validate_id, validate_pass));
				std::wstring s;
				std::wsmatch m;
				std::string s_row;
				while (!file.eof()) {
					std::getline(file, s_row);

					s.assign(s_row.begin(), s_row.end());

					if (std::regex_match(s, m, re))
					{
						int id = _wtoi(m[1].str().c_str());
						users[m[2].str()] = std::make_tuple(m[3].str(), id);
						idx = id;
					}
				}
			}
			else
			{
				std::ofstream o(fmt::format("{}/{}", DB_PATH, "register_data.txt"));
				o.close();
			}
			file.close();
			validate.close();
			});

		if (users.find(id_str) == users.end())
		{
			if (!bnew_acc) return -1;
			std::wregex re_id(validate_id);
			std::wregex re_pass(validate_pass);
			if (std::regex_match(id_str, re_id) && std::regex_match(pass_str, re_pass))
			{
				std::ofstream o(fmt::format("{}/{}", DB_PATH, "register_data.txt"), std::ios::app);
				auto ws = fmt::format(L"{},{},{}\n", ++idx, id_str, pass_str);
				std::string s;
				s.assign(ws.cbegin(), ws.cend());
				o.write(s.c_str(), s.length());
				o.close();

				users[id_str] = std::make_tuple(pass_str, idx);
				return idx;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			if (bnew_acc) return -1;
			return
				std::get<0>(users[id_str]) == pass_str ?
				std::get<1>(users[id_str]) : -1;
		}
	}

	template <typename T>
	static void once_set(const T& p)
	{
		std::ofstream file(fmt::format("{}/{}.txt", DB_PATH, typeid(T).name()));
		auto byte = ((packet_t<T>*) (&p))->ToByte();
		file.write((const char*)(byte), sizeof(T));
		file.close();
	}

	template <typename T>
	static auto once_get()
	{
		std::ifstream file(fmt::format("{}/{}.txt", DB_PATH, typeid(T).name()));
		if (!file.is_open()) return std::tuple(false, T());
		char byte[sizeof(T)];
		file.read(byte, sizeof(T));
		file.close();
		return std::tuple(true, *((T*)((void*)byte)));
	}

	template <typename T>
	static void set(int user_id, const T& p)
	{
		std::ofstream file(fmt::format("{}/{}@{}.txt", DB_PATH, user_id, typeid(T).name()));
		auto byte = ((packet_t<T>*) (&p))->ToByte();
		file.write((const char*)(byte), sizeof(T));
		file.close();
	}
	template <typename T>
	static auto get(int user_id)
	{
		std::ifstream file(fmt::format("{}/{}@{}.txt", DB_PATH, user_id, typeid(T).name()));
		if (!file.is_open()) return std::tuple(false, T());
		char byte[sizeof(T)];
		file.read(byte, sizeof(T));
		file.close();
		return std::tuple(true, *((T*)((void*)byte)));
	}
};