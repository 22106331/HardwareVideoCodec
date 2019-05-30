/*
* Copyright (c) 2018-present, lmyooyo@gmail.com.
*
* This source code is licensed under the GPL license found in the
* LICENSE file in the root directory of this source tree.
*/

#ifndef HARDWAREVIDEOCODEC_HWMEMFIFO_H
#define HARDWAREVIDEOCODEC_HWMEMFIFO_H

#include "Object.h"
#include "HwAbsFrame.h"
#include "SimpleLock.h"

class HwFIFOBuffer : public Object{
public:
    HwFIFOBuffer(size_t capacity);

    virtual ~HwFIFOBuffer();

    /**
     * 把数据写入fifo
     * @data 数据指针
     * @size data的大小
     * @return 返回成功push的size，0表示失败
     */
    size_t push(uint8_t *data, size_t size);

    /**
     * 从fifo读取数据
     * @size 期望得到的size
     * @return 返回数据片段映射，大小小于或等于size，该内存片段由fifo维护，切勿进行写操作
     */
    HwAbsFrame *take(size_t size);

    size_t size();

    void flush();

private:
    bool wantWrite(size_t size);

    uint8_t *first();

    uint8_t *end();

    size_t leftCapacity();

    bool empty();

    void movePosition();

    bool willCross(uint8_t *flag, uint8_t *pointer, size_t size);

private:
    uint8_t *buf = nullptr;
    size_t capacity = 0;
    size_t _size = 0;
    uint8_t *reader = nullptr;
    uint8_t *writer = nullptr;
    uint8_t *endFlag = nullptr;

    SimpleLock notifyLock;
};


#endif //HARDWAREVIDEOCODEC_HWMEMFIFO_H
