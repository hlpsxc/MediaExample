#include<iostream>
#include <zconf.h>

#include "FfmpegHeader.h"



int main()
{

    const std::string inFileName = "/home/skl/pcode/video/doupo.mp4";
    const std::string outFileName = "/home/skl/pcode/video/doupo.avi";


    AVOutputFormat* ofmt = NULL;
    AVBitStreamFilterContext* vbsf = NULL;
    AVFormatContext* ifmtCtx = NULL;
    AVFormatContext* ofmtCtx = NULL;
    AVPacket pkt;
    int frameIndex = 0;

    av_register_all();

    int ret = 0;
    if(avformat_open_input(&ifmtCtx, inFileName.c_str(), NULL, 0) < 0)
    {
        std::cout << "could not open input file" << std::endl;
        goto end;
    }

    ret = avformat_find_stream_info(ifmtCtx, 0);
    if(ret < 0)
    {
        std::cout << "find stream failed" << std::endl;
        goto end;
    }

    vbsf = av_bitstream_filter_init("h264_mp4toannexb");

    av_dump_format(ifmtCtx, 0, inFileName.c_str(), 0);


    avformat_alloc_output_context2(&ofmtCtx, NULL, NULL, outFileName.c_str());
    if(ofmtCtx == NULL)
    {
        std::cout << "could not create output context" << std::endl;
        goto  end;
    }

    ofmt = ofmtCtx->oformat;


    for(int i = 0; i < ifmtCtx->nb_streams; i++)
    {
        AVStream *inStream = ifmtCtx->streams[i];
        AVStream* outStream = avformat_new_stream(ofmtCtx, inStream->codec->codec);

        if(outStream == NULL)
        {
            std::cout << "Failed new Stream" << std::endl;
            goto end;
        }

        if(avcodec_copy_context(outStream->codec, inStream->codec) < 0)
        {
            std::cout << "failed to copy context form input to output stream codec context" << std::endl;
            goto end;
        }

        outStream->codec->codec_tag = 0;
        if(ofmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
        {
            outStream->codec->flags != AV_CODEC_FLAG_GLOBAL_HEADER;
        }
    }

    av_dump_format(ofmtCtx, 0, outFileName.c_str(), 1);


    if(!(ofmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&ofmtCtx->pb, outFileName.c_str(), AVIO_FLAG_WRITE);
        if(ret < 0)
        {
            std::cout << "could not open output file:" << outFileName << std::endl;
            goto end;
        }
    }

    if(avformat_write_header(ofmtCtx, NULL) < 0)
    {
        std::cout << "Error occurred when opening out file" << std::endl;
        goto end;
    }



    while (1)
    {
        AVStream *inStream = NULL;
        AVStream *outStream = NULL;

        ret = av_read_frame(ifmtCtx, &pkt);
        if (ret < 0)
        {
            break;
        }

        inStream = ifmtCtx->streams[pkt.stream_index];
        outStream = ofmtCtx->streams[pkt.stream_index];


        pkt.pts = av_rescale_q_rnd(pkt.pts, inStream->time_base, outStream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, inStream->time_base, outStream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, inStream->time_base, outStream->time_base);


        pkt.pos = -1;


        if (pkt.stream_index == 0)
        {
            AVPacket fpkt = pkt;
            int a = av_bitstream_filter_filter(vbsf, outStream->codec, NULL, &fpkt.data, &fpkt.size,
                    pkt.data, pkt.size, pkt.flags % AV_PKT_FLAG_KEY);
            pkt.data = fpkt.data;
            pkt.size = fpkt.size;
        }

        if(av_write_frame(ofmtCtx, &pkt) < 0)
        {
            std::cout << "error muxing packet" << std::endl;
        }
        std::cout << "write " << frameIndex << " frames to output file" << std::endl;
        av_packet_unref(&pkt);
        frameIndex ++;

    }

    end:
    if(ofmtCtx && !(ofmt->flags & AVFMT_NOFILE))
    {
        avio_close(ofmtCtx->pb);
    }
    if(ofmtCtx)
    {
        avformat_free_context(ofmtCtx);
        ofmtCtx = NULL;

    }
    if(ifmtCtx)
    {
        avformat_close_input(&ifmtCtx);
    }

    return 0;
}
