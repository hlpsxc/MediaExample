//
// Created by skl on 18-12-12.
//

#ifndef VFILTER_VIDEOINPUTFILE_H
#define VFILTER_VIDEOINPUTFILE_H

#include <iostream>
#include "FfmpegHeader.h"

struct VideoInputFileListener
{
    virtual void onVideoFrame(const AVFrame* frame) = 0;
    virtual void onAudioFrame(const AVFrame* frame) = 0;
    virtual void onAudioPacket(const AVPacket* packet) = 0;
    virtual void onVideoPacket(const AVPacket* packet) = 0;

};

class VideoInputFile
{
public:
    VideoInputFile(const std::string& fileName);
    ~VideoInputFile();

    void setListener(VideoInputFileListener* listener) { m_pListener = listener;}

    void readFileUtilEnd(bool needDecode);

private:
    bool openFile(const std::string& fileName);

    void handleVideoPacket(AVPacket* packet, AVFrame* pFrame);
    void handleAudioPacket(AVPacket* packet,  AVFrame* pFrame);

private:
    AVFormatContext* m_pFormatCtx;
    AVCodecContext*  m_pVideoCodecCtx;
    AVCodecContext*  m_pAudioCodecCtx;
    VideoInputFileListener* m_pListener;
    int               m_nVideoStreamIndex;
    int               m_nAudioStreamIndex;
    bool              m_bFileOpened;
};


#endif //VFILTER_VIDEOINPUTFILE_H
