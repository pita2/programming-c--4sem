#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <algorithm>
#include <random>
#include <utility>
#include <exception>

template <typename T>
struct noise {
    int size;
    T generated_data;
    std::vector<int> operator()() {
        std::random_device d;
        std::mt19937 gen(d());
        std::vector<int> noise(size);

        for (int i = 0; i < size; ++i)
            noise[i] = rand() % 10;
        auto data = generated_data.get();
        for (int i = 0; i < size; ++i)
            data[i] += noise[i];
        std::shuffle(data.begin(), data.end(), gen);
        return data;
    }
};

template <typename T>
struct mask {
    int size;
    T noised_data;
    std::vector<int> operator()() {
        std::vector<bool> mask(size);

        for (int i = 0; i < size; ++i)
            mask[i] = rand() % 2;
        auto data = noised_data.get();
        for (int i = 0; i < size; ++i)
            data[i] *= mask[i];
        return data;
    }
};

auto task_package(int sz) {
    if (sz < 0) throw std::runtime_error{ "Negative size of vector" };
    std::packaged_task generate_data{
        [size = std::as_const(sz)]() {
            std::vector<int> data(size);
            for (int i = 0; i < size; ++i)
                data[i] = size - i;
            return data;
        }
    };

    std::future generated_data = generate_data.get_future();

    std::packaged_task add_noise{ noise<decltype(generated_data)>(sz, std::move(generated_data)) };

    std::future noised_data = add_noise.get_future();

    std::packaged_task apply_mask{ mask<decltype(noised_data)>(sz, std::move(noised_data)) };

    std::future final_data = apply_mask.get_future();

    std::jthread t1{ std::move(generate_data) };
    std::jthread t2{ std::move(add_noise) };
    std::jthread t3{ std::move(apply_mask) };


    return final_data.get();
}

int main() {
    auto res = task_package(15);
    for (auto x : res)
        std::cout << x << ' ';
    std::cout << '\n';
}