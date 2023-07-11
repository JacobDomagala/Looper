#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

namespace looper {
class ThreadPool
{
 public:
   ThreadPool(size_t threads)
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

   template < class F, class... Args >
   auto
   enqueue(F&& f, Args&&... args)
   {
      using return_type = typename std::invoke_result< F, Args... >::type;

      auto task = std::make_shared< std::packaged_task< return_type() > >(
         std::bind(std::forward< F >(f), std::forward< Args >(args)...));

      std::future< return_type > res = task->get_future();
      {
         std::unique_lock< std::mutex > lock(queueMutex_);

         // don't allow enqueueing after stopping the pool
         if (stop_)
            throw std::runtime_error("enqueue on stopped ThreadPool");

         tasks_.emplace([task]() { (*task)(); });
      }
      condition_.notify_one();
      return res;
   }

   ~ThreadPool()
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

 private:
   // need to keep track of threads so we can join them
   std::vector< std::thread > workers_ = {};
   // the task queue
   std::queue< std::function< void() > > tasks_ = {};

   // synchronization
   std::mutex queueMutex_ = {};
   std::condition_variable condition_ = {};
   bool stop_ = false;
};

} // namespace looper