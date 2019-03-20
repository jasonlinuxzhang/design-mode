#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <deque>

enum Result{OK, STOP};

class Cache
{
public:
    Cache()
    {
        status = OK;
    }
    Result get(int &products_id);
    bool put(int product_id);
    void stop()
    {
        std::unique_lock<std::mutex> lck(mtx);
        status = STOP;
        cv.notify_one();
    }
private:
    std::deque<int> products;
    std::mutex mtx;
    std::condition_variable cv;
    Result status;
};

Cache cache;

inline bool Cache::put(int product_id)
{
    std::unique_lock<std::mutex> lck(mtx);
    products.push_back(product_id);
    cv.notify_one();
}

inline Result Cache::get(int &products_id)
{
    std::unique_lock<std::mutex> lck(mtx);
    while(products.empty() && OK == status) cv.wait(lck);

    if(STOP == status)
        return STOP;

    products_id = products.front();
    products.pop_front();

    return OK;
}

void consumer_thread()
{
    int products_id = 0;
    while(true)
    {
        Result res_code = cache.get(products_id);
        if(OK == res_code)
            std::cout<<"consumer get success, value: "<<products_id<<std::endl;
        else
        {
            std::cout<<"consumer get exit code"<<std::endl;
            break;
        }
    }
}

void producer_thread(int products_num = 0)
{
    for(int i = 0; i < products_num; ++i)
    {
        cache.put(i);
    }
}

int main()
{
    std::thread producer(producer_thread, 10);
    std::thread consumer(consumer_thread);

    int stop_key;
    std::cin>>stop_key;

    producer.join();
    std::cout << "producer threads joined!\n";
    cache.stop();
    consumer.join();
    std::cout << "consumer threads joined!\n";

    return 0;
}

