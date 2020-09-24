#pragma once
#include <iostream>
#include <unordered_map>
#include <optional>

#include "YCDB.h"

struct account_t
{
	int user_code;
	std::wstring id;
	std::wstring pass;
};

static std::unordered_map<int, account_t> acc_of_code;
static std::unordered_map<std::wstring, account_t> acc_of_id;

static std::optional<account_t> get_user_info(std::wstring id, std::wstring pass)
{
	auto r = YCDB::register_id(false, id, pass);
	return r == -1 ? std::nullopt 
				   : std::make_optional(account_t { r, std::move(id), std::move(pass)});
}

static std::optional<int> add_account(std::wstring id, std::wstring pass)
{
	auto r = YCDB::register_id(true, id, pass);
	return r == -1 ? std::make_optional(r)
				   : std::nullopt;
}