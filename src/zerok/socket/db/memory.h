#include <iostream>
#include <future>
#include <chrono>
#include <queue>
#include <map>

namespace zk{
    class ZkMemory{
        private:
            std::queue<std::string> dataQueue;
            static std::map<std::string, ZkMemory*>* idToZkMemoryMap;
            
        public:
            int frequency;
            static ZkMemory* instance(std::string identifier){
                if (idToZkMemoryMap == nullptr){
                    std::map<std::string, ZkMemory*>* idToZkMemoryMapLocal = new std::map<std::string, ZkMemory*>();
                    idToZkMemoryMap = idToZkMemoryMapLocal;
                }
                // ZkMemory* zkMemory = idToZkMemoryMap->find(identifier)
                if(idToZkMemoryMap->count(identifier) <= 0){
                    ZkMemory* zkmemory = new ZkMemory();
                    (*idToZkMemoryMap)[identifier] = zkmemory;
                }
                return (*idToZkMemoryMap)[identifier];
            }

            void push(std::string data){
                dataQueue.push(data);
            }

            std::string get(){
                std::string data = dataQueue.front();
                dataQueue.pop();
                return data;
            }

            std::string get(int quantity){
                int x = quantity;
                std::string finaldata = "";
                while (x > 0 && !dataQueue.empty()) {
                    std::string data = get();
                    finaldata += data;
                    x--;
                }
                return finaldata;
            }
            
    };
    std::map<std::string, ZkMemory*>* ZkMemory::idToZkMemoryMap = nullptr;
}