#ifndef _HILL_CLIMBING_H_
#define _HILL_CLIMBING_H_

#include <thread>
#include <mutex>

#include <functional>
#include <optional>
#include <random>
#include <vector>
#include <iostream>
#include "cost_function.h"

namespace sbox {

	template <typename T>
	struct hill_climbing_info_t {
		int32_t thread_count;
		int32_t try_per_thread;
		int32_t max_frozen_count;
		int32_t target_nonlinearity;
		bool is_log_enabled;

		std::unique_ptr< cost_function_data_t> cost_data;
		std::function< cost_info_t<T>(cost_function_data_t*,std::array<uint8_t,256>)> cost_function;
	};

	template <typename T>
	struct shared_info_t {
		std::array<uint8_t, 256> best_sbox;
		cost_info_t<T> best_cost;
		std::mutex sbox_mutex;
		bool is_found;
		int32_t frozen_count;
	};

	template<typename T>
	void hill_climbing_thread_function(shared_info_t<T>& params, hill_climbing_info_t<T>& info) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> distrib(0, 255);
		std::array<uint8_t, 256> new_sbox;
		cost_info_t<T> new_cost;

		for (int i = 0;i < info.try_per_thread; i++) {
			int pos_1 = 0;
			int pos_2 = 0;

			params.sbox_mutex.lock();
			new_sbox = params.best_sbox;
			params.sbox_mutex.unlock();

			while (pos_1 == pos_2) {
				pos_1 = distrib(gen);
				pos_2 = distrib(gen);
			}
			std::swap(new_sbox[pos_1], new_sbox[pos_2]);

			new_cost = info.cost_function(info.cost_data.get(), new_sbox);

			if (new_cost.nonlinearity >= info.target_nonlinearity) {
				params.sbox_mutex.lock();
				params.best_sbox = new_sbox;
				params.best_cost = new_cost;
				params.is_found = true;
				params.frozen_count = 0;

				if (info.is_log_enabled)
					std::cout << "cost=" << params.best_cost.cost << " NL=" << params.best_cost.nonlinearity << std::endl;

				params.sbox_mutex.unlock();
				return;
			}
			else {
				params.sbox_mutex.lock();
				params.frozen_count++;
				if (params.frozen_count > info.max_frozen_count) {
					params.sbox_mutex.unlock();
					return;
				}
				params.sbox_mutex.unlock();
			}

			params.sbox_mutex.lock();
			if (params.is_found) {
				params.sbox_mutex.unlock();
				return;
			}

			if (new_cost.cost < params.best_cost.cost) {
				params.best_sbox = new_sbox;
				params.best_cost = new_cost;
				params.frozen_count = 0;

				if(info.is_log_enabled)
					std::cout <<"cost=" << params.best_cost.cost<<" NL="<<params.best_cost.nonlinearity << std::endl;
				
			}
			params.sbox_mutex.unlock();
		}
		return;
	}

	template<typename T>
	std::optional<std::array<uint8_t,256>> hill_climbing(hill_climbing_info_t<T> &info) {
		shared_info_t<T> thread_data;
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> distrib(0, 255);
		std::vector<std::thread> workers;
		

		for (int i = 0;i <= 255;i++)
			thread_data.best_sbox[i] = i;

		for (int i = 255;i > 0;i--) {
			int j = distrib(gen) % i;
			std::swap(thread_data.best_sbox[i], thread_data.best_sbox[j]);
		}

		thread_data.is_found = false;
		thread_data.frozen_count = 0;
		thread_data.best_cost = info.cost_function(info.cost_data.get(), thread_data.best_sbox);

		for (int i = 0;i < info.thread_count;i++) {
			workers.push_back(
				std::thread(&hill_climbing_thread_function<T>,
					std::ref(thread_data), std::ref(info))
			);
		}

		for (std::thread& worker : workers)
			worker.join();
		workers.clear();

		if (thread_data.is_found)
			return thread_data.best_sbox;

		return {};

	}

};

#endif // _HILL_CLIMBING_H_