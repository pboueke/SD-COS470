/**
        C++ Producer Consumer using C++11 thread facilities
        To compile: g++ -std=c++11 <program name> -pthread -lpthread -o pc
*/
#include <iostream>
#include <sstream>
#include <vector>
#include <stack>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>
using namespace std;

// print function for "thread safe" printing using a stringstream
void print(ostream& s) { cout << s.rdbuf(); cout.flush(); s.clear(); }

//
//      Constants
//
//
const int producer_delay_to_produce = 10;   // in miliseconds
const int consumer_delay_to_consume = 10;
const int max_production = 10000;              // When producers has produced this quantity they will stop to produce
int max_products;                // Maximum number of products that can be stored

//      Variables
//
//
int num_producers;
int num_consumers;



atomic<int> num_producers_working(0);       // When there's no producer working the consumers will stop, and the program will stop.
stack<int> products;                        // The products stack, here we will store our products

mutex xmutex;                               // Our mutex, without this mutex our program will cry
condition_variable is_not_full;             // to indicate that our stack is not full between the thread operations

condition_variable is_not_empty;            // to indicate that our stack is not empty between the thread operations

//      Functions
//
inline const char * const boolToString(bool b)
{
    return b ? "true" : "false";
}

bool isPrime(int number ) //LostInTheCode, SO
{
    // tests the number in the formats 6k-1, 6k and 6k+1 up until it's square root
    if ( ( (!(number & 1)) && number != 2 ) || (number < 2) || (number % 3 == 0 && number != 3) )
        return (false);
    for( int k = 1; 36*k*k-12*k < number;++k)
        if ( (number % (6*k+1) == 0) || (number % (6*k-1) == 0) )
            return (false);
    return true;
}

//      Produce function, producer_id will produce a product
void produce(int producer_id)
{
    unique_lock<mutex> lock(xmutex);

    int product;
    product = (rand() % (10000000+1-1))+1;

    is_not_full.wait(lock, [] { return products.size() != max_products; });
    products.push(product);

    //print(stringstream() << "Producer " << producer_id << " produced " << product << "\n");
    is_not_empty.notify_all();
}

//      Consume function, consumer_id will consume a product
void consume(int consumer_id)
{
    unique_lock<mutex> lock(xmutex);
    int product;

    is_not_empty.wait(lock, [] { return products.size() > 0; });

    product = products.top();
    products.pop();

    //print(stringstream() << "Consumer: " << consumer_id << " - " << product << " is prime? " << boolToString(isPrime(product)) << "\n");
    is_not_full.notify_all();

}

//      Producer function, this is the body of a producer thread
void producer(int id)
{
    ++num_producers_working;
    for(int i = 0; i < max_production; ++i)
    {
        srand(((int) time(NULL)) * i);
        produce(id);
        //this_thread::sleep_for(chrono::milliseconds(producer_delay_to_produce));
    }

   //print(stringstream() << "Producer " << id << " has exited\n");
    --num_producers_working;
}

//      Consumer function, this is the body of a consumer thread
void consumer(int id)
{
    // Wait until there is any producer working
    while(num_producers_working == 0) this_thread::yield();

    while(num_producers_working != 0 || products.size() > 0)
    {
        consume(id);
        //this_thread::sleep_for(chrono::milliseconds(consumer_delay_to_consume));
    }

    //print(stringstream() << "Consumer " << id << " has exited\n");
}

//
//      Main
//ls

int main(int argc, char* argv[])
{
    num_producers = atoi(argv[1]);
    num_consumers = atoi(argv[2]);
    max_products = atoi(argv[3]);

    vector<thread> producers_and_consumers;

    // Create producers
    for(int i = 0; i < num_producers; ++i)
        producers_and_consumers.push_back(thread(producer, i));

    // Create consumers
    for(int i = 0; i < num_consumers; ++i)
        producers_and_consumers.push_back(thread(consumer, i));

    // Wait for consumers and producers to finish
    for(auto& t : producers_and_consumers)
        t.join();

    print(stringstream() << "Cabo\n");
    return 0;

}