# WHS Hill Climbing S-Box Generator
Header-only multithread c++17 implementation of hill climbing method for s-box generation task with WHS cost function. Usage example:

```cpp
    #include "hill_climbing.h"
    #include "utils.h"

	sbox::hill_climbing_info_t<double> info;

	info.thread_count = 2;
	info.is_log_enabled = true;

	info.try_per_thread = 1000000;
	info.max_frozen_count = 100000;
	info.target_nonlinearity = 104;


	info.cost_function = sbox::whs<double>;
	info.cost_data.reset(new sbox::whs_function_data_t(3, 0));

	//s-box stored in table format in std::optional<std::array<uint8_t,256>>
	auto sbox = sbox::hill_climbing<double>(info);

	if (sbox.has_value())
		PRINT_SBOX(sbox.value());
```
