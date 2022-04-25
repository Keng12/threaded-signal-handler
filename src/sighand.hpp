#ifndef SIGHAND_HPP
#define SIGHAND_HPP

#include <array>
#include <atomic>
#include <cassert>
#include <cerrno>
#include <csignal>
#include <functional>
#include <memory>
#include <unordered_map>
#include <iostream>
#include <thread>

namespace sth
{
    void sigwait_handler(const sigset_t set, std::shared_ptr<std::atomic_int> result, const std::shared_ptr<std::atomic_bool> quit, std::unordered_map<int, std::function<void()>> map_func);
    void sigwait_handler(const sigset_t set, std::shared_ptr<std::atomic_int> result, const std::shared_ptr<std::atomic_bool> quit, std::function<void()> sig_func);
    int init_mask(sigset_t *set_ptr);
    int add_sig(sigset_t *set_ptr, const int signal);
    int handle_signal(std::shared_ptr<std::atomic_int> result, const std::shared_ptr<std::atomic_bool> quit, std::unordered_map<int, std::function<void()>> map_func, std::thread &thread);
    int handle_signal(int signal, std::shared_ptr<std::atomic_int> result, const std::shared_ptr<std::atomic_bool> quit, std::function<void()> sig_func, std::thread &thread);

    template <size_t n_signal>
    int inline handle_signal(std::array<int, n_signal> signal_array, std::shared_ptr<std::atomic_int> result, std::shared_ptr<std::atomic_bool> quit, std::function<void()> sig_func, std::thread &thread)
    {
        sigset_t set{};
        int exit_code = init_mask(&set);
        if (0 == exit_code)
        {
            for (const int &signal : signal_array)
            {
                exit_code = add_sig(&set, signal);
                if (0 != exit_code)
                {
                    return exit_code;
                }
            }
            exit_code = pthread_sigmask(SIG_BLOCK, &set, nullptr);
            if (0 == exit_code)
            {
                auto f = static_cast<void (*)(sigset_t, std::shared_ptr<std::atomic_int>, std::shared_ptr<std::atomic_bool>, std::function<void()>)>(sigwait_handler);
                thread = std::thread(f, set, result, quit, sig_func);
            }
        }
        return exit_code;
    }

}

#endif