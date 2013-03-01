
#include "ssd.h"
#include "scheduler.h"
#include "Operating_System.h"

using namespace ssd;

//*****************************************************************************************
//				GRACE HASH JOIN WORKLOAD
//*****************************************************************************************
Grace_Hash_Join_Workload::Grace_Hash_Join_Workload(long min_lba, long max_lba)
 : min_lba(min_lba), max_lba(max_lba), r1(0.2), r2(0.2), fs(0.6), use_flexible_reads(false) {}

vector<Thread*> Grace_Hash_Join_Workload::generate_instance() {
	Grace_Hash_Join::initialize_counter();

	int relation_1_start = min_lba;
	int relation_1_end = min_lba + (max_lba - min_lba) * r1;
	int relation_2_start = relation_1_end + 1;
	int relation_2_end = relation_2_start + (max_lba - min_lba) * r2;
	int temp_space_start = relation_2_end + 1;
	int temp_space_end = max_lba;

	Thread* first = new Asynchronous_Sequential_Trimmer(temp_space_start, temp_space_end);
	Thread* preceding_thread = first;
	for (int i = 0; i < 1000; i++) {
		Thread* grace_hash_join = new Grace_Hash_Join(	relation_1_start,	relation_1_end,
				relation_2_start,	relation_2_end,
				temp_space_start, temp_space_end,
				use_flexible_reads, false, 32, 31 * i + 1);

		if (i == 0) grace_hash_join->set_time_to_wait_before_starting(10000);

		grace_hash_join->set_experiment_thread(true);
		preceding_thread->add_follow_up_thread(grace_hash_join);
		preceding_thread = grace_hash_join;
	}
	vector<Thread*> threads(1, first);
	return threads;
}

//*****************************************************************************************
//				RANDOM WORKLOAD
//*****************************************************************************************
Random_Workload::Random_Workload(long min_lba, long max_lba, long num_threads)
 : min_lba(min_lba), max_lba(max_lba), num_threads(num_threads) {}

vector<Thread*> Random_Workload::generate_instance() {
	Simple_Thread* init_write = new Asynchronous_Sequential_Writer(min_lba, max_lba);
	for (int i = 0; i < num_threads; i++) {
		int seed = 23621 * i + 62;
		Simple_Thread* writer = new Synchronous_Random_Writer(min_lba, max_lba, seed);
		Simple_Thread* reader = new Synchronous_Random_Reader(min_lba, max_lba, seed * 136);
		writer->set_experiment_thread(true);
		reader->set_experiment_thread(true);
		init_write->add_follow_up_thread(reader);
		init_write->add_follow_up_thread(writer);
		writer->set_num_ios(INFINITE);
		reader->set_num_ios(INFINITE);
	}
	vector<Thread*> threads(1, init_write);
	return threads;
}
