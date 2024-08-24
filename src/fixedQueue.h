/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#ifndef _FIXED_QUEUE_H
#define _FIXED_QUEUE_H

#include <stdint.h>
#include "ta-log.h"

template<typename T, size_t items> struct FixedQueue {
  size_t readPos, writePos;
  T data[items];

  T& operator[](size_t pos);
  T& front();
  T& back();
  bool pop();
  bool push(const T& item);

  bool erase(size_t pos);

  bool pop_front();
  bool pop_back();
  bool push_front(const T& item);
  bool push_back(const T& item);
  void clear();
  bool empty();
  size_t size();
  size_t capacity();
  FixedQueue():
    readPos(0),
    writePos(0) {}
};

template <typename T, size_t items> T& FixedQueue<T,items>::operator[](size_t pos) {
  if (pos>=size()) {
    logW("accessing invalid position. bug!");
  }
  return data[(readPos+pos)%items];
}

template <typename T, size_t items> T& FixedQueue<T,items>::front() {
  return data[readPos];
}

template <typename T, size_t items> T& FixedQueue<T,items>::back() {
  if (writePos==0) return data[items-1];
  return data[writePos-1];
}

template <typename T, size_t items> bool FixedQueue<T,items>::erase(size_t pos) {
  size_t curSize=size();
  if (pos>=curSize) {
    logW("accessing invalid position. bug!");
    return false;
  }
  if (pos==0) {
    return pop_front();
  }
  if (pos==curSize-1) {
    return pop_back();
  }

  for (size_t i=pos+1, p=(readPos+pos)%items, p1=(readPos+pos+1)%items; i<curSize; i++) {
    if (p>=items) p-=items;
    if (p1>=items) p1-=items;
    data[p]=data[p1];
    p++;
    p1++;
  }

  if (writePos>0) {
    writePos--;
  } else {
    writePos=items-1;
  }
  
  return true;
}

template <typename T, size_t items> bool FixedQueue<T,items>::pop() {
  if (readPos==writePos) return false;
  if (++readPos>=items) readPos=0;
  return true;
}

template <typename T, size_t items> bool FixedQueue<T,items>::push(const T& item) {
  if (writePos==(readPos-1)) {
    //logW("queue overflow!");
    return false;
  }
  if (writePos==items-1 && readPos==0) {
    //logW("queue overflow!");
    return false;
  }
  data[writePos]=item;
  if (++writePos>=items) writePos=0;
  return true;
}

template <typename T, size_t items> bool FixedQueue<T,items>::pop_front() {
  if (readPos==writePos) return false;
  if (++readPos>=items) readPos=0;
  return true;
}

template <typename T, size_t items> bool FixedQueue<T,items>::push_back(const T& item) {
  if (writePos==(readPos-1)) {
    //logW("queue overflow!");
    return false;
  }
  if (writePos==items-1 && readPos==0) {
    //logW("queue overflow!");
    return false;
  }
  data[writePos]=item;
  if (++writePos>=items) writePos=0;
  return true;
}

template <typename T, size_t items> bool FixedQueue<T,items>::pop_back() {
  if (readPos==writePos) return false;
  if (writePos>0) {
    writePos--;
  } else {
    writePos=items-1;
  }
  return true;
}

template <typename T, size_t items> bool FixedQueue<T,items>::push_front(const T& item) {
  if (readPos==(writePos+1)) {
    //logW("stack overflow!");
    return false;
  }
  if (readPos==0 && writePos==items-1) {
    //logW("stack overflow!");
    return false;
  }
  if (readPos>0) {
    readPos--;
  } else {
    readPos=items-1;
  }
  data[readPos]=item;
  return true;
}

template <typename T, size_t items> void FixedQueue<T,items>::clear() {
  readPos=0;
  writePos=0;
}

template <typename T, size_t items> bool FixedQueue<T,items>::empty() {
  return (readPos==writePos);
}

template <typename T, size_t items> size_t FixedQueue<T,items>::size() {
  if (readPos>writePos) {
    return items+writePos-readPos;
  }
  return writePos-readPos;
}

template <typename T, size_t items> size_t FixedQueue<T,items>::capacity() {
  return items-1;
}

#endif
