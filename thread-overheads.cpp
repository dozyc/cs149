#include <cstdio>
#include <vector>
#include <chrono>
#include <limits>
#include <algorithm>
#include <thread>
#include <atomic>

#define NUM_WORKER_THREADS 20
#define NUM_TASKS 100000

/* do no or little work to check overhead costs of different strategies*/
__attribute__ ((noinline, optnone))
void dowork() {
    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    return;
}

float sequential_strategy(int num_tasks) {
    printf("\n");
    printf("TEST 0 START: %d Sequential calls\n", num_tasks);

    auto start_time = std::chrono::system_clock::now();

    for (int i=0; i<num_tasks; i++) {
        dowork();
    }

    auto end_time = std::chrono::system_clock::now();
    auto elapsed = float((end_time - start_time).count()) / std::chrono::system_clock::period::den;

    printf("TEST 0 RESULT: elapsed time %lf seconds, %.2e tasks per second\n", elapsed, num_tasks / elapsed);

    return num_tasks / elapsed;
}
class WorkerPool {
    int num_tasks;
    int num_workers;
    std::atomic<int> next_task;

public:
    WorkerPool(int num_workers, int num_tasks) :
        num_workers{num_workers}, num_tasks{num_tasks}, next_task{0} {
            printf("WorkerPool created with %d workers and %d tasks\n", num_workers, num_tasks);
    }

    float launch_tasks() {
        auto worker_lambda = [this] {
            int worker_task_count = 0;
            while (true) {
                // grab the next task unless there are no more
                int task = next_task++;
                if (task >= num_tasks) {
                    break;
                }
                dowork();
                worker_task_count++;
            }
            //  how to store and print a worker ID?
            //printf("Worker finished %d tasks\n", worker_task_count);
        };
        
        std::vector<std::thread> workers;
        auto launch_start = std::chrono::system_clock::now();
        for (int i=0; i<num_workers; ++i) {
            workers.emplace_back(worker_lambda);
        }
        for (auto& worker: workers) {
            worker.join();
        }
        auto launch_end = std::chrono::system_clock::now();
        // get elapsed time in seconds
        auto elapsed = float((launch_end - launch_start).count()) / std::chrono::system_clock::period::den;
        printf("TEST 1 RESULT: elapsed time %f seconds, %.2e tasks per second\n", elapsed, num_tasks / elapsed);
        return num_tasks / elapsed;
    }
};

void test2( int num_tasks) {
    // spawn a thread for each task
    auto start_time = std::chrono::system_clock::now();
    for (int i=0; i<num_tasks; ++i) {
        auto thr = std::thread(dowork);
        thr.join();
    }
    auto end_time = std::chrono::system_clock::now();
    auto elapsed = float((end_time - start_time).count()) / std::chrono::system_clock::period::den;
    printf("TEST 2 RESULT: elapsed time %f seconds, %.2e tasks per second\n", elapsed, num_tasks / elapsed);
}

int main() {
    printf("Apparent number of cores: %d\n", std::thread::hardware_concurrency());

    float tps_seq = sequential_strategy(NUM_TASKS);

    auto wp = new WorkerPool(NUM_WORKER_THREADS, NUM_TASKS);
    float tps_pool = wp->launch_tasks();

    printf("WorkerPool speedup vs loop: %.3f\n", tps_pool / tps_seq);

    //test2(NUM_TASKS); // this is super slow

    return 0;
}