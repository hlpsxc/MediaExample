//
// Created by skl on 18-12-12.
//

#include "VideoInputFile.h"

VideoInputFile::VideoInputFile(const std::string &fileName)
: m_pFormatCtx(NULL)
, m_pVideoCodecCtx(NULL)
, m_pAudioCodecCtx(NULL)
, m_pListener(NULL)
, m_nVideoStreamIndex(-1)
, m_nAudioStreamIndex(-1)
, m_bFileOpened(false)
{
    if(!openFile(fileName))
    {
        std::cout << "open file failed," << fileName;
    }
}

VideoInputFile::~VideoInputFile()
{
    if(m_pFormatCtx)
    {
        avformat_close_input(&m_pFormatCtx);
    }
    if(m_pVideoCodecCtx)
    {
        avcodec_close(m_pVideoCodecCtx);
        avcodec_free_context(&m_pVideoCodecCtx);
    }
    if(m_pAudioCodecCtx)
    {
        avcodec_close(m_pAudioCodecCtx);
        avcodec_free_context(&m_pAudioCodecCtx);
    }
}

bool VideoInputFile::openFile(const std::string &fileName)
{
    std::cout << "try to open file, " << fileName << std::endl;
    int ret = avformat_open_input(&m_pFormatCtx, fileName.c_str(), NULL, NULL);
    if (0 != ret)
    {
        std::cout << "open file failed" << std::endl;
        return false;
    }

    ret = avformat_find_stream_info(m_pFormatCtx, NULL);
    if (ret < 0)
    {
        std::cout << "not found any stream" << std::endl;
        return false;
    }

    //video
    AVCodec *pAvVideoCodec = NULL;
    ret = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pAvVideoCodec, 0);
    if (ret < 0)
    {
        std::cout << "not found video stream" << std::endl;
        return false;
    }

    m_nVideoStreamIndex = ret;
    m_pVideoCodecCtx = avcodec_alloc_context3(pAvVideoCodec);
    if (m_pFormatCtx == NULL)
    {
        std::cout << "alloc acodec context faild" << std::endl;
        return false;
    }

    avcodec_parameters_to_context(m_pVideoCodecCtx, m_pFormatCtx->streams[m_nVideoStreamIndex]->codecpar);

    ret = avcodec_open2(m_pVideoCodecCtx, pAvVideoCodec, NULL);
    if (0 != ret)
    {
        std::cout << "open codec failed" << std::endl;
        return false;
    }

    //audio
    AVCodec *pAvAudioCodec = NULL;
    ret = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &pAvAudioCodec, 0);
    if (ret < 0)
    {
        std::cout << "not found audio stream" << std::endl;
        return false;
    }

    m_nAudioStreamIndex = ret;

    m_pAudioCodecCtx = avcodec_alloc_context3(pAvAudioCodec);
    if (m_pFormatCtx == NULL)
    {
        std::cout << "alloc acodec context faild" << std::endl;
        return false;
    }

    avcodec_parameters_to_context(m_pAudioCodecCtx, m_pFormatCtx->streams[m_nAudioStreamIndex]->codecpar);

    ret = avcodec_open2(m_pAudioCodecCtx, pAvAudioCodec, NULL);
    if (0 != ret)
    {
        std::cout << "open codec failed" << std::endl;
        return false;
    }
    std::cout << "open file success, videoStreamIndex:" << m_nVideoStreamIndex << ", audioStreamIndex:"  << m_nAudioStreamIndex << std::endl;
    m_bFileOpened = true;
    return true;
}


void VideoInputFile::readFileUtilEnd(bool needDecode)
{
    if (m_pListener == NULL)
    {
        std::cout << "not set Listener" << std::endl;
        return;
    }

    if (m_bFileOpened == false)
    {
        std::cout << "file not opened" << std::endl;
        return;
    }

    AVFrame* pFrame = av_frame_alloc();
    AVPacket packet;
    int ret = 0;
    while(1)
    {

        ret = av_read_frame(m_pFormatCtx, &packet);
        if (ret < 0)
        {
            std::cout << "read frame failed" << std::endl;
            break;
        }

        if(packet.stream_index == m_nAudioStreamIndex)
        {
            m_pListener->onAudioPacket(&packet);
            if(needDecode)
                handleAudioPacket(&packet, pFrame);
        }else if(packet.stream_index == m_nVideoStreamIndex)
        {
            m_pListener->onVideoPacket(&packet);
            if(needDecode)
                handleVideoPacket(&packet, pFrame);
        }else
        {

        }
        av_frame_unref(pFrame);
    }
    av_packet_unref(&packet);
    av_frame_free(&pFrame);
    std::cout << "end file" << std::endl;
}

void VideoInputFile::handleVideoPacket(AVPacket *packet, AVFrame *pFrame)
{


    int ret = avcodec_send_packet(m_pVideoCodecCtx, packet);
    if (ret < 0)
    {
        std::cout << "Error while sending a packet to the video  decoder" << std::endl;
        return;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(m_pVideoCodecCtx, pFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            break;
        } else if (ret < 0)
        {
            std::cout << "Error while receiving a frame from the video decoder\n" << std::endl;
            break;
        }

        pFrame->pts = pFrame->best_effort_timestamp;
        m_pListener->onVideoFrame(pFrame);
    }
}

void VideoInputFile::handleAudioPacket(AVPacket *packet, AVFrame *pFrame)
{


    int ret = avcodec_send_packet(m_pAudioCodecCtx, packet);
    if (ret < 0)
    {
        std::cout << "Error while sending a packet to the audio decoder" << std::endl;
        return;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(m_pAudioCodecCtx, pFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            break;
        } else if (ret < 0)
        {
            std::cout << "Error while receiving a frame from the audio decoder\n" << std::endl;
            break;
        }

        pFrame->pts = pFrame->best_effort_timestamp;
        m_pListener->onAudioFrame(pFrame);
    }
}