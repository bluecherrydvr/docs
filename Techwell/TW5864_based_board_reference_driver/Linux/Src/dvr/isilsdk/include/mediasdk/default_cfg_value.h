#ifndef __DEFAULT_CFG_VALUE_H__
#define __DEFAULT_CFG_VALUE_H__

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#define WIDTH_FRAME_720P            (1280)
#define WIDTH_FRAME_4CIF_PAL        (704)
#define WIDTH_FRAME_4CIF_NTSC       (704)
#define    WIDTH_FRAME_D1_PAL          (720)
#define    WIDTH_FRAME_D1_NTSC         (720)
#define    WIDTH_FRAME_HALF_D1_PAL     (720)
#define    WIDTH_FRAME_HALF_D1_NTSC    (720)
#define    WIDTH_FRAME_VGA             (640)
#define    WIDTH_FRAME_CIF_PAL         (WIDTH_FRAME_D1_PAL>>1)
#define    WIDTH_FRAME_CIF_NTSC        (WIDTH_FRAME_D1_NTSC>>1)
#define    WIDTH_FRAME_HCIF_PAL        (WIDTH_FRAME_CIF_PAL)
#define    WIDTH_FRAME_HCIF_NTSC       (WIDTH_FRAME_CIF_NTSC)
#define    WIDTH_FRAME_QCIF_PAL        (WIDTH_FRAME_D1_PAL>>2)
#define    WIDTH_FRAME_QCIF_NTSC       (WIDTH_FRAME_D1_NTSC>>2)
#define    WIDTH_FRAME_QVGA            (WIDTH_FRAME_VGA>>1)
#define    HEIGHT_FRAME_720P           (720)
#define HEIGHT_FRAME_4CIF_PAL       (576)
#define HEIGHT_FRAME_4CIF_NTSC      (480)
#define    HEIGHT_FRAME_D1_PAL         (576)
#define    HEIGHT_FRAME_D1_NTSC        (480)
#define    HEIGHT_FRAME_HALF_D1_PAL    (288)
#define    HEIGHT_FRAME_HALF_D1_NTSC   (240)
#define    HEIGHT_FRAME_VGA            (480)
#define    HEIGHT_FRAME_CIF_PAL        (HEIGHT_FRAME_D1_PAL>>1)
#define    HEIGHT_FRAME_CIF_NTSC       (HEIGHT_FRAME_D1_NTSC>>1)
#define    HEIGHT_FRAME_QCIF_PAL       (HEIGHT_FRAME_D1_PAL>>2)
#define    HEIGHT_FRAME_QCIF_NTSC      (HEIGHT_FRAME_D1_NTSC>>2)
#define    HEIGHT_FRAME_QVGA           (HEIGHT_FRAME_VGA>>1)
#endif
#define WIDTH_FRAME_1080P           (1920)
#define WIDTH_FRAME_720P            (1280)
#define WIDTH_FRAME_H960            (960)
#define WIDTH_FRAME_4CIF_PAL        (704)
#define WIDTH_FRAME_2CIF_PAL        (WIDTH_FRAME_4CIF_PAL)
#define WIDTH_FRAME_4CIF_NTSC       (704)
#define WIDTH_FRAME_2CIF_NTSC       (WIDTH_FRAME_4CIF_NTSC)
#define WIDTH_FRAME_D1_PAL          (720)
#define WIDTH_FRAME_D1_NTSC         (720)
#define WIDTH_FRAME_HALF_D1_PAL     (WIDTH_FRAME_D1_PAL)
#define WIDTH_FRAME_HALF_D1_NTSC    (WIDTH_FRAME_D1_NTSC)
#define WIDTH_FRAME_VGA             (640)
#define WIDTH_FRAME_CIF_PAL         (WIDTH_FRAME_4CIF_PAL>>1)
//#define        WIDTH_FRAME_CIF_PAL         (WIDTH_FRAME_D1_PAL>>1)
#define WIDTH_FRAME_HCIF_PAL        (WIDTH_FRAME_CIF_PAL)
#define WIDTH_FRAME_CIF_NTSC        (WIDTH_FRAME_4CIF_NTSC>>1)
//#define        WIDTH_FRAME_CIF_NTSC        (WIDTH_FRAME_D1_NTSC>>1)
#define WIDTH_FRAME_HCIF_NTSC       (WIDTH_FRAME_CIF_NTSC)
#define WIDTH_FRAME_QCIF_PAL        (WIDTH_FRAME_4CIF_PAL>>2)
#define WIDTH_FRAME_QCIF_NTSC       (WIDTH_FRAME_4CIF_NTSC>>2)
#define WIDTH_FRAME_QVGA            (WIDTH_FRAME_VGA>>1)
#define WIDTH_FRAME_SVGA            (800)
#define WIDTH_FRAME_XGA             (1024)

#define HEIGHT_FRAME_1080P          (1080)
#define HEIGHT_FRAME_720P           (720)
#define HEIGHT_FRAME_H960           (576)
#define HEIGHT_FRAME_4CIF_PAL       (576)
#define HEIGHT_FRAME_2CIF_PAL       (HEIGHT_FRAME_4CIF_PAL>>1)
#define HEIGHT_FRAME_HCIF_PAL       (HEIGHT_FRAME_2CIF_PAL>>1)
#define HEIGHT_FRAME_4CIF_NTSC      (480)
#define HEIGHT_FRAME_2CIF_NTSC      (HEIGHT_FRAME_4CIF_NTSC>>1)
#define HEIGHT_FRAME_HCIF_NTSC      (HEIGHT_FRAME_2CIF_NTSC>>1)
#define HEIGHT_FRAME_D1_PAL         (576)
#define HEIGHT_FRAME_D1_NTSC        (480)
#define HEIGHT_FRAME_HALF_D1_PAL    (HEIGHT_FRAME_D1_PAL>>1)
#define HEIGHT_FRAME_HALF_D1_NTSC   (HEIGHT_FRAME_D1_NTSC>>1)
#define HEIGHT_FRAME_VGA            (480)
#define HEIGHT_FRAME_CIF_PAL        (HEIGHT_FRAME_4CIF_PAL>>1)
#define HEIGHT_FRAME_CIF_NTSC       (HEIGHT_FRAME_4CIF_NTSC>>1)
#define HEIGHT_FRAME_QCIF_PAL       (HEIGHT_FRAME_4CIF_PAL>>2)
#define HEIGHT_FRAME_QCIF_NTSC      (HEIGHT_FRAME_4CIF_NTSC>>2)
#define HEIGHT_FRAME_QVGA           (HEIGHT_FRAME_VGA>>1)
#define HEIGHT_FRAME_SVGA           (600)
#define HEIGHT_FRAME_XGA            (768)



#define    MAX_FRAME_WIDTH             (WIDTH_FRAME_720P)
#define    MIN_FRAME_WIDTH             (WIDTH_FRAME_QCIF_PAL)
#define    MAX_FRAME_HEIGHT            (HEIGHT_FRAME_720P)
#define    MIN_FRAME_HEIGHT            (HEIGHT_FRAME_QCIF_NTSC)



#define    MAX_FRAME_RATE_PAL          (25)
#define    MAX_FRAME_RATE_NTSC         (30)
#define    MAX_FRAME_RATE_VGA          (60)
#define    MIN_FRAME_RATE              (1)
#define    MAX_GOP                     (65535)
#define    MIN_GOP                     (100)
#define    MAX_I_P_STRIDE              (MAX_GOP)
#define    MIN_I_P_STRIDE              (25)
#define    MASTER_MAX_BIT_RATE         (3000000)
#define    MASTER_MIN_BIT_RATE         (64000)
#define    SUB_MAX_BIT_RATE            (1000000)
#define    SUB_MIN_BIT_RATE            (32000)
#define    DEFAULT_I_P_STRIDE          (MIN_I_P_STRIDE)
#define    DEFAULT_GOP_VALUE           (MIN_GOP)
#define    DEFAULT_BITRATE_D1SIZE      (1500000)
#define    DEFAULT_BITRATE_VGASIZE     (2000000)    //for 60 frame
#define    DEFAULT_BITRATE_QVGASIZE    (700000)    //for 60 frame
#define    DEFAULT_BITRATE_CIFSIZE     (500000)
#define    DEFAULT_BITRATE_QCIFSIZE    (150000)
#define    DEFAULT_QP                  (28)
#define    DEFAULT_LEVEL_IDC           (30)
#define    DEFAULT_FRAMERATE_PAL       (MAX_FRAME_RATE_PAL)
#define    DEFAULT_FRAMERATE_NTSC      (MAX_FRAME_RATE_NTSC)
#define    DEFAULT_FRAMERATE_VGA       (MAX_FRAME_RATE_VGA)

#define DEFAULT_VI_BRIGHTNESS   (128)
#define DEFAULT_VI_CONTRAST     (100)
#define DEFAULT_VI_SATURATION   (128)
#define DEFAULT_VI_HUE          (128)
#define DEFAULT_VI_SHARPNESS    (1)
#define DEFAULT_VI_STANDARD     CHIP_VI_STANDARD_AUTO

#ifdef __cplusplus
}
#endif
#endif
