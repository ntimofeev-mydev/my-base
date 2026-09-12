// #my_engine_source_file
#include "my/async/work_queue.h"
#include "my/threading/barrier.h"

using namespace std::chrono_literals;

namespace my::test
{
    TEST(WorkQueue, Sample)
    {
        constexpr size_t kThreadCount = 1;

        WorkQueuePtr queue = CreateWorkQueue();

        std::vector<std::thread> threads;
        threads.resize(kThreadCount);

        threading::Barrier barrier {kThreadCount};

        for (size_t i = 0; i < kThreadCount; ++i)
        {
            threads.emplace_back([](WorkQueuePtr queue, threading::Barrier& barrier)
            {
                barrier.Enter();

                for (size_t i = 0; i < 5; ++i)
                {
                    queue->Schedule([](void*) noexcept
                    {
                        std::cout << "OKAY !\n";
                    }, reinterpret_cast<void*>(static_cast<uintptr_t>(i)));
                }
            }, queue, std::ref(barrier));
        }

        for (size_t i = 0; i < 2; ++i)
        {
            std::this_thread::sleep_for(10ms);
            queue->Poll();
        }
    }
}  // namespace my::test
