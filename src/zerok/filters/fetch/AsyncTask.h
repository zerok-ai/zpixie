#include <iostream>
#include <future>
#include <chrono>

// namespace zk{
//     class AsyncTask{
//         public:
//             int frequency;
//             AsyncTask(int frequency){
//                 this->frequency = frequency;
//             }

//             void start(void (*functionPtr)()){
//                 std::chrono::milliseconds interval(frequency);
//                 while (true) {
//                     // std::async(std::launch::async, functionPtr);
//                     // std::this_thread::sleep_for(interval);

//                     std::future<void> task = std::async(std::launch::async, functionPtr);
//                     std::this_thread::sleep_for(interval);

//                     // Wait for the task to complete before launching the next one
//                     task.get();
//                 }
//             }
            
//     };
// }


namespace zk{
    class AsyncTask {
        public:
            using FunctionPtr = void (*)();  // Function pointer type

            AsyncTask(FunctionPtr function, int intervalMs)
                : function_(function), intervalMs_(intervalMs), running_(false) {}

            ~AsyncTask() {
                Stop();
            }

            void Start() {
                if (!running_) {
                    running_ = true;
                    thread_ = std::thread(&AsyncTask::TaskLoop, this);
                }
            }

            void Stop() {
                if (running_) {
                    running_ = false;
                    condition_.notify_one();
                    thread_.join();
                }
            }

        private:
            void TaskLoop() {
                while (running_) {
                    function_();  // Invoke the function

                    std::unique_lock<std::mutex> lock(mutex_);
                    condition_.wait_for(lock, std::chrono::milliseconds(intervalMs_));
                }
            }

            FunctionPtr function_;
            int intervalMs_;
            std::thread thread_;
            std::mutex mutex_;
            std::condition_variable condition_;
            bool running_;
        };
}

