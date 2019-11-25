#include <cstdlib>
#include <vector>
#include <algorithm>
#include <string>
#include "UniformRandom.h"
#include <utility> 
#include <stdlib.h>
using namespace std;

template <typename AnyType>
class CuckooHashFamily
{
  public:
    size_t hash( const AnyType & x, int which ) const;
    int getNumberOfFunctions( );
    void generateNewFunctions( );
};

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



int nextPrime( int n );



template <typename AnyType, typename HashFamily>
class HashTable
{
  public:
    explicit HashTable( int size = 101 ) : array1( nextPrime( size )),array2( nextPrime( size ))
    {
        numHashFunctions = hashFunctions1.getNumberOfFunctions( );
        rehashes = 0;
        COUNT_LIMIT=100;
        MAX_LOAD=1;
        makeEmpty( );
    }

    bool contains( const AnyType & x ) const
    {
        std::pair<int,int> res=findPos( x );
        return res.first != -1;
    }

    void makeEmpty( )
    {
        currentSize1 = 0;
        currentSize2 = 0;
        for( auto & entry : array1 )
            entry.isActive = false;
        for( auto & entry : array2 )
            entry.isActive = false;
    }

    bool insert( const AnyType & x )
    {
        
        //cout<< active_in_both_tables("4");
        //cout<< contains("82");
        //std::cout.flush();
        if( contains( x ) ) 
            return false;
        
        if( currentSize1+currentSize2 >= (array1.size( )+array2.size()) * MAX_LOAD )
            expand( );
        
        return insertHelper1( x );
    }

   /*  int size( ) const
    {
        return currentSize;
    } */

    int size_array() const
    {
        return array1.size();
    }
    
    int capacity( ) const
    {
        return array1.size( )+array2.size();
    }
    
    bool remove( const AnyType & x )
    {
        std::pair<int,int> res=findPos( x );
        if (res.first==-1)
            return false;
        else
        {
            if (res.second==1)
            {
                array1[res.first].isActive = false;
                currentSize1--;
            }
                
            
            if (res.second==2)
            {
                array2[res.first].isActive=false;
                currentSize2--;
            }
                
        }
        
        /* if( !isActive( currentPos ) )
            return false; */

        return true;
    }

  public:
      
    struct HashEntry
    {
        AnyType element;
        bool isActive;

        HashEntry( const AnyType & e = AnyType( ), bool a = false )
          : element{ e }, isActive{ a } { }    

    };
    
    vector<HashEntry> array1;
    vector<HashEntry> array2;
    int currentSize1;
    int currentSize2;
    int rehashes;
    UniformRandom r;
    HashFamily hashFunctions1;
    HashFamily hashFunctions2;
    int numHashFunctions;
    int COUNT_LIMIT;
    float MAX_LOAD;

    static const int ALLOWED_REHASHES = 5;

    std::pair <bool,AnyType> try_to_insert(AnyType x)
    {

            int pos1;
            int pos2;
            std::pair <bool,AnyType> res;
            
            for( int count = 0; count < COUNT_LIMIT; ++count )
            {
                if (count==0)
                {

                    pos1=hashFunctions1.hash(x, 0) % array1.size( );  
                    if(!isActive(pos1,array1))
                    {
                        array1[pos1] = std::move( HashEntry{ std::move( x ), true } );
                        ++currentSize1;
                        res=std::make_pair(true,x);
                        return res;
                    }

                    pos2=hashFunctions2.hash(x, 1) % array2.size( ) ; 
                    if(!isActive(pos2,array2))
                    {
                        array2[pos2] = std::move(HashEntry{ std::move( x ), true });
                        ++currentSize2;
                        res=std::make_pair(true,x);
                        return res;
                    }
                }

                else
                {
                    if (count%2==1)
                    {
                        pos2=hashFunctions2.hash(x, 1) % array2.size( ) ; 
                        if(!isActive(pos2,array2))
                        {
                            array2[pos2] = std::move(HashEntry{ std::move( x ), true });
                            ++currentSize2;
                            res=std::make_pair(true,x);
                            return res;
                        }

                    }
                    else
                    {
                        pos1=hashFunctions1.hash(x, 0) % array1.size( ) ; 
                        if(!isActive(pos1,array1))
                        {
                            array1[pos1] = std::move( HashEntry{ std::move( x ), true } );
                            ++currentSize1;
                            res=std::make_pair(true,x);
                            return res;
                        }
                        
                    }
                    
                }
                
                // None of the spots are available. Kick out random one
               
                if (count%2==0)
                    std::swap( x, array1[pos1].element );
                else
                    std::swap( x, array2[pos2].element );

            }

            res=std::make_pair(false,x);
            return res;

    }



       bool insertHelper1( const AnyType & xx )
    {
        
        AnyType x = xx;
        std::pair <bool,AnyType> res;
        res=std::make_pair(false,x);
        bool modified=true;
        
        while( true )
        {
                res=try_to_insert(res.second);
                if (res.first)
                    return true;
                else
                {
                    
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
                    modified=rehash( );
                    
                }
            
        }
    }
    
    

    bool isActive( int currentPos,vector<HashEntry> array ) const
      {  return currentPos != -1 &&  array[ currentPos ].isActive; }

    bool active_in_both_tables(const AnyType & x )  
    {
        int pos1;
        int pos2;
       
        pos1=hashFunctions1.hash(x, 0) % array1.size( ) ; 
        pos2=hashFunctions2.hash(x, 1) % array2.size( ) ;

        if( isActive(pos1,array1) && array1[pos1].element == x && isActive(pos2,array2) && array2[pos2].element == x)
            return true;

        else
        {
            return false;
        }
    }

    // Method that search all hash function places
    std::pair <int,int> findPos( const AnyType & x ) const
    {
        int pos;
        std::pair <int,int> res;
        pos=hashFunctions1.hash(x, 0) % array1.size( ) ; 
        if( isActive(pos,array1) && array1[ pos ].element == x )
        {
            res = std::make_pair(pos,1); 
            return res;
        }
            
        pos=hashFunctions2.hash(x, 1) % array2.size( ) ; 
        if( isActive(pos,array2) && array2[ pos ].element == x )
        {
            res = std::make_pair(pos,2); 
            return res;
        }

        res = std::make_pair(-1,0); 

        return res;
    }

    bool expand()
    {
        hashFunctions1.generateNewFunctions();
        hashFunctions2.generateNewFunctions();
        return rehash(static_cast<int>(array1.size( ) / MAX_LOAD));
        
    }
    
    bool rehash()
    {
        hashFunctions1.generateNewFunctions();
        hashFunctions2.generateNewFunctions();
        return rehash(array1.size());
    }
    
    bool rehash(int newSize)
    {
        std::pair <bool,AnyType> res;
        vector<HashEntry> oldArray1 = array1;
        vector<HashEntry> oldArray2 = array2;
        int cur_size1=currentSize1;
        int cur_size2=currentSize2;
            // Create new double-sized, empty table
        array1.resize( nextPrime( newSize ) );
        for( auto & entry : array1 )
            entry.isActive = false;

        array2.resize( nextPrime( newSize ) );
        for( auto & entry : array2 )
            entry.isActive = false;
        
            // Copy table over
        currentSize1 = 0;
        currentSize2 = 0;
        for( auto & entry : oldArray1 )
            if( entry.isActive )
            {
                res=try_to_insert(entry.element);
                if (!res.first)
                {
                    array1=oldArray1;
                    array2=oldArray2;
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
                    array1=oldArray1;
                    array2=oldArray2;
                    currentSize1=cur_size1;
                    currentSize2=cur_size2;
                    return false;
                }
                    
            }

        return true;
    }
    
    
};
