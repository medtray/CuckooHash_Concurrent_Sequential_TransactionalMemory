#pragma once
#include <cstddef>
#include <mutex>
#include<vector>
#include <regex>
#include <numeric>
#include "UniformRandom.h"
#include <shared_mutex>
#include <set>
#include <atomic>

using namespace std;
int nextPrime( int n );

template <int count>
class StringHashFamily
{
  public:
    StringHashFamily( ) : MULTIPLIERS( count )
    {
        generateNewFunctions( );
    }
    
    int getNumberOfFunctions( ) const
    {
        return count;
    }
    
    void generateNewFunctions( )
    {
        for( auto & mult : MULTIPLIERS )
            //mult = r.nextInt( );
            mult=rand() % 100000 + 1;
    }
    
    size_t hash( const string & x, int which ) const
    {
        const int multiplier = MULTIPLIERS[ which ];
        size_t hashVal = 0;

        for( auto ch : x )
            hashVal = multiplier * hashVal + ch;
        
        return hashVal;
    }

  private:
    vector<int> MULTIPLIERS;
    UniformRandom r;
};



template <typename AnyType, typename HashFamily>

class simplemap
{

    public:

    int tableSize;
    int lock_size;
    int half_size_lock;

    //int currentSize1;
    //int currentSize2;
    atomic_int currentSize1;
    atomic_int currentSize2;
    int rehashes;
    UniformRandom r;
    int numHashFunctions;
    int COUNT_LIMIT;
    float MAX_LOAD;
    static const int ALLOWED_REHASHES = 5;
    std::atomic_int rehash_or_expand_counter;

    simplemap(int tab_size,int llock_size):tableSize(tab_size), lock_size(llock_size), //constructor of the class
    array3(2,vector<HashEntry>(nextPrime(tableSize))),mutexes3(2*nextPrime(lock_size)), 
    hashFunctions3(2),half_size_lock(nextPrime(lock_size))
        
    {
       // mutexes4.push_back(mutexes3);
       numHashFunctions = hashFunctions3[0].getNumberOfFunctions( );
        rehashes = 0;
        COUNT_LIMIT=100;
        MAX_LOAD=1;
        makeEmpty( );
    }

    struct HashEntry
    {
        AnyType element;
        bool isActive;

        HashEntry( const AnyType & e = AnyType( ), bool a = false )
          : element{ e }, isActive{ a } { }    

    };

    vector<vector<HashEntry>> array3;
    mutable vector<mutex> mutexes3;
    vector<HashFamily> hashFunctions3;
     std::shared_mutex rehash_expand_lock;
    

    


    public:
    
    void makeEmpty( )
    {
        currentSize1 = 0;
        currentSize2 = 0;
        
        rehash_or_expand_counter=0;
        for( auto & entry : array3[0] )
            entry.isActive = false;
        for( auto & entry : array3[1] )
            entry.isActive = false;
    }

    bool isActive( int currentPos,vector<HashEntry> array ) const
      {  return currentPos != -1 &&  array[ currentPos ].isActive; }

    bool contains( const AnyType & x ) const
    {
        std::pair<int,int> res=findPos( x );
        return res.first != -1;
    }

    int size_array() const
    {
        return array3[0].size();
    }
    
    int capacity( ) const
    {
        return array3[0].size( )+array3[1].size();
    }

    bool check_if_any_element_is_locked()
    {
        for (int i=0;i<mutexes3.size();i++)
        {
            mutex* m=&mutexes3[i];
            m->lock();
                
        }
        return true;
    }

    bool active_in_both_tables(const AnyType & x )  
    {
        int pos1;
        int pos2;
       
        pos1=hashFunctions3[0].hash(x, 0) % array3[0].size( );
        pos2=hashFunctions3[1].hash(x, 1) % array3[1].size( );

        if( isActive(pos1,array3[0]) && array3[0][pos1].element == x && isActive(pos2,array3[1]) && array3[1][pos2].element == x)
            return true;

        else
        {
            return false;
        }
    }


    std::pair <int,int> findPos( const AnyType & x ) const
    {
        int pos1;
        int pos2;
        std::pair <int,int> res;
        int pos2_table;
        int pos1_table;
        pos1_table=hashFunctions3[0].hash(x, 0) % array3[0].size( );
        pos2_table=hashFunctions3[1].hash(x, 1) % array3[1].size( );
        pos1=hashFunctions3[0].hash(x, 0) % half_size_lock;
        pos2=hashFunctions3[1].hash(x, 1) % half_size_lock+half_size_lock;

        mutex* m1 = &mutexes3[pos1];
        mutex* m2 = &mutexes3[pos2];

        std::vector<int> locks_prepare;
        locks_prepare.push_back(0);
        locks_prepare.push_back(0);

        
        while(1)
        {
            if (m1->try_lock()) {
                locks_prepare[0]=1;
        } else {
            locks_prepare[0]=0; 
        }

        if (m2->try_lock()) {
                locks_prepare[1]=1;
        } else {
            locks_prepare[1]=0; 
        }

        if (locks_prepare[0]==1&&locks_prepare[1]==1)
        {

            if( isActive(pos1_table,array3[0]) && array3[0][pos1_table].element == x )
        {
            res = std::make_pair(pos1_table,1); 
             m1->unlock();
            m2->unlock();
            return res;
        }
            
         
        if( isActive(pos2_table,array3[1]) && array3[1][pos2_table].element == x )
        {
            res = std::make_pair(pos2_table,2); 
             m1->unlock();
            m2->unlock();
            return res;
        }

        res = std::make_pair(-1,0); 
         m1->unlock();
        m2->unlock();

        return res;

        }

        else
        {
            if (locks_prepare[0]==1)
                m1->unlock();

            if (locks_prepare[1]==1)
                m2->unlock();   
            
        }

    }
        
    } 


     std::pair <int,int> findPos_locked_buckets( const AnyType & x ) const
    {
        int pos1;
        int pos2;
        std::pair <int,int> res;
        int pos2_table;
        int pos1_table;
        pos1_table=hashFunctions3[0].hash(x, 0) % array3[0].size( );
        pos2_table=hashFunctions3[1].hash(x, 1) % array3[1].size( );

        if( isActive(pos1_table,array3[0]) && array3[0][pos1_table].element == x )
        {
            res = std::make_pair(pos1_table,1); 
            return res;
        }
            
         
        if( isActive(pos2_table,array3[1]) && array3[1][pos2_table].element == x )
        {
            res = std::make_pair(pos2_table,2); 
            return res;
        }

        res = std::make_pair(-1,0); 

        return res;
        
    } 


    std::tuple<bool, vector<HashEntry>, vector<int>> discover_path(const AnyType & xx)
    {
        std::vector<HashEntry> discovered_elements;
        vector<int> discovered_positions;
        int pos;
        AnyType x=xx;
        HashEntry el;
        int curr_array;

        for (int count=0;count<COUNT_LIMIT;count++)
        {
            if (count%2==0)
            {
                curr_array=0;
                
            }
            else
            {
                curr_array=1;  
            }

            pos=hashFunctions3[curr_array].hash(x, curr_array) % array3[curr_array].size( );
            el=array3[curr_array][pos];
            discovered_elements.push_back(el);
            discovered_positions.push_back(pos);
            if (!isActive(pos,array3[curr_array]))
            {
                return make_tuple(true,discovered_elements,discovered_positions);

            }
            else
            {
                x=el.element;
            } 
            
        }

        return make_tuple(false,discovered_elements,discovered_positions);

    }

    tuple<bool,vector<int>,vector<int>> acquire_path_locks(vector<HashEntry> discovered_elements)
    {
        vector<int> succesful_locked;
        int current_array;
        int pos_to_lock;
        vector<int> pos_to_lock_vector;

        while(1)
        {

            for (int count=0;count<discovered_elements.size();count++)
        {

            if (count%2==0)
            {
                current_array=0;
                pos_to_lock=hashFunctions3[0].hash(discovered_elements[count].element, 0) % half_size_lock;               
            }
            else
            {
                current_array=1;
                pos_to_lock=hashFunctions3[1].hash(discovered_elements[count].element, 1) % half_size_lock+half_size_lock; 
                
            }

            pos_to_lock_vector.push_back(pos_to_lock);

            if(std::find(succesful_locked.begin(), succesful_locked.end(), pos_to_lock) == succesful_locked.end())
            {
                mutex* m = &mutexes3[pos_to_lock];
            if(m->try_lock())
                succesful_locked.push_back(pos_to_lock);
            else
                break; 
            }                

        }

        if (succesful_locked.size()==std::set<int>( pos_to_lock_vector.begin(), pos_to_lock_vector.end()).size())
            return make_tuple(true,succesful_locked,pos_to_lock_vector);
        else
        {
            for( int  entry : succesful_locked )
            {
                mutex* m = &mutexes3[entry];
                m->unlock();
            }
                   

            succesful_locked.clear();
            pos_to_lock_vector.clear();

        }

        }

    }

    void release_path_locks(vector<int> discovered_lock_positions)
    {
        int current_pos;
        int current_array;
        
        
        for (int count=0;count<discovered_lock_positions.size();count++)
        {
            current_pos=discovered_lock_positions[count];

            mutex* m = &mutexes3[current_pos];
            m->unlock();              

        }

    }

    bool try_to_insert_in_first_or_second_positions(const AnyType & xx)
    {
        int pos1;
        int pos2;

        pos1=hashFunctions3[0].hash(xx, 0) % array3[0].size( );
        /* if(pos1==100)
            cout<<"heree"; */
        if(!isActive(pos1,array3[0]))
        {
            // if (currentSize1.load()>8)
            //     cout<<"here";

            array3[0][pos1] = std::move( HashEntry{ std::move( xx ), true } );
            //++currentSize1;
            //currentSize1.store(currentSize1+1);
            
            ++currentSize1;
            //currentSize1.fetch_add(1);
            
            return true;
        }

        pos2=hashFunctions3[1].hash(xx, 1) % array3[1].size( );

        if(!isActive(pos2,array3[1]))
        {
            array3[1][pos2] = std::move( HashEntry{ std::move( xx ), true } );
            ++currentSize2;
            return true;
        }

        return false;


    }

    bool check_if_contains_with_locked_buckets(const AnyType & xx)
    {
        int pos1;
        int pos2;

        pos1=hashFunctions3[0].hash(xx, 0) % array3[0].size( );
        if( isActive(pos1,array3[0]) && array3[0][pos1].element == xx )
            return true;

        pos2=hashFunctions3[1].hash(xx, 1) % array3[1].size( );
        if( isActive(pos2,array3[1]) && array3[1][pos2].element == xx )
            return true;

        return false;

        

    }

    pair<int,int> count_active_buckets()
    {

        int nb1=0;
        int nb2=0;

        for( auto & entry : array3[0] )
        {
            if(entry.isActive)
                ++nb1;
        }
            
        for( auto & entry : array3[1] )
        {
            if(entry.isActive)
                ++nb2;
        }

    return make_pair(nb1,nb2);

    }

    void release_two_buckets(const AnyType & xx)
    {
        int pos1;
        int pos2;
        std::pair <int,int> res;
        pos1=hashFunctions3[0].hash(xx, 0) % half_size_lock;
        pos2=hashFunctions3[1].hash(xx, 1) % half_size_lock+half_size_lock;
        mutex* m1 = &mutexes3[pos1];
        mutex* m2 = &mutexes3[pos2];

        /* if (m1->try_lock())
            cout<<"not locked!!";
        if (m2->try_lock())
            cout<<"not locked!!"; */

        m1->unlock();
        m2->unlock();

    }

    bool lock_two_buckets(const AnyType & xx)
    {
        int pos1;
        int pos2;
        std::pair <int,int> res;
        pos1=hashFunctions3[0].hash(xx, 0) % half_size_lock;
        pos2=hashFunctions3[1].hash(xx, 1) % half_size_lock+half_size_lock;
        mutex* m1 = &mutexes3[pos1];
        mutex* m2 = &mutexes3[pos2];

        std::vector<int> locks_prepare;
        locks_prepare.push_back(0);
        locks_prepare.push_back(0);

        

        
        while(1)
        {
            if (m1->try_lock()) {
                locks_prepare[0]=1;
        } else {
            locks_prepare[0]=0; 
        }

        if (m2->try_lock()) {
                locks_prepare[1]=1;
        } else {
            locks_prepare[1]=0; 
        }

        if (locks_prepare[0]==1&&locks_prepare[1]==1)
        {
            return true;
        }

        else
        {
            if (locks_prepare[0]==1)
                m1->unlock();

            if (locks_prepare[1]==1)
                m2->unlock();     
        }
    }
    }


    std::pair <bool,AnyType> try_to_insert(AnyType x)
    {

            int pos1;
            int pos2;
            std::pair <bool,AnyType> res;
            
            for( int count = 0; count < COUNT_LIMIT; ++count )
            {
                if (count==0)
                {

                    pos1=hashFunctions3[0].hash(x, 0) % array3[0].size( );  
                    if(!isActive(pos1,array3[0]))
                    {
                        array3[0][pos1] = std::move( HashEntry{ std::move( x ), true } );
                        ++currentSize1;
                        res=std::make_pair(true,x);
                        return res;
                    }

                    pos2=hashFunctions3[1].hash(x, 1) % array3[1].size( ) ; 
                    if(!isActive(pos2,array3[1]))
                    {
                        array3[1][pos2] = std::move(HashEntry{ std::move( x ), true });
                        ++currentSize2;
                        res=std::make_pair(true,x);
                        return res;
                    }
                }

                else
                {
                    if (count%2==1)
                    {
                        pos2=hashFunctions3[1].hash(x, 1) % array3[1].size( ) ; 
                        if(!isActive(pos2,array3[1]))
                        {
                            array3[1][pos2] = std::move(HashEntry{ std::move( x ), true });
                            ++currentSize2;
                            res=std::make_pair(true,x);
                            return res;
                        }

                    }
                    else
                    {
                        pos1=hashFunctions3[0].hash(x, 0) % array3[0].size( ) ; 
                        if(!isActive(pos1,array3[0]))
                        {
                            array3[0][pos1] = std::move( HashEntry{ std::move( x ), true } );
                            ++currentSize1;
                            res=std::make_pair(true,x);
                            return res;
                        }
                        
                    }
                    
                }
                
                // None of the spots are available. Kick out random one
               
                if (count%2==0)
                    std::swap( x, array3[0][pos1].element );
                else
                    std::swap( x, array3[1][pos2].element );

            }

            res=std::make_pair(false,x);
            return res;

    }

    bool expand()
    {
        hashFunctions3[0].generateNewFunctions();
        hashFunctions3[1].generateNewFunctions();
        return rehash(static_cast<int>(array3[0].size( ) / MAX_LOAD));
        
    }
    
    bool rehash()
    {
        hashFunctions3[0].generateNewFunctions();
        hashFunctions3[0].generateNewFunctions();
        return rehash(array3[0].size());
    }
    
    bool rehash(int newSize)
    {
        //bool rr=check_if_any_element_is_locked();

        std::pair <bool,AnyType> res;
        vector<HashEntry> oldArray1 = array3[0];
        vector<HashEntry> oldArray2 = array3[1];
        int cur_size1=currentSize1;
        int cur_size2=currentSize2;
            // Create new double-sized, empty table
        array3[0].resize( nextPrime( newSize ) );
        for( auto & entry : array3[0] )
            entry.isActive = false;

        array3[1].resize( nextPrime( newSize ) );
        for( auto & entry : array3[1] )
            entry.isActive = false;

        //int ss=array3[0].size();
        
            // Copy table over
        currentSize1 = 0;
        currentSize2 = 0;
        for( auto & entry : oldArray1 )
            if( entry.isActive )
            {
                res=try_to_insert(entry.element);
                if (!res.first)
                {
                    array3[0]=oldArray1;
                    array3[1]=oldArray2;
                    currentSize1=cur_size1;
                    currentSize2=cur_size2;
                    return false;
                }
                    
            }
                

        for( auto & entry : oldArray2)
            if( entry.isActive )
            {
                res=try_to_insert(entry.element);
                if (!res.first)
                {
                    array3[0]=oldArray1;
                    array3[1]=oldArray2;
                    currentSize1=cur_size1;
                    currentSize2=cur_size2;
                    return false;
                }
                    
            }

        

        return true;
    }


     bool insertHelper1(const AnyType & xx )
    {
        
        AnyType x = xx;
        std::pair <bool,AnyType> res;
        res=std::make_pair(false,x);
        bool modified=true;

        std::tuple<bool, vector<HashEntry>, vector<int>>  res1;
        std::vector<HashEntry> discovered_elements;
        vector<int> discovered_positions;
        int nb_locked=0;
        HashEntry curr;
        int current_pos;
        int current_array;
        int pos_to_lock;
        vector<int> locked_pos;

        
        rehash_expand_lock.lock_shared();
        while( true )
        {
                
                lock_two_buckets(x);
                if(check_if_contains_with_locked_buckets(x))
                {
                    release_two_buckets(x);
                    
                    rehash_expand_lock.unlock_shared();
                    return false;
                }

                else if(try_to_insert_in_first_or_second_positions(x))
                {
                    release_two_buckets(x);
                    rehash_expand_lock.unlock_shared();
                    return true;

                }

                else
                {
                    release_two_buckets(x);
                    res1=discover_path(x);
                    if (get<0>(res1)==true)

                    {
                        discovered_elements=get<1>(res1);
                        discovered_positions=get<2>(res1);
                        int verify=0;
                        tuple<bool,vector<int>,vector<int>> res_lock=acquire_path_locks(discovered_elements);
                        for (int count=0;count<discovered_elements.size()-1;count++)
                        {
                            current_pos=discovered_positions[count];
                            curr=discovered_elements[count];

                            if (count%2==0)
                            {
                                current_array=0;             
                            }
                            else
                            {
                                current_array=1;
                                    
                            }
                            
                            if ((curr.element==array3[current_array][current_pos].element) && isActive(current_pos,array3[current_array]))
                                ++verify;
                            else
                                break;
                                              

                        }

                        if (verify==(discovered_elements.size()-1))
                        {
                            int count=discovered_elements.size()-1;
                            current_pos=discovered_positions[count];
                            if (count%2==0)
                            {
                                current_array=0;

                            }
                            else
                            {
                                current_array=1;
                                
                            }

                            if (!isActive(current_pos,array3[current_array]))
                            {
                                for (int count=discovered_elements.size()-1;count>0;count--)
                                {
                                    current_pos=discovered_positions[count];
                                    curr=discovered_elements[count-1];

                                    if (count%2==0)
                                    {
                                        current_array=0;

                                    }

                                    else
                                    {
                                        current_array=1;
                                        
                                    }

                                    array3[current_array][current_pos]=curr;
                                    pos_to_lock=get<2>(res_lock)[count];
                                    vector<int>::const_iterator first = get<2>(res_lock).begin();
                                    vector<int>::const_iterator last = get<2>(res_lock).begin() + count;
                                    vector<int> newVec(first, last);

                                    if(std::find(newVec.begin(), newVec.end(), pos_to_lock) == newVec.end())
                                        mutexes3[pos_to_lock].unlock();
                                    
                                }

                                current_pos=discovered_positions[0];
                                array3[0][current_pos]=std::move(HashEntry{std::move(xx),true});
                                if (discovered_positions.size()%2==1)
                                    ++currentSize1;
                                else
                                    ++currentSize2;
                                    
                                pos_to_lock=get<2>(res_lock)[0];
                                mutexes3[pos_to_lock].unlock();
                                rehash_expand_lock.unlock_shared();
                                return true;


                            }

                            else
                            {

                                //path changed (unlock)
                                release_path_locks(get<1>(res_lock));
                                
                            }
                            


                        }
                        else
                        {
                            //path changed (unlock)
                            release_path_locks(get<1>(res_lock));
                        }
                        

                    }
                    else
                    {
                        rehash_expand_lock.unlock_shared();
                        int cur_before_rehash_expand_val=rehash_or_expand_counter;
                        rehash_expand_lock.lock();
                        int cur_after_rehash_expand_val=rehash_or_expand_counter;

                        if (cur_after_rehash_expand_val==cur_before_rehash_expand_val)

                        {
                            modified=false;
                        while(!modified)
                        {
                                //if there is no path (rehash or expand)
                                if( ++rehashes > ALLOWED_REHASHES )
                                    {
                                        modified=expand( );     // Make the table bigger
                                        if (modified==true)
                                            rehashes = 0;
                                        else
                                        {
                                            MAX_LOAD=std::max(MAX_LOAD-0.1,0.1);
                                        }
                                    }
                                    else
                                    {
                                        modified=rehash( );
                                    }

                        }

                        ++rehash_or_expand_counter;

                        rehash_expand_lock.unlock();
                        rehash_expand_lock.lock_shared();

                        }

                        else
                        {
                            rehash_expand_lock.unlock();
                            rehash_expand_lock.lock_shared();
                        }
                                                                         
                    }
                    
                }              
            
        }
    }


bool insert( const AnyType & x )
    {
        
        //cout<< active_in_both_tables("4");
        //cout<< contains("82");
        //std::cout.flush();
        // if( contains( x ) ) 
        //     return false;
        
        // if( currentSize1+currentSize2 >= (array1.size( )+array2.size()) * MAX_LOAD )
        //     expand( );

        //acquire both locks check then release
        
        return insertHelper1( x );
    }


    bool remove( const AnyType & x )
    {
        rehash_expand_lock.lock_shared();
        lock_two_buckets(x);
        std::pair<int,int> res=findPos_locked_buckets( x );
        if (res.first==-1)
        {
            release_two_buckets(x);
            rehash_expand_lock.unlock_shared();
            return false;
        }
            
        else
        {
            if (res.second==1)
            {
                // if (currentSize1.load()>8)
                //     cout<<"here";

                array3[0][res.first].isActive = false;
                --currentSize1;
                //currentSize1.store(currentSize1-1);
                //currentSize1.fetch_sub(1);
                
            }
                
            
            if (res.second==2)
            {
                array3[1][res.first].isActive=false;
                --currentSize2;
                //currentSize2.store(currentSize2-1);
            }

            release_two_buckets(x);
            rehash_expand_lock.unlock_shared();
                
        }
        /* if( !isActive( currentPos ) )
            return false; */

        return true;
    }




};
