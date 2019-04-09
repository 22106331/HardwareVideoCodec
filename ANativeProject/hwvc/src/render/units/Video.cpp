/*
* Copyright (c) 2018-present, lmyooyo@gmail.com.
*
* This source code is licensed under the GPL license found in the
* LICENSE file in the root directory of this source tree.
*/
#include "../include/Video.h"
#include "ObjectBox.h"
#include "Size.h"
#include "TimeUtils.h"

Video::Video() : Unit() {
    name = __FUNCTION__;
    this->lock = new SimpleLock();
    registerEvent(EVENT_COMMON_PREPARE, reinterpret_cast<EventFunc>(&Video::eventPrepare));
    registerEvent(EVENT_VIDEO_START, reinterpret_cast<EventFunc>(&Video::eventStart));
    registerEvent(EVENT_VIDEO_PAUSE, reinterpret_cast<EventFunc>(&Video::eventPause));
    registerEvent(EVENT_VIDEO_SEEK, reinterpret_cast<EventFunc>(&Video::eventSeek));
    registerEvent(EVENT_VIDEO_SET_SOURCE, reinterpret_cast<EventFunc>(&Video::eventSetSource));
    registerEvent(EVENT_VIDEO_LOOP, reinterpret_cast<EventFunc>(&Video::eventLoop));
    decoder = new AsynVideoDecoder();
}

Video::Video(HandlerThread *handlerThread) : Unit(handlerThread) {
    name = __FUNCTION__;
    this->lock = new SimpleLock();
    registerEvent(EVENT_COMMON_PREPARE, reinterpret_cast<EventFunc>(&Video::eventPrepare));
    registerEvent(EVENT_VIDEO_START, reinterpret_cast<EventFunc>(&Video::eventStart));
    registerEvent(EVENT_VIDEO_PAUSE, reinterpret_cast<EventFunc>(&Video::eventPause));
    registerEvent(EVENT_VIDEO_SEEK, reinterpret_cast<EventFunc>(&Video::eventSeek));
    registerEvent(EVENT_VIDEO_SET_SOURCE, reinterpret_cast<EventFunc>(&Video::eventSetSource));
    registerEvent(EVENT_VIDEO_LOOP, reinterpret_cast<EventFunc>(&Video::eventLoop));
    decoder = new AsynVideoDecoder();
}

Video::~Video() {
    LOGI("Video::~Video");
    lock->lock();
    if (audioPlayer) {
        audioPlayer->stop();
        delete audioPlayer;
        audioPlayer = nullptr;
    }
    LOGI("Video::~audioPlayer");
    if (frame) {
        delete frame;
        frame = nullptr;
    }
    if (decoder) {
        delete decoder;
        decoder = nullptr;
    }
    if (path) {
        delete[]path;
        path = nullptr;
    }
    lock->unlock();
    if (lock) {
        delete lock;
        lock = nullptr;
    }
}

bool Video::eventRelease(Message *msg) {
    LOGI("Video::eventRelease");
    post([this] {
        eventStop(nullptr);
        if (texAllocator) {
            egl->makeCurrent();
            delete texAllocator;
            texAllocator = nullptr;
        }
        if (egl) {
            delete egl;
            egl = nullptr;
        }
    });
    return true;
}

bool Video::eventPrepare(Message *msg) {
    playState = PAUSE;
    NativeWindow *nw = static_cast<NativeWindow *>(msg->tyrUnBox());
    if (decoder->prepare(path)) {
        createAudioPlayer();
    } else {
        LOGE("Video::open %s failed", path);
        return true;
    }
    post([this, nw] {
        initEGL(nw);
        wait(10000);
        lock->notify();
    });
    lock->wait();
    return true;
}

bool Video::eventStart(Message *msg) {
    LOGI("Video::eventStart");
    if (STOP != playState) {
        playState = PLAYING;
        sendLoop();
    }
    if (audioPlayer) {
        audioPlayer->flush();
    }
    if (decoder) {
        decoder->start();
    }
    return true;
}

bool Video::eventPause(Message *msg) {
    if (STOP != playState) {
        playState = PAUSE;
    }
    if (decoder) {
        decoder->pause();
    }
    return true;
}

bool Video::eventSeek(Message *msg) {
    int64_t us = msg->arg2;
    decoder->seek(us);
    return true;
}

bool Video::eventStop(Message *msg) {
    playState = STOP;
    return true;
}

bool Video::eventInvalidate(Message *m) {
    Message *msg = new Message(EVENT_RENDER_FILTER, nullptr);
    msg->obj = new ObjectBox(new Size(frame->width, frame->height));
    msg->arg1 = yuvFilter->getFrameBuffer()->getFrameTexture();
    postEvent(msg);
    return true;
}

bool Video::eventSetSource(Message *msg) {
    this->path = static_cast<char *>(msg->tyrUnBox());
    return true;
}

void Video::sendLoop() {
    postEvent(new Message(EVENT_VIDEO_LOOP, nullptr));
}

bool Video::eventLoop(Message *msg) {
    post([this] {
        if (PLAYING != playState) {
            return;
        }
        if (!texAllocator || !decoder) {
            eventPause(nullptr);
            return;
        }
        lock->lock();
        sendLoop();
        int ret = grab();
        lock->unlock();
        checkFilter();
        if (MEDIA_TYPE_VIDEO != ret) {
            if (MEDIA_TYPE_AUDIO == ret && audioPlayer && frame) {
                audioPlayer->write(frame->data, frame->size);
            }
            return;
        }
        lock->lock();
        glViewport(0, 0, frame->width, frame->height);
        lock->unlock();
        yuvFilter->draw(yuv[0], yuv[1], yuv[2]);
        eventInvalidate(nullptr);
    });
    return true;
}

void Video::checkFilter() {
    if (!yuvFilter) {
        yuvFilter = new YUV420PFilter();
        yuvFilter->init(decoder->width(), decoder->height());
        yuv[0] = texAllocator->alloc();
        yuv[1] = texAllocator->alloc();
        yuv[2] = texAllocator->alloc();
    }
}

int Video::grab() {
    if (!frame) {
        frame = new Frame(decoder->width(), decoder->height());
    }
//    long long time = getCurrentTimeUS();
    int ret = decoder->grab(frame);
//    LOGI("Video::grab cost: %lld", getCurrentTimeUS() - time);
    if (MEDIA_TYPE_VIDEO != ret) {
        return ret;
    }
    int64_t curPts = frame->pts;

    if (lastPts > 0) {
        int64_t t = (curPts - lastPts) - (getCurrentTimeUS() - lastShowTime);
        lock->wait(t);
        LOGI("Video::grab %d x %d, delta time: %lld, wait time: %lld", 0, 0,
             (getCurrentTimeUS() - lastShowTime) / 1000, t);
    }
    lastShowTime = getCurrentTimeUS();

    lastPts = curPts;
    int size = frame->width * frame->height;
    egl->makeCurrent();
    glBindTexture(GL_TEXTURE_2D, yuv[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame->width, frame->height, 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 frame->data);
    glBindTexture(GL_TEXTURE_2D, yuv[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame->width / 2, frame->height / 2, 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 frame->data + size);
    glBindTexture(GL_TEXTURE_2D, yuv[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame->width / 2, frame->height / 2, 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 frame->data + size + size / 4);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    return MEDIA_TYPE_VIDEO;
}

void Video::createAudioPlayer() {
    int format;
    switch (decoder->getSampleFormat()) {
        case AV_SAMPLE_FMT_S16:
            format = SL_PCMSAMPLEFORMAT_FIXED_16;
            break;
        case AV_SAMPLE_FMT_U8:
            format = SL_PCMSAMPLEFORMAT_FIXED_8;
            break;
        default:
            format = SL_PCMSAMPLEFORMAT_FIXED_32;
    }
    audioPlayer = new AudioPlayer(decoder->getChannels(),
                                  decoder->getSampleHz(),
                                  format,
                                  decoder->getPerSampleSize());
    audioPlayer->start();
}

void Video::initEGL(NativeWindow *nw) {
    if (nw->egl) {
        LOGI("Video::init EGL with context");
        egl = new Egl(nw->egl, nullptr);
    } else {
        LOGI("Video::init EGL");
        egl = new Egl();
        nw->egl = egl;
    }
    egl->makeCurrent();
    if (!texAllocator) {
        texAllocator = new TextureAllocator();
    }
}