/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _WORKPOOL_H
#define _WORKPOOL_H

#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>

struct DivWorkThread {
  std::mutex lock;
  std::thread* thread;
  std::condition_variable notify;
  bool busy, terminate;

  void run();
  DivWorkThread():
    busy(false) {}
};

/**
 * this class provides an implementation of a "thread pool" for executing tasks in parallel.
 * it is highly recommended to use `new` when allocating a DivWorkPool.
 */
class DivWorkPool {
  DivWorkThread* workThreads;
  public:
    /**
     * push a new job to this work pool.
     * if all work threads are busy, this will block until one is free.
     */
    bool push();
    
    /**
     * check whether this work pool is busy.
     */
    bool busy();

    /**
     * wait for all work threads to finish.
     */
    bool wait();

    DivWorkPool(unsigned int threads=0);
    ~DivWorkPool();
};

#endif