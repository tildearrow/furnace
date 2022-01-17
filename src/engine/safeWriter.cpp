#include "safeWriter.h"

#define WRITER_BUF_SIZE 16384

unsigned char* SafeWriter::getFinalBuf() {
  return buf;
}

void SafeWriter::checkSize(size_t amount) {
  while ((curSeek+amount)>=bufLen) {
    unsigned char* newBuf=new unsigned char[bufLen+WRITER_BUF_SIZE];
    memcpy(newBuf,buf,bufLen);
    delete[] buf;
    buf=newBuf;
    bufLen+=WRITER_BUF_SIZE;
  }
}

bool SafeWriter::seek(ssize_t where, int whence) {
  ssize_t supposed;
  switch (whence) {
    case SEEK_SET:
      supposed=where;
      break;
    case SEEK_CUR:
      supposed=curSeek+where;
      break;
    case SEEK_END:
      supposed=len+where;
      break;
    default:
      return false;
  }
  if (supposed<0) supposed=0;
  if (supposed>(ssize_t)len) supposed=len;
  curSeek=supposed;
  return true;
}

size_t SafeWriter::tell() {
  return curSeek;
}

size_t SafeWriter::size() {
  return len;
}

int SafeWriter::write(const void* what, size_t count) {
  if (!operative) return 0;
  checkSize(count);
  memcpy(buf+curSeek,what,count);
  curSeek+=count;
  if (curSeek>len) len=curSeek;
  return count;
}

int SafeWriter::writeC(signed char val) {
  return write(&val,1);
}

int SafeWriter::writeS(short val) {
  return write(&val,2);
}

int SafeWriter::writeI(int val) {
  return write(&val,4);
}
int SafeWriter::writeL(int64_t val) {
  return write(&val,8);
}
int SafeWriter::writeF(float val) {
  return write(&val,4);
}
int SafeWriter::writeD(double val) {
  return write(&val,8);
}
int SafeWriter::writeString(String val, bool pascal) {
  if (pascal) {
    writeC((unsigned char)val.size());
    return write(val.c_str(),val.size())+1;
  } else {
    return write(val.c_str(),val.size()+1);
  }
}

void SafeWriter::init() {
  if (operative) return;
  buf=new unsigned char[WRITER_BUF_SIZE];
  bufLen=WRITER_BUF_SIZE;
  len=0;
  curSeek=0;
  operative=true;
}

SafeReader* SafeWriter::toReader() {
  return new SafeReader(buf,len);
}

void SafeWriter::finish() {
  if (!operative) return;
  delete[] buf;
  buf=NULL;
  operative=false;
}