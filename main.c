//
// Created by mumumusuc on 19-1-19.
//

#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <math.h>
#include <getopt.h>
#include "factory.h"
#include "driver/linux/default.h"

#ifdef FFMPEG

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <gpio.h>

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
static char *file = "../BadApple.mp4";

static void clean_up(int signo) {
    LOG("%s", __func__);
    //if (display) {
    //display_turn_off(display);
    //    display_end(display);
    //    delete(object(display));
    //}
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
                break;
            case 'd':
                strcpy(driver, optarg);
                break;
            case 't':
                strcpy(type, optarg);
                break;
            case 'i':
                file = optarg;
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
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define scale(pinx, piny, poutx, pouty) \
do{                                     \
   float fix = (float)*pinx;            \
   float fiy = (float)*piny;            \
   float fox = (float)*poutx;           \
   float foy = (float)*pouty;           \
   float r = fminf(fix/fox,fiy/foy);    \
   *poutx = (typeof(*poutx))(fox*r);    \
   *pouty = (typeof(*pouty))(foy*r);    \
}while(0)

int main(int argc, char *const argv[]) {
    parse_args(argc, argv);
    char _device[32];
    sprintf(_device, "/%s/%s/%s", device, driver, type);
    LOG("%s", _device);
    int fd = open("/dev/fb1", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "open %d failed\n", fd);
        return -1;
    }
    printf("open %d success \n", fd);
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    uint8_t *screen_buffer = MAP_FAILED;
    if (ioctl(fd, FBIOBLANK, FB_BLANK_NORMAL) < 0) {
        perror("ioctl FBIOBLANK");
        close(fd);
        return -1;
    }
    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) < 0) {
        perror("ioctl FBIOGET_FSCREENINFO\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        perror("ioctl FBIOGET_VSCREENINFO\n");
        close(fd);
        return -1;
    }
    size_t width = vinfo.xres_virtual;
    size_t height = vinfo.yres_virtual;
    size_t size = finfo.smem_len;
    printf("w = %d, h = %d, s = %d\n", width, height, size);
    /*uint8_t *screen_buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (screen_buffer == MAP_FAILED) {
        perror("mmap failed\n");
        close(fd);
        return -1;
    }*/
    //display = create_display(_device);
    //if (!display) {
    //    ERROR();
    //    exit(-1);
    //}
    //signal(SIGINT, clean_up);
    //display_begin(display);
    //display_reset(display);
    //display_turn_on(display);
    //display_clear(display);
    //DisplayInfo info;
    //display_get_info(display, &info);
    //int width = info.width;
    //int height = info.height;
    //LOG("%s : w = %d , h = %d , s = %d", finfo.id, width, height, size);
    //size_t size = sizeof(uint8_t) * width * height * info.pixel_format / 8;
    //uint8_t *screen_buffer = (uint8_t *) calloc(1, size);

#ifdef FREE_IMAGE
    FreeImage_Initialise(TRUE);
    const char *imageFile = file;
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
    int bmp_w = FreeImage_GetWidth(bmp);
    int bmp_h = FreeImage_GetHeight(bmp);
    printf("bmp_w = %d, bmp_h = %d,\n", bmp_w, bmp_h);
    scale(&width, &height, &bmp_w, &bmp_h);
    vinfo.xres = bmp_w;
    vinfo.yres = bmp_h;
    vinfo.xoffset = (width - bmp_w) / 2;
    vinfo.yoffset = (height - bmp_h) / 2;
    printf("bw = %d, bh = %d, ox = %d, oy = %d\n", vinfo.xres, vinfo.yres, vinfo.xoffset, vinfo.yoffset);
    if (ioctl(fd, FBIOPUT_VSCREENINFO, &vinfo) < 0) {
        perror("ioctl FBIOPUT_VSCREENINFO\n");
        close(fd);
        return -1;
    }
    width = vinfo.xres;
    height = vinfo.yres;
    size = width * height * vinfo.bits_per_pixel / 8;
    printf("sw = %d, sh = %d, ss = %d\n", width, height, size);
    screen_buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (screen_buffer == MAP_FAILED) {
        perror("mmap fb failed\n");
        close(fd);
        return -1;
    }

    FIBITMAP *gray = FreeImage_ConvertToGreyscale(bmp);
    FIBITMAP *dst = FreeImage_Rescale(gray, width, height, FILTER_BOX);
    FreeImage_FlipVertical(dst);
    uint8_t *bits = FreeImage_GetBits(dst);
    convert(bits, screen_buffer, width, height);
    sleep(1);
    //display_update(display, screen_buffer);
#elif defined(FFMPEG)
    av_register_all();
    AVFormatContext *pFmtCtx = NULL;
    if (avformat_open_input(&pFmtCtx, file, NULL, NULL) != 0) {
        perror("open input\n");
        exit(1);
    }
    if (avformat_find_stream_info(pFmtCtx, NULL) < 0) {
        perror("find stream info\n");
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
        perror("video stream\n");
        exit(1);
    }
    AVCodecParameters *pCodecParams = pFmtCtx->streams[videoStream]->codecpar;
    AVCodec *pCodec = avcodec_find_decoder(pCodecParams->codec_id);
    if (!pCodec) {
        perror("find codec\n");
        exit(1);
    }
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);
    if (avcodec_parameters_to_context(pCodecCtx, pCodecParams) < 0) {
        perror("alloc avcodec context\n");
        exit(1);
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        perror("open codec context\n");
        exit(1);
    }
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameG = av_frame_alloc();
    enum AVPixelFormat format = AV_PIX_FMT_GRAY8;
    int align = 1;

    int bmp_w = pCodecCtx->width;
    int bmp_h = pCodecCtx->height;
    printf("bmp_w = %d, bmp_h = %d,\n", bmp_w, bmp_h);
    scale(&width, &height, &bmp_w, &bmp_h);
    vinfo.xres = bmp_w;
    vinfo.yres = bmp_h;
    vinfo.xoffset = (width - bmp_w) / 2;
    vinfo.yoffset = (height - bmp_h) / 2;
    vinfo.bits_per_pixel = 8;
    printf("bw = %d, bh = %d, ox = %d, oy = %d,bpp=%d\n", vinfo.xres, vinfo.yres, vinfo.xoffset, vinfo.yoffset,
           vinfo.bits_per_pixel);
    if (ioctl(fd, FBIOPUT_VSCREENINFO, &vinfo) < 0) {
        perror("ioctl FBIOPUT_VSCREENINFO");
        close(fd);
        return -1;
    }
    width = vinfo.xres;
    height = vinfo.yres;
    size = width * height * vinfo.bits_per_pixel / 8;
    //printf("sw = %d, sh = %d, ss = %d\n", width, height, size);
    printf("bw = %d, bh = %d, ox = %d, oy = %d,bpp=%d\n", vinfo.xres, vinfo.yres, vinfo.xoffset, vinfo.yoffset,
           vinfo.bits_per_pixel);
    screen_buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    /*
     float r = 1.0f / fmax(pCodecCtx->width / (float) width, pCodecCtx->height / (float) height);
     int w = (int) (pCodecCtx->width * r);
     int h = (int) (pCodecCtx->height * r);
     int offset = (width - w) / 2;
 */
    //size_t buffer_size = sizeof(uint8_t) * av_image_get_buffer_size(format, width, height, align);
    //uint8_t *buffer = (uint8_t *) av_malloc(buffer_size);
    //memset(buffer, 0, buffer_size);
    av_image_fill_arrays(pFrameG->data, pFrameG->linesize, screen_buffer, format, width, height, align);
    int fps = pFmtCtx->streams[videoStream]->avg_frame_rate.num / pFmtCtx->streams[videoStream]->avg_frame_rate.den;
    struct SwsContext *pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                                width, height, format,
                                                SWS_BILINEAR, NULL, NULL, NULL);
    AVPacket packet;
    float tt = 1000.0f / fps;
    struct timeval tBeginTime, t_middle_time, tEndTime;
    /*
    uint8_t *dst_data[AV_NUM_DATA_POINTERS] = {0};
    int linesize[AV_NUM_DATA_POINTERS] = {0};
    for (int i = 0; i < AV_NUM_DATA_POINTERS; i++) {
        dst_data[i] = pFrameG->data[i] + offset;
        linesize[i] = pFrameG->linesize[i];
    }*/
    while (av_read_frame(pFmtCtx, &packet) >= 0) {
        if (packet.stream_index == videoStream) {
            gettimeofday(&tBeginTime, NULL);
            avcodec_send_packet(pCodecCtx, &packet);
            if (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0, pFrame->height, pFrameG->data, pFrameG->linesize);
                //cvAdaptiveThreshold(src, src, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 3, 0);
                //convert(pFrameG->data[0], screen_buffer, width, height);
                //display_update(display, screen_buffer);
                gettimeofday(&t_middle_time, NULL);
                float time = 1000000 * (t_middle_time.tv_sec - tBeginTime.tv_sec) +
                             (t_middle_time.tv_usec - tBeginTime.tv_usec);
                float dt = tt - time / 1000;
                if (dt) delay(dt);
            }
            gettimeofday(&tEndTime, NULL);
            float fCostTime = 1000000 * (tEndTime.tv_sec - tBeginTime.tv_sec) + (tEndTime.tv_usec - tBeginTime.tv_usec);
#ifdef OPENCV
            sprintf(char_fps, "%2.0f", 1000000 / fCostTime);
#else
            LOG("%02.0f fps", 1000000 / fCostTime);
#endif
        }
    }
    avformat_close_input(&pFmtCtx);
    avcodec_close(pCodecCtx);
    av_frame_free(&pFrame);
    sws_freeContext(pSwsCtx);
    av_packet_unref(&packet);
#else
    memset(screen_buffer,0xf0,size);
    display_update(display, screen_buffer);
#endif

    //free(screen_buffer);
    if (screen_buffer != MAP_FAILED)
        munmap(screen_buffer, size);
    close(fd);

#ifdef FREE_IMAGE
    FreeImage_Unload(bmp);
    FreeImage_Unload(gray);
    FreeImage_Unload(dst);
    FreeImage_DeInitialise();
#endif
    /*


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