#include "thread_pool.hpp"

namespace looper {

ThreadPool::ThreadPool(size_t threads)
{
   for (size_t i = 0; i < threads; ++i)
   {
      workers_.emplace_back([this] {
         for (;;)
         {
            std::function< void() > task;

            {
               std::unique_lock< std::mutex > lock(queueMutex_);
               condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
               if (stop_ && tasks_.empty())
               {
                  return;
               }

               task = std::move(tasks_.front());
               tasks_.pop();
            }

            task();
         }
      });
   }
}

ThreadPool::~ThreadPool()
{
   {
      std::unique_lock< std::mutex > lock(queueMutex_);
      stop_ = true;
   }

   condition_.notify_all();
   for (auto& worker : workers_)
   {
      worker.join();
   }
}

} // namespace looper