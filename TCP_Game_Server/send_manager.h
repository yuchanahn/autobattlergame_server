#pragma once
#include <unordered_map>
#include "YCServer.h"
#include "user.h"

namespace yc
{
	template <typename T>
	static void send(YCServer& server_, int clnt_id, T* packet)
	{
		server_.Send(clnt_id, packet);
	}

	template <typename T>
	static void broadcast(YCServer& server_, const std::unordered_map<int, user_t>& users, T* packet)
	{
		for (auto& i : users) server_.Send(i.first, packet);
	}
};