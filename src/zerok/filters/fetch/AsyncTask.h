#include <iostream>
#include <future>
#include <chrono>

namespace zk{
    class AsyncTask{
        public:
            int frequency;
            AsyncTask(int frequency){
                this->frequency = frequency;
            }

            void start(void (*functionPtr)()){
                std::chrono::milliseconds interval(frequency);
                while (true) {
                    std::async(std::launch::async, functionPtr);
                    std::this_thread::sleep_for(interval);
                }
            }
            
    };
}