#include "cache.h"

void CACHE::l1d_prefetcher_initialize()
{
  prefetch_offset = 1;

  for (int i=0; i<(1<<RRINDEX); i++) {
    rr.recent_request[0][i] = 0;
    rr.recent_request[1][i] = 0;
  }

  for (int i=0; i<NOFFSETS; i++) {
    os.score[i] = 0;
  }
  os.max_score = 0;
  os.best_offset = 0;
  os.round = 0;
  os.p = 0;

  for (int i=0; i<DELAYQSIZE; i++) {
    dq.lineaddr[i] = 0;
    dq.cycle[i] = 0;
    dq.valid[i] = 0;
  }
  dq.tail = 0;
  dq.head = 0;

  pt.mshr_threshold = MSHR_THRESHOLD_MAX;
  pt.prefetch_score = SCORE_MAX;
  pt.llc_rate = 0;
  pt.llc_rate_gauge = GAUGE_MAX/2;
  pt.last_cycle = 0;

  for (int i=0; i<L1D_SET; i++) {
    for (int j=0; j<L1D_WAY; j++) {
      prefetch_bit[i][j] = 0;
    }
  }
}

void CACHE::l1d_prefetcher_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, uint8_t type)
{
  uint64_t line_addr = addr >> LOG2_BLOCK_SIZE;

  DP(if (warmup_complete[cpu]) {
        cout << "[" << NAME << "] " << __func__ << hex << " base_cl: " << (addr >> LOG2_BLOCK_SIZE);
        cout << " pf_cl: " << (pf_addr >> LOG2_BLOCK_SIZE) << " ip: " << ip << " cache_hit: " << +cache_hit
             << " type: " << +type << endl;
  });

  // write the prefetch bit
  int s = get_set(addr);
  int w = get_way(addr,s);

  int l2_hit = 0;

  if (w < NUM_WAY && w >= 0){
    l2_hit = 1;
  }

  int prefetched = 0; // set if line was prefetched

  if (l2_hit){ //L2 hit
    prefetched = prefetch_bit[s][w];
    prefetch_bit[s][w] = 0; //resetting
  } else {
    pt_llc_access();
  }

  dq_pop();

  int prefetch_issued = 0; // for throttling

  if (! l2_hit || prefetched) {
    os_learn_best_offset(line_addr); // Learning the best offset in this phase

    // rr_insert_left(line_addr);
    if (((line_addr << LOG2_BLOCK_SIZE) >> LOG2_PAGE_SIZE) == (((line_addr + prefetch_offset) << LOG2_BLOCK_SIZE) >> LOG2_PAGE_SIZE)){
        dq_push(line_addr);

        if (prefetch_offset != 0 && MSHR.occupancy < pt.mshr_threshold){
          int success = prefetch_line(ip, addr, (line_addr + prefetch_offset) << LOG2_BLOCK_SIZE, FILL_L1, 0);
          if (success) pt_llc_access();
        }

        // SW
        else if (pt.prefetch_score > LOW_SCORE && prefetch_offset != 0) {
            if (prefetch_line(ip, addr, (line_addr + prefetch_offset) << LOG2_BLOCK_SIZE, FILL_L2, 0)){
                pt_llc_access();
            }
        }
    }
  }
}

void CACHE::l1d_prefetcher_cache_fill(uint64_t addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_addr, uint32_t metadata_in)
{
  uint64_t line_addr = addr >> LOG2_BLOCK_SIZE;

  // write the prefetch bit
  int s = get_set(addr);
  int w = get_way(addr,s);

  prefetch_bit[s][w] = prefetch;

  uint64_t baselineaddr;
  if (prefetch || (prefetch_offset == 0)){
    baselineaddr = line_addr - prefetch_offset;
    if (((line_addr << LOG2_BLOCK_SIZE) >> LOG2_PAGE_SIZE) == ((baselineaddr << LOG2_BLOCK_SIZE) >> LOG2_PAGE_SIZE)){
      rr.insert_right(baselineaddr);
    }
  }
}


void CACHE::l1d_prefetcher_final_stats()
{cout << "CPU " << cpu << " L1D best offset prefetcher final stats" << endl;

}
//used cache_org.h from pmod of github and for L2C bop_org.cc from pmod