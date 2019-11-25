#include <iostream>
#include <sstream>
//#include "cucko.h"
#include "cucko_tm.h"
using namespace std;
#include <thread>
#include <chrono>
using namespace std::chrono;

    // Simple main
int main( )
{
    const int NUMS = 10;
    
    const int ATTEMPTS = 1;
    int i;

    int nb_threads=2;
    int nb_iter=10000;
    int max_key=5000;

    float prob_insert=1;
    simplemap<int,IntHashFamily<3>> h1(100);

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
				
				h1.insert(acc1);
				//cout <<"done from thread "<<thread_id << "\n";
				}

				else
				{
					h1.remove(acc1);
				}
				

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

	auto stop = high_resolution_clock::now();

    auto elapsed_time=duration_cast<milliseconds>(stop - start);
    std::cout<<"overall elapsed time<"<<elapsed_time.count()<<">"<< std::endl;
        
        
        //cout<<h1.check_if_any_element_is_locked()<< endl;

        //cout <<"number of elements in map "<< h1.currentSize1+h1.currentSize2 << endl;

    pair<int,int> nb=h1.count_active_buckets();
    cout <<"number of elements in map "<< nb.first+nb.second << endl;

    cout<<"size of map after all operations "<<2*h1.size_array()<< endl;

        cout << "finish" << endl;

        
        

    return 0;
}