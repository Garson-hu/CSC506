#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>
typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned int uint;
extern unsigned long Total_execution_time;

/****add new states, based on the protocol****/
enum {
    INVALID = 0,
    Shared,
    Modified,
    Exclusive,
    Owner
};

class cacheLine {
protected:
    ulong tag;
    ulong Flags;
    ulong seq;

public:
    cacheLine() : tag(0), Flags(0), seq(0) {}
    ulong getTag() { return tag; }
    ulong getFlags() { return Flags; }
    ulong getSeq() { return seq; }
    void setSeq(ulong Seq) { seq = Seq; }
    void setFlags(ulong flags) { Flags = flags; }
    void setTag(ulong a) { tag = a; }
    void invalidate() { tag = 0; Flags = INVALID; }
    bool isValid() { return ((Flags) != INVALID); }
};

class Cache {
protected:
    ulong size, lineSize, assoc, sets, log2Sets, log2Blk, tagMask, numLines;
    ulong reads;
    ulong readMisses;
    ulong writes;
    ulong writeMisses;
    ulong writeBacks;
    ulong Readhits;
    ulong Writehits;
    ulong interventions;
    ulong invalidations;
    ulong mem_trans;
    ulong c_to_c_trans;
    ulong flushes;
    ulong busrdxes;

    cacheLine **cache;
    ulong calcTag(ulong addr) { return (addr >> (log2Blk)); }
    ulong calcIndex(ulong addr) { return ((addr >> log2Blk) & tagMask); }
    ulong calcAddr4Tag(ulong tag) { return (tag << (log2Blk)); }

public:
    ulong currentCycle;
    int busrd, busrdx, busupgr, busupd;

    // Timing latencies (remove inline initialization)
    int read_hit_latency;
    int write_hit_latency;
    int memory_latency;
    int flush_transfer;

    // Constructor to initialize all variables
    Cache(int, int, int);

    ~Cache() { delete cache; }

    cacheLine *findLineToReplace(ulong addr);
    cacheLine *fillLine(ulong addr);
    cacheLine *findLine(ulong addr);
    cacheLine *getLRU(ulong);

    ulong getRM() { return readMisses; }
    ulong getWM() { return writeMisses; }
    ulong getReads() { return reads; }
    ulong getWrites() { return writes; }
    ulong Invalidations() { return invalidations; }
    ulong Flushes() { return flushes; }
    ulong BusRdX() { return busrdxes; }
    ulong getRH() { return Readhits; }
    ulong getWH() { return Writehits; }
    ulong getTotalExecution() { return Total_execution_time; }

    void writeBack(ulong) { writeBacks++; mem_trans++; }
    void MOESI_Processor_Access(ulong addr, uchar rw, int copy, Cache **cache, int current_processor, int total_processor);
    void MESI_Processor_Access(ulong addr, uchar rw, int copy, Cache **cache, int current_processor, int total_processor);
    void printStats();
    void printCacheState(ulong state);
    void updateLRU(cacheLine *);
    void MOESI_Bus_Snoop(ulong addr, int busread, int busreadx, int busupgrade);
    void MESI_Bus_Snoop(ulong addr, int busread, int busreadx, int busupgrade);
};

#endif

