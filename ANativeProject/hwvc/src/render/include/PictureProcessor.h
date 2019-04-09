//
// Created by mingyi.li on 2018/12/25.
//

#ifndef HARDWAREVIDEOCODEC_PICTUREPROCESSOR_H
#define HARDWAREVIDEOCODEC_PICTUREPROCESSOR_H

#include "Object.h"
#include "UnitPipeline.h"
#include "Screen.h"
#include "Filter.h"

class PictureProcessor : public Object {
public:
    ANativeWindow *win;

    PictureProcessor();

    virtual ~PictureProcessor();

    void prepare(ANativeWindow *win, int width, int height);

    void show(char *file);

    void setFilter(Filter *filter);

    void invalidate();

private:
    UnitPipeline *pipeline = nullptr;
};


#endif //HARDWAREVIDEOCODEC_PICTUREPROCESSOR_H
