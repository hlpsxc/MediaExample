//
// Created by skl on 18-12-12.
//

#include <iostream>
#include "VideoInputFile.h"


class VideoHandle : public VideoInputFileListener
{
    virtual void onVideoFrame(const AVFrame* frame)
    {
        std::cout << "video frame:"  << frame->pts << ",pic type:" << frame->pict_type << std::endl;
    }
    virtual void onAudioFrame(const AVFrame* frame)
    {
        //std::cout << "audio frame:"  << frame->pts << std::endl;
    }
    virtual void onAudioPacket(const AVPacket* packet)
    {
        //std::cout << "audio packet:"  << packet->pts << std::endl;
    }
    virtual void onVideoPacket(const AVPacket* packet)
    {
       // std::cout << "video packet:"  << packet->pts << std::endl;
    }
};

int main()
{
    VideoInputFile inputFile("/home/skl/pcode/video/test.mp4"); //http://www.170mv.com/
    VideoHandle handle;
    inputFile.setListener(&handle);
    inputFile.readFileUtilEnd(true);
    return 0;
}