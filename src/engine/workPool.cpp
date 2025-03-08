/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

#include "workPool.h"
#include "../ta-log.h"
#include <thread>

void* _workThread(void* inst) {
  ((DivWorkThread*)inst)->run();
  return NULL;
}

void DivWorkThread::run() {
  //std::unique_lock<std::mutex> unique(selfLock);
  DivPendingTask task;
  bool setFuckingPromise=false;

  logV("running work thread");

  while (true) {
    lock.lock();
    if (tasks.empty()) {
      lock.unlock();
      isBusy=false;
      if (setFuckingPromise) {
        parent->notify.set_value();
        setFuckingPromise=false;
        //std::this_thread::yield();
      }
      if (terminate) {
        break;
      }
      std::future<void> future=notify.get_future();
      future.wait();
      lock.lock();
      notify=std::promise<void>();
      promiseAlreadySet=false;
      lock.unlock();
      continue;
    } else {
      task=tasks.front();
      tasks.pop();
      lock.unlock();

      task.func(task.funcArg);

      int busyCount=--parent->busyCount;
      if (busyCount<0) {
        logE("oh no PROBLEM...");
      }
      if (busyCount==0) {
        setFuckingPromise=true;
      }
    }
  }
}

bool DivWorkThread::assign(void (*what)(void*), void* arg) {
  lock.lock();
  if (tasks.size()>=30) {
    lock.unlock();
    return false;
  }
  tasks.push(DivPendingTask(what,arg));
  parent->busyCount++;
  isBusy=true;
  lock.unlock();
  return true;
}

void DivWorkThread::wait() {
  if (!isBusy) return;
}

bool DivWorkThread::busy() {
  return isBusy;
}

void DivWorkThread::finish() {
  lock.lock();
  terminate=true;
  try {
    notify.set_value();
  } catch (std::future_error& e) {
    logE("future error! beware!");
  }
  lock.unlock();
  thread->join();
}

bool DivWorkThread::init(DivWorkPool* p) {
  parent=p;
  try {
    thread=new std::thread(_workThread,this);
  } catch (std::system_error& e) {
    logE("could not start thread! %s",e.what());
    thread=NULL;
    return false;
  }
  return true;
}

void DivWorkPool::push(void (*what)(void*), void* arg) {
  // if no work threads, just execute
  if (!threaded) {
    what(arg);
    return;
  }

  for (unsigned int tryCount=0; tryCount<count; tryCount++) {
    if (pos>=count) pos=0;
    if (workThreads[pos++].assign(what,arg)) return;
  }

  // all threads are busy
  logW("DivWorkPool: all work threads busy!");
  what(arg);
}

bool DivWorkPool::busy() {
  if (!threaded) return false;
  for (unsigned int i=0; i<count; i++) {
    if (workThreads[i].busy()) return true;
  }
  return false;
}

void DivWorkPool::wait() {
  if (!threaded) return;

  if (busyCount==0) {
    return;
  }

  std::future<void> future=notify.get_future();

  // start running
  for (unsigned int i=0; i<count; i++) {
    if (!workThreads[i].promiseAlreadySet && !workThreads[i].tasks.empty()) {
      try {
        workThreads[i].lock.lock();
        workThreads[i].promiseAlreadySet=true;
        workThreads[i].notify.set_value();
        workThreads[i].lock.unlock();
      } catch (std::exception& e) {
        logE("ERROR IN THREAD SYNC! %s",e.what());
        abort();
      }
    }
  }
  //std::this_thread::yield();

  // wait
  future.wait();

  notify=std::promise<void>();

  pos=0;
}

DivWorkPool::DivWorkPool(unsigned int threads):
  threaded(threads>0),
  count(threads),
  pos(0),
  busyCount(0) {
  if (threaded) {
    workThreads=new DivWorkThread[threads];
    for (unsigned int i=0; i<count; i++) {
      if (!workThreads[i].init(this)) { 
        count=i;
        break;
      }
    }
    if (count<=0) {
      logE("DivWorkPool: couldn't start any threads! falling back to non-threaded mode.");
      delete[] workThreads;
      threaded=false;
      workThreads=NULL;
    }
  } else {
    workThreads=NULL;
  }
}

DivWorkPool::~DivWorkPool() {
  if (threaded) {
    if (workThreads!=NULL) {
      for (unsigned int i=0; i<count; i++) {
        workThreads[i].finish();
      }
      delete[] workThreads;
    }
  }
}
