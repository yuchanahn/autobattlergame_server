#pragma once
#include <vector>

#include "YCDB.h"
#include "champion.h"
#include "YCRandom.h"

struct percentage_data
{
	int code;
	float percentage;
};

struct champion_percentages
{
	percentage_data data[100];
	int count;

	static champion_percentages get_defult_per()
	{
		return champion_percentages
		{
		   {
			   percentage_data{0, 0.90f},
			   percentage_data{1, 0.09f},
			   percentage_data{2, 0.01f}
		   },
		3 };
	}
};

struct item_type_percentages
{
	percentage_data data[100];
	int count;

	static item_type_percentages get_defult_per()
	{
		return item_type_percentages
		{
		   {
			   percentage_data{0, 0.33f},
			   percentage_data{1, 0.33f},
			   percentage_data{2, 0.34f},
		   },
		3 };
	}
};

struct item_percentages
{
	percentage_data data[100];
	int count;

	static item_percentages get_defult_per()
	{
		return item_percentages
		{
		   {
			   percentage_data{0, 0.25f},
			   percentage_data{1, 0.25f},
			   percentage_data{2, 0.25f},
			   percentage_data{3, 0.25f}
		   },
		4 };
	}
};


template <typename T>
class gacha
{
	T data;

	gacha()
	{
		auto r = YCDB::once_get<T>();
		T d =
			std::get<0>(r) ? std::get<1>(r)
						   : T::get_defult_per();
		if (!std::get<0>(r)) YCDB::once_set(d);
		data = std::move(d);
	}
public:

	static int run()
	{
		static gacha<T> _gacha;

		auto r = yc::rand(0.f, 1.f);

		float t = 0.f;

		for (int i = 0; i < _gacha.data.count; i++)
		{
			t += _gacha.data.data[i].percentage;
			if (r <= t) return _gacha.data.data[i].code;
		}

		return -1;
	}
};