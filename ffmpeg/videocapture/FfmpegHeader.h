//
// Created by 王伟 on 2018/12/4.
//

#ifndef VIDEOCAPTURE_FFMPEGHEADER_H
#define VIDEOCAPTURE_FFMPEGHEADER_H

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavdevice/avdevice.h"
#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavutil/avassert.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
}
#endif

#endif //VIDEOCAPTURE_FFMPEGHEADER_H
