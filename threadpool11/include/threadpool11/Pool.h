﻿/*
Copyright (c) 2013, Tolga HOŞGÖR
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies, 
either expressed or implied, of the FreeBSD Project.
*/

#pragma once

#include <iostream>

#include <cassert>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <future>
#include <list>
#include <memory>
#include <mutex>

#include "threadpool11/Worker.h"
#include "threadpool11/Helper.hpp"

#ifdef WIN32
	#ifdef threadpool11_EXPORTING
		#define threadpool11_EXPORT __declspec(dllexport)
	#else
		#define threadpool11_EXPORT __declspec(dllimport)
	#endif
#else
	#define threadpool11_EXPORT
#endif

namespace threadpool11
{
	
class Pool
{
friend class Worker;

public:
	typedef size_t WorkerCountType;

private:
	Pool(Pool&&);
	Pool(Pool const&);
	Pool& operator=(Pool&&);
	Pool& operator=(Pool const&);

	std::deque<Worker> workers;
	WorkerCountType activeWorkerCount;

	mutable std::mutex workerContainerMutex;
		
	mutable std::mutex enqueuedWorkMutex;
	std::deque<decltype(Worker::work)> enqueuedWork;

	mutable std::mutex notifyAllFinishedMutex;
	std::condition_variable notifyAllFinished;
	bool areAllReallyFinished;
	
	//std::atomic<WorkerCountType> workCallCounter;
		
	void spawnWorkers(WorkerCountType const& n);

public:
	threadpool11_EXPORT Pool(WorkerCountType const& workerCount = std::thread::hardware_concurrency());
	
  template<typename T>
	threadpool11_EXPORT 
  std::future<T> postWork(std::function<T()> callable)
  {
    auto promise = new std::promise<T>;
    auto future = promise->get_future();
    {
      std::lock_guard<std::mutex> lock(workerContainerMutex);
      
      if(activeWorkerCount < workers.size())
      {
        for(auto& it : workers)
        {
          if(it.status == Worker::Status::DEACTIVE)
          {
            ++activeWorkerCount;
            //TODO: how to avoid copy of callable into this lambda and the ones below? in a decent way!
            auto move_hack = make_move_on_copy(std::move(callable));
            it.setWork([move_hack, promise](){ promise->set_value((move_hack.Value())()); delete promise; });
            return future;
          }
        }
      }
    }
  
    auto move_hack = make_move_on_copy(std::move(callable));
    std::lock_guard<std::mutex> lock(enqueuedWorkMutex);
    enqueuedWork.emplace_back([move_hack, promise](){ promise->set_value((move_hack.Value())()); delete promise; });
    return future;
  }
  
	threadpool11_EXPORT void waitAll();
	threadpool11_EXPORT void joinAll();
	
	threadpool11_EXPORT WorkerCountType getWorkQueueCount() const;
  threadpool11_EXPORT WorkerCountType getWorkerCount() const;
	threadpool11_EXPORT WorkerCountType getActiveWorkerCount() const;
	threadpool11_EXPORT WorkerCountType getInactiveWorkerCount() const;
		
	threadpool11_EXPORT void increaseWorkerCountBy(WorkerCountType const& n);
	threadpool11_EXPORT WorkerCountType decreaseWorkerCountBy(WorkerCountType n);
};
  
template<>
threadpool11_EXPORT inline
std::future<void> Pool::postWork<void>(std::function<void()> callable)
{
  auto promise = new std::promise<void>;
  auto future = promise->get_future();
  {
    std::lock_guard<std::mutex> lock(workerContainerMutex);
      
    if(activeWorkerCount < workers.size())
    {
      for(auto& it : workers)
      {
        if(it.status == Worker::Status::DEACTIVE)
        {
          ++activeWorkerCount;
          auto move_hack = make_move_on_copy(std::move(callable));
          it.setWork([move_hack, promise]() { (move_hack.Value())(); promise->set_value(); delete promise; });
          return future;
        }
      }
    }
  }
  
  auto move_hack = make_move_on_copy(std::move(callable));
  std::lock_guard<std::mutex> lock(enqueuedWorkMutex);
  enqueuedWork.emplace_back([move_hack, promise](){ (move_hack.Value())(); promise->set_value(); delete promise; });
  return future;
}

#undef threadpool11_EXPORT
#undef threadpool11_EXPORTING
}