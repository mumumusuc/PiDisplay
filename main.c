//
// Created by mumumusuc on 19-1-19.
//

#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <math.h>
#include <getopt.h>
#include "factory.h"

#ifdef FFMPEG

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#endif

#ifdef FREE_IMAGE

#include "FreeImage.h"

#endif

#define API_BEFORE  __attribute__((constructor))
#define API_AFTER   __attribute__((destructor))

static void convert(uint8_t *src, uint8_t *dst, int w, int h) {
    size_t index = 0;
    uint8_t p = 0;
    for (int i = 0; i < h / 8; i++) {
        for (int j = 0; j < w; j++) {
            p = 0;
            for (int k = 0; k < 8; k++) {
                index = (i * 8 + k) * w + j;
                p |= (src[index] > 127 ? 0x01 : 0) << k;
            }
            dst[i * w + j] = p;
        }
    }
}

static void convert2(const uint8_t *src, uint8_t *dst, int w, int h) {
    int block_w = w / 8;    //16
    int block_h = h / 8;    //8
    uint8_t tmp = 0;
    for (int i = 0; i < block_h; i++) {
        for (int j = 0; j < block_w; j++) {

            for (int k = 0; k < 8; k++) {
                tmp = 0;
                for (int t = 0; t < 8; t++) {
                    tmp |= ((src[(i * 8 + t) * block_w + j] >> (7 - k)) & 0x01) << t;
                }
                dst[i * block_w * 8 + j * block_h + k] = tmp;
            }


        }
    }
}


static Display *display = NULL;
static char device[8] = "ssd1306";
static char driver[8] = "default";
static char type[4] = "spi";
static char *file = "BadApple.mp4";

static void clean_up(int signo) {
    LOG("%s", __func__);
    if (display) {
        //display_turn_off(display);
        display_end(display);
        delete(object(display));
    }
}

API_BEFORE static void register_device() {
    LOG("%s", __func__);
}

API_AFTER static void unregister_devices() {
    LOG("%s", __func__);
    clean_up(0);
}

static print_usage(const char *prog) {
    puts("  -D --device    display device( default ssd1306 )\n"
         "  -d --driver    display driver( i2c|spi, default spi4 )\n"
         "  -t --type      driver type( default linux )\n"
         "  -i --input     input file( default BadApple )\n");
    exit(1);
}

static void parse_args(int argc, char *const argv[]) {
    int result;
    opterr = 0;
    static const struct option lopts[] = {
            {"device", 1, 0, 'D'},
            {"driver", 1, 0, 'd'},
            {"type",   1, 0, 't'},
            {"input",  1, 0, 'i'},
            {NULL,     0, 0, 0},
    };
    while (1) {
        result = getopt_long(argc, argv, "D:d:t:i:", lopts, NULL);
        if (result == -1) break;
        switch (result) {
            case 'D':
                strcpy(device, optarg);
                printf("-D %s.\n", device);
                break;
            case 'd':
                strcpy(driver, optarg);
                printf("-d %s.\n", driver);
                break;
            case 't':
                strcpy(type, optarg);
                printf("-t %s.\n", type);
                break;
            case 'i':
                file = optarg;
                printf("-i %s.\n", file);
                break;
            default:
                print_usage(argv[0]);
                break;
        }
    }
}

/*
static void draw_text(IplImage *src, const char *text, CvPoint origin, CvFont *font) {
    cvPutText(src, text, origin, font, cvScalarAll(255));
}
*/
int main(int argc, char *const argv[]) {
    parse_args(argc, argv);
    char _device[32];
    sprintf(_device, "/%s/%s/%s", device, driver, type);
    LOG("%s", _device);
    display = create_display(_device);
    if (!display) {
        ERROR();
        exit(-1);
    }
    signal(SIGINT, clean_up);
    display_begin(display);
    display_reset(display);
    display_turn_on(display);
    display_clear(display);
    DisplayInfo info;
    display_get_info(display, &info);
    LOG("%s : w = %d , h = %d , fmt = %d", info.vendor, info.width, info.height, info.pixel_format);
    size_t size = sizeof(uint8_t) * info.width * info.height * info.pixel_format / 8;
    uint8_t *screen_buffer = (uint8_t *) calloc(1, size);

#ifdef FREE_IMAGE
    FreeImage_Initialise(TRUE);
    const char *imageFile = "test_2.png";
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    fif = FreeImage_GetFileType(imageFile, 0);
    if (fif == FIF_UNKNOWN)
        fif = FreeImage_GetFIFFromFilename(imageFile);
    FIBITMAP *bmp = NULL;
    if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
        bmp = FreeImage_Load(fif, imageFile, PNG_DEFAULT);
    }
    if (!bmp) {
        ERROR("Fail to Load Image!");
        exit(1);
    }
    int width = FreeImage_GetWidth(bmp);
    int height = FreeImage_GetHeight(bmp);
    FIBITMAP *gray = FreeImage_ConvertToGreyscale(bmp);
    FIBITMAP *dst = FreeImage_Rescale(gray, 128, 64, FILTER_BOX);
    uint8_t *bits = FreeImage_GetBits(dst);

    convert(bits, screen_buffer, info.width, info.height);
#else
    memset(screen_buffer,0xf0,size);
#endif

    display_update(display, screen_buffer);
    free(screen_buffer);

#ifdef FREE_IMAGE
    FreeImage_Unload(bmp);
    FreeImage_Unload(gray);
    FreeImage_Unload(dst);
    FreeImage_DeInitialise();
#endif
    /*


    display = create_display(_device);

    if (!display) {
        ERROR();
        exit(-1);
    }
    signal(SIGINT, clean_up);
    display_begin(display);
    display_reset(display);
    display_turn_on(display);
    //display_clear(display);
    uint8_t *screen_buffer = (uint8_t *) calloc(1, sizeof(uint8_t) * 128 * 64);

    av_register_all();
    AVFormatContext *pFmtCtx = NULL;
    if (avformat_open_input(&pFmtCtx, file, NULL, NULL) != 0) {
        exit(1);
    }
    if (avformat_find_stream_info(pFmtCtx, NULL) < 0) {
        exit(1);
    }
    int videoStream = -1;
    for (int i = 0; i < pFmtCtx->nb_streams; ++i) {
        if (pFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }
    if (videoStream == -1) {
        exit(1);
    }
    AVCodecParameters *pCodecParams = pFmtCtx->streams[videoStream]->codecpar;
    AVCodec *pCodec = avcodec_find_decoder(pCodecParams->codec_id);
    if (!pCodec) {
        exit(1);
    }
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);
    if (avcodec_parameters_to_context(pCodecCtx, pCodecParams) < 0) {
        exit(1);
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        exit(1);
    }
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameG = av_frame_alloc();

    enum AVPixelFormat format = AV_PIX_FMT_GRAY8;

    int width = 128;
    int height = 64;
    int align = 1;

    float r = 1.0f / fmax(pCodecCtx->width / (float) width, pCodecCtx->height / (float) height);
    int w = (int) (pCodecCtx->width * r);
    int h = (int) (pCodecCtx->height * r);
    int offset = (width - w) / 2;

    size_t size = sizeof(uint8_t) * av_image_get_buffer_size(format, width, height, align);
    uint8_t *buffer = (uint8_t *) av_malloc(size);
    av_image_fill_arrays(pFrameG->data, pFrameG->linesize, buffer, format, width, height, align);
    //pFrameG->format = format;
    int fps = pFmtCtx->streams[videoStream]->avg_frame_rate.num / pFmtCtx->streams[videoStream]->avg_frame_rate.den;
    printf("size = %ld, linesize = %d, fps = %d.\n", size, pFrameG->linesize[0], fps);
    struct SwsContext *pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                                w + 2, h, format,
                                                SWS_BILINEAR, NULL, NULL, NULL);
    AVPacket packet;

    IplImage *src = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
    src->imageData = pFrameG->data[0];

    float tt = 1000.0f / fps;
    struct timeval tBeginTime, t_middle_time, tEndTime;
    uint8_t *dst_data[AV_NUM_DATA_POINTERS] = {0};
    int linesize[AV_NUM_DATA_POINTERS] = {0};
    for (int i = 0; i < AV_NUM_DATA_POINTERS; i++) {
        dst_data[i] = pFrameG->data[i] + offset;
        linesize[i] = pFrameG->linesize[i];
        //printf("linesize[%d] = %d .\n", i, pFrameG->linesize[i]);
    }
    bool flush = false;

    char char_fps[4];
    CvFont font = {};
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, 8);
    int baseline;
    CvSize t_size = cvSize(0, 0);
    cvGetTextSize("00", &font, &t_size, &baseline);
    CvPoint origin = cvPoint(0, t_size.height);
    memset(src->imageData, 0, size);
    while (av_read_frame(pFmtCtx, &packet) >= 0) {
        if (packet.stream_index == videoStream) {
            gettimeofday(&tBeginTime, NULL);
            avcodec_send_packet(pCodecCtx, &packet);
            if (flush = avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                memset(src->imageData, 0, size / 6);
                sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0, pFrame->height, dst_data, linesize);
                //cvAdaptiveThreshold(src, src, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 3, 0);
                draw_text(src, char_fps, origin, &font);
                convert(src->imageData, screen_buffer, width, height);
                display_update(display, screen_buffer);
                gettimeofday(&t_middle_time, NULL);
                float time = 1000000 * (t_middle_time.tv_sec - tBeginTime.tv_sec) +
                             (t_middle_time.tv_usec - tBeginTime.tv_usec);
                float dt = tt - time / 1000;
                if (dt > 0) delay(dt);
            }
            gettimeofday(&tEndTime, NULL);
            float fCostTime = 1000000 * (tEndTime.tv_sec - tBeginTime.tv_sec) + (tEndTime.tv_usec - tBeginTime.tv_usec);
            sprintf(char_fps, "%2.0f", 1000000 / fCostTime);
            //LOG("%s fps", char_fps);
        }
    }

    avformat_close_input(&pFmtCtx);
    avcodec_close(pCodecCtx);
    av_frame_free(&pFrame);
    sws_freeContext(pSwsCtx);
    av_packet_unref(&packet);
    free(screen_buffer);
    /*
    size_t width = 320;
    size_t height = 240;
    CvCapture *capture = cvCreateCameraCapture(0);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, width);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, height);
    // cvNamedWindow("camera", CV_WINDOW_AUTOSIZE);
    IplImage *frame;
    IplImage *src = cvCreateImage(cvSize(width, height), 8, 1);
    IplImage *dst = cvCreateImage(cvSize(128, 64), 8, 1);
    cvSetZero(src);
    cvSetZero(dst);
    float r = 1.0f / max(width / (float) dst->width, height / (float) dst->height);
    int w = (int) (width * r);
    int h = (int) (height * r);
    printf("(%d,%d).\n", w, h);
    CvRect roi = cvRect(abs(128 - w) / 2, abs(64 - h) / 2, w, h);
    cvSetImageROI(dst, roi);

    Canvas *canvas = new_Canvas();

    while (true) {
        frame = cvQueryFrame(capture);
        cvCvtColor(frame, src, CV_RGB2GRAY);
        cvResize(src, dst, CV_INTER_LINEAR);
        cvAdaptiveThreshold(dst,dst,1,CV_ADAPTIVE_THRESH_GAUSSIAN_C,CV_THRESH_BINARY,21,0);
        convert(dst->imageData, (uint8_t *) (canvas->display_buffer), 128, 8);
        canvas->flush(canvas);
        //cvShowImage("camera", dst);
        //uint8_t key = cvWaitKey(16);
        //if (key == 27) {
        //    break;
        //}
    }

    del_Canvas(canvas);

    cvReleaseImage(src);
    cvReleaseImage(dst);
    cvReleaseCapture(&capture);
    cvDestroyAllWindows();
     */
}