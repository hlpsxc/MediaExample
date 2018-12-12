#include<iostream>
#include <zconf.h>

#include "FfmpegHeader.h"

/* other way:
   scale=78:24 [scl]; [scl] transpose=cclock // assumes "[in]" and "[out]" to be input output pads respectively
 */
static void display_frame(const AVFrame *frame, AVRational time_base, int64_t &lastPts)
{
    int x, y;
    uint8_t *p0, *p;
    int64_t delay;

    if (frame->pts != AV_NOPTS_VALUE)
    {
        if (lastPts != AV_NOPTS_VALUE)
        {
            /* sleep roughly the right amount of time;
             * usleep is in microseconds, just like AV_TIME_BASE. */
            delay = av_rescale_q(frame->pts - lastPts,
                                 time_base, AV_TIME_BASE_Q);
            if (delay > 0 && delay < 1000000)
                usleep(delay);
        }
        lastPts = frame->pts;
    }

    /* Trivial ASCII grayscale display. */
    p0 = frame->data[0];
    puts("\033c");
    for (y = 0; y < frame->height; y++)
    {
        p = p0;
        for (x = 0; x < frame->width; x++)
            putchar(" .-+#"[*(p++) / 52]);
        putchar('\n');
        p0 += frame->linesize[0];
    }
    fflush(stdout);
}

int main()
{
//    const std::string fileName = "/Users/skl/Desktop/testvideo/xinyu.mp4";
    const std::string fileName = "/Users/skl/Desktop/testvideo/feinimoshu.AVI";
    const char *pFilterDescr = "scale=78:24,transpose=cclock";
    AVFormatContext *pFmtCtx = NULL;
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFilterFrame = av_frame_alloc();
    AVFilterGraph *pFilterGraph = NULL;
    AVCodecContext *pCodecContext = NULL;

    //1.0 open file
    int ret = avformat_open_input(&pFmtCtx, fileName.c_str(), NULL, NULL);
    if (0 != ret)
    {
        std::cout << "open file failed" << std::endl;
        return 0;
    }

    ret = avformat_find_stream_info(pFmtCtx, NULL);
    if (ret < 0)
    {
        std::cout << "not found any stream" << std::endl;
        return 0;
    }

    AVCodec *pAvCodec = NULL;
    ret = av_find_best_stream(pFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pAvCodec, 0);
    if (ret < 0)
    {
        std::cout << "not found video stream" << std::endl;
        return 0;
    }

    int videoStreamIndex = ret;

    pCodecContext = avcodec_alloc_context3(pAvCodec);
    if (pCodecContext == NULL)
    {
        std::cout << "alloc acodec context faild" << std::endl;
        return 0;
    }

    avcodec_parameters_to_context(pCodecContext, pFmtCtx->streams[videoStreamIndex]->codecpar);

    ret = avcodec_open2(pCodecContext, pAvCodec, NULL);
    if (0 != ret)
    {
        std::cout << "open codec failed" << std::endl;
        return 0;
    }


    //2. init filter
    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    AVRational timeBase = pFmtCtx->streams[videoStreamIndex]->time_base;
    enum AVPixelFormat pixFmts[] = {AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE};


    pFilterGraph = avfilter_graph_alloc();
    if (!outputs || !inputs || !pFilterGraph)
    {
        ret = AVERROR(ENOMEM);
        return 0;
    }


    AVFilterContext *pBufferSrcCtx = NULL;
    char args[512];
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             pCodecContext->width, pCodecContext->height, pCodecContext->pix_fmt,
             timeBase.num, timeBase.den,
             pCodecContext->sample_aspect_ratio.num, pCodecContext->sample_aspect_ratio.den);
    ret = avfilter_graph_create_filter(&pBufferSrcCtx, buffersrc, "in",
                                       args, NULL, pFilterGraph);
    if (ret < 0)
    {
        std::cout << "cannot create buffer source" << std::endl;
        return 0;
    }

    AVFilterContext *pBufferSinkCtx = NULL;
    /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&pBufferSinkCtx, buffersink, "out",
                                       NULL, NULL, pFilterGraph);
    if (ret < 0)
    {
        std::cout << "cannot create buffer source" << std::endl;
        return 0;
    }

    ret = av_opt_set_int_list(pBufferSinkCtx, "pix_fmts", pixFmts,
                              AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
    {
        std::cout << "cannot set output pixel format" << std::endl;
        return 0;
    }


    outputs->name = av_strdup("in");
    outputs->filter_ctx = pBufferSrcCtx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    inputs->name = av_strdup("out");
    inputs->filter_ctx = pBufferSinkCtx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    if ((ret = avfilter_graph_parse_ptr(pFilterGraph, pFilterDescr,
                                        &inputs, &outputs, NULL)) < 0)
    {

    }

    if ((ret = avfilter_graph_config(pFilterGraph, NULL)) < 0)
    {

    }

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    //3. filter
    AVPacket packet;
    int64_t lastPts = AV_NOPTS_VALUE;
    while (1)
    {
        ret = av_read_frame(pFmtCtx, &packet);
        if (ret < 0)
        {
            std::cout << "read frame failed" << std::endl;
            break;
        }

        if (packet.stream_index != videoStreamIndex)
        {
            continue;
        }

        ret = avcodec_send_packet(pCodecContext, &packet);
        if (ret < 0)
        {
            std::cout << "Error while sending a packet to the decoder" << std::endl;
            continue;
        }
        while (ret >= 0)
        {
            ret = avcodec_receive_frame(pCodecContext, pFrame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                break;
            } else if (ret < 0)
            {
                std::cout << "Error while receiving a frame from the decoder\n" << std::endl;
                return 0;
            }

            pFrame->pts = pFrame->best_effort_timestamp;

            /* push the decoded frame into the filtergraph */
            if (av_buffersrc_add_frame_flags(pBufferSrcCtx, pFrame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0)
            {
                std::cout << "Error while feeding the filtergraph" << std::endl;
                break;
            }

            /* pull filtered frames from the filtergraph */
            while (1)
            {
                ret = av_buffersink_get_frame(pBufferSinkCtx, pFilterFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                if (ret < 0)
                    return 0;
                std::cout << pFilterFrame->width << "," << pFilterFrame->height << std::endl;
//                display_frame(pFilterFrame, pBufferSinkCtx->inputs[0]->time_base, lastPts);
                av_frame_unref(pFilterFrame);
            }
            av_frame_unref(pFrame);
        }
    }
    av_packet_unref(&packet);


    FAILED:
    /*if (pFmtCtx)
    {
        avformat_close_input(&pFmtCtx);
    }

    if (pCodecContext)
    {
        avcodec_close(pCodecContext);
        avcodec_free_context(&pCodecContext);
    }

    if (pFilterGraph)
    {
        avfilter_graph_free(&pFilterGraph);
    }

    if(pFrame)
    {
        av_frame_free(&pFrame);
        av_frame_free(&pFilterFrame);
    }*/
    return 0;
}
