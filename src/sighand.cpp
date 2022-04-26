#include "sighand.hpp"

namespace sth
{
    void *Thread::sigwait_handler(void *arguments)
    {
        struct arg_struct *args = static_cast<struct arg_struct *>(arguments);
        bool quit{};
        while (!(*args->thread_quit))
        {
            int sig{};
            int tmp_result = sigwait(&(args->set), &sig);
            if (args->result)
            {
                *(args->result) = tmp_result;
            }
            if (0 == tmp_result)
            {
                if (args->F_w_args)
                {
                    args->F_w_args(sig);
                }
                else if (args->F)
                {
                    args->F();
                }
                else
                {
                    args->map_F[sig]();
                }
            }
            else
            {
                *(args->thread_quit) = true;
            }
        }
        pthread_exit(nullptr);
    }

    int Thread::init_mask(sigset_t *set_ptr)
    {
        int result_sig = sigemptyset(set_ptr);
        if (-1 == result_sig)
        {
            result_sig = errno;
        }
        return result_sig;
    }

    int Thread::add_sig(sigset_t *set_ptr, const int signal)
    {
        int result_sig = sigaddset(set_ptr, signal);
        if (-1 == result_sig)
        {
            result_sig = errno;
        }
        return result_sig;
    }

    Thread::Thread(int &exit_code, std::shared_ptr<std::atomic_int> result, const std::unordered_map<int, std::function<void()>> &map_func)
        : mArgs{arg_struct{std::move(result), map_func}}
    {
        sigset_t set{};
        exit_code = init_mask(&set);
        if (0 == exit_code)
        {
            for (const auto &item : map_func)
            {
                int tmp_exit_code = add_sig(&set, item.first);
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
                    exit_code = pthread_create(&mThread, nullptr, sigwait_handler, static_cast<void *>(&mArgs));
                    if (0 == exit_code)
                    {
                        mRunning = true;
                    }
                }
            }
        }
    }

    Thread::Thread(int &exit_code, std::shared_ptr<std::atomic_int> result, int signal, std::function<void()> sig_func)
        : mArgs{arg_struct{std::move(result), std::move(sig_func)}}
    {
        sigset_t set{};
        exit_code = init_mask(&set);
        if (0 == exit_code)
        {
            exit_code = add_sig(&set, signal);
            if (0 == exit_code)
            {
                exit_code = pthread_sigmask(SIG_BLOCK, &set, nullptr);
                if (0 == exit_code)
                {
                    mArgs.set = set;
                    exit_code = pthread_create(&mThread, nullptr, sigwait_handler, static_cast<void *>(&mArgs));
                    if (0 == exit_code)
                    {
                        mRunning = true;
                    }
                }
            }
        }
    }
    int Thread::join()
    {
        int exit_code{};
        if (mRunning)
        {
            if (!(*mArgs.thread_quit))
            {
                exit_code = pthread_cancel(mThread);
            }
            if (0 == exit_code)
            {
                exit_code = pthread_join(mThread, nullptr);
                mRunning = false;
            }
        }
        return exit_code;
    }

    Thread::~Thread()
    {
        join();
    }
}