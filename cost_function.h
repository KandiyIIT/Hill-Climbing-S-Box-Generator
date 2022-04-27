#ifndef _COST_FUNCTION_H_
#define _COST_FUNCTION_H_

#include <algorithm>
#include <array>
#include <cstdint>
#include <cassert>

namespace sbox {

	template <typename T>
	struct cost_info_t {
		T cost;
		int32_t nonlinearity;
	};

	class cost_function_data_t {
	};

	class whs_function_data_t : public cost_function_data_t {
	public:
		int32_t r;
		int32_t x;

		whs_function_data_t(int _r, int _x) : r(_r), x(_x) {};
	};

	//count of "1" in binary representation on numbers 0 - 255
	static uint8_t one_bits[256] = {
				0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
	};

	static void fwht(uint8_t* truth_table, int* spectre)
	{
		int step = 1;

		for (int i = 0;i < 256;i++)
			spectre[i] = -2 * truth_table[255 - i] + 1;

		while (step < 256)
		{
			int left = 0;
			int numOfBlocks = (256 / (step * 2));

			for (int i = 0;i < numOfBlocks;i++)
			{
				int right = left + step;

				for (int j = 0;j < step;j++)
				{
					int a = spectre[right];
					int b = spectre[left];
					spectre[left] = a + b;
					spectre[right] = a - b;
					left++;
					right++;
				}

				left = right;
			}
			step *= 2;
		}
		return;
	}

	/**
		WHS cost function
	*/
	template <typename T>
	cost_info_t<T> whs(cost_function_data_t* _data, std::array<uint8_t,256> sbox) {
		uint8_t truth_table[256];
		int32_t max_spectre = 0;
		int spectre[256];
		cost_info_t<T> cost;

		whs_function_data_t* data = static_cast<whs_function_data_t*>(_data);


		cost.cost = 0;
		for (int b = 1; b < 256; b++)
		{
			for (int i = 0;i < 256;i++)
				truth_table[i] = one_bits[sbox[i] & b] & 0x01;

			
			fwht(truth_table, spectre);

			for (int i = 0;i < 256;i++)
			{
				if (spectre[i] < 0)
					spectre[i] = -spectre[i];

				T val = ((spectre[i]-data->x) >= 0) ? (spectre[i]-data->x) : -(spectre[i]-data->x);

				assert(((void)"error: absolute spectre value less 0", val > 0));

				T part = 1;

				for (int k = 0; k < data->r; k++)
					part *= val;
				cost.cost += part;

				if (spectre[i] > max_spectre)
					max_spectre = spectre[i];
			}
		}

		cost.nonlinearity = 128 - max_spectre / 2;

		return cost;
	}

};

#endif // _COST_FUNCTION_H_