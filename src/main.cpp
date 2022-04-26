#include "sighand.hpp"

#include <iostream>
#include <thread>
#include <memory>
#include <atomic>
#include <functional>
#include <unordered_map>

void sighand1(std::shared_ptr<std::atomic_bool> quit, const int signal)
{
    static int count = 1;
    if (signal == SIGINT)
    {
        std::cout << "Caught signal " << signal << " " << count << " time(s)." << std::endl;
        ++count;
    }
    else
    {
        std::cout << "Caught unknown signal" << std::endl;
    }
    if (count % 3 == 0)
    {
        *quit = true;
    }
}

void sighand2(std::shared_ptr<std::atomic_bool> quit)
{
    static int count = 1;
    std::cout << "Caught SIGINT " << count << " time(s)." << std::endl;
    ++count;
    if (count % 3 == 0)
    {
        *quit = true;
    }
}

int main()
{
    std::shared_ptr<std::atomic_bool> quit = std::make_shared<std::atomic_bool>();
    std::shared_ptr<std::atomic_int> result = std::make_shared<std::atomic_int>();
    std::array<int, 1> sigarray{};
    sigarray[0] = SIGINT;
    sigset_t set{};
    auto f1 = std::bind(sighand2, quit);
    int exit_code{};
    {
        sth::Thread t = sth::Thread(exit_code, result, SIGINT, f1);
        while (!*quit)
        {
        }
    }
    *quit = false;
    std::unordered_map<int, std::function<void()>> map_func{{SIGINT, f1}};
    {
        sth::Thread t = sth::Thread(exit_code, result, map_func);
        while (!*quit)
        {
        }
    }
    *quit = false;
    std::array<int, 1> sig_array{};
    sig_array[0] = SIGINT;
    using namespace std::placeholders;
    auto f2 = std::bind(sighand1, quit, _1);
    {
        sth::Thread t = sth::Thread(exit_code, result, sigarray, f2);
        while (!*quit)
        {
        }
    }
}