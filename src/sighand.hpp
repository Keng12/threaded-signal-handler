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
#include <pthread.h>

namespace sth
{
    class Thread
    {
        struct arg_struct
        {
            const std::function<void(int)> F_w_args{};
            const std::function<void()> F{};
            std::unordered_map<int, std::function<void()>> map_F{};
            sigset_t set{};
            std::shared_ptr<std::atomic_int> result{};
            std::shared_ptr<std::atomic_bool> thread_quit = std::make_shared<std::atomic_bool>();
            inline arg_struct() = default;
            inline arg_struct(std::shared_ptr<std::atomic_int> l_result, std::unordered_map<int, std::function<void()>> map_func) : result{std::move(l_result)}, map_F{std::move(map_func)} {};
            inline arg_struct(std::shared_ptr<std::atomic_int> l_result, std::function<void(int)> sig_func) : result{std::move(l_result)}, F_w_args{std::move(sig_func)} {};
            inline arg_struct(std::shared_ptr<std::atomic_int> l_result, std::function<void()> sig_func) : result{std::move(l_result)}, F{std::move(sig_func)} {};
        };
        static void *sigwait_handler(void *arguments);
        int init_mask(sigset_t *set_ptr);
        int add_sig(sigset_t *set_ptr, const int signal);
        bool mRunning{};
        pthread_t mThread{};
        arg_struct mArgs{};

    public:
        ~Thread();
        int join();
        Thread(int &exit_code, std::shared_ptr<std::atomic_int> result, const std::unordered_map<int, std::function<void()>> &map_func);
        Thread(int &exit_code, std::shared_ptr<std::atomic_int> result, int signal, std::function<void()> sig_func);
        template <size_t n_signal>
        inline Thread(int &exit_code, std::shared_ptr<std::atomic_int> result, std::array<int, n_signal> signal_array, std::function<void(int)> sig_func)
            : mArgs{arg_struct{std::move(result), std::move(sig_func)}}
        {
            sigset_t set{};
            exit_code = init_mask(&set);
            if (0 == exit_code)
            {
                for (const int &signal : signal_array)
                {
                    int tmp_exit_code = add_sig(&set, signal);
                    if (0 != tmp_exit_code)
                    {
                        exit_code = tmp_exit_code;
                    }
                }
                if (0 == exit_code)
                {
                    exit_code = pthread_sigmask(SIG_BLOCK, &set, nullptr);
                    if (0 == exit_code)
                    {
                        mArgs.set = set;
                        exit_code = pthread_create(&mThread, nullptr, &sigwait_handler, static_cast<void *>(&mArgs));
                        if (0 == exit_code)
                        {
                            mRunning = true;
                        }
                    }
                }
            }
        }
    };
}

#endif