#include "../include/sighand.hpp"

#include <iostream>
#include <thread>
#include <memory>
#include <atomic>
#include <functional>
#include <unordered_map>

void sighand(std::shared_ptr<std::atomic_bool> quit)
{
    static int count = 0;
    std::cout << "Caught signal" << count << std::endl;
    ++count;
    if (count == 3)
    {
        *quit = true;
    }
}

int main()
{
    std::array<int, 1> sigarray{};
    sigarray[0] = SIGINT;
    std::shared_ptr<std::atomic_bool> quit = std::make_shared<std::atomic_bool>();
    std::shared_ptr<std::atomic_int> result = std::make_shared<std::atomic_int>();
    sigset_t set{};
    /* int result_sig{};
     result_sig = sigemptyset(&set);
     result_sig = sigaddset(&set, SIGINT);
     result_sig = pthread_sigmask(SIG_BLOCK, &set, nullptr); */
    *quit;
    auto f = std::bind(sighand, quit);
    std::unordered_map<int, std::function<void()>> map_func{{SIGINT, f}};
    std::array<int, 1> sig_array{};
    sig_array[0] = SIGINT;
    std::thread t1{};
    sth::handle_signal(result, quit, map_func, t1);
    while (!(*quit))
    {
        t1.join();
    }
    std::cout << "Quit after joining" << std::endl;
}