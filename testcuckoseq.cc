#include <iostream>
#include <sstream>
//#include "cucko.h"
#include "cucko_seq.h"
using namespace std;
#include <chrono>
using namespace std::chrono;
#include <time.h>
    // Simple main
int main( )
{
    int nb_iter=10000;
    int max_key=5000;
    //const int GAP  =   37;
    const int ATTEMPTS = 1;
    float prob_insert=0.6;
    float r;
    int i;
    HashTable<string,StringHashFamily<3>> h1(100);
    cout<<"initial size of map "<<2*h1.size_array()<< endl;
    //srand (time(NULL));

    auto start = high_resolution_clock::now();
    for (int i=0;i<nb_iter;i++)
    {
        r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        int acc1 = rand() % max_key;

        if (r<=prob_insert)
        {
        
        h1.insert(to_string(acc1));
        //cout <<"done from thread "<<thread_id << "\n";
        }

        else
        {
            h1.remove(to_string(acc1));
        }
        

    }

    auto stop = high_resolution_clock::now();

    auto elapsed_time=duration_cast<milliseconds>(stop - start);

    std::cout<<"overall elapsed time<"<<elapsed_time.count()<<">"<< std::endl;

    cout <<"number of elements in map "<< h1.currentSize1+h1.currentSize2 << endl;

    cout<<"size of map after all operations "<<2*h1.size_array()<< endl;

    return 0;
}