
#ifndef HW_1_4SEM__MUTEXWRAPPER_H
#define HW_1_4SEM__MUTEXWRAPPER_H
#include <functional>
#include <mutex>

template <typename T>
struct FunctorGuard;


template <typename Ret, typename... Args>
struct FunctorGuard<Ret(Args...)> {
    std::function<Ret(Args...)> functor;
    static std::mutex mtx;

    auto operator()(Args&&... args) -> decltype(auto) {
        const std::lock_guard lock {mtx};
        return std::invoke(functor, args...);
    }

};

template <typename Ret, typename... Args>
std::mutex FunctorGuard<Ret(Args...)>::mtx;

template <typename Ret, typename... Args>
FunctorGuard(Ret(Args...)) -> FunctorGuard<Ret(Args...)>;

#endif
