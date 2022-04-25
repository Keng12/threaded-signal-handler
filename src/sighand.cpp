#include "sighand.hpp"

namespace sth
{
    void sigwait_handler(const sigset_t set, std::shared_ptr<std::atomic_int> result, const std::shared_ptr<std::atomic_bool> quit, std::unordered_map<int, std::function<void()>> map_func)
    {
        while (!(*quit))
        {
            int sig{};
            *result = sigwait(&set, &sig);
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
            *result = sigwait(&set, &sig);
            if (0 == *result)
            {
                sig_func();
            }
        }
    }
    int init_mask(sigset_t *set_ptr)
    {
        int result_sig = sigemptyset(set_ptr);
        if (-1 == result_sig)
        {
            result_sig = errno;
        }
        return result_sig;
    }
    int add_sig(sigset_t *set_ptr, const int signal)
    {
        int result_sig = sigaddset(set_ptr, signal);
        if (-1 == result_sig)
        {
            result_sig = errno;
        }
        return result_sig;
    }
    int handle_signal(std::shared_ptr<std::atomic_int> result, const std::shared_ptr<std::atomic_bool> quit, std::unordered_map<int, std::function<void()>> map_func, std::thread &thread)
    {
        sigset_t set{};
        int exit_code = init_mask(&set);
        if (0 == exit_code)
        {
            for (const auto &item : map_func)
            {
                exit_code = add_sig(&set, item.first);
                if (0 != exit_code)
                {
                    return exit_code;
                }
            }
            exit_code = pthread_sigmask(SIG_BLOCK, &set, nullptr);
            if (0 == exit_code)
            {
                auto f = static_cast<void (*)(const sigset_t, std::shared_ptr<std::atomic_int>, const std::shared_ptr<std::atomic_bool>, std::unordered_map<int, std::function<void()>>)>(sigwait_handler);
                thread = std::thread(f, set, result, quit, map_func);
            }
        }
        return exit_code;
    }

    int handle_signal(int signal, std::shared_ptr<std::atomic_int> result, const std::shared_ptr<std::atomic_bool> quit, std::function<void()> sig_func, std::thread &thread)
    {
        sigset_t set{};
        int exit_code = init_mask(&set);
        if (0 == exit_code)
        {
            exit_code = add_sig(&set, signal);
            if (0 == exit_code)
            {
                exit_code = pthread_sigmask(SIG_BLOCK, &set, nullptr);
                if (0 == exit_code)
                {
                    auto f = static_cast<void (*)(sigset_t, std::shared_ptr<std::atomic_int>, std::shared_ptr<std::atomic_bool>, std::function<void()>)>(sigwait_handler);
                    thread = std::thread(f, set, result, quit, sig_func);
                }
            }
        }
        return exit_code;
    }
}