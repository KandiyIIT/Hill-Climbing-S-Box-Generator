# WHS Hill Climbing S-Box Generator
Header-only multithread c++17 implementation of hill climbing method for s-box generation task with WHS cost function. Usage example:

```cpp
    #include "hill_climbing.h"
    #include "utils.h"

    sbox::hill_climbing_info_t<uint64_t> info;

    info.thread_count = 8;
    info.is_log_enabled = true;

    info.try_per_thread = 64000;
    info.max_frozen_count = 10000;
    info.target_nonlinearity = 100;
    
    
    info.cost_function = sbox::whs<uint64_t>;
    info.cost_data.reset(new sbox::whs_function_data_t(7, 21));

    //s-box stored in table format in std::optional<std::array<uint8_t,256>>
    auto sbox = sbox::hill_climbing<uint64_t>(info);

    if (sbox.has_value())
        PRINT_SBOX(sbox.value());
```
