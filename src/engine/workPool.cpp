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

#include "workPool.h"
#include "../ta-log.h"
#include <thread>

void* _workThread(void* inst) {
  ((DivWorkThread*)inst)->run();
  return NULL;
}

void DivWorkThread::run() {
  std::unique_lock<std::mutex> unique(selfLock);
  DivPendingTask task;

  logV("running work thread");

  while (true) {
    lock.lock();
    if (tasks.empty()) {
      lock.unlock();
      isBusy=false;
      parent->notify.notify_one();
      if (terminate) {
        break;
      }
      notify.wait(unique);
      continue;
    } else {
      task=tasks.front();
      tasks.pop();
      lock.unlock();

      task.func(task.funcArg);

      if (--parent->busyCount<0) {
        logE("oh no PROBLEM...");
      }
      parent->notify.notify_one();
    }
  }
}

bool DivWorkThread::assign(const std::function<void(void*)>& what, void* arg) {
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
  lock.unlock();
  notify.notify_one();
  thread->join();
}

void DivWorkThread::init(DivWorkPool* p) {
  parent=p;
  thread=new std::thread(_workThread,this);
}

void DivWorkPool::push(const std::function<void(void*)>& what, void* arg) {
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
  std::unique_lock<std::mutex> unique(selfLock);

  // start running
  for (unsigned int i=0; i<count; i++) {
    workThreads[i].notify.notify_one();
  }

  // wait
  while (busyCount>0) {
    if (notify.wait_for(unique,std::chrono::milliseconds(30))==std::cv_status::timeout) {
      logW("DivWorkPool: wait() timed out!");
    }
  }
}

DivWorkPool::DivWorkPool(unsigned int threads):
  threaded(threads>0),
  count(threads),
  pos(0),
  busyCount(0) {
  if (threaded) {
    workThreads=new DivWorkThread[threads];
    for (unsigned int i=0; i<count; i++) {
      workThreads[i].init(this);
    }
  } else {
    workThreads=NULL;
  }
}

DivWorkPool::~DivWorkPool() {
  if (threaded) {
    for (unsigned int i=0; i<count; i++) {
      workThreads[i].finish();
    }
    delete[] workThreads;
  }
}
