#pragma once
#define _SCL_SECURE_NO_WARNINGS
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <list>
#include <functional>

#pragma pack(push, 1)
union int_to_byte
{
	int i;
	char b[sizeof(int)];
};
union size_t_to_byte
{
	size_t i;
	char b[sizeof(size_t)];
};
class PacketEvent;

template <typename T>
union packet_t
{
	packet_t() { memset(this, 0, sizeof(packet_t<T>)); }
	T value;
private:
	unsigned char byte_data[sizeof(T)];
public:
	const unsigned char* ToByte()
	{
		return byte_data;
	}

	int pack(unsigned char* buffer)
	{
		auto curBuffer = buffer;
		int_to_byte i_to_b;
		i_to_b.i = sizeof(int) + sizeof(int) + sizeof(T);

		std::copy(i_to_b.b, i_to_b.b + sizeof(int), curBuffer);
		curBuffer += sizeof(int);

		int_to_byte s_to_b;
		s_to_b.i = PacketEvent::packet_events[typeid(T).hash_code()];
		std::copy(s_to_b.b, s_to_b.b + sizeof(int), curBuffer);
		curBuffer += sizeof(int);
		std::copy(ToByte(), ToByte() + sizeof(T), curBuffer);
		return i_to_b.i;
	}
};
#pragma pack(pop)

static class PacketEvent
{

	static std::unordered_map<int, std::list<std::function<void(void*, int)>>> event;
public:
	static std::unordered_map<size_t, int> packet_events;
	template <typename T, typename F>
	static void bind_event(F f)
	{
		event[packet_events[typeid(T).hash_code()]].push_back([f](void* d, int id) { f((T*)d, id); });
	}

	template <typename T>
	static void signal_event(int id, T p)
	{
		for (auto ev : event[id])
		{
			ev(p, -1);
		}
	}

	template <typename T>
	static void signal_event(int id, T p, int user_id)
	{
		for (auto ev : event[id])
		{
			ev(p, user_id);
		}
	}
};





class YC_Packet_ReadManager
{
	unsigned char buf[2048]; // 자기 저장용 버퍼.
	int i = 0;		// 버퍼 현재 인덱스.
	int _size = -1;

public:
	void read(unsigned char* r_buf, int r_size, int id = -1)
	{
		bool trigger = false;

		std::copy(r_buf + i, r_buf + i + r_size, &buf[i]);

		if ((i += r_size) < 4)
		{
			return;
		}
		if (_size == -1)
		{
			int_to_byte i_to_b;
			std::copy(buf, buf + sizeof(int), i_to_b.b);
			_size = i_to_b.i;
		}

		if (i >= _size)
		{
			int_to_byte i_to_b;
			std::copy(buf + sizeof(int), buf + sizeof(int) + sizeof(int), i_to_b.b);
			auto _id = i_to_b.i;

			if(id != -1)
				PacketEvent::signal_event(_id, buf + sizeof(int) + sizeof(int), id);
			else
				PacketEvent::signal_event(_id, buf + sizeof(int) + sizeof(int));

			if (i > _size)
			{
				std::copy(&buf[_size], (&buf[_size]) + ((size_t)i - _size), buf);
				trigger = true;
			}
			i = i - _size;
			_size = -1;
		}
		if (trigger)
		{
			auto start_idx = i;
			i = 0;
			if (id == -1)
				read(buf, start_idx);
			else
				read(buf, start_idx, id);
		}

	}

};
class yc_template_packet_to_row
{
	size_t hashcode = -1;
	friend class ioev;
public:
	template<int i>
	void To()
	{
		PacketEvent::packet_events[hashcode] = i;
	}
};
class ioev
{
public:
	template<typename T>
	static yc_template_packet_to_row Map()
	{
		yc_template_packet_to_row p;
		p.hashcode = typeid(T).hash_code();
		return p;
	}

	template<typename T, typename F>
	static void Signal(F f)
	{
		PacketEvent::bind_event<T>(f);
	}
};

