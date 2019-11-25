#include <condition_variable>
#include <chrono>
#include <iostream>
#include <cassert>
std::mutex m;

void notify(std::condition_variable& cond){
        std::unique_lock lck{m};
        cond.notify_one();
}

auto get_notify(std::condition_variable& cond){
            std::unique_lock lck{m};
            auto now = std::chrono::system_clock::now();
            return cond.wait_until(lck,now + std::chrono::seconds(1));
}

int main(){

    std::condition_variable cond;
    notify(cond);
    //notify(cond);

    assert(get_notify(cond) == std::cv_status::no_timeout);
    std::cout << (int) get_notify(cond) << "\n";
    assert(get_notify(cond) == std::cv_status::no_timeout);
 

}