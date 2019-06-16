/*
 * Copyright (c) 2018-present, lmyooyo@gmail.com.
 *
 * This source code is licensed under the GPL license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HARDWAREVIDEOCODEC_HWFRAMEALLOCATOR_H
#define HARDWAREVIDEOCODEC_HWFRAMEALLOCATOR_H

#include "Object.h"
#include "HwSourcesAllocator.h"
#include "HwAbsMediaFrame.h"
#include <list>
#include "Logcat.h"
#include <SimpleLock.h>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
#include "ff/libavformat/avformat.h"
#ifdef __cplusplus
}
#endif

class HwFrameAllocator : public HwSourcesAllocator {
public:
    HwFrameAllocator();

    ~HwFrameAllocator();

    HwAbsMediaFrame *ref(AVFrame *avFrame);

    HwAbsMediaFrame *ref(HwAbsMediaFrame *src);

    void unRef(HwSources **entity);

    void printInfo() {
        Logcat::i("HWVC", "HwFrameAllocator::info: ref=%d, unRef=%d", refQueue.size(),
                  unRefQueue.size());
    }

private:
    SimpleLock refLock;
    SimpleLock unRefLock;
    list<HwAbsMediaFrame *> refQueue;
    list<HwAbsMediaFrame *> unRefQueue;

    HwAbsMediaFrame *refVideo(AVFrame *avFrame);

    HwAbsMediaFrame *refAudio(AVFrame *avFrame);

    void copyInfo(HwAbsMediaFrame *dest, AVFrame *src);
};


#endif //HARDWAREVIDEOCODEC_HWFRAMEALLOCATOR_H
