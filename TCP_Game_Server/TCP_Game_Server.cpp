#include "pch.h"

#include <range/v3/all.hpp>
#include <regex>
#include <fstream>
#include <wchar.h>
#include <functional>
#include <cmath>
#include <variant>
#include <optional>
#include <experimental/coroutine>

#include "YCServer.h"
#include "YCPacket.h"
#include "YCSync.h"
#include "YCTime.h"
#include "YCCE.h"
#include "YCFunc.h"

#include "user.h"
#include "packets.h"
#include "yc_str.h"
#include "send_manager.h"
#include "server.h"

int main()
{
	packet_mapping();
	
	auto& server_ = server::get_master_server();
	auto& users = server::get_users();

	auto broadcast_ = [&](auto&& d) { return yc::broadcast(server_, users, d); };

	ioev::Signal<chatting_t>([&](chatting_t* c, int i) {
		server_.get_server_sync()->Add([&broadcast_, &users, id = i, chat = *c] {
			auto str = fmt::format(L"{} : {}", users[id].name, chat.text);
			fmt::print("recv chat!\n");
			yc_str::copy(str, (wchar_t*)chat.text);
			broadcast_(&chat);
		});
	});
	
	server::run();
}