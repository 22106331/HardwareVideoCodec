/*
* Copyright (c) 2018-present, lmyooyo@gmail.com.
*
* This source code is licensed under the GPL license found in the
* LICENSE file in the root directory of this source tree.
*/
#ifndef HARDWAREVIDEOCODEC_AUDIORECORDER_H
#define HARDWAREVIDEOCODEC_AUDIORECORDER_H

#include <string>
#include "AudioDevice.h"
#include "ObjectBox.h"
#include "RecyclerBlockQueue.h"
#include "EventPipeline.h"
#include "SimpleLock.h"
#include "SLEngine.h"
#include "HwResult.h"

class AudioRecorder : public SLAudioDevice {
public:
    AudioRecorder(unsigned int channels, unsigned int sampleHz, int format, int minBufferSize);

    AudioRecorder(SLEngine *engine, unsigned int channels, unsigned int sampleHz, int format,
                  int minBufferSize);

    virtual ~AudioRecorder();

    virtual HwResult start();

    virtual void stop();

    virtual size_t read(uint8_t *buffer);

    virtual void flush();

    void bufferDequeue(SLAndroidSimpleBufferQueueItf slBufferQueueItf);

private:
    unsigned int channels = 0;
    unsigned int sampleHz = 0;
    SLuint32 format = SL_PCMSAMPLEFORMAT_FIXED_16;
    int minBufferSize = 0;
    RecyclerBlockQueue<ObjectBox> *recycler = nullptr;

    SLEngine *engine = nullptr;
    bool ownEngine = false;

    SLObjectItf recordObject = nullptr;
    SLRecordItf recordItf = nullptr;
    SLAndroidSimpleBufferQueueItf bufferQueueItf = nullptr;
    ObjectBox *buffer = nullptr;
    FILE *pcmFile = nullptr;

    HwResult createEngine();

    void destroyEngine();

    HwResult createBufferQueueObject();

    void initialize(SLEngine *engine, int channels, int sampleHz, int format, int minBufferSize);

};


#endif //HARDWAREVIDEOCODEC_AUDIORECORDER_H
