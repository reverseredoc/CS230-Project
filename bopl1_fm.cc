#include "cache.h"

void CACHE::l1d_prefetcher_initialize()
{
    prefetch_offset_1 = 1;
  prefetch_offset_2 = 1;

  for (int i=0; i<NOFFSETS; i++) {
    score_counter.score[i] = 0;
  }

  score_counter.max_score_1 = 0;
  score_counter.max_score_2 = 0;
  score_counter.best_offset_1 = 0;
  score_counter.best_offset_2 = 0;
  score_counter.round = 0;
  score_counter.p = 0;

  for (int i=0; i<DELAYQSIZE; i++) {
    d_queue.lineaddr[i] = 0;
    d_queue.cycle[i] = 0;
    d_queue.valid[i] = 0;
  }
  d_queue.end = 0;
  d_queue.start = 0;

  throttler.mshr_threshold = MSHR_THRESHOLD_MAX;
  throttler.prefetch_score_1 = SCORE_MAX;
  throttler.prefetch_score_2 = SCORE_MAX;
  throttler.llc_rate = 0;
  throttler.llc_rate_gauge = GAUGE_MAX/2;
  throttler.last_cycle = 0;

  for (int i=0; i<(1<<RRINDEX); i++) {
    recent_request_table.recent_request[0][i] = 0;
    recent_request_table.recent_request[1][i] = 0;
  }

  for (int i=0; i<L1D_SET; i++) {
    for (int j=0; j<L1D_WAY; j++) {
        prefetch_bit[i][j] = 0;
    }
  }
    cout << "CPU " << cpu << " L1D next line prefetcher" << endl;
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

  // int l2_hit = (w>=0);
  int l2_hit = 0;

  if (w < NUM_WAY && w >= 0){
    l2_hit = 1;
  }

  int prefetched = 0; // set if line was prefetched

  if (l2_hit){ //L2 hit
    prefetched = prefetch_bit[s][w];
    prefetch_bit[s][w] = 0; //resetting
  } else {
    throttle_controller();
  }

  dq_pop();

  int prefetch_issued = 0; // for throttling

  if (! l2_hit || prefetched) {
    learn_offset(line_addr); // Learning the best offset in this phase

    // rr_insert_left(line_addr);
    if (((line_addr << LOG2_BLOCK_SIZE) >> LOG2_PAGE_SIZE) == (((line_addr + prefetch_offset_1) << LOG2_BLOCK_SIZE) >> LOG2_PAGE_SIZE)){
        dq_push(line_addr);
        if (prefetch_offset_1 != 0 && MSHR.occupancy < throttler.mshr_threshold){
          int success = prefetch_line(ip, addr, (line_addr + prefetch_offset_1) << LOG2_BLOCK_SIZE, FILL_L1, 0);
          if (success) throttle_controller();
        }

        // SW
        else if (throttler.prefetch_score_1 > LOW_SCORE && prefetch_offset_1 != 0) {
            if (prefetch_line(ip, addr, (line_addr + prefetch_offset_1) << LOG2_BLOCK_SIZE, FILL_L2, 0)){
                throttle_controller();
            }
        }
    }

    if (((line_addr << LOG2_BLOCK_SIZE) >> LOG2_PAGE_SIZE) == (((line_addr + prefetch_offset_2) << LOG2_BLOCK_SIZE) >> LOG2_PAGE_SIZE)){
        dq_push(line_addr);
        if (prefetch_offset_2 != 0 && MSHR.occupancy < throttler.mshr_threshold){
          int success = prefetch_line(ip, addr, (line_addr + prefetch_offset_2) << LOG2_BLOCK_SIZE, FILL_L1, 0);
          if (success) throttle_controller();
        }

        else if (throttler.prefetch_score_2 > LOW_SCORE && prefetch_offset_2 != 0) {
            if (prefetch_line(ip, addr, (line_addr + prefetch_offset_2) << LOG2_BLOCK_SIZE, FILL_L2, 0)){
                throttle_controller();
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
  if (prefetch || (prefetch_offset_1 == 0)){
    baselineaddr = line_addr - prefetch_offset_1;
    if (((line_addr << LOG2_BLOCK_SIZE) >> LOG2_PAGE_SIZE) == ((baselineaddr << LOG2_BLOCK_SIZE) >> LOG2_PAGE_SIZE)){
      recent_request_table.insert_right(baselineaddr);
    }
  }
  if (prefetch || (prefetch_offset_2 == 0)){
    baselineaddr = line_addr - prefetch_offset_2;
    if (((line_addr << LOG2_BLOCK_SIZE) >> LOG2_PAGE_SIZE) == ((baselineaddr << LOG2_BLOCK_SIZE) >> LOG2_PAGE_SIZE)){
      recent_request_table.insert_right(baselineaddr);
    }
  }

}

void CACHE::l1d_prefetcher_final_stats()
{
    cout << "CPU " << cpu << " L1D next line prefetcher final stats" << endl;
}
//used cache.h from submission folder and l2c pref from submission folder