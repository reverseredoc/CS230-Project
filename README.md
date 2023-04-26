# CS230-Project
## Cache Hierarchy Optimizations for Graph Analytics
#### Team members: Karan Godara (210050082), Isha Arora (210050070), Shantanu Welling (210010076)
* We implemented Mockingjay, Hawkeye, FIFO, LFU, MRU and random cache replacement policies and tested them along with other replacement policies like Signature based Hit Prediction (SHIP), SRRIP (Static Re-Reference Interval Prediction) and DRRIP (Dynamic Re-Reference Interval Prediction). <br>
* We implemented exclusive and inclusive cache hierarchies and tested them along with default non-inclusive cache hierarchy of ChampSim. Inclusive cache hierarchy's champsim/ChampSim folder is in Inclusive.zip file whereas exclusive cache hierarchy can be implemented by using cacheEXC_final.cc instead of cache.cc in the original ChampSim docker. <br>
* We tried optimizations in mockingjay, hawkeye and BOP.<br>
* Prefetcher files are present in the prefetcher folder. Other prefetcher files are bop.l1d_pref and bop.l2c_pref. <br>
* bopl2_fm.cc is the BOP_FM for L2C and bopl1_fm.cc is the BOP_FM for L1D. <br>
* bopl1_og.cc is the original BOP for L1D.<br> 
* cache_original.cc is the original cache.cc <br>
* cache_fm.h is the cache.h required to implement BOP & BOP_FM <br>
* hawkeye_predictor.h and optgen.h are required in the inc folder to run hawkeye replacement policy (hawk.llc_repl)<br>
* mj.llc_repl is the mockingjay implementation<br>
* Presentation1.pptx is our project presentation.<br>
* results folder, bfs.zip and sssp.zip has the results<br>
* Traces used are BFS-14 and SSSP-14<br>
* plots.ipynb is used to generate the graphs and results present in plots.zip

## References
* https://www.cs.utexas.edu/~lin/papers/hpca22.pdf
* https://www.cs.utexas.edu/~lin/papers/crc17.pdf
* https://inria.hal.science/hal-01254863/document#:~:text=Best%2DOffset%20prefetching%20is%20intended,prefet%2D%20ches%20%5B33%5D.
* https://dpc3.compas.cs.stonybrook.edu/pdfs/Pangloss.pdf
* https://mrmgroup.cs.princeton.edu/papers/MICRO11_SHiP_Wu_Final.pdf
* https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=8675225
* https://www.cs.utexas.edu/~lin/papers/isca16.pdf
