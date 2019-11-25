#include <iostream>
#include <sstream>
//#include "cucko.h"
#include "cucko_concurrent.h"
using namespace std;
#include <thread>
#include <chrono>
using namespace std::chrono;
#include <time.h>

    // Simple main
int main( )
{
    const int NUMS = 10;
    
    const int ATTEMPTS = 1;
    int i;

    int nb_threads=1;
    int nb_iter=10000;
    int max_key=20000;
    srand (time(NULL));

    float prob_insert=0.6;
    simplemap<string,StringHashFamily<3>> h1(100,1000);

    cout<<"initial size of map "<<2*h1.size_array()<< endl;
   
   std::vector<double> proc_time(nb_threads, 0.0);
  // Define a Lambda Expression 
    auto do_work = [&](int iter, int max_key,int thread_id) { 
        float d=10;
		float r;
		auto start = high_resolution_clock::now();
			for (int i=0;i<iter;i++)
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
			std::cout<<"<"<<elapsed_time.count()<<","<<thread_id<<">"<< std::endl;

		proc_time[thread_id]=elapsed_time.count();
    }; 


     auto do_insert = [&](int iter, int max_key,int thread_id) { 
        float d=10;
		float r;
		auto start = high_resolution_clock::now();
			for (int i=0;i<iter;i++)
			{
				r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                int acc1 = rand() % max_key;

				h1.insert(to_string(acc1));
				//cout <<"done from thread "<<thread_id << "\n";

			}

			auto stop = high_resolution_clock::now();

			auto elapsed_time=duration_cast<milliseconds>(stop - start);
			std::cout<<"<"<<elapsed_time.count()<<","<<thread_id<<">"<< std::endl;

		proc_time[thread_id]=elapsed_time.count();
    }; 

    auto do_remove = [&](int iter, int max_key,int thread_id) { 
        float d=10;
		float r;
		auto start = high_resolution_clock::now();
			for (int i=0;i<iter;i++)
			{
				r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                int acc1 = rand() % max_key;

				h1.remove(to_string(acc1));
				//cout <<"done from thread "<<thread_id << "\n";

			}

			auto stop = high_resolution_clock::now();

			auto elapsed_time=duration_cast<milliseconds>(stop - start);
			std::cout<<"<"<<elapsed_time.count()<<","<<thread_id<<">"<< std::endl;

		proc_time[thread_id]=elapsed_time.count();
    }; 

    auto start = high_resolution_clock::now();

		 std::vector<thread> threads(nb_threads);
    // spawn n threads:
    for (int i = 0; i < nb_threads; i++) {
        threads[i] = thread(do_work, nb_iter,max_key,i);
    }

    for (auto& th : threads) {
        th.join();
    }

     /* std::vector<thread> threads_ins(nb_threads);
    // spawn n threads:
    for (int i = 0; i < nb_threads; i++) {
        threads_ins[i] = thread(do_insert, nb_iter,max_key,i);
    }

    for (auto& th : threads_ins) {
        th.join();
    }


     std::vector<thread> threads_rem(nb_threads);
    // spawn n threads:
    for (int i = 0; i < nb_threads; i++) {
        threads_rem[i] = thread(do_remove, nb_iter,max_key,i);
    }

    for (auto& th : threads_rem) {
        th.join();
    } */

	auto stop = high_resolution_clock::now();

    auto elapsed_time=duration_cast<milliseconds>(stop - start);
    std::cout<<"overall elapsed time<"<<elapsed_time.count()<<">"<< std::endl;
        
        
        //cout<<h1.check_if_any_element_is_locked()<< endl;

        //cout <<"number of elements in map "<< h1.currentSize1+h1.currentSize2 << endl;

    pair<int,int> nb=h1.count_active_buckets();
    cout <<"number of elements in map "<< nb.first+nb.second << endl;

   // cout<<"no bucket is locked after finishing "<<h1.check_if_any_element_is_locked()<< endl;
    cout<<"size of map after all operations "<<2*h1.size_array()<< endl;

    /* for(i = 0;i<max_key;i++)
        if( h1.active_in_both_tables( to_string( i ) ) )
            cout << "active in both tables!!! " << i << endl; */

        cout << "finish" << endl;

        
        

    return 0;
}