By Suicheng Su
CS5007 Summer 2019 A10


1.	From what the benchmarker reported, it took
		### single core
		1385.77 seconds to index 52 files with single thread;
		1370.40 seconds to index 52 files with 5 threads;
		1493.60 seconds to index 52 files with 10 threads;
		
		### two cores
		929.99 seconds to index 30 files with single thread;
		1095.74 seconds to index 30 files with 5 threads;
		1377.32 seconds to index 30 files with 10 threads;

		252.08 seconds to index 10 files with single thread;
		197.98 seconds to index 10 files with ORIGINAL single thread;
		222.12 seconds to index 10 files with 5 threads;
		303.95 seconds to index 10 files with 10 threads;
2.	The difference between running DirectoryParser/_MT with one thread and the original single-threaded is noticible yet insignificant. The original single-threaded is faster because it don't need extra process to apply multi threaded instructions, 
3.	It seems like the more threads the lower the performance for DirectoryParser/_MT. With more cores, the overal performance is increased yet the performance decreases with more threads. In my opinion, it may due to the increase in the need for context sharing for multi threads, so they are fighting for cpu resource basically, the more they fight about it, the more time/resource it takes to finish their tasks/processes.
