#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <algorithm>
#include <random>
#include <utility>
#include <exception>

auto task_package(int sz) {
    if (sz < 0) throw std::runtime_error{"Negative size of vector"};
    std::packaged_task generate_data {
            [size = std::as_const(sz)]() {
                std::vector<int> data;
                data.reserve(size);
                std::generate_n(std::back_inserter(data), size, [i = size]() mutable { return --i; });
                return data;
            }
    };

    std::future generated_data = generate_data.get_future();

    std::packaged_task add_noise {
            [size = std::as_const(sz), generated_data = std::move(generated_data)]() mutable {
                std::random_device d;
                std::mt19937 gen(d());
                std::vector<int> noise;
                noise.reserve(size);
                std::generate_n(std::back_inserter(noise), size, [&gen = gen]() mutable { return (gen() % 2) ? 1 : -1;});
                auto data = generated_data.get();
                for (int i = 0; i < size; ++i)
                    data[i] += noise[i];
                std::shuffle(data.begin(), data.end(), gen);
                return data;
            }
    };

    std::future noised_data = add_noise.get_future();

    std::packaged_task apply_mask {
            [size = std::as_const(sz), noised_data = std::move(noised_data)]() mutable {
                std::random_device d;
                std::mt19937 gen(d());
                std::vector<bool> mask;
                mask.reserve(size);
                std::generate_n(std::back_inserter(mask), size, [&gen = gen]() mutable { return gen() % 2; });
                auto data = noised_data.get();
                for (int i = 0; i < size; ++i)
                    data[i] *= mask[i];
                return data;
            }
    };

    std::future final_data = apply_mask.get_future();

    std::thread t1 {std::move(generate_data)};
    std::thread t2 {std::move(add_noise)};
    std::thread t3 {std::move(apply_mask)};
    t3.join();
    t2.join();
    t1.join();

    return final_data.get();
}

int main() {
    auto res = task_package(15);
    for (auto x : res)
        std::cout << x << ' ';
    std::cout << '\n';
}
