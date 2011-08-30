/*
 * File: dsdemo.c
 *
 * Description: Loop test program and net server demo for BT878 card.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/vfs.h>                /* statfs() */
#include <sys/stat.h>               /* stat() */
#include <dirent.h>                 /* opendir() */
#include <unistd.h>                 /* unlink(), getcwd() */
#include <semaphore.h>
#include <pthread.h>
#include <sys/timeb.h>

#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <SDL/SDL.h>

#include "DataType.h"
#include "solo6010sdk.h"
#include "tmserverpu.h"

#define WRITE_FILE

#define BMPFILE
#define OSDDEMO

#define LOGO

#define PREVIEWOPEN
int                     bPreviewOpen[MAX_PORT];
PREVIEWCONFIG           previewconf[MAX_PORT];
SDL_Overlay*            pOverlaySurface[MAX_PORT];
static int              g_bPreviewStart = 0;
int                     preview_loop(gpointer data);
SDL_Rect                tmpRect;

/* define USE_PREVIEW_TIMER */
#ifdef USE_PREVIEW_TIMER
static int              useSyncSem = 0;
#else
static int              useSyncSem = 1;
#endif
static int              previewtimer = 0;
pthread_t               hPreviewThread[MAX_PORT];
void*                   videopreview(void* arg);
void*                   videocap(void* arg);
void*                   videorecord(void* arg);
void*                   videodecode(void* arg);
UINT                    bPreviewThreadRun[MAX_PORT];
sem_t                   previewSem[MAX_PORT];

/* #define TEST_GETORIIMAGE */
static int              imagetimer = 0;

/*
 * #define MOTION_DEBUG ;
 * #define RAWSTREAM
 */
#define YUVFILE
int                     qcif_flag[MAX_PORT];
int                     hcif_flag[MAX_PORT];
int                     cifqcif_flag[MAX_PORT];
int                     D1_flag[MAX_PORT];
int                     dcif_flag[MAX_PORT];

static int              totalcard, totalport;
RECT                    video_rect[MAX_PORT];
video_win               v_win[MAX_PORT];
VideoStandard_t         videostandard[MAX_PORT];
video_info              v_info[MAX_PORT];

/* add at 2002.4.10 */
GtkWidget*              w_full_area;
video_win               full_win;
SDL_Rect                full_rect;

GdkColormap*            camp;
GdkGC*                  gc1, *gc2;  /* gc1 for title window, gc2 for video window */

GtkWidget*              title;
GtkWidget*              w_buttons[3];
char*                   button_str[3] = { " Start ", "  Stop ", "  Exit  " };
GtkWidget*              w_image;
GtkWidget*              w_record_flag;
GtkWidget*              w_motion_detect_flag;
GtkWidget*              w_I_frame;

GtkWidget*              w_status[3];
GtkTextBuffer*          w_status_buffer[3];

char*                   status_str[3] = { "Data", "Time", "Free space" };

GtkWidget*              w_info_scale[4];
char*                   info_str[4] = { "SCALE", "  QP ", "INTVL", " GOP " };
GtkObject*              w_adj[4];

/* timer */
int                     processtimer = 0;
time_t                  now;
struct tm*              tm;
char                    timebuf[100];
char                    savepath[50] = ".";

GTimer*                 count_timer = NULL;
gdouble                 elapse_time = 0;

/* add at 2002.4.1 */
int                     count[MAX_PORT];
int                     fixsize = 512 * 1024;   /* 4M bitrate */
struct stat             statbuf;
char                    filename[MAX_PORT][40];

/* add at 2002 4.4 */
int                     old_count[MAX_PORT];
static int              fullflag = 0;           /* space full flag */
static int              limitsize = 0;
static int              selectport = 0;

/* frame infomations add at 2002.4.11 */
GtkWidget*              w_audio_framerate;
GtkTextBuffer*          w_audio_framerate_buffer;
GtkWidget*              w_video_framerate;
GtkTextBuffer*          w_video_framerate_buffer;
GtkWidget*              w_totalframe;
GtkTextBuffer*          w_totalframe_buffer;

GtkWidget*              w_channel;
GtkTextBuffer*          w_channel_buffer;

GtkWidget*              w_netusercount;
GtkTextBuffer*          w_netusercount_buffer;

GtkWidget*              w_net_transfer;

/* add for audio preview at 2002.5.14 */
GtkWidget*              w_audio_preview;
int                     audiopreviewport = 0;

/* add at 2003.5.28, for start/stop preview */
GtkWidget*              w_preview;

/* add at 2004.8 for test that send string to client */
GtkWidget*              w_stringtoclient;

int                     ChannelHandle[MAX_PORT];

int                     record_fd[MAX_PORT];
int                     index_fd[MAX_PORT];     /* add at 2005.6.2 to test the index create */
INDEX_NODE              nodeinfo[MAX_PORT];     /* add at 2005.6.2 to test the index */
int                     cifqcif_fd[MAX_PORT];
int                     video_height[MAX_PORT];

int                     dec_file;
int                     dec_in_fd;
int                     dec_out_fd;

/*
 * int captureflag[MAX_PORT];
 */
char                    file_head[MAX_PORT][40];
char                    cifqciffile_head[MAX_PORT][40];
int                     record_flag[MAX_PORT];

/*
 -----------------------------------------------------------------------------------------------------------------------
 * add at 2002.4.27
 -----------------------------------------------------------------------------------------------------------------------
 */
struct statfs           bbuf;
int                     looptimer = 0;
int                     timeconst = 60 * 5; /* 5 minuts */
char                    starttime[100];

/* for motion detection */
int                     sensitivity = 1;
unsigned char           motion_data[MAX_PORT][4 * 18];

/*
 * unsigned short motion_data[MAX_PORT][(22) * (18)];
 */
RECT                    motion_rect[MAX_PORT];
RECT                    motion_list[MAX_PORT];
int                     full_win_flag = 0;
static int              clickflag = 1;
static int              clickport = -1;

static int              net_flag = 0;
static int              net_head_flag[MAX_PORT];
static int              subchan_net_head_flag[MAX_PORT];

int                     head_size = 0;

/* watch file dparaments */
unsigned long           old_frames[MAX_PORT];
unsigned long           current_frames[MAX_PORT];
unsigned long           zero_count[MAX_PORT];

/* add for set logo by wrh at 2002.4.12 */
int                     logo_w, logo_h;
unsigned char*          logoptr;
int                     flogoyuv;

unsigned short          format1[] = { 16, 19, 'O', 'f', 'f', 'i', 'c', 'e', '\0' };

/*
 * unsigned short format1[] = {16, 19, '°ì', '¹«', 'ÊÒ','\0'};
 * unsigned short format1[] = {76, 240,/*500,
 */
unsigned short          format2[] = {200, 19, /* 500, */_OSD_YEAR4, '-', _OSD_MONTH2, '-',
                                     _OSD_DAY, ' ', _OSD_HOUR24, ':', _OSD_MINUTE, ':',
                                     _OSD_SECOND, '\0'
                                    };
char INT2STR_data[512][10] = {
                        "0","1","2","3","4","5","6","7","8","9",
                        "10","11","12","13","14","15","16","17","18","19",
                        "20","21","22","23","24","25","26","27","28","29",
                        "30","31","32","33","34","35","36","37","38","39",
                        "40","41","42","43","44","45","46","47","48","49",
                        "50","51","52","53","54","55","56","57","58","59",
                        "60","61","62","63","64","65","66","67","68","69",
                        "70","71","72","73","74","75","76","77","78","79",
                        "80","81","82","83","84","85","86","87","88","89",
                        "90","91","92","93","94","95","96","97","98","99",
                        "100","101","102","103","104","105","106","107","108","109",
                        "110","111","112","113","114","115","116","117","118","119",
                        "120","121","122","123","124","125","126","127","128","129",
                        "130","131","132","133","134","135","136","137","138","139",
                        "140","141","142","143","144","145","146","147","148","149",
                        "150","151","152","153","154","155","156","157","158","159",
                        "160","161","162","163","164","165","166","167","168","169",
                        "170","171","172","173","174","175","176","177","178","179",
                        "180","181","182","183","184","185","186","187","188","189",
                        "190","191","192","193","194","195","196","197","198","199",
                        "200","201","202","203","204","205","206","207","208","209",    
                        "210","211","212","213","214","215","216","217","218","219",
                        "220","221","222","223","224","225","226","227","228","229",
                        "230","231","232","233","234","235","236","237","238","239",
                        "240","241","242","243","244","245","246","247","248","249",
                        "250","251","252","253","254","255","256","257","258","259",
                        "260","261","262","263","264","265","266","267","268","269",
                        "270","271","272","273","274","275","276","277","278","279",
                        "280","281","282","283","284","285","286","287","288","289",
                        "290","291","292","293","294","295","296","297","298","299",
                        "300","301","302","303","304","305","306","307","308","309",
                        "310","311","312","313","314","315","316","317","318","319",
                        "320","321","322","323","324","325","326","327","328","329",
                        "330","331","332","333","334","335","336","337","338","339",
                        "340","341","342","343","344","345","346","347","348","349",
                        "350","351","352","353","354","355","356","357","358","359",
                        "360","361","362","363","364","365","366","367","368","369",
                        "370","371","372","373","374","375","376","377","378","379",
                        "380","381","382","383","384","385","386","387","388","389",
                        "390","391","392","393","394","395","396","397","398","399",
                        "400","401","402","403","404","405","406","407","408","409",
                        "410","411","412","413","414","415","416","417","418","419",
                        "420","421","422","423","424","425","426","427","428","429",
                        "430","431","432","433","434","435","436","437","438","439",
                        "440","441","442","443","444","445","446","447","448","449",
                        "450","451","452","453","454","455","456","457","458","459",
                        "460","461","462","463","464","465","466","467","468","469",
                        "470","471","472","473","474","475","476","477","478","479",
                        "480","481","482","483","484","485","486","487","488","489",
                        "490","491","492","493","494","495","496","497","498","499",
                        "500","501","502","503","504","505","506","507","508","509",
                        "510","511"
                        };
/*
 * yxq test at 2004.4.15 ;
 * unsigned short format2[] = {16,19,0xbcfa,0xc249,0xf1b9,'\0'};
 * unsigned short format2[] = {16,19,0xe7ae,0xe1aa,0x73b7,0x4bac,0x50b6,0x64a5,'\0'};
 * unsigned short format2[] = {16,19,0xa2b3,0xa3ce,0x6cdb,0x6ebd,'\0'};
 * ;
 * for bit rate watch
 */
GtkWidget*              w_set_bit_rate;
GtkWidget*              w_set_scale;
GtkWidget*              w_set_qp;
GtkWidget*              w_set_intvl;
GtkWidget*              w_set_gop;

/*
 * BitrateControlType_t brc = brCBR;
 */
BitrateControlType_t    brc = brVBR;

gdouble                 bit_rate_time[MAX_PORT];
unsigned int            bit_rate_size[MAX_PORT];
char                    set_bit_rate_str[MAX_PORT][20];
int                     port_enable[16]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

#if 0
GtkWidget*              w_bit_rate;
GtkTextBuffer*          w_bit_rate_buffer;
#endif 
GtkTextBuffer*          w_poly;
GtkTextBuffer*          w_key;
GtkTextBuffer*          w_str;

static SDL_Surface*     pOverlayScreen;
SDL_Rect                dstRect[MAX_PORT];

#define WIN_W   800
#define WIN_H   720
unsigned short          stream_type = STREAM_TYPE_AVSYNC;

void                    my_exit();

void                    ImageStreamCallback(unsigned int channelNumber, void* context);
int                     imagefile_fd = 0;
unsigned char           imageBuf[704 * 576 * 2];

/*
 * timer for get original image
 */
int getimage_loop(gpointer data)
{
	static int      image_count = 0;
	unsigned char   imagebuf[704 * 576 * 2];
	int             size;   /* ;
                             * = 384 * 288
                             * 2;
                             * */
	char            filename[64];
	int             port;
	int             retval;

	for (port = 0; port < totalport; port++)
	{
		retval = GetOriginalImage(ChannelHandle[port], imagebuf, &size);
		sprintf(filename, "cap%d%d.bmp", port, image_count);
		if (retval < 0)
		{
			printf("GetOriginalImage return %d\n", retval);
			continue;
		}
		else
		{
			printf("GetOriginalImage OK size = %d\n", size);

			if (size == 704 * 576 * 2)  /* PAL */
				SaveYUVToBmpFile(filename, imagebuf, 704, 576);
			if (size == 704 * 480 * 2)  /* NTSC */
				SaveYUVToBmpFile(filename, imagebuf, 704, 480);
		}
	}

	image_count++;
	return TRUE;
}

/*
 *
 */
void end_record(int port)
{
	if (record_fd[port])
	{
		close(record_fd[port]);
		record_fd[port] = -1;
	}

	if (cifqcif_fd[port])
	{
		close(cifqcif_fd[port]);
		cifqcif_fd[port] = -1;
	}

	if (index_fd[port])
	{
		close(index_fd[port]);
		index_fd[port] = -1;
	}
}

/*
 * callback for net server
 */
void message_callback(char* buf, int length)
{
	printf("Get a message from client: length = %d, message = %s\n", length, buf);
}

/*
 *
 */
int check_IP_callback(int channel, char* sIP)
{
	/*
	 * printf("Check IP: channel = %d, IP= %s\n", channel, sIP);
	 */
	return 0;
}

/*
 *
 */
int check_password_callback(char* username, int namelen, char* password, int passlen)
{
	/*
	 * printf("Check password: username = %s, namelen = %d, password = %s, passname
	 * %d\n", username, namelen, password, passlen);
	 */
	return 0;
}

/*
 *
 */
int checkIpAndPass(int channel, char* username, int namelen, char* password, int passlen, char* sIP)
{
	printf("Check IP: channel = %d, IP= %s; username = %s, namelen = %d, password = %s, passname = %d\n",
	       channel, sIP, username, namelen, password, passlen);
	return 0;
}

/*
 *
 */
void disconnect_callback(int nport, char* sIP)
{
	printf("the client from ip %s to channel %d disconnect!\n", sIP, nport);
}

/*
 *
 */
void start_capture_callback(int port)   /* for server start */
{
	if ((!record_flag[port]) && (port < totalport)) /* fixed at 2004.11.11 */
	{
		g_timer_start(count_timer);
		StartVideoCapture(ChannelHandle[port]);
		if (cifqcif_flag[port])
		{
			StartSubVideoCapture(ChannelHandle[port]);
			fprintf(stderr, "<demo_info> start sub channel capture for chan%d!\n", port);
		}
	}
}

/*
 *
 */
void stop_capture_callback(int port)    /* for server stop */
{
	if ((!record_flag[port]) && (port < totalport)) /* fixed at 2004.11.11 */
	{
		g_timer_stop(count_timer);
		StopVideoCapture(ChannelHandle[port]);

        GetEncodePara(ChannelHandle[port],
                      &videostandard[port],
                      &v_info[port].Scale,
                      &v_info[port].Qp,
                      &v_info[port].Intvl,
                      &v_info[port].Gop);
        
        if(v_info[selectport].Scale == 0)
            gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "0. Off");
        else if(v_info[selectport].Scale == 5)
            gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "1. 176x112");
        else if(v_info[selectport].Scale == 2)
            gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "2. 352x240");
        else if(v_info[selectport].Scale == 1)
            gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "3. 704x240");
        else if(v_info[selectport].Scale == 9)
            gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "4. 704x480");
        
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_qp)->entry), INT2STR_data[(v_info[selectport].Qp)]);
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_intvl)->entry), INT2STR_data[(v_info[selectport].Intvl)]);
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_gop)->entry), INT2STR_data[(v_info[selectport].Gop)]);    
	}
}

/*
 * end callback for net server
 */
void net_transfer_callback(GtkWidget* widget, gpointer data)
{
	int                 port;
	SERVER_VIDEOINFO    videoinfo;

	if (GTK_TOGGLE_BUTTON(widget)->active)
	{
		/* start net transfer */
		for (port = 0; port < totalport; port++)
		{
			MP4_ServerSetBufNum(port, 60);
		}

		MP4_ServerSetStart(start_capture_callback);
		MP4_ServerSetStop(stop_capture_callback);
		MP4_ServerSetMessage(message_callback);
		MP4_ServerSetNetPort(5050, 6050);
		videoinfo.m_channum = totalport;

		/*
		 * printf("net_transfer_callback(): videoinfo.m_channum %d, totalport = %d\n",
		 * videoinfo.m_channum, totalport);
		 */
		if (MP4_ServerStart(&videoinfo) < 0)
		{
			net_flag = 0;
			printf("Net start failed;\n");
		}
		else    /* fixed by wrh */
		{
			MP4_ServerCheckIP(check_IP_callback);
			MP4_ServerCheckPassword(check_password_callback);
			MP4_ServerCheckIPandPassword(checkIpAndPass);
			MP4_ServerDisconnectEvent(disconnect_callback);
			net_flag = 1;
		}
	}
	else        /* stop net transfer */
	{
		net_flag = 0;
		videoinfo.m_channum = 0;
		printf("call MP4_serverStop(), totalport = %d\n", totalport);
		MP4_ServerStop();
		memset(net_head_flag, 0, sizeof(net_head_flag));
		memset(subchan_net_head_flag, 0, sizeof(subchan_net_head_flag));
	}
}

/*
 *
 */
void preview_callback(GtkWidget* widget, gpointer data)
{
	int port;
#if 1
#ifdef PREVIEWOPEN
	if (GTK_TOGGLE_BUTTON(widget)->active)
	{
		/* start preview */
		for (port = 0; port < totalport; port++)
		{
			/*
			 * StartVideoPreview(ChannelHandle[port], dstRect[port].w, dstRect[port].h);
			 */
			bPreviewOpen[port] = 1;
		}
	}
	else    /* stop preview */
	{
		gdk_window_clear(w_full_area->window);
		if (full_win_flag)
		{
			/*
			 * StopVideoPreview(ChannelHandle[clickport]);
			 */
			bPreviewOpen[clickport] = 0;
			clickflag ^= 1;
		}
		else
		{
			for (port = 0; port < totalport; port++)
			{
				/*
				 * StopVideoPreview(ChannelHandle[port]);
				 */
				bPreviewOpen[port] = 0;
			}
		}
	}

	full_win_flag = 0;
#endif
#else
#ifdef RAWSTREAM
	port = 1;

	/* yxq test the get image stream at 4.19 */
	if (GTK_TOGGLE_BUTTON(widget)->active)
	{
		/* start */
		if (SetImageStream(ChannelHandle[port], 1, 5, 704, 576, imageBuf) < 0)
			fprintf(stderr, "<demo_info> set the image stream failed!\n");
		fprintf(stderr, "<demo_info> start image raw stream ok!\n");
	}
	else if (SetImageStream(ChannelHandle[port], 0, 5, 704, 576, imageBuf) < 0)
		fprintf(stderr, "<demo_info> set the image stream failed!\n");
#endif
#endif
}

/*
 *
 */
void set_bit_rate_callback(GtkWidget* widget)
{
	const char*     str;
	long            max_bps;
	str = gtk_entry_get_text(GTK_ENTRY(w_set_bit_rate));
	strcpy(set_bit_rate_str[selectport], str);

	max_bps = atol(str);

	printf("selectport = %d, bps = %ld\n", selectport, max_bps);
	SetupBitrateControl(ChannelHandle[selectport], max_bps);
}

void set_wm_callback(GtkWidget* widget)
{
	const char*     string;
	long            poly, key, str;
	string = gtk_entry_get_text(GTK_ENTRY(w_poly));
	poly = atol(string);

	string = gtk_entry_get_text(GTK_ENTRY(w_key));
	key = atol(string);

	string = gtk_entry_get_text(GTK_ENTRY(w_str));
	str = atol(string);

    SetupWM(poly, key, str);
}

/*
 * add at 2002.5.14 for audio preview
 */
void audio_preview_callback(GtkWidget* widget, gpointer data)
{
	int ret;
	ret = SetAudioPreview(ChannelHandle[audiopreviewport], GTK_TOGGLE_BUTTON(widget)->active);
	if (ret < 0)
		printf("Audio preview error.\n");
}

/*
 * record operation
 */
void record_callback(GtkWidget* widget, gpointer data)
{
	int port, retval;

#if 1
	if (GTK_TOGGLE_BUTTON(w_record_flag)->active)   /* start capture */
	{
		for (port = 0; port < totalport; port++)
		{
            record_flag[port] = 1;
		}
	}
	else    /* stop capture */
	{
		for (port = 0; port < totalport; port++)
		{
            record_flag[port] = 0;
            if (StopVideoCapture(ChannelHandle[port]) < 0)
                printf("record_callbacl: StopvideoCapture(port %d) failed\n", port);
            
		}
		memset(count, 0, sizeof(count));
	}
#endif
}

/*
 *
 */
void stringtoclient_callback(GtkWidget* widget, gpointer data)
{
	int     port;
	char    messagebuf[256];

	/* test the string to client */
	for (port = 0; port < 20; port++)
	{
		sprintf(messagebuf, "server message to client, no.%d!\n", port);
		MP4_ServerStringToClient("192.0.0.128", messagebuf, strlen(messagebuf));
		fprintf(stderr, "<demo_info> have send the len %d message to client!\n", strlen(messagebuf));
	}
}

/*
 *
 */
void motion_detect_callback(GtkWidget* widget, gpointer data)
{
	int port;
	if (GTK_TOGGLE_BUTTON(w_motion_detect_flag)->active) /* start motion_detect */
	{
		for (port = 0; port < totalport; port++)
		{
			fprintf(stderr, "<demo_info> start chan%d 's motion detection!\n", port);
			StartMotionDetection(ChannelHandle[port]);
		}
	}
	else    /* stop motion detect */
	{
		for (port = 0; port < totalport; port++)
		{
			StopMotionDetection(ChannelHandle[port]);
		}

		memset(motion_data, 0, sizeof(motion_data));
	}
}

/*
 * for motion detect
 */
void motion_detect(int channel)
{
	int             x, y;
	float           ratioX, ratioY;
	unsigned int*   pBuf = (unsigned int*)motion_data[channel];
	int             maxy = video_height[channel] / 32;  /* add at 2005.3.1 */

	if (full_win_flag)
	{
		/* fixed at 2005.3.1 */
		if (channel == clickport)
		{
			/*
			 * printf("full_win_flag = %d, channel = %d,clickport = %d, maxy = %d\n",
			 * full_win_flag, channel, clickport, maxy);
			 */
			ratioX = (float)full_win.w / (float)704;
			ratioY = (float)full_win.h / (float)video_height[channel];
			pBuf = (unsigned int*)motion_data[channel];
#ifdef MOTION_DEBUG
			printf("full_win(port %d): ratioX = %f, ratioY = %f,video_height = %d\n", channel,
			       ratioX, ratioY, video_height[channel]);
#endif
			for (y = 0; y < maxy; y++)
			{
				unsigned int    val = pBuf[y];
				for (x = 0; x < 22; x++)
				{
					if (val & (1 << x))
					{
						gdk_draw_rectangle(w_full_area->window, gc2, TRUE, full_win.x + x * ratioX * 32,
						                   full_win.y + y * ratioY * 32, ratioX * 32, ratioY * 32);
					}
				}
			}
		}

		return;
	}
	else
	{
		ratioX = (float)v_win[channel].w / (float)704;
		ratioY = (float)v_win[channel].h / (float)video_height[channel];
#ifdef MOTION_DEBUG
		if (channel == 2)
		{
			printf("port %02d: ratioX= %08f, ratioY = %08f, RectLeft = %03d, RectTop = %03d\n",
			       channel, ratioX, ratioY, motion_list[channel].RectLeft,
			       motion_list[channel].RectTop);
		}
#endif
		for (y = 0; y < maxy; y++)
		{
			unsigned int    val = pBuf[y];
			for (x = 0; x < 22; x++)
			{
				if (val & (1 << x))
				{
					gdk_draw_rectangle(w_full_area->window, gc2, TRUE,
					                   motion_list[channel].RectLeft + x * ratioX * 32,
					                   motion_list[channel].RectTop + y * ratioY * 32, ratioX * 32,
					                   ratioY * 32);
				}
			}
		}
	}   /* else */
}

/*
 *
 */
void StreamRead_callback(int channel, char* databuf, int frametype, int framelength)
{
	int subType;
#if 1
#if 0
	if ((channel == 0) && (frametype != PktAudioFrames))
		printf("channel=%d, frametype=0x%x, framelength=%6d\n", channel, frametype, framelength);
#endif
	subType = GetSubChannelStreamType(databuf, frametype);
    printf("StreamRead_callback \n");
	/*
	 * fprintf(stderr,"<demo_info> have get the data for type %d! for chan%d\n",
	 * subType, channel);
	 */
	switch (frametype)
	{
	case PktSubIFrames:
	case PktSubPFrames:
	case PktSubBBPFrames:
	case PktSubSysHeader:
		if (frametype == PktSubSysHeader)
		{
			fprintf(stderr, "<demo_info> have get the sub channel sys header for chan%d!\n", channel);
			memcpy(cifqciffile_head[channel], databuf, framelength);
			head_size = framelength;
		}

		if (record_flag[channel] && cifqcif_fd[channel] && ((subType == 2) || (subType == 4) || (subType == 5)))
		{
			write(cifqcif_fd[channel], databuf, framelength);
		}

	case PktSysHeader:
	case PktIFrames:
	case PktPFrames:
	case PktBBPFrames:
	case PktAudioFrames:
	case PktSFrames:
	{
#if 0
		if ((frametype == PktIFrames) || (channel == 1))
			fprintf(stderr, "<yxq_info> now the I frame len is %d!\n", framelength);
#endif
		if (cifqcif_flag[channel] == 0)
		{
			if (frametype == PktSysHeader)
			{
				memcpy(file_head[channel], databuf, framelength);
				head_size = framelength;
			}

			if (record_flag[channel] && record_fd[channel])
			{
				write(record_fd[channel], databuf, framelength);

				/* add at 2005.6.2 to create the index file */
				if (frametype == PktSysHeader)
					nodeinfo[channel].FrameNum = 0;
				if (frametype == PktIFrames)
				{
					write(index_fd[channel], &nodeinfo[channel], sizeof(nodeinfo[channel]));

					/*
					 * fprintf(stderr,"<demo_info> write to index file!\n");
					 */
					nodeinfo[channel].FrameNum++;
				}

				nodeinfo[channel].FilePos += framelength;
				if (frametype == PktBBPFrames)
					nodeinfo[channel].FrameNum += 3;
			}

			bit_rate_size[channel] += framelength;

			if (net_flag)
			{
				/*
				 * fprintf(stderr,"StreamRead_callback(): net_flag is 1, net_head_flag[%d] = %d,
				 * frametype = %d\n",channel, net_head_flag[channel], frametype);
				 */
				if (net_head_flag[channel] == 0)
				{
					if (frametype != PktSysHeader)
					{
						/*
						 * fprintf(stderr,"frametype != PktSyaHeader\n");
						 */
						MP4_ServerWriteDataEx(channel, file_head[channel], head_size,
						                      PktSysHeader, 0, 0);
					}
					else
						MP4_ServerWriteDataEx(channel, (char*)databuf, framelength, frametype,
						                      0, 0);
					net_head_flag[channel] = 1;
				}
				else
				{
					MP4_ServerWriteDataEx(channel, (char*)databuf, framelength, frametype, 0, 0);

					/*
					 * fprintf(stderr,"<yxq_info> the data len is %d!\n", framelength);
					 */
				}
			}
		}
		else
		{
			if (frametype == PktSubSysHeader)
			{
				fprintf(stderr, "<demo_info> have get the sub channel sys header for chan%d!\n",
				        channel);
				memcpy(cifqciffile_head[channel], databuf, framelength);
				head_size = framelength;
			}

			if (subType == 1)
			{
				memcpy(file_head[channel], databuf, framelength);
				head_size = framelength;
			}

			if (record_flag[channel] && record_fd[channel] &&
			        ((subType == 1) || (subType == 3) || (subType == 5)))
			{
				write(record_fd[channel], databuf, framelength);
			}

			bit_rate_size[channel] += framelength;
			if (net_flag)
			{
				switch (subType)
				{
				case 1:
				case 3:
					/* for cif data to put to subchan0 */
					if (net_head_flag[channel] == 0)
					{
						if (frametype != PktSysHeader)
						{
							MP4_ServerWriteDataEx(channel, file_head[channel], head_size,
							                      PktSysHeader, 0, 0);
						}
						else
							MP4_ServerWriteDataEx(channel, (char*)databuf, framelength,
							                      frametype, 0, 0);
						net_head_flag[channel] = 1;
					}
					else
						MP4_ServerWriteDataEx(channel, (char*)databuf, framelength, frametype,
						                      0, 0);
					break;

				case 2:
				case 4:
					/* for qcif/qqcif data to put to subchan1 */
					if (subchan_net_head_flag[channel] == 0)
					{
						if (frametype != PktSysHeader)
						{
							MP4_ServerWriteDataEx(channel, cifqciffile_head[channel], head_size,
							                      PktSysHeader, 0, 1);
						}
						else
							MP4_ServerWriteDataEx(channel, (char*)databuf, framelength,
							                      frametype, 0, 1);
						subchan_net_head_flag[channel] = 1;
					}
					else
						MP4_ServerWriteDataEx(channel, (char*)databuf, framelength, frametype,
						                      0, 1);
					break;

				case 5: /* audio data */
					MP4_ServerWriteDataEx(channel, (char*)databuf, framelength, frametype, 0, 0);
					MP4_ServerWriteDataEx(channel, (char*)databuf, framelength, frametype, 0, 1);
					break;

				default:
					printf("have get other data!\n");
				}
			}
		}
		break;
	}

	case PktMotionDetection:
	{
		int length;
		length = framelength;
		fprintf(stderr, "motion pkt!\n");
		memcpy(motion_data[channel], databuf, length);      /* fixed at 2005.3.1 */
		if (GTK_TOGGLE_BUTTON(w_motion_detect_flag)->active) /* motion_detect started */
			motion_detect(channel);
		break;
	}

	default:
		break;
	}
#endif
}

/*
 *
 */
void my_exit()
{
	int port;

	printf("End ......");
	MP4_ServerStop();

	if (processtimer)
	{
		gtk_timeout_remove(processtimer);
		processtimer = 0;
	}

	if (looptimer)
	{
		gtk_timeout_remove(looptimer);
		looptimer = 0;
	}

	if (count_timer)
	{
		g_timer_destroy(count_timer);
		count_timer = NULL;
	}

#ifdef PREVIEWOPEN
	if (previewtimer)
	{
		gtk_timeout_remove(previewtimer);
		previewtimer = 0;
	}
#endif
	if (imagetimer)
	{
		gtk_timeout_remove(imagetimer);
		imagetimer = 0;
	}

	if (logoptr)
		free(logoptr);

	if (GTK_TOGGLE_BUTTON(w_audio_preview)->active)
	{
		SetAudioPreview(ChannelHandle[audiopreviewport], 0);    /* port audio preview */
	}

#ifdef PREVIEWOPEN
	g_bPreviewStart = 0;
	memset(bPreviewOpen, 0, sizeof(bPreviewOpen));
	gdk_threads_leave();
	for (port = 0; port < totalport; port++)
	{
#ifndef USE_PREVIEW_TIMER
		sem_post(previewconf[port].SyncSem);                    /* for the sem_wait is a block wait */
		bPreviewThreadRun[port] = 3;
		while (bPreviewThreadRun[port] == 3)
			usleep(100);
#endif
	}

	gdk_threads_enter();
	memset(bPreviewThreadRun, 0, sizeof(bPreviewThreadRun));

	if (GTK_TOGGLE_BUTTON(w_preview)->active)
	{
		if (full_win_flag)
		{
			if (StopVideoPreview(ChannelHandle[clickport]) < 0)
				printf("StopVideopreview(port %d) failed, error 0x%x\n", clickport,
				       GetLastErrorNum());
			if (pOverlaySurface[clickport] != NULL)
			{
				SDL_FreeYUVOverlay(pOverlaySurface[clickport]);
				pOverlaySurface[clickport] = NULL;
			}
		}
		else
		{
			/* yxq test the preview */
			for (port = 0; port < totalport; port++)
			{
				if (StopVideoPreview(ChannelHandle[port]) < 0)
					printf("StopVideopreview(port %d) failed, error 0x%x\n", port, GetLastErrorNum());
				if (pOverlaySurface[port] != NULL)
				{
					SDL_FreeYUVOverlay(pOverlaySurface[port]);
					pOverlaySurface[port] = NULL;
				}
			}
		}
	}

	for (port = 0; port < totalport; port++)
		sem_destroy(previewconf[port].SyncSem);
#endif
	for (port = 0; port < totalport; port++)
	{
		StopMotionDetection(ChannelHandle[port]);
		end_record(port);
		if ((record_flag[port]) && (StopVideoCapture(ChannelHandle[port]) < 0))
		{
			printf("StopVideoCapture(port %d) failed, error 0x%x\n", port, GetLastErrorNum());
			record_flag[port] = 0;
		}

		ChannelClose(ChannelHandle[port]);
	}

	memset(motion_data, 0, sizeof(motion_data));

#ifdef RAWSTREAM
	if (imagefile_fd)
		close(imagefile_fd);
#endif
	gdk_flush();
	SDL_Quit(); /* yxq add at 2004.8.27 */
	gtk_main_quit();
	printf("ok\n");
}

/*
 *
 */
int process_time_loop(gpointer data)
{
	int port;

	/* If no enough space, reset count */
	if (fullflag)
	{
		for (port = 0; port < totalport; port++)
		{
			if (old_count[port] < count[port])
			{
				memcpy(old_count, count, sizeof(count));
				memset(count, -1, sizeof(count));
				break;
			}
		}
	}
	else
	{
		if ((bbuf.f_bavail * (bbuf.f_bsize / 1024)) <= limitsize)                   /* count in 1KB */
		{
			printf("No enough space.avialable blocks = %ld\n", bbuf.f_bavail);
			memcpy(old_count, count, sizeof(count));

			memset(count, -1, sizeof(count));
			fullflag = 1;
		}
	}

	return TRUE;    /* if return 0, will never call this func. */
}

/*
 *
 */
int process_time(gpointer data)
{
	/*
	 * time_t now;
	 * struct tm *tm;
	 */
	char                buf[100];
	char                tmp[] = "%";

	/*
	 * struct statfs bbuf;
	 */
	int                 freespace;
	static int          totalframe;
	float               audioframerate, videoframerate;
	FRAMES_STATISTICS   frame_statistics;
	int                 usercount;
	int                 i;
	int                 bit_rate, tmptime;
	static int          timecount = 0;

	/*
	 * printf("%ld\n", SDL_GetTicks());
	 */
	now = time(NULL);
	tm = localtime(&now);
	strftime(timebuf, sizeof(timebuf), "%Y-%m-%d", tm);

	gtk_text_buffer_set_text(w_status_buffer[0], timebuf, strlen(timebuf));
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(w_status[0]), w_status_buffer[0]);

	/*
	 * gtk_text_backward_delete(GTK_TEXT_VIEW(w_status[0]), strlen(timebuf));
	 * gtk_text_insert(GTK_TEXT_VIEW(w_status[0]), NULL, NULL, NULL, timebuf, -1);
	 */
	strftime(timebuf, sizeof(timebuf), "%T", tm);
	gtk_text_buffer_set_text(w_status_buffer[1], timebuf, strlen(timebuf));
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(w_status[1]), w_status_buffer[1]);

	/*
	 * gtk_text_backward_delete(GTK_TEXT_VIEW(w_status[1]), strlen(timebuf));
	 * gtk_text_insert(GTK_TEXT_VIEW(w_status[1]), NULL, NULL, NULL, timebuf, -1);
	 */
	if (statfs(savepath, &bbuf) < 0)
	{
		printf("statsfs() faied, error: %d\n", errno);

		/*
		 * printf("savepath = %s, bbuf,f_bavail = %d, bbuf.f_blocks = %d\n", savepath,
		 * bbuf.f_bavail, bbuf.f_blocks);
		 */
	}
	else
	{
		freespace = (100 * bbuf.f_bavail) / bbuf.f_blocks;
		sprintf(buf, "%3d%s", freespace, tmp);
	}

	/*
	 * printf("total block = %d, availble block = %d, block size = %d\n",
	 * bbuf.f_blocks, bbuf.f_bavail, bbuf.f_bsize);
	 */
	gtk_text_buffer_set_text(w_status_buffer[2], buf, strlen(buf));
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(w_status[2]), w_status_buffer[2]);

	/*
	 * gtk_text_backward_delete(GTK_TEXT_VIEW(w_status[2]), strlen(buf));
	 * gtk_text_insert(GTK_TEXT_VIEW(w_status[2]), NULL, NULL, NULL, buf, -1);
	 */
	sprintf(buf, "%9d", selectport);
	gtk_text_buffer_set_text(w_channel_buffer, buf, strlen(buf));
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(w_channel), w_channel_buffer);

	/*
	 * gtk_text_backward_delete(GTK_TEXT_VIEW(w_channel), strlen(buf));
	 * gtk_text_insert(GTK_TEXT_VIEW(w_channel), NULL, NULL, NULL, buf, -1);
	 * ;
	 * frame information
	 */
	elapse_time = g_timer_elapsed(count_timer, NULL);
	GetFramesStatistics(ChannelHandle[selectport], &frame_statistics);
//	audioframerate = frame_statistics.AudioFrames / elapse_time;

//	sprintf(buf, "%8.1f", audioframerate);
//	gtk_text_buffer_set_text(w_audio_framerate_buffer, buf, strlen(buf));
//	gtk_text_view_set_buffer(GTK_TEXT_VIEW(w_audio_framerate), w_audio_framerate_buffer);

	/*
	 * gtk_text_backward_delete(GTK_TEXT_VIEW(w_audio_framerate), strlen(buf));
	 * gtk_text_insert(GTK_TEXT_VIEW(w_audio_framerate), NULL, NULL, NULL, buf, -1);
	 */
//	videoframerate = frame_statistics.VideoFrames / elapse_time;
//	sprintf(buf, "%8.1f", videoframerate);
//	gtk_text_buffer_set_text(w_video_framerate_buffer, buf, strlen(buf));
//	gtk_text_view_set_buffer(GTK_TEXT_VIEW(w_video_framerate), w_video_framerate_buffer);

	/*
	 * gtk_text_backward_delete(GTK_TEXT_VIEW(w_video_framerate), strlen(buf));
	 * gtk_text_insert(GTK_TEXT_VIEW(w_video_framerate), NULL, NULL, NULL, buf, -1);
	 * ;
	 * bit rate
	 */
	#if 0
	tmptime = elapse_time - bit_rate_time[selectport];
	if (tmptime > 0)
	{
		bit_rate = (bit_rate_size[selectport] * 8) / (elapse_time - bit_rate_time[selectport]);
		sprintf(buf, "%10d", bit_rate);
		gtk_text_buffer_set_text(w_bit_rate_buffer, buf, strlen(buf));
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(w_bit_rate), w_bit_rate_buffer);

		/*
		 * gtk_text_backward_delete(GTK_TEXT_VIEW(w_bit_rate), strlen(buf));
		 * gtk_text_insert(GTK_TEXT_VIEW(w_bit_rate), NULL, NULL, NULL, buf, -1);
		 */
	}
    #endif
	totalframe = frame_statistics.VideoFrames + frame_statistics.AudioFrames;
	sprintf(buf, "%10d", totalframe);
	gtk_text_buffer_set_text(w_totalframe_buffer, buf, strlen(buf));
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(w_totalframe), w_totalframe_buffer);

	/*
	 * gtk_text_backward_delete(GTK_TEXT_VIEW(w_totalframe), strlen(buf));
	 * gtk_text_insert(GTK_TEXT_VIEW(w_totalframe), NULL, NULL, NULL, buf, -1);
	 */
	usercount = MP4_ServerGetState();
	if (usercount > 0)
	{
		sprintf(buf, "%7d", usercount);
		gtk_text_buffer_set_text(w_netusercount_buffer, buf, strlen(buf));
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(w_netusercount), w_netusercount_buffer);

		/*
		 * gtk_text_backward_delete(GTK_TEXT_VIEW(w_netusercount), strlen(buf));
		 * gtk_text_insert(GTK_TEXT_VIEW(w_netusercount), NULL, NULL, NULL, buf, -1);
		 */
	}
	else
	{
		sprintf(buf, "       0");
		gtk_text_buffer_set_text(w_netusercount_buffer, buf, strlen(buf));
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(w_netusercount), w_netusercount_buffer);

		/*
		 * gtk_text_backward_delete(GTK_TEXT_VIEW(w_netusercount), strlen(buf));
		 * gtk_text_insert(GTK_TEXT_VIEW(w_netusercount), NULL, NULL, NULL, buf, -1);
		 */
	}

	return TRUE;    /* if return 0, will never call this func. */
}

/*
 *
 */
void process_buttons(GtkWidget* widget, gpointer data)
{
	int i, port;

	/*
	 * char filename[40];
	 */
	int retval = 0;

	i = GPOINTER_TO_INT(data);
	switch (i)
	{
	case 2: /* exit */
		my_exit();
		break;

	case 3: /* get image */
	{
		static int      image_count = 0;
		unsigned char   imagebuf[704 * 576 * 2];
		int             size;               /* ;
                                                    * = 384 * 288
                                                    * 2;
                                                    * */
		char            filename[100];
#ifdef YUVFILE
		int             fid;
#endif
		for (port = 0; port < totalport; port++)
		{
			retval = GetOriginalImage(ChannelHandle[port], imagebuf, &size);
#ifdef BMPFILE
			sprintf(filename, "cap%d%d.bmp", port, image_count);
			if (retval < 0)
			{
				printf("GetOriginalImage return %d\n", retval);
				continue;
			}
			else
			{
				printf("GetOriginalImage OK size = %d\n", size);

				if (size == 704 * 576 * 2)  /* PAL */
					SaveYUVToBmpFile(filename, imagebuf, 704, 576);
				if (size == 704 * 480 * 2)  /* NTSC */
					SaveYUVToBmpFile(filename, imagebuf, 704, 480);
			}
#endif
#ifdef YUVFILE
			sprintf(filename, "cap%d.yuv", port);
			fid = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
			if (fid < 0)
			{
				perror("get image:");
				break;
			}

			size = 0;
			retval = GetOriginalImage(ChannelHandle[port], imagebuf, &size);
			if (retval < 0)
			{
				printf("GetOriginalImage return %d\n", retval);
				continue;
			}
			else
			{
				printf("   size = %d\n", size);
				write(fid, imagebuf, size);
			}

			close(fid);
#endif
		}

		image_count++;
		break;
	}

	case 4: /* capture I frame */
		for (port = 0; port < totalport; port++)
		{
			CaptureIFrame(ChannelHandle[port]);
			break;
		}

	default:
		break;
	}
}

void ch_encodescale(GtkWidget *widget, gpointer *data)
{
    char tmpdata[100];
    
    memcpy(tmpdata, data, sizeof(data));

    if(tmpdata[0]=='0')
    {
        v_info[selectport].Scale = 0;
    }
    else if(tmpdata[0]=='1')
    {
        v_info[selectport].Scale = 5;
    }
    else if(tmpdata[0]=='2')
    {
        v_info[selectport].Scale = 2;
    }
    else if(tmpdata[0]=='3')
    {
        v_info[selectport].Scale = 1;
    }
    else if(tmpdata[0]=='4')
    {
        v_info[selectport].Scale = 9;
    }

    if(record_flag[0] == 1)
    {
        SetEncodePara(ChannelHandle[selectport], 
                      v_info[selectport].Scale,
                      v_info[selectport].Qp,
                      v_info[selectport].Intvl,
                      v_info[selectport].Gop,
                      1);
    }
}

void ch_encodeqp(GtkWidget *widget, gpointer *data)
{
    char tmpdata[100];
    
    memcpy(tmpdata, data, sizeof(data));

    if(strlen(tmpdata) == 2)
    {
        v_info[selectport].Qp = (tmpdata[1]-'0') + (tmpdata[0]-'0')*10;
    }
    else if(strlen(tmpdata) == 1)
    {
        v_info[selectport].Qp = tmpdata[0]-'0';
    }
    if(record_flag[0] == 1)
    {
        SetEncodePara(ChannelHandle[selectport], 
                      v_info[selectport].Scale,
                      v_info[selectport].Qp,
                      v_info[selectport].Intvl,
                      v_info[selectport].Gop,
                      2);
    }
}

void ch_encodeintvl(GtkWidget *widget, gpointer *data)
{
    char tmpdata[100];
    
    memcpy(tmpdata, data, sizeof(data));
    
    if(strlen(tmpdata) == 3)
    {
        v_info[selectport].Intvl = (tmpdata[2]-'0') + (tmpdata[1]-'0')*10 + (tmpdata[0]-'0')*100;
    }
    else if(strlen(tmpdata) == 2)
    {
        v_info[selectport].Intvl = (tmpdata[1]-'0') + (tmpdata[0]-'0')*10;
    }
    else if(strlen(tmpdata) == 1)
    {
        v_info[selectport].Intvl = tmpdata[0]-'0';
    }
    if(record_flag[0] == 1)
    {
        SetEncodePara(ChannelHandle[selectport], 
                          v_info[selectport].Scale,
                          v_info[selectport].Qp,
                          v_info[selectport].Intvl,
                          v_info[selectport].Gop,
                          4);
    }
}

void ch_encodegop(GtkWidget *widget, gpointer *data)
{
    char tmpdata[100];
    
    memcpy(tmpdata, data, sizeof(data));
    
    if(strlen(tmpdata) == 3)
    {
        v_info[selectport].Gop = (tmpdata[2]-'0') + (tmpdata[1]-'0')*10 + (tmpdata[0]-'0')*100;
    }
    else if(strlen(tmpdata) == 2)
    {
        v_info[selectport].Gop = (tmpdata[1]-'0') + (tmpdata[0]-'0')*10;
    }
    else if(strlen(tmpdata) == 1)
    {
        v_info[selectport].Gop = tmpdata[0]-'0';
    }
    if(record_flag[0] == 1)
    {
        SetEncodePara(ChannelHandle[selectport], 
                      v_info[selectport].Scale,
                      v_info[selectport].Qp,
                      v_info[selectport].Intvl,
                      v_info[selectport].Gop,
                      8);
    }
}


void ch_norm(GtkWidget* widget, gpointer data)
{
    if (GTK_TOGGLE_BUTTON (widget)->active) 
    {
        /* If control reaches here, the toggle button is depressed. */
        SetDefaultVideoStandard(StandardNTSC);
    }
    else
    {
        SetDefaultVideoStandard(StandardPAL);
    }
}

/*
 * video info changed
 */
void ch_bscale(GtkAdjustment* adjust, gpointer data)
{
	int i, port;
	i = GPOINTER_TO_INT(data);
    
	switch (i)
	{
    	case 0:
    		v_info[selectport].Scale = (gint) adjust->value;
    		break;

    	case 1:
    		v_info[selectport].Qp = (gint) adjust->value;
    		break;

    	case 2:
    		v_info[selectport].Intvl = (gint) adjust->value;
    		break;

    	case 3:
    		v_info[selectport].Gop = (gint) adjust->value;
    		break;

    	default:
    		break;
	}

}

/*
 * add 2002.4.10
 */
int PtInRect(SDL_Rect rect, int x, int y)
{
	if ((x > (full_win.x + rect.x)) && (x < (full_win.x + rect.x + rect.w)) &&
	        (y > (full_win.y + rect.y)) && (y < (full_win.y + rect.y + rect.h)))
		return 1;   /* point in rect */
	return 0;
}

/*
 *
 */
int GetPort()
{
	int port = -1;
	int x, y, i;

	gdk_window_get_pointer(w_full_area->window, &x, &y, NULL);
	x += full_win.x;
	y += full_win.y;

	for (i = 0; i < totalport; i++)
	{
		if (PtInRect(dstRect[i], x, y))
		{
			port = i;
			break;
		}
	}

	return port;
}

/*
 *
 */
int click2(GtkWidget* widget, GdkEventButton* event, gpointer data)
{
	int port, i;

	if (!GTK_TOGGLE_BUTTON(w_preview)->active)
		return 0;

	port = GetPort();
	if (port >= 0)
	{
		selectport = port;
		if (strlen(set_bit_rate_str[selectport]) == 0)
			gtk_entry_set_text(GTK_ENTRY(w_set_bit_rate), "0");
		else
			gtk_entry_set_text(GTK_ENTRY(w_set_bit_rate), set_bit_rate_str[selectport]);
	}

#if 1
    {
/*        GetEncodePara(ChannelHandle[selectport],
                      &videostandard[selectport],
                      &v_info[selectport].Scale,
                      &v_info[selectport].Qp,
                      &v_info[selectport].Intvl,
                      &v_info[selectport].Gop);*/

        if(v_info[selectport].Scale == 0)
            gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "0. Off");
        else if(v_info[selectport].Scale == 5)
            gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "1. 176x112");
        else if(v_info[selectport].Scale == 2)
            gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "2. 352x240");
        else if(v_info[selectport].Scale == 1)
            gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "3. 704x240");
        else if(v_info[selectport].Scale == 9)
            gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "4. 704x480");

        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_qp)->entry), INT2STR_data[(v_info[selectport].Qp)]);
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_intvl)->entry), INT2STR_data[(v_info[selectport].Intvl)]);
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_gop)->entry), INT2STR_data[(v_info[selectport].Gop)]);    
    }
#endif

    if ((event->type == GDK_BUTTON_PRESS)&&(event->button == 3) )
    { 
        port_enable[port] ^=1 ; 
    } 

	/* button press twice */
	if (event->type != GDK_2BUTTON_PRESS /* 5 */ )
	{
		if (port < 0)
			return -1;
		if (GTK_TOGGLE_BUTTON(w_audio_preview)->active)
		{
			/* add at 2002.5.14 for audio preview */
			SetAudioPreview(ChannelHandle[audiopreviewport], 0);    /* stop old port audio preview */
			audiopreviewport = port;
			SetAudioPreview(ChannelHandle[audiopreviewport], 1);    /* start new port audio preview */
		}

		return(TRUE);
	}

#ifdef PREVIEWOPEN
	if (clickflag)
	{
		if (port < 0)
			return TRUE;

		gdk_threads_leave();
		for (i = 0; i < totalport; i++)
		{
			bPreviewOpen[i] = 0;
#ifndef USE_PREVIEW_TIMER
			sem_post(previewconf[i].SyncSem);                       /* yxq add at 2005.9.7 */
			bPreviewThreadRun[i] = 3;
			while (bPreviewThreadRun[i] == 3)
				usleep(100);
#endif
			if (StopVideoPreview(ChannelHandle[i]) < 0)
				printf("StoptVideoPreview(channel %d) failed\n", i);
			if (pOverlaySurface[i] != NULL)
			{
				SDL_FreeYUVOverlay(pOverlaySurface[i]);
				pOverlaySurface[i] = NULL;
			}
		}

		gdk_threads_enter();

		clickport = port;
#if 0        
		previewconf[port].w = full_rect.w;
		previewconf[port].h = full_rect.h;
#endif
        previewconf[port].w = 176;
        if(videostandard[0]==1)
            previewconf[port].h = 144;
        else if(videostandard[0]==0)
            previewconf[port].h = 120;
        
		if (StartVideoPreview(ChannelHandle[port], &previewconf[port], useSyncSem) < 0)
			printf("start video preview(port %d) failed\n", port);
		pOverlaySurface[port] = SDL_CreateYUVOverlay(previewconf[port].w, previewconf[port].h,
		                        SDL_UYVY_OVERLAY, pOverlayScreen);
		if (pOverlaySurface[port] == NULL)
		{
			fprintf(stderr, "the SDL overlay surface create failed!\n");
			return 0;
		}

		memcpy(&tmpRect, &dstRect[port], sizeof(SDL_Rect));
		memcpy(&dstRect[port], &full_rect, sizeof(SDL_Rect));

		bPreviewOpen[port] = 1;
		full_win_flag = 1;
	}
	else
	{
		gdk_threads_leave();
		bPreviewOpen[clickport] = 0;
#ifndef USE_PREVIEW_TIMER
		sem_post(previewconf[clickport].SyncSem);
		bPreviewThreadRun[clickport] = 3;
		while (bPreviewThreadRun[clickport] == 3)
			usleep(100);
#endif

		/*
		 * GdkRectangle GdkRect;
		 */
		StopVideoPreview(ChannelHandle[clickport]);
		if (pOverlaySurface[clickport] != NULL)
		{
			SDL_FreeYUVOverlay(pOverlaySurface[clickport]);
			pOverlaySurface[clickport] = NULL;
		}

		memcpy(&dstRect[clickport], &tmpRect, sizeof(SDL_Rect));

		/* generate expose event */
		gdk_window_hide(w_full_area->window);
		usleep(1000);
		gdk_window_show(w_full_area->window);

		gdk_window_clear(w_full_area->window);
		for (i = 0; i < totalport; i++)
		{
#if 0		
			previewconf[i].w = dstRect[i].w;
			previewconf[i].h = dstRect[i].h;
            if(previewconf[i].w < 320)
                previewconf[i].w = 320;
            if(previewconf[i].h < 240)
                previewconf[i].h = 240;
            if(previewconf[i].w > 720)
                previewconf[i].w = 720;
            if(previewconf[i].h > 576)
                previewconf[i].h = 576;
#endif

            previewconf[i].w = 176;
            if(videostandard[0]==1)
                previewconf[i].h = 144;
            else if(videostandard[0]==0)
                previewconf[i].h = 120;

			if (StartVideoPreview(ChannelHandle[i], &previewconf[i], useSyncSem) < 0)
				printf("start video preview(port %d) failed\n", i);
			pOverlaySurface[i] = SDL_CreateYUVOverlay(previewconf[i].w, previewconf[i].h,
			                     SDL_UYVY_OVERLAY, pOverlayScreen);
			if (pOverlaySurface[i] == NULL)
			{
				fprintf(stderr, "the SDL overlay surface create failed!\n");
				return 0;
			}

			bPreviewOpen[i] = 1;
		}

		gdk_threads_enter();
		full_win_flag = 0;
	}

	clickflag ^= 1;
#endif
	return 0;
}

/*
 *
 */
gint expose_ev(GtkWidget* widget, GdkEvent* event, gpointer data)
{
	char*       str1 = "scode demo - loop test for solo6010";
	GdkFont*    font = NULL;
	GdkColor    color;
	char        title_str[160];
	char        tmpstr[] = " ";

	sprintf(title_str, "%s %20s Start Time: %s", str1, tmpstr, starttime);

	camp = gdk_colormap_get_system();
	gc1 = gdk_gc_new(title->window);                    /* get GC */

	/* title expose */
	if (widget->window == title->window)
	{
		/* set title background */
		color.red = 0;
		color.green = 0;
		color.blue = 0xffff;
		if (!gdk_color_alloc(camp, &color))
		{
			g_error("Cannot allocate color\n");
		}

		gdk_window_set_background(widget->window, &color);

		/* set foreground */
		color.red = 0xffff;
		color.green = 0xffff;
		color.blue = 0xffff;
		if (!gdk_color_alloc(camp, &color))
		{
			g_error("Cannot allocate color\n");
		}

		gdk_gc_set_foreground(gc1, &color);
		gdk_window_clear(widget->window);

		/*
		 * font = gdk_fontset_load("7x14,hanzigb16fs");
		 */
		font = gdk_fontset_load("-*-*-*-*-normal--15-*-*-*-*-*-*");
		if (font != NULL)
		{
			gdk_draw_text(widget->window, font, gc1, 10, 20, title_str, strlen(title_str));
			gdk_font_unref(font);
		}

		return(TRUE);
	}

	/*
	 * video_area expose ;
	 * allocte color
	 */
	gc2 = gdk_gc_new(w_full_area->window);              /* get GC */
	if (gdk_color_parse("black", &color))
	{
		if (!gdk_colormap_alloc_color(camp, &color, FALSE, TRUE))
		{
			g_error("Cannot allocate color\n");
		}
	}

	gdk_window_set_background(widget->window, &color);  /* set background color */

	/* set foreground to sign motion block */
	color.red = 0;
	color.green = 0xffff;
	color.blue = 0;
	if (!gdk_color_alloc(camp, &color))
	{
		g_error("Cannot allocate color\n");
	}

	gdk_gc_set_foreground(gc2, &color);
	gdk_window_clear(widget->window);

	return(TRUE);
}

/*
 *
 */
int main(int argc, char* argv[])
{
	GtkWidget*              window;
	GtkWidget*              frame, *main_box, *hbox, *hbox1, *vbox1, *vbox2, *hbox2;
	GtkWidget*              label;
    GtkWidget*              button;
    GSList *group;
    GList        *glist;
    GtkWidget *combo;
	GdkVisual*              visual;
	int                     win_width, win_height;  /* yxq modified at 2004.2.23 */
	int                     i, j;
	char                    SDL_windowhack[32];     /* yxq add at 2004.8.25 */

	/* for callback function, add at 2002.4.24 */
	STREAM_READ_CALLBACK    pStreamRead;
	pStreamRead = (STREAM_READ_CALLBACK) StreamRead_callback;

	if (!g_thread_supported())
	{
		g_thread_init(NULL);
	}
	else
	{
		fprintf(stderr, "your glib is not support multi-threads\n");
		exit(-1);
	}

	gdk_threads_init();

        if (InitDSPs() == 0) /* fixed at 2004.12.3 */
        {
            //printf("Initial DSP failed.error code is 0x%x\n", GetLastErrorNum());
            //exit(1);
        }
    
        /* get total cards and ports */
        totalcard = GetTotalDSPs();
        totalport = GetTotalChannels();
    
        /*
         * totalport = 1;
         * //debug /* yxq test at 2005.9.29 for high quality d1
         */
        printf("total cards = %d, total ports = %d\n", totalcard, totalport);
    
        limitsize = timeconst * (totalport * fixsize) / 1024;       /* KB */
        printf("limit free space : %luKB\n", limitsize);
    
#ifdef LOGO
        /*
         * set logo ;
         * logoptr = (unsigned char *)malloc(128 * 32 * 2);
         */
        logoptr = (unsigned char*)malloc(128 * 128 * 2);
        memset(logoptr, 0, sizeof(logoptr));
        if (LoadYUVFromBmpFile("logo.bmp", logoptr, 128 * 128 * 2, &logo_w, &logo_h) < 0)
            fprintf(stderr, "<demo_info> load bmp file error! error code is 0x%x!\n", GetLastErrorNum());
#endif
    
        /* open channel */
        for (i = 0; i < totalport; i++)
        {
            if ((ChannelHandle[i] = ChannelOpen(i, pStreamRead)) < 0)
            {
                fprintf(stderr, "open port %d failed\n", i);
                for (j = 0; j < i; j++)
                    ChannelClose(ChannelHandle[j]);
                exit(1);
            }
    
//            fprintf(stderr, "the Chan%d open ok!\n", i);
    
            /*
             * //test the mask setup RECT r={8,327,16,335};
             * SetupMask(ChannelHandle[i],&r,1);
             * ;
             * SetStreamType(ChannelHandle[i], STREAM_TYPE_VIDEO);
             * SetDefaultQuant(ChannelHandle[i],12,12,17);
             */
            GetEncodePara(ChannelHandle[i], &videostandard[i], &v_info[i].Scale,
                         &v_info[i].Qp, &v_info[i].Intvl, &v_info[i].Gop);
#ifdef OSDDEMO
            /*
             * SetOsdDisplayMode(ChannelHandle[i], 0xff, 1, 0x0000, format1, format2);
             */
            {
                unsigned short*     p[2];
                p[0] = format1;
                p[1] = format2;
//                fprintf(stderr, "<demo_info> to set osd display mode ex!\n");
                SetOsdDisplayModeEx(ChannelHandle[i], 0xff, 1, 0x2020000, 2, p);
            }
    
            /* yxq test at 2004.4.28 */
            SetOsd(ChannelHandle[i], 1);
#endif
#ifdef LOGO
            if (videostandard[i] == StandardNTSC)
                SetLogo(ChannelHandle[i], 576, 448, logo_w, logo_h, logoptr);
            else if (SetLogo(ChannelHandle[i], 576, 544, logo_w, logo_h, logoptr) == 0)
            {
//                fprintf(stderr, "<demo_info> set the logo ok! chan%d\n", i);
            }
            else
            {
                fprintf(stderr, "<demo_info> set the logo failed!chan%d,w*h: %d*%d,error:0x%x\n", i,
                        logo_w, logo_h, GetLastErrorNum());
            }
    
            /*
             * SetLogoDisplayMode(ChannelHandle[i], 0x0, 0x0, 0x0, 1, 0x0201);
             */
            SetLogoDisplayMode(ChannelHandle[i], 0xff, 0x0, 0x0, 0, 0);
#endif
            AdjustMotionDetectPrecision(ChannelHandle[i], 2, 2, 0); /* disable slow motion detect */
    
            /* There is constant bit rate now! */
            SetBitrateControlMode(ChannelHandle[i], brc);
    
            SetStreamType(ChannelHandle[i], stream_type);
    
            if (videostandard[i] == StandardNTSC)
                video_height[i] = 480;
            else
                video_height[i] = 576;
        }



	/*
	 * XInitThreads();
	 * ;
	 * init display
	 */
	gtk_set_locale();
	gtk_init(&argc, &argv);
	camp = gdk_colormap_get_system();

	/* get display size */
	win_width = gdk_screen_width();
	win_height = gdk_screen_height();

	fprintf(stderr, "<demo_info> the screen w:%d h:%d!\n", win_width, win_height);
	if (win_width > 1024 || win_height > 768)
	{
		win_width = 1024;
		win_height = 768;
	}

	visual = gdk_visual_get_system();

	/*
	 * get current directory ;
	 * getcwd(savepath, 50);
	 * create window ;
	 * window = gtk_window_new(GTK_WINDOW_POPUP);
	 */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(my_exit), NULL);
	gtk_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(my_exit), NULL);

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_widget_set_usize(window, win_width, win_height);

	frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(window), frame);

	main_box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), main_box);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 0);
	gtk_box_pack_start(GTK_BOX(main_box), hbox, FALSE, /* TRUE */ FALSE, 0);

	/* title window */
	title = gtk_drawing_area_new();
	gtk_widget_set_usize(title, win_width, 30);

	gtk_widget_set_events(title, GDK_EXPOSURE_MASK);
	gtk_signal_connect(GTK_OBJECT(title), "expose_event", GTK_SIGNAL_FUNC(expose_ev), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), title, FALSE, FALSE, 0);

	/* end title window */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(main_box), hbox, FALSE, TRUE, 0);

	/* channel window */
	frame = gtk_frame_new(NULL);
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);

	/*
	 * yxq modified at 2004.8.27 vbox1 = gtk_vbox_new(FALSE, 5);
	 * gtk_container_add(GTK_CONTAINER(frame), vbox1);
	 * ;
	 * modify at 2002.4.10
	 */
	full_win.w = WIN_W;
	full_win.h = WIN_H;
	w_full_area = gtk_drawing_area_new();
	gtk_widget_set_usize(w_full_area, full_win.w, full_win.h);
	gtk_widget_set_events(w_full_area, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect(GTK_OBJECT(w_full_area), "expose_event", GTK_SIGNAL_FUNC(expose_ev), NULL);
	gtk_signal_connect(GTK_OBJECT(w_full_area), "button_press_event", GTK_SIGNAL_FUNC(click2),
	                   GINT_TO_POINTER(-1));

	/*
	 * gtk_box_pack_start(GTK_BOX(vbox1), w_full_area, FALSE, FALSE, 0);
	 * ;
	 * yxq modified at 2004.8.27
	 */
	gtk_container_add(GTK_CONTAINER(frame), w_full_area);

	/*
	 * end channel window ;
	 * opearate window
	 */
	frame = gtk_frame_new(NULL);
	gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE /* FALSE */, TRUE /* FALSE */, 0);

	vbox1 = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);

	/* status information */
	frame = gtk_frame_new("Status Information");
	gtk_box_pack_start(GTK_BOX(vbox1), frame, TRUE, FALSE, 0);

	vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);

	for (i = 0; i < 3; i++)
	{
		hbox2 = gtk_hbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(vbox2), hbox2, FALSE, FALSE, 5);
		label = gtk_label_new(status_str[i]);
		gtk_box_pack_start(GTK_BOX(hbox2), label, FALSE, FALSE, 5);

		w_status_buffer[i] = gtk_text_buffer_new(NULL);
		w_status[i] = gtk_text_view_new_with_buffer(w_status_buffer[i]);

		gtk_widget_set_usize(w_status[i], 100, 25);
		if (i == 2)
			gtk_widget_set_usize(w_status[i], 60, 25);
		gtk_text_view_set_editable(GTK_TEXT_VIEW(w_status[i]), FALSE);
		gtk_box_pack_start(GTK_BOX(hbox2), w_status[i], FALSE, FALSE, 0);
	}

	/*
	 * //display ports frame = gtk_frame_new(NULL);
	 * gtk_box_pack_start(GTK_BOX (vbox1), frame, TRUE, FALSE, 0);
	 * vbox2 = gtk_vbox_new(FALSE, 0);
	 * gtk_container_set_border_width(GTK_CONTAINER(vbox2), 4);
	 * gtk_container_add(GTK_CONTAINER(frame), vbox2);
	 * ;
	 * frame information
	 */
	frame = gtk_frame_new("Channel Info");
	gtk_box_pack_start(GTK_BOX(vbox1), frame, /* FALSE */ TRUE, FALSE, 0);
	vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);

	hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE, FALSE, 2);
	label = gtk_label_new("Channel");
	gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, FALSE, 2);

	w_channel_buffer = gtk_text_buffer_new(NULL);
	w_channel = gtk_text_view_new_with_buffer(w_channel_buffer);

	gtk_widget_set_usize(w_channel, 95, 24);
	gtk_box_pack_start(GTK_BOX(hbox2), w_channel, FALSE, FALSE, 0);

	hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE, FALSE, 2);
	label = gtk_label_new("a_frame rate");
	gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, FALSE, 2);

	w_audio_framerate_buffer = gtk_text_buffer_new(NULL);
	w_audio_framerate = gtk_text_view_new_with_buffer(w_audio_framerate_buffer);

	gtk_widget_set_usize(w_audio_framerate, 95, 24);
	gtk_box_pack_start(GTK_BOX(hbox2), w_audio_framerate, FALSE, FALSE, 0);

	hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE, FALSE, 2);
	label = gtk_label_new("v_frame rate");
	gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, FALSE, 2);

	w_video_framerate_buffer = gtk_text_buffer_new(NULL);
	w_video_framerate = gtk_text_view_new_with_buffer(w_video_framerate_buffer);

	gtk_widget_set_usize(w_video_framerate, 95, 24);
	gtk_box_pack_start(GTK_BOX(hbox2), w_video_framerate, FALSE, FALSE, 0);

	hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE, FALSE, 2);
	label = gtk_label_new("Total frame");
	gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, FALSE, 2);

	w_totalframe_buffer = gtk_text_buffer_new(NULL);
	w_totalframe = gtk_text_view_new_with_buffer(w_totalframe_buffer);

	gtk_widget_set_usize(w_totalframe, 95, 24);
	gtk_box_pack_start(GTK_BOX(hbox2), w_totalframe, FALSE, FALSE, 0);

#if 0
	/* get bit rate */
	hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE, FALSE, 2);
	label = gtk_label_new("BitRate(bps)");
	gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, FALSE, 2);

	w_bit_rate_buffer = gtk_text_buffer_new(NULL);
	w_bit_rate = gtk_text_view_new_with_buffer(w_bit_rate_buffer);

	gtk_widget_set_usize(w_bit_rate, 95, 24);
	gtk_box_pack_start(GTK_BOX(hbox2), w_bit_rate, FALSE, FALSE, 0);

	/* set bit_rate(bps) */
	hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE, FALSE, 0);
	label = gtk_label_new("SetBitRate(bps)");
	gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, FALSE, 0);
	w_set_bit_rate = gtk_entry_new_with_max_length(10);
	gtk_widget_set_usize(w_set_bit_rate, 95, 24);
	gtk_entry_set_text(GTK_ENTRY(w_set_bit_rate), "0");
	gtk_box_pack_start(GTK_BOX(hbox2), w_set_bit_rate, TRUE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(w_set_bit_rate), "activate",
	                   GTK_SIGNAL_FUNC(set_bit_rate_callback), NULL);
#endif 
	hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox2, TRUE, FALSE, 2);
	label = gtk_label_new("Net User Count");
	gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, FALSE, 2);
	w_netusercount_buffer = gtk_text_buffer_new(NULL);
	w_netusercount = gtk_text_view_new_with_buffer(w_netusercount_buffer);

	gtk_widget_set_usize(w_netusercount, 95, 24);
	gtk_box_pack_start(GTK_BOX(hbox2), w_netusercount, FALSE, FALSE, 0);

    videostandard[0] = GetVideoStandard();

	/* set video info */
	frame = gtk_frame_new("Encode Para");
	gtk_box_pack_start(GTK_BOX(vbox1), frame, /* FALSE */ TRUE, FALSE, 0);

	vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);

    hbox2 = gtk_vbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox2), 5);
    gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE , FALSE, 0);
    gtk_widget_show (hbox2);

	hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE, FALSE, 0);
	label = gtk_label_new("SCALE    ");
	gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, FALSE, 0);
    glist = NULL;
    glist = g_list_append(glist,"0. Off");
    glist = g_list_append(glist,"1. 176x112");
    glist = g_list_append(glist,"2. 352x240");
    glist = g_list_append(glist,"3. 704x240");
    glist = g_list_append(glist,"4. 704x480");
    w_set_scale = gtk_combo_new();
    gtk_combo_set_popdown_strings(GTK_COMBO(w_set_scale),glist);
    gtk_box_pack_start(GTK_BOX(hbox2),w_set_scale,TRUE,TRUE, 0);
    gtk_signal_connect(GTK_OBJECT(GTK_COMBO(w_set_scale)->entry),"changed",GTK_SIGNAL_FUNC(ch_encodescale),
              gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry)));


    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE, FALSE, 0);
    label = gtk_label_new("  QP      ");
    gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, FALSE, 0);
    glist = NULL;
    for(i = 3 ; i < 32 ; i++)
        glist = g_list_append(glist,INT2STR_data[i]);

    w_set_qp = gtk_combo_new();
//    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo)->entry), "10");
    
    gtk_combo_set_popdown_strings(GTK_COMBO(w_set_qp),glist);
    gtk_box_pack_start(GTK_BOX(hbox2),w_set_qp,TRUE,TRUE, 0);
    gtk_signal_connect(GTK_OBJECT(GTK_COMBO(w_set_qp)->entry),"changed",GTK_SIGNAL_FUNC(ch_encodeqp),
              gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(w_set_qp)->entry)));

    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE, FALSE, 0);
    label = gtk_label_new("INTVL    ");
    gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, FALSE, 0);
    glist = NULL;
    for(i = 0 ; i < 512 ; i++)
        glist = g_list_append(glist,INT2STR_data[i]);
    
    w_set_intvl = gtk_combo_new();
    gtk_combo_set_popdown_strings(GTK_COMBO(w_set_intvl),glist);
    gtk_box_pack_start(GTK_BOX(hbox2),w_set_intvl,TRUE,TRUE, 0);
    gtk_signal_connect(GTK_OBJECT(GTK_COMBO(w_set_intvl)->entry),"changed",GTK_SIGNAL_FUNC(ch_encodeintvl),
              gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(w_set_intvl)->entry)));


    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE, FALSE, 0);
    label = gtk_label_new(" GOP     ");
    gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, FALSE, 0);
    glist = NULL;
    for(i = 1 ; i < 256 ; i++)
        glist = g_list_append(glist,INT2STR_data[i]);

    w_set_gop = gtk_combo_new();
    gtk_combo_set_popdown_strings(GTK_COMBO(w_set_gop),glist);
    gtk_box_pack_start(GTK_BOX(hbox2),w_set_gop,TRUE,TRUE, 0);
    gtk_signal_connect(GTK_OBJECT(GTK_COMBO(w_set_gop)->entry),"changed",GTK_SIGNAL_FUNC(ch_encodegop),
              gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(w_set_gop)->entry)));

    if(v_info[selectport].Scale == 0)
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "0. Off");
    else if(v_info[selectport].Scale == 5)
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "1. 176x112");
    else if(v_info[selectport].Scale == 2)
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "2. 352x240");
    else if(v_info[selectport].Scale == 1)
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "3. 704x240");
    else if(v_info[selectport].Scale == 9)
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "4. 704x480");

    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_qp)->entry), INT2STR_data[(v_info[selectport].Qp)]);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_intvl)->entry), INT2STR_data[(v_info[selectport].Intvl)]);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_gop)->entry), INT2STR_data[(v_info[selectport].Gop)]);    

	frame = gtk_frame_new("WaterMark");
	gtk_box_pack_start(GTK_BOX(vbox1), frame, /* FALSE */ TRUE, FALSE, 0);

	vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);
    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE, FALSE, 0);
    label = gtk_label_new(" POLY     ");
    gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, FALSE, 0);
	w_poly = gtk_entry_new_with_max_length(NULL);
	gtk_entry_set_text(GTK_ENTRY(w_poly), "0");
    gtk_box_pack_start(GTK_BOX(hbox2),w_poly,TRUE,TRUE, 0);
	gtk_signal_connect(GTK_OBJECT(w_poly), "activate",
	                   GTK_SIGNAL_FUNC(set_wm_callback), NULL);

    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE, FALSE, 0);
    label = gtk_label_new(" KEY     ");
    gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, FALSE, 0);
    w_key  = gtk_entry_new_with_max_length(NULL);
	gtk_entry_set_text(GTK_ENTRY(w_key), "0");
    gtk_box_pack_start(GTK_BOX(hbox2),w_key,TRUE,TRUE, 0);
	gtk_signal_connect(GTK_OBJECT(w_key), "activate",
	                   GTK_SIGNAL_FUNC(set_wm_callback), NULL);

    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE, FALSE, 0);
    label = gtk_label_new(" STR     ");
    gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, FALSE, 0);
    w_str  = gtk_entry_new_with_max_length(NULL);
	gtk_entry_set_text(GTK_ENTRY(w_str), "0");
    gtk_box_pack_start(GTK_BOX(hbox2),w_str,TRUE,TRUE, 0);
	gtk_signal_connect(GTK_OBJECT(w_str), "activate",
	                   GTK_SIGNAL_FUNC(set_wm_callback), NULL);
	/*
	 * end video info ;
	 * capture ;
	 * frame = gtk_frame_new("Â¼Ïñ");
	 */
	frame = gtk_frame_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox1), frame, TRUE /* FALSE */, /* TRUE */ FALSE, 0);

	vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);

	hbox1 = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox1), 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox1, TRUE /* FALSE */, /* TRUE */ FALSE, 5);

	w_record_flag = gtk_check_button_new_with_label("Record");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_record_flag), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox1), w_record_flag, TRUE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(w_record_flag), "clicked", GTK_SIGNAL_FUNC(record_callback), NULL);

	/* motion detect */
	w_motion_detect_flag = gtk_check_button_new_with_label("MotionDetect");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_motion_detect_flag), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox1), w_motion_detect_flag, TRUE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(w_motion_detect_flag), "clicked",
	                   GTK_SIGNAL_FUNC(motion_detect_callback), NULL);

	hbox1 = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox1), 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox1, TRUE /* FALSE */, /* TRUE */ FALSE, 5);

	/* audio preview */
	w_audio_preview = gtk_check_button_new_with_label("AudioPreview");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_audio_preview), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox1), w_audio_preview, TRUE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(w_audio_preview), "clicked",
	                   GTK_SIGNAL_FUNC(audio_preview_callback), NULL);

	/* net transfer */
	w_net_transfer = gtk_check_button_new_with_label("NetTransfer");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_net_transfer), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox1), w_net_transfer, TRUE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(w_net_transfer), "clicked",
	                   GTK_SIGNAL_FUNC(net_transfer_callback), NULL);

	hbox1 = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox1), 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox1, TRUE /* FALSE */, /* TRUE */ FALSE, 5);

	/* preview */
	w_preview = gtk_check_button_new_with_label("Preview");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_preview), TRUE);                       /* active preview */

	/*
	 * gtk_box_pack_start(GTK_BOX(vbox2), w_preview, TRUE, FALSE, 0);
	 */
	gtk_box_pack_start(GTK_BOX(hbox1), w_preview, TRUE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(w_preview), "clicked", GTK_SIGNAL_FUNC(preview_callback), NULL);

	/* string to client : test add at 2004.8 */
	w_stringtoclient = gtk_check_button_new_with_label("StringToClient");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_stringtoclient), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox1), w_stringtoclient, TRUE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(w_stringtoclient), "clicked",
	                   GTK_SIGNAL_FUNC(stringtoclient_callback), NULL);

	/* get original image */
	vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 0);
	gtk_box_pack_start(GTK_BOX(vbox1), vbox2, FALSE, FALSE, 0);
	w_image = gtk_button_new_with_label("Get Image");
	gtk_widget_set_usize(w_image, 120, 25);
	gtk_signal_connect(GTK_OBJECT(w_image), "clicked", GTK_SIGNAL_FUNC(process_buttons),
	                   GINT_TO_POINTER(3));
	gtk_box_pack_start(GTK_BOX(vbox2), w_image, TRUE, FALSE, 10);

	/* Capture a I frame */
	w_I_frame = gtk_button_new_with_label("Capture I Frame");
	gtk_signal_connect(GTK_OBJECT(w_image), "clicked", GTK_SIGNAL_FUNC(process_buttons),
	                   GINT_TO_POINTER(4));
	gtk_widget_set_usize(w_I_frame, 120, 25);
	gtk_box_pack_start(GTK_BOX(vbox2), w_I_frame, FALSE, FALSE, 0);

	/* exit */
	hbox1 = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox1), 0);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox1, FALSE, FALSE, 5);
	w_buttons[2] = gtk_button_new_with_label(button_str[2]);
	gtk_signal_connect(GTK_OBJECT(w_buttons[2]), "clicked", GTK_SIGNAL_FUNC(process_buttons),
	                   GINT_TO_POINTER(2));
	gtk_box_pack_start(GTK_BOX(hbox1), w_buttons[2], FALSE, /* TRUE */ FALSE, 0);

	/* initial datas */
	memset(old_frames, 0, sizeof(old_frames));
	memset(current_frames, 0, sizeof(current_frames));
	memset(bit_rate_time, 0, sizeof(bit_rate_time));
	memset(bit_rate_size, 0, sizeof(bit_rate_size));
	memset(net_head_flag, 0, sizeof(net_head_flag));
	memset(subchan_net_head_flag, 0, sizeof(subchan_net_head_flag));

	/* setup timer */
	count_timer = g_timer_new();

	processtimer = gtk_timeout_add(1000 / 5, (GtkFunction) process_time, NULL);             /* 1/5s */
	looptimer = gtk_timeout_add(1000 * timeconst, (GtkFunction) process_time_loop, NULL);   /* timeconst/60 minutes */
#ifdef USE_PREVIEW_TIMER
	/* set the preview flush timer */
	previewtimer = gtk_timeout_add(40, (GtkFunction) preview_loop, NULL);                   /* 40 millisecond */
#endif
#ifdef TEST_GETORIIMAGE
	/* set the get original image timer */
	imagetimer = gtk_timeout_add(4 * 1000, (GtkFunction) getimage_loop, NULL);              /* 4 second */
#endif
	gtk_widget_show_all(window);

	/* the sdl hack must set after the show */
	sprintf(SDL_windowhack, "SDL_WINDOWID=%ld", GDK_WINDOW_XWINDOW(w_full_area->window));
	putenv(SDL_windowhack);

	/*
	 * inital the sdl ;
	 * XInitThreads();
	 */
	putenv("SDL_VIDEO_YUV_HWACCEL=0");

	fprintf(stderr, "Init the sdl...");
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "<sdk_error> init sdl failed!%s\n", SDL_GetError());
		//return -1;
	}

	fprintf(stderr, "ok!\n");

	pOverlayScreen = SDL_SetVideoMode(WIN_W, WIN_H, 0, SDL_HWSURFACE);
	if (pOverlayScreen == NULL)
	{
		fprintf(stderr, "<sdk_error> create the sdl screen failed! for %s!\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	/* SDL init finished */
	for (i = 0; i < MAX_PORT; i++)
		count[i] = 0;



#ifdef RAWSTREAM
	{
		char    filename[64];
		RegisterImageStreamCallback((IMAGE_STREAM_CALLBACK) ImageStreamCallback, NULL);
		sprintf(filename, "imagefile.yuv");
		imagefile_fd = open(filename, O_WRONLY | O_CREAT, 0777);
	}
#endif

	/* set QCIF and 2CIF or CIFQCIF */
	memset(qcif_flag, 0, sizeof(qcif_flag));
	memset(hcif_flag, 0, sizeof(hcif_flag));

	/*
	 * memset(cifqcif_flag, 1, sizeof(cifqcif_flag));
	 */
	memset(D1_flag, 0, sizeof(D1_flag));
	memset(dcif_flag, 0, sizeof(dcif_flag));

	/*
	 * qcif_flag[0] = 1;
	 * dcif_flag[1] = 1;
	 * hcif_flag[2] = 1;
	 * D1_flag[0] = 1;
	 * D1_flag[1] = 1;
	 * dcif_flag[0] = 1;
	 * qcif_flag[1] = 1;
	 * cifqcif_flag[0] = 1;
	 */
	for (i = 0; i < totalport; i++)
	{
		SetupBitrateControl(ChannelHandle[i], 768000);

		/*
		 * SetupBitrateControl(ChannelHandle[i],1500000 );
		 */
		if (qcif_flag[i])
		{
			SetEncoderPictureFormat(ChannelHandle[i], ENC_QCIF_FORMAT);
		}
		else if (hcif_flag[i])
		{
			SetEncoderPictureFormat(ChannelHandle[i], ENC_2CIF_FORMAT);
		}
		else if (cifqcif_flag[i])
		{
			/*
			 * SetEncoderPictureFormat(ChannelHandle[i], ENC_CIFQCIF_FORMAT);
			 */
			SetEncoderPictureFormat(ChannelHandle[i], ENC_4CIF_FORMAT);
			SetSubEncoderPictureFormat(ChannelHandle[i], ENC_QCIF_FORMAT);
			SetupSubChannel(ChannelHandle[i], 1);
			SetIBPMode(ChannelHandle[i], 25, 0, 0, 15);
			SetupSubChannel(ChannelHandle[i], 0);
		}
		else if (D1_flag[i])
		{
			SetEncoderPictureFormat(ChannelHandle[i], ENC_4CIF_FORMAT);
		}
		else if (dcif_flag[i])
		{
			SetEncoderPictureFormat(ChannelHandle[i], ENC_DCIF_FORMAT);
		}
		else
		{
			SetEncoderPictureFormat(ChannelHandle[i], ENC_CIF_FORMAT);
			SetIBPMode(ChannelHandle[i], 25, 2, 0, 25);
			SetDefaultQuant(ChannelHandle[i], 15, 15, 20);
		}
	}

	/* get cardno */
	{
		unsigned int    boardtype;
		char            serialno[20], serialno_str[20];

		for (i = 0; i < totalport; i++)
		{
			GetBoardInfo(ChannelHandle[i], &boardtype, serialno);
			for (j = 0; j < 12; j++)
			{
				serialno_str[j] = serialno[j] + 0x30;
			}

			serialno_str[j] = '\0';
//			printf("card[%d]'s serialno = %s\n", i, serialno_str);
		}
	}

#if 1
	/*
	 * get coordinate of video output window ;
	 * and start preview
	 */
	gdk_window_get_deskrelative_origin(w_full_area->window, &full_win.x, &full_win.y);
	full_rect.x = 0;    /* full_win.x;
                         * */
	full_rect.y = 0;    /* full_win.y;
                         * */
	full_rect.w = full_win.w;
	full_rect.h = full_win.h;

	printf("full window at (%d, %d), w = %d, h = %d\n", full_win.x, full_win.y, full_win.w,
	       full_win.h);

	int chan = 1;
	if (totalport <= 4)
		chan = 2;
	else
	{
		if (totalport <= 9)
			chan = 3;
		else
		{
			if (totalport <= 16)
				chan = 4;
			else
			{
				if (totalport <= 25)
					chan = 5;
				else
					chan = 6;
			}
		}
	}

	memset(bPreviewOpen, 0, sizeof(bPreviewOpen));
	memset(previewconf, 0, sizeof(previewconf));
	memset(bPreviewThreadRun, 0, sizeof(bPreviewThreadRun));

	/* yxq test the preivew */
	for (i = 0; i < totalport; i++)
	{
#if 1
		v_win[i].w = full_win.w / chan;
		v_win[i].h = full_win.h / chan;
		dstRect[i].x = (i % chan) * v_win[i].w;
		dstRect[i].y = (i / chan) * v_win[i].h;
		dstRect[i].w = v_win[i].w;
		dstRect[i].h = v_win[i].h;
#else
		/* test the different preview window at 2005.6.25 */
		chan = 3;
		v_win[i].w = full_win.w / chan;
		v_win[i].h = full_win.h / chan;
		{
			int w, h;
			w = full_win.w / chan;
			h = full_win.h / chan;
			if (i == 0)
			{
				dstRect[i].x = (i % chan) * w;
				dstRect[i].y = (i / chan) * h;
				dstRect[i].w = w * 3;
				dstRect[i].h = h * 2;
			}
			else
			{
				dstRect[i].x = ((i + 5) % chan) * w;
				dstRect[i].y = ((i + 5) / chan) * h;
				dstRect[i].w = w;
				dstRect[i].h = h;
			}

			fprintf(stderr, "<demo_info> the chan%d dstrect x:%d y:%d, w:%d, h:%d!\n", i,
			        dstRect[i].x, dstRect[i].y, dstRect[i].w, dstRect[i].h);
		}
#endif
#ifdef PREVIEWOPEN
		{
			pthread_attr_t  attr;

			previewconf[i].w = dstRect[i].w;
			previewconf[i].h = dstRect[i].h;
            if(previewconf[i].w < 320)
                previewconf[i].w = 320;
            if(previewconf[i].h < 240)
                previewconf[i].h = 240;
            
			sem_init(&previewSem[i], 0, 0);
			previewconf[i].SyncSem = &previewSem[i];
            previewconf[i].w = 176;
            if(videostandard[0]==1)
                previewconf[i].h = 144;
            else if(videostandard[0]==0)
                previewconf[i].h = 120;

			if (StartVideoPreview(ChannelHandle[i], &previewconf[i], useSyncSem) < 0)
				fprintf(stderr, "<demo_info>start video preview(port %d) failed\n", i);
//			fprintf(stderr, "<demo_info> Start video preview ok, the sync sem is 0x%x!\n",
//			        previewconf[i].SyncSem);
			pOverlaySurface[i] = SDL_CreateYUVOverlay(previewconf[i].w, previewconf[i].h,
			                     SDL_UYVY_OVERLAY, pOverlayScreen);
			if (pOverlaySurface[i] == NULL)
			{
				fprintf(stderr, "the SDL overlay surface create failed!\n");
				return 0;
			}

			bPreviewOpen[i] = 1;
			bPreviewThreadRun[i] = 1;
#ifndef USE_PREVIEW_TIMER
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			pthread_create(&hPreviewThread[i], &attr, videopreview, &i);
			while (bPreviewThreadRun[i] == 1)
				usleep(100);
#endif
		}
#endif

		/*
		 * printf("port[%d]: (%d,%d), (%d,%d), full_win.w = %d, full_win.h = %d, chan
		 * %d\n",i, dstRect[i].x, dstRect[i].y,dstRect[i].w,dstRect[i].h,full_win.w,full_win.h,chan);
		 * ;
		 * for motion detect
		 */
		motion_list[i].RectLeft = (i % chan) * v_win[i].w;
		motion_list[i].RectTop = (i / chan) * v_win[i].h;
		motion_list[i].RectRight = (i % chan + 1) * v_win[i].w;
		motion_list[i].RectBottom = (i / chan + 1) * v_win[i].h;
		motion_rect[i].RectLeft = 0;
		motion_rect[i].RectTop = 0;
		motion_rect[i].RectRight = 704 - 1;
		motion_rect[i].RectBottom = video_height[i] - 1;
		if (SetupMotionDetection(ChannelHandle[i], &motion_rect[i], 1) < 0)
			printf("setup motion detect(port %d) failed\n", i);
	}

	g_bPreviewStart = 1;

#if 1
    {
        pthread_t thread[4];
        for(i=0;i<totalcard;i++)
        {
            pthread_attr_t  attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            pthread_create(&thread[i], &attr, videocap, &i);
            usleep(1000);
        }
    }
#endif
    
#if 1
    {
        pthread_t thread[4];
        for(i=0;i<totalcard;i++)
        {
            pthread_attr_t  attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            pthread_create(&thread[i], &attr, videorecord, &i);
            usleep(1000);
        }
    }
#endif

#if 1
    if(argc > 1)
    {
        pthread_t thread[4];
        dec_in_fd = open(argv[argc-1], O_RDONLY);
        
        for(i=0;i<totalcard;i++)
        {
            pthread_attr_t  attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            pthread_create(&thread[i], &attr, videodecode, &i);
            usleep(1000);
        }
    }
#endif

	/* get time */
	now = time(NULL);
	tm = localtime(&now);
	strftime(timebuf, sizeof(timebuf), "%Y-%m-%d", tm);

	sprintf(starttime, "%s  ", timebuf);
	gtk_text_buffer_set_text(w_status_buffer[0], timebuf, strlen(timebuf));
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(w_status[0]), w_status_buffer[0]);

	/*
	 * gtk_text_insert(GTK_TEXT_VIEW(w_status[0]), NULL, NULL, NULL, timebuf, -1);
	 */
	strftime(timebuf, sizeof(timebuf), "%T", tm);
	strcat(starttime, timebuf);
	gtk_text_buffer_set_text(w_status_buffer[1], timebuf, strlen(timebuf));
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(w_status[1]), w_status_buffer[1]);

	/*
	 * gtk_text_insert(GTK_TEXT_VIEW(w_status[1]), NULL, NULL, NULL, timebuf, -1);
	 */
#endif
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	return 0;
}

/*
 *
 */
void ImageStreamCallback(unsigned int channelNumber, void* context)
{
	/*
	 * fprintf(stderr,"Enter the image stream call back for chan %d!\n", channelNumber);
	 */
	if (imagefile_fd)
		write(imagefile_fd, imageBuf, 704 * 576 * 3 / 2);
}

#ifdef PREVIEWOPEN

/*
 * timer for preview
 */
int preview_loop(gpointer data)
{
	UINT            i;
	static clock_t  old_clock = 0;

	/*
	 * fprintf(stderr,"<demo_info> enter the preview loop, the time interval is
	 * %ims!\n", (times(NULL)-old_clock)*10);
	 */
	old_clock = times(NULL);
	if (!g_bPreviewStart)
		return TRUE;    /* must be ture for loop auto */
#if 1
	for (i = 0; i < totalport; i++)
	{
		if (bPreviewOpen[i])
		{
			SDL_LockYUVOverlay(pOverlaySurface[i]);

			/*
			 * memset(*(pOverlaySurface[i]->pixels), 0, previewconf[i].imageSize[i]);
			 */
			memcpy(*(pOverlaySurface[i]->pixels), previewconf[i].dataAddr, previewconf[i].imageSize);
			SDL_UnlockYUVOverlay(pOverlaySurface[i]);
			SDL_DisplayYUVOverlay(pOverlaySurface[i], &dstRect[i]);
		}
	}
#endif
	return TRUE;
}

/*
 *
 */
void* videopreview(void* arg)
{
	UINT chan = *(UINT*)arg;
	while (bPreviewThreadRun[chan])
	{
		bPreviewThreadRun[chan] = 2;
		usleep(100);
		if (!bPreviewOpen[chan])
			continue;
		if (sem_wait(previewconf[chan].SyncSem) == 0)
		{
			gdk_threads_enter();
			SDL_LockYUVOverlay(pOverlaySurface[chan]);
			memcpy(*(pOverlaySurface[chan]->pixels), previewconf[chan].dataAddr,
			       previewconf[chan].imageSize);
			SDL_UnlockYUVOverlay(pOverlaySurface[chan]);
			SDL_DisplayYUVOverlay(pOverlaySurface[chan], &dstRect[chan]);
			gdk_threads_leave();
		}
		else
		{
			usleep(100);
		}
	}

	pthread_exit(NULL);
}
#endif
int save_yuv (char *buf, int x, int y, int ch)
{
    FILE *fp;
    char bewf[128];
    sprintf (bewf, "image-%d.yuv", ch);
    if ((fp = fopen (bewf, "a+")) == NULL)
    {
        exit(1);
    }
    
    fwrite ((unsigned char *) buf, x * y * 3/2, 1, fp);
    fclose (fp);
}

void* videocap(void* arg)
{
	UINT card = *(UINT*)arg;
    int en;
    int i, j;

    
    unsigned char _rawdata[1024*576*2+64];
    unsigned char *rawdata = (unsigned char *)(((unsigned long)_rawdata + 63) & (~63));
    unsigned char tmpdata[176*144*2];
    struct timeb timebuffer;    
    int tv_usec_old;
    int sleep_time;

    nice(-2);
    while(1)
    {
        CaptureFrames(card ,en , &(rawdata[0]));

        for(i=0;i<totalport; i++)
        {
            if(pOverlaySurface[i+4*card])
            {
                int total = previewconf[i+4*card].w*previewconf[i+4*card].h;
                int maxj = 0;
                gdk_threads_enter();
                SDL_LockYUVOverlay(pOverlaySurface[i+4*card]);
                
                if(videostandard[0]==1)
                    maxj = 144;
                else if(videostandard[0]==0)
                    maxj = 120;
                
                for(j=0; j<maxj; j++)
                {
                    memcpy( (unsigned char *)&(tmpdata[j*176*2]),
                            (unsigned char *)&(rawdata[((i/4)*maxj+j)*1024*2+(i%4)*176*2]),
                            176*2);
                }

                memcpy(*(pOverlaySurface[i+4*card]->pixels), (unsigned char *)&(tmpdata[0]), 176*maxj*2);                

                SDL_UnlockYUVOverlay(pOverlaySurface[i+4*card]);
                SDL_DisplayYUVOverlay(pOverlaySurface[i+4*card], &dstRect[i+4*card]);
                gdk_threads_leave();
            }
        }
#if 1        
        {
            int maxtime;
            if(videostandard[0]==1)
                maxtime = 40000;
            else if(videostandard[0]==0)
                maxtime = 33333;
            ftime( &timebuffer );
            sleep_time = ((1000*timebuffer.millitm - tv_usec_old)%1000)* 1000;
            if(sleep_time > maxtime)
                sleep_time = 0;
            else
                sleep_time = maxtime - sleep_time;
            
            tv_usec_old = 1000*timebuffer.millitm;
        }
#endif        
        usleep(1000);
    }
	pthread_exit(NULL);
}

void* videorecord(void* arg)
{
	UINT card = *(UINT*)arg;

    int port = 0;
    int read_size;
    int ret_val;
    unsigned char tmp;
    struct timeb timebuffer;    
    int enc_fd = open("/dev/solo6010_enc0", O_RDWR);
    unsigned int total_size = 0;
    int maxsize=1<<20;
    unsigned char *_buf;
    unsigned char *buf;
    _buf = malloc(maxsize + 16);
    if(!_buf)
    {
        printf("malloc fail\n");
        exit(EXIT_FAILURE);
    }
    buf = (unsigned char *)(((unsigned long)_buf + 15) & (~15));

    nice(-2);
    while(1)
    {
        if( record_flag[0] == 1)
        {
            if(record_fd[0] <= 0)
            {
                char nametime[100];
                time_t timenow;
                timenow = time(NULL);
                strftime(nametime, sizeof(nametime), "%Y%m%d%H%M%S", localtime(&timenow));
                sprintf(filename[0], "%s/test%s.mp4", savepath, nametime);
            
                if ((record_fd[0] = open(filename[0], O_WRONLY | O_CREAT, 0777)) < 0)
                    perror(">>>open");
                else
                {
                    ftruncate(record_fd[0], 0);
                }
                
                for (port = 0; port < totalport; port++)
                {
                    SetEncodePara(ChannelHandle[port], 
                                  v_info[port].Scale,
                                  v_info[port].Qp,
                                  v_info[port].Intvl,
                                  v_info[port].Gop,
                                  15);
//                    StartVideoCapture(ChannelHandle[port]);
/*                    GetEncodePara(ChannelHandle[selectport],
                                  &videostandard[selectport],
                                  &v_info[selectport].Scale,
                                  &v_info[selectport].Qp,
                                  &v_info[selectport].Intvl,
                                  &v_info[selectport].Gop);
     */               
                    if(v_info[selectport].Scale == 0)
                        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "0. Off");
                    else if(v_info[selectport].Scale == 5)
                        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "1. 176x112");
                    else if(v_info[selectport].Scale == 2)
                        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "2. 352x240");
                    else if(v_info[selectport].Scale == 1)
                        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "3. 704x240");
                    else if(v_info[selectport].Scale == 9)
                        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "4. 704x480");
                    
                    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_qp)->entry), INT2STR_data[(v_info[selectport].Qp)]);
                    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_intvl)->entry), INT2STR_data[(v_info[selectport].Intvl)]);
                    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_gop)->entry), INT2STR_data[(v_info[selectport].Gop)]);    
                    
                }   
            }
            
            read_size = read(enc_fd, buf, maxsize);
            if(read_size == 0)
                continue;
            if(read_size < 0)
                perror(">>>read");

            if(buf[36] >= 16)
            {
                printf(stderr, " channel error 1 \n" );
                continue;
            }
            if(buf[36]!=(buf[3]&0xf))
            {
                printf(stderr, " channel error 2 \n" );            
                continue;
            }
            
            tmp = (unsigned char)(0xff & (read_size>>0));
            write(record_fd[0], &tmp, 1);
            tmp = (unsigned char)(0xff & (read_size>>8));
            write(record_fd[0], &tmp, 1);
            tmp = (unsigned char)(0xff & (read_size>>16));
            write(record_fd[0], &tmp, 1);
            tmp = (unsigned char)(0xff & (read_size>>24));
            write(record_fd[0], &tmp, 1);
            
            ret_val = write(record_fd[0], buf, read_size);
            if(ret_val != read_size)
            {
                printf("write(%d, %d)\n", ret_val, read_size);
                return 0;
            }

            total_size += ret_val;
            if(total_size > (1<<30))
                return 0;
        }
        else
        {
            if(record_fd[0] > 0)
            {
                end_record(0);

                for (port = 0; port < totalport; port++)
                {
                    if (StopVideoCapture(ChannelHandle[port]) < 0)
                        printf("record_callbacl: StopvideoCapture(port %d) failed\n", port);

                    GetEncodePara(ChannelHandle[port],
                                  &videostandard[port],
                                  &v_info[port].Scale,
                                  &v_info[port].Qp,
                                  &v_info[port].Intvl,
                                  &v_info[port].Gop);
                }   
                if(v_info[selectport].Scale == 0)
                    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "0. Off");
                else if(v_info[selectport].Scale == 5)
                    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "1. 176x112");
                else if(v_info[selectport].Scale == 2)
                    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "2. 352x240");
                else if(v_info[selectport].Scale == 1)
                    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "3. 704x240");
                else if(v_info[selectport].Scale == 9)
                    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_scale)->entry), "4. 704x480");
                
                gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_qp)->entry), INT2STR_data[(v_info[selectport].Qp)]);
                gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_intvl)->entry), INT2STR_data[(v_info[selectport].Intvl)]);
                gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(w_set_gop)->entry), INT2STR_data[(v_info[selectport].Gop)]);    
                
            }
        }
        
        usleep(1000);
    }
	pthread_exit(NULL);
}

void* videodecode(void* arg)
{
    unsigned char *buf;
    unsigned int header_size = 64;
    unsigned int dec_channel = 0xffff;
    unsigned int wait_i_frame = 0xffff;

    if(dec_in_fd == -1)
        return -1;

    buf = malloc(1<<20);
    if(buf == NULL)
        return -1;

    buf = (unsigned char *)(((unsigned long)buf + 0x0f) & (~0xf));

    while(1)
    {
        unsigned char dec_index[64];
        unsigned char *dec_buf;
        int read_size;
        unsigned int mpeg4_size;
        off_t current_offset;
        unsigned char tmp;
        int channel;

        current_offset = lseek(dec_in_fd, 0, SEEK_CUR);

        read(dec_in_fd, &tmp, 1);
        read_size = ((int)tmp)<<0;
        read(dec_in_fd, &tmp, 1);
        read_size |= ((int)tmp)<<8;
        read(dec_in_fd, &tmp, 1);
        read_size |= ((int)tmp)<<16;
        read(dec_in_fd, &tmp, 1);
        read_size |= ((int)tmp)<<24;
        
        if((read_size > (1<<20)) || (read_size <= header_size))
        {
            printf("read_size : %d. Playback Finish.\n", read_size);
            exit(-1);
        }

        if(read_size != read(dec_in_fd, buf, read_size))
        {
            printf("read_size : %d\n", read_size);
            return -1;
        }

        mpeg4_size = read_size - header_size;

        channel = buf[36];

        if(channel > 16)
        {
            printf("channel : %d\n", channel);
            return -1;
        }

        if(!(dec_channel & (1<<channel)))
        {
            continue;
        }

        if(wait_i_frame & (1<<channel))
        {
            if(buf[2] & 0xc0)
                continue;
            else
                wait_i_frame &= ~(1<<channel);
        }

        memset(dec_index, 0, 64);
        dec_index[0] = buf[0];
        dec_index[1] = buf[1];
        dec_index[2] = buf[2] & 0xcf;
        dec_index[3] = (buf[3] & 0xf0) | channel;
        dec_index[4] = buf[4];
        dec_index[5] = buf[5];
        dec_index[7] = (buf[7] & 0xf0) | channel;

        // time
        dec_index[20] = buf[20];
        dec_index[21] = buf[21];
        dec_index[22] = buf[22];
        dec_index[23] = buf[23];
        dec_index[24] = buf[24];
        dec_index[25] = buf[25];
        dec_index[26] = buf[26];
        dec_index[27] = buf[27];

        dec_buf = buf;

        memcpy(dec_buf, dec_index, 64);
        VideoDecode(dec_buf, 64 + mpeg4_size);
        usleep(1000);
        
    }
	pthread_exit(NULL);
}

