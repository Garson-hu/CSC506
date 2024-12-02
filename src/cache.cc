
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <stdio.h>
#include "cache.h"
using namespace std;

Cache::Cache(int s,int a,int b )
{
   
    ulong i, j;

    lineSize = (ulong)(b);
    sets = 1;                // Only one set for fully associative
    assoc = 30000;           // Large associativity to simulate "infinite" cache
    numLines = assoc;        // The number of lines is equal to associativity
    
    
    cache = new cacheLine*[sets];
    for (i = 0; i < sets; i++) {
        cache[i] = new cacheLine[assoc];
        for (j = 0; j < assoc; j++) {
            cache[i][j].invalidate();
        }
    }    
   
}


void Cache::MESI_Processor_Access(ulong addr,uchar rw, int copy , Cache **cache, int processor, int num_processors )
{
	// check if the cache line is in the processer's cache
    cacheLine *line = findLine(addr);

    // ! check the cache line is hit or not
    if(line)
    {
        ulong cache_flag = line->getFlags();
        // ! read or write
        // ! read
        if(rw == 'r')
        {
            if((cache_flag == Shared) || (cache_flag == Modified) || (cache_flag == Exclusive))
            {   
                reads++;
                Readhits++;
                Total_execution_time += 1;
                updateLRU(line);
                return;
            }
            
        }else
        {   // ! else for read or write, now write
            // printf("write1\n");
            if((cache_flag == Exclusive) || (cache_flag == Modified))
            {
                line->setFlags(Modified);
                writes++;
                Writehits++;
                Total_execution_time += 3;
                updateLRU(line);
                return;
            }

            if(cache_flag == Shared)
            {
                writes++;
                Writehits++;
                busupgr++;
                // printf("write2\n");
                Total_execution_time += 3;
                for (int i = 0; i < num_processors; i++) 
                {
                    if (i != processor) 
                    {
                        cache[i]->MESI_Bus_Snoop(addr, 0, 0, 1);
                    }
                }
                line->setFlags(Modified);
                updateLRU(line);
                return;
            }
        }

    }

    // ! if cache line not hit
    if (rw == 'r') // ! if read miss
    {
        reads++;
        readMisses++;
        if(copy)
        { 
            for (int i = 0; i < num_processors; i++) 
            {
                if (i != processor) 
                {
                    cache[i]->MESI_Bus_Snoop(addr, 1, 0, 0);
                }
            }
            Total_execution_time += 15;

            fillLine(addr)->setFlags(Shared);
            printf("After fill line shared: %d\n", findLine(addr)->getFlags());
        }else
        {
            mem_trans++;
            fillLine(addr)->setFlags(Exclusive);
            Total_execution_time += 100;
        }
    }

    if(rw == 'w') // ! if write miss
    {

        writes++;  
        writeMisses++;
        mem_trans++;
        fillLine(addr)->setFlags(Exclusive);
        Total_execution_time += 100;
        printf("after fill line write: %d\n", findLine(addr)->getFlags());

    }
}

void Cache::MESI_Bus_Snoop(ulong addr , int busread,int busreadx, int busupgrade )
{
    cacheLine *line = findLine(addr);

    if(!line)
        return;

    ulong cache_flag = line->getFlags();

    if(busread)
    {
        if (cache_flag == Modified)
        {
            // writeBacks++;
            writeBack(addr);
            line->setFlags(Shared);
            flushes++;
            Total_execution_time += 100;
        }

        if(cache_flag == Exclusive)
        {
            line->setFlags(Shared);
            flushes++;
            Total_execution_time += 15;
        }

        if(cache_flag == Shared)
        {
            flushes++;
            Total_execution_time += 15;
        }

    }
    
    if(busreadx)
    {
        if((cache_flag == Shared) || (cache_flag == Modified) || (cache_flag == Exclusive))
        {
            flushes++;
            line->setFlags(INVALID);
            invalidations++;
            // printf();
            if(cache_flag == Shared || (cache_flag == Exclusive))
                Total_execution_time += 15;
            if(cache_flag == Modified)
                Total_execution_time += 100;
        }
    }
    // printf("middle RdX and upgrade\n");
    if(busupgrade)
    {
        if(cache_flag == Shared)
        {
            // flushes++;
            line->setFlags(INVALID);
            invalidations++;
            // Total_execution_time += 15;

        }
    }
}

void Cache::MOESI_Processor_Access(ulong addr,uchar rw, int copy, Cache **cache, int processor, int num_processors )
{
	cacheLine *line = findLine(addr);
    // ! check the line is hit or not
    if(line) // ! if hit
    {
        
        ulong cache_flag = line->getFlags();
        // ! check it's read or write, now read
        if(rw == 'r')
        {
            if((cache_flag == Owner))
            {
                reads++;
                Readhits++;
                Total_execution_time += 1;
                updateLRU(line);
                return;
            }

            if((cache_flag == Exclusive) || (cache_flag == Shared) || (cache_flag == Modified))
            {
                reads++;
                Readhits++;
                Total_execution_time += 1;
                updateLRU(line);
                return;
            }
        }
        else // ! if rw is write
        {
            if((cache_flag == Owner) || (cache_flag == Shared))
            {
                writes++;
                Writehits++;
                Total_execution_time += 3;
                updateLRU(line);
                for (int i = 0; i < num_processors; i++) 
                {
                    if (i != processor) 
                    {
                        cache[i]->MOESI_Bus_Snoop(addr, 0, 0, 1);
                    }
                }
                line->setFlags(Modified);
                return;
            }

            if((cache_flag == Modified))
            {
                writes++;
                Writehits++;
                Total_execution_time += 3;
                updateLRU(line);
                return;
            }

            if((cache_flag == Exclusive))
            {
                writes++;
                Writehits++;
                Total_execution_time += 3;
                updateLRU(line);
                line->setFlags(Modified);
                return;
            }
        }

    }

    // ! if miss
    if(rw == 'r')
    {
        reads++;
        readMisses++;
        if(copy)
        {
            for (int i = 0; i < num_processors; i++) 
            {
                if (i != processor) 
                {
                    cache[i]->MOESI_Bus_Snoop(addr, 1, 0, 0);
                }
            }
            fillLine(addr)->setFlags(Shared);
            Total_execution_time += 15;
            
        }
        else
        {
            mem_trans++;
            fillLine(addr)->setFlags(Exclusive);
            Total_execution_time += 100;

        }
    }

    if(rw == 'w')
    {
        writes++;
        writeMisses++;
        mem_trans++;
        Total_execution_time += 100;
        fillLine(addr)->setFlags(Modified);
    }

}

void Cache::MOESI_Bus_Snoop(ulong addr , int busread,int busreadx, int busupgrade )
{
	cacheLine *line = findLine(addr);

    if(!line)
        return;

    ulong cache_flag = line->getFlags();

    if(busread)
    {
        if (cache_flag == Modified)
        {
            flushes++;
            writeBack(addr);
            line->setFlags(Owner);
            Total_execution_time += 100;

        }

        if(cache_flag == Owner)
        {
            flushes++;
            Total_execution_time += 15;
        }

        if(cache_flag == Exclusive)
        {
            // flushes++;
            line->setFlags(Shared);
            Total_execution_time += 15;

        }
    }
    
    if(busreadx)
    {
        if((cache_flag == Modified) || (cache_flag == Exclusive) || (cache_flag == Owner))
        {
            line->setFlags(INVALID);
            invalidations++;
            flushes++;
            if(cache_flag == Shared || (cache_flag == Exclusive))
                Total_execution_time += 15;
            if(cache_flag == Modified)
                Total_execution_time += 15;
        }

        if((cache_flag == Shared))
        {
            // flushes++;
            line->setFlags(INVALID);
            invalidations++;
        }

    }

    if(busupgrade)
    {
        if((cache_flag == Shared) || (cache_flag == Owner))
        {
            line->setFlags(INVALID);
            // printf("invalidation here");
            invalidations++;
        }
    }
}

/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
	if(cache[i][j].isValid())
	    if(cache[i][j].getTag() == tag)
		{
		     pos = j; break; 
		}
    if(pos == assoc)
        return NULL;
    else
	    return &(cache[i][pos]); 
}


/*Most of these functions are redundant so you can use/change it if you want to*/

/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
  line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);
   
   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) 
	    return &(cache[i][j]);     
   }   

   for(j=0; j<assoc; j++)
   {
	 if(cache[i][j].getSeq() <= min) { victim = j; min = cache[i][j].getSeq();}
   } 
   assert(victim != assoc);
   std::cout << "victim" << victim << std::endl;
   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);
  
   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);

   assert(victim != 0);
   if ((victim->getFlags() == Modified))
   {
	   writeBack(addr);
   }
   victim->setFlags(Shared);	
   tag = calcTag(addr);   
   victim->setTag(tag);

   return victim;
}

void Cache::printStats()
{ 
	//printf("===== Simulation results      =====\n");
	float miss_rate = (float)(getRM() + getWM()) * 100 / (getReads() + getWrites());
	
printf("01. number of reads:                                 %10lu\n", getReads());
printf("02. number of read misses:                           %10lu\n", getRM());
printf("03. number of writes:                                %10lu\n", getWrites());
printf("04. number of write misses:                          %10lu\n", getWM());
printf("05. number of write hits:                            %10lu\n", getWH());
printf("06. number of read hits:                             %10lu\n", getRH()); // Changed from getRM() to getRH()
printf("07. total miss rate:                                 %10.2f%%\n", miss_rate);
printf("08. number of memory accesses (exclude writebacks):  %10lu\n", mem_trans);
printf("09. number of invalidations:                         %10lu\n", Invalidations());
printf("10. number of flushes:                               %10lu\n", Flushes());

	
}

void Cache::printCacheState(ulong state) {
    switch (state) {
        case INVALID:
            std::cout << "I";
            break;
        case Shared:
            std::cout << "S";
            break;
        case Modified:
            std::cout << "M";
            break;
        case Exclusive:
            std::cout << "E";
            break;
        default:
            std::cout << "-";
            break;
    }
}
