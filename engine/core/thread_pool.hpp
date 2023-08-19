#pragma once

#include "utils/assert.hpp"

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
   explicit ThreadPool(size_t threads);
   ~ThreadPool();

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

         utils::Assert(!stop_, "enqueue on stopped ThreadPool");

         tasks_.emplace([task]() { (*task)(); });
      }
      condition_.notify_one();
      return res;
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