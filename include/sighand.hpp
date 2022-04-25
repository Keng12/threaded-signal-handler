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

    void sigwait_handler(const sigset_t set, std::shared_ptr<std::atomic_int> result, const std::shared_ptr<std::atomic_bool> quit, std::unordered_map<int, std::function<void()>> map_func)
    {
        while (!(*quit))
        {
            int sig{};
            sigwait(&set, &sig);
            if (0 == *result)
            {
                map_func[sig]();
            }
        }
    }

    void sigwait_handler(const sigset_t set, std::shared_ptr<std::atomic_int> result, const std::shared_ptr<std::atomic_bool> quit, std::function<void()> sig_func)
    {
        while (!(*quit))
        {
            int sig{};
            sigwait(&set, &sig);
            if (0 == *result)
            {
                sig_func();
            }
        }
    }

    int handle_signal(std::shared_ptr<std::atomic_int> result, const std::shared_ptr<std::atomic_bool> quit, std::unordered_map<int, std::function<void()>> map_func, std::thread &thread)
    {
        sigset_t set{};
        int result_sig{};
        result_sig = sigemptyset(&set);
        int exit_code{};
        if (-1 == result_sig)
        {
            exit_code = errno;
        }
        else
        {
            for (const auto &item : map_func)
            {
                result_sig = sigaddset(&set, item.first);
                if (-1 == result_sig)
                {
                    exit_code = errno;
                }
            }
            if (0 == result_sig)
            {
                exit_code = pthread_sigmask(SIG_BLOCK, &set, nullptr);
                if (0 == exit_code)
                {
                    std::cout << "Start thread" << std::endl;
                    auto f = static_cast<void (*)(const sigset_t, std::shared_ptr<std::atomic_int>, const std::shared_ptr<std::atomic_bool>, std::unordered_map<int, std::function<void()>>)>(sigwait_handler);
                    thread = std::thread(f, set, result, quit, map_func);
                }
            }
        }
        return exit_code;
    }

    int handle_signal(int signal, std::shared_ptr<std::atomic_int> result, const std::shared_ptr<std::atomic_bool> quit, std::function<void()> sig_func, std::thread &thread)
    {
        sigset_t set{};
        int result_sig{};
        result_sig = sigemptyset(&set);
        int exit_code{};
        if (-1 == result_sig)
        {
            exit_code = errno;
        }
        else
        {
            result_sig = sigaddset(&set, signal);
            if (-1 == result_sig)
            {
                exit_code = errno;
            }
            else
            {
                exit_code = pthread_sigmask(SIG_BLOCK, &set, nullptr);
                if (0 == exit_code)
                {
                    std::cout << "Start thread" << std::endl;
                    auto f = static_cast<void (*)(sigset_t, std::shared_ptr<std::atomic_int>, std::shared_ptr<std::atomic_bool>, std::function<void()>)>(sigwait_handler);
                    thread = std::thread(f, set, result, quit, sig_func);
                }
            }
        }
        return exit_code;
    }

    template <size_t n_signal>
    int handle_signal(std::array<int, n_signal> signal_array, std::shared_ptr<std::atomic_int> result, std::shared_ptr<std::atomic_bool> quit, std::function<void()> sig_func, std::thread &thread)
    {
        sigset_t set{};
        int result_sig{};
        result_sig = sigemptyset(&set);
        int exit_code{};
        if (-1 == result_sig)
        {
            exit_code = errno;
        }
        else
        {
            for (const int &signal : signal_array)
            {
                result_sig = sigaddset(&set, signal);
                if (-1 == result_sig)
                {
                    exit_code = errno;
                }
            }
            if (0 == result_sig)
            {
                exit_code = pthread_sigmask(SIG_BLOCK, &set, nullptr);
                if (0 == exit_code)
                {
                    std::cout << "Start thread" << std::endl;
                    auto f = static_cast<void (*)(sigset_t, std::shared_ptr<std::atomic_int>, std::shared_ptr<std::atomic_bool>, std::function<void()>)>(sigwait_handler);
                    thread = std::thread(f, set, result, quit, sig_func);
                }
            }
        }
        return exit_code;
    }

}

#endif