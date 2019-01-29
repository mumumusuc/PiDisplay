//
// Created by mumumusuc on 19-1-19.
//

#include <string.h>
//#include "canvas.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

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

//#include "display_.h"
#include <sys/time.h>
#include <math.h>
#include <getopt.h>
#include <opencv2/imgproc/imgproc_c.h>
#include "driver/bcm/bcm.h"
#include "driver/linux/default.h"
#include "device/ssd1306/ssd1306.h"

#define API_BEFORE  __attribute__((constructor))
#define API_AFTER   __attribute__((destructor))
#define DEVICE_NUM  4

typedef struct {
    char name[24];

    void (*display)(void);

    void (*gpio)(void);

    void (*i2c)(void);

    void (*spi)(void);
} Device;

static Display *display = NULL;
static char device[8] = "ssd1306";
static char type[8] = "default";
static char driver[4] = "spi";
static Device devices[DEVICE_NUM] = {
        {.name="/ssd1306/default/i2c", .display=new_ssd1306_i2c, .gpio=new_default_gpio, .i2c=new_default_i2c, .spi=new_default_spi},
        {.name="/ssd1306/default/spi", .display=new_ssd1306_spi4, .gpio=new_default_gpio, .i2c=new_default_i2c, .spi=new_default_spi},
        {.name="/ssd1306/bcm/i2c", .display=new_ssd1306_i2c, .gpio=new_bcm_gpio, .i2c=new_bcm_i2c, .spi=new_bcm_spi},
        {.name="/ssd1306/bcm/spi", .display=new_ssd1306_spi4, .gpio=new_bcm_gpio, .i2c=new_bcm_i2c, .spi=new_bcm_spi},
};

API_BEFORE static void register_device() {
    LOG("%s", __func__);
}

API_AFTER static void unregister_devices() {
    LOG("%s", __func__);
    if (display) {
        delete(object(display));
    }
}

static print_usage(const char *prog) {
    puts("  -D --device    display device( default ssd1306 )\n"
         "  -d --driver    display driver( i2c|spi, default spi4 )\n"
         "  -t --type      driver type\n");
    exit(1);
}

static void parse_args(int argc, char *const argv[]) {
    int result;
    opterr = 0;
    static const struct option lopts[] = {
            {"device", 1, 0, 'D'},
            {"driver", 1, 0, 'd'},
            {"type",   1, 0, 't'},
            {NULL,     0, 0, 0},
    };
    while (1) {
        result = getopt_long(argc, argv, "D:d:t:", lopts, NULL);
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
                printf("-i %s.\n", type);
                break;
            default:
                print_usage(argv[0]);
                break;
        }
    }
}

static void draw_text(IplImage *src, const char *text, CvPoint origin, CvFont *font) {
    cvPutText(src, text, origin, font, cvScalarAll(255));
}

int main(int argc, char *const argv[]) {
    parse_args(argc, argv);
    char _device[32];
    sprintf(_device, "/%s/%s/%s", device, type, driver);
    LOG("%s", _device);
    /*
    Gpio *gpio = superclass(new_default_gpio(), Gpio);
    GpioInfo info = {
            .pin = 22,
            .mode = GPIO_MODE_OUTPUT,
    };
    gpio_begin(gpio);
    gpio_init(gpio, &info);
    size_t cnt = 10;
    while (--cnt > 0) {
        gpio_write(gpio, &info.pin, GPIO_HIGH);
        delay(500);
        gpio_write(gpio, &info.pin, GPIO_LOW);
        delay(500);
    }
    gpio_end(gpio);
    delete(object(gpio));
*/
    if (!new_bcm_gpio) {
        ERROR("bcm gpio not linked");
        exit(1);
    }
    LOG("bcm gpio linked");

    if (argc > 1) {
        display = find_superclass(
                new_ssd1306_i2c(
                        superclass(new_default_gpio(), Gpio),
                        superclass(new_bcm_i2c(), I2c)),
                Display, "DISPLAY"
        );
    } else {
        display = find_superclass(
                new_ssd1306_spi4(
                        superclass(new_default_gpio(), Gpio),
                        superclass(new_default_spi(), Spi)),
                Display, "DISPLAY"
        );
    }*/
    if (!display) {
        ERROR();
        exit(-1);
    }
    //Display *display = find_superclass(new_ssd1306_i2c(new_gpio(),new_i2c()),Display,"DISPLAY");
    display_begin(display);
    display_reset(display);
    display_turn_on(display);
    //display_clear(display);
    uint8_t *screen_buffer = (uint8_t *) calloc(1, sizeof(uint8_t) * 128 * 64);


    char *file = "BadApple.mp4";
    if (argc > 1) {
        //   file = argv[1];
    }

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

    //Canvas *canvas = new_Canvas(use_i2c);
    //canvas->bind(canvas, pFrameG->data[0], size);
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
//display_turn_off(display);
    display_end(display);

    delete(object(display));
    //IplImage *frame;

    /*
     while (true) {

         cvShowImage("camera", frame);
         uint8_t key = cvWaitKey(16);
         if (key == 27) {
             break;
         }
     }
 */
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

/*
Canvas *canvas = new_Canvas();

const char *file = "test_2.png";
CvMat *img = cvLoadImageM(file, CV_LOAD_IMAGE_GRAYSCALE);
printf("step = %d , type = %x.\n", img->step, img->type);
CvMat src = cvMat(64, 128, CV_8UC1, malloc(128 * 64));
CvMat dst = cvMat(8, 128, CV_8UC1, malloc(128 * 8));
cvResize(img, &src, CV_INTER_LINEAR);
convert(img->data.ptr, dst.data.ptr, 128, 8);

void *old = canvas->bind(canvas, dst.data.ptr, 8 * 128);
free(old);
canvas->flush(canvas);

del_Canvas(canvas);
*/


//cvNamedWindow(file, CV_WINDOW_AUTOSIZE);
//cvShowImage(file, &src);
//cvWaitKey(0);
//cvDestroyAllWindows();
}