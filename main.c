//
// Created by mumumusuc on 19-1-19.
//

#include <string.h>
//#include "canvas.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

static void convert(uint8_t *src, uint8_t *dst, int w, int h, int offset) {
    size_t index = 0;
    uint8_t p = 0;
    for (int i = 0; i < h / 8; i++) {
        for (int j = 0; j < w; j++) {
            p = 0;
            if ((j > offset) && (j < w - offset - 1))
                for (int k = 0; k < 8; k++) {
                    index = (i * 8 + k) * w + j - offset;
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
#include <math.h>
#include "driver/bcm/bcm_.h"
#include "device/ssd1306/ssd1306.h"

int main(int argc, char *argv[]) {
    if (!new_bcm_gpio) {
        ERROR("bcm gpio not linked");
        exit(1);
    }
    LOG("bcm gpio linked");

    Display *display = find_superclass(
            new_ssd1306_spi4(
                    superclass(new_bcm_gpio(), Gpio),
                    superclass(new_bcm_spi(), Spi)),
            Display,
            "DISPLAY"
    );

    //Display *display = find_superclass(new_ssd1306_i2c(new_gpio(),new_i2c()),Display,"DISPLAY");
    display_begin(display);
    display_reset(display);
    display_turn_on(display);
    //display_clear(display);
    uint8_t *screen_buffer = (uint8_t *) calloc(1, sizeof(uint8_t) * 128 * 64);


    const char *file = "BadApple.mp4";
    int use_i2c = argc > 1;
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
    printf("size = %ld, linesize = %d.\n", size, pFrameG->linesize[0]);
    struct SwsContext *pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                                w, h, format,
                                                SWS_BILINEAR, NULL, NULL, NULL);
    AVPacket packet;
    int frameCount = 0;

    //Canvas *canvas = new_Canvas(use_i2c);
    //canvas->bind(canvas, pFrameG->data[0], size);

    while (av_read_frame(pFmtCtx, &packet) >= 0) {
        if (packet.stream_index == videoStream) {
            avcodec_send_packet(pCodecCtx, &packet);
            if (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0, pFrame->height, pFrameG->data, pFrameG->linesize);

                convert(pFrameG->data[0], screen_buffer, width, height, offset);
                display_update(display, screen_buffer);
                delay(16);

            }
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