
/*** 
NOTE: This file requires modifications in order to be usable. The extractor does not have semaphores anymore. The 
only way to implement the extractor application correctly is to use polling, which increases CPU usage and drops 
packets when multiple channels are extracted. At some point, the application should be modified to have semaphores
for proper synchronisation b/w producer(extractor_thread) and consumer(sender_thread). The semaphore granularity for 
extractor seems to be coarse and inefficient. Alternative mechanism need to be implemented for better granularity. 
This may include a callback function or a separate producer stepm followed by put_full step. This will remove 
extractor from the critical section. The "put_full" ( with small run time ) will be the critical section. 
***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "Windows.h"
#include "stRtpIO.h"
extern "C" {
#include "svc_extractor.h"
#include "pthread.h"
#include "cfg.h"
#include "semaphore.h"
}

/*** 
To Play all 9 layers, use 
 -c avc_s2_t2.cfg avc_s2_t1.cfg avc_s2_t0.cfg avc_s1_t2.cfg avc_s1_t1.cfg avc_s1_t0.cfg avc_s0_t2.cfg avc_s0_t1.cfg avc_s0_t0.cfg 
 ***/
#define PROFILE_EXTRACTOR

#define NUM_OUTPUT_BUFFERS		4
#define FORCE_DUMMY_FRAMES		1

#define MAX_FRAME_WIDTH			1280
#define MAX_FRAME_HT			720
#define MAX_FRAME_SIZE          ((MAX_FRAME_WIDTH) * (MAX_FRAME_HT))
#define MAX_SLICE_NAL_SIZE      ((MAX_FRAME_SIZE) * 3/2)

/* For RTP rcv channel */
#define NUM_RCV_BUF                             128
#define RCV_BUF_SIZE                            MAX_SLICE_NAL_SIZE

/* For RTP snd channel */
#define NUM_SND_BUF								80
#define SND_BUF_SIZE							MAX_SLICE_NAL_SIZE

typedef unsigned char   sx_uint8;
typedef char            sx_int8;
typedef unsigned short  sx_uint16;
typedef short           sx_int16;
typedef unsigned long   sx_uint32;
typedef long            sx_int32;
typedef int             sx_bool;

#define False           0
#define True            1

typedef struct {
	stRtpIOSndHandle_t		snd_chan;
	FILE					*fout;
	int						spatial_id;
	int						temporal_id;
	int						avc;
	int						channel_count;
	unsigned int			write_output_to_file;
	int						cnt;
	st_svc_channel_t		channel_handle;
	sem_t					thread_sync;
} snd_context_t;

typedef struct {
	stRtpIORcvHandle_t		rcv_chan;
	st_svc_extractor_t		extractor_handle;
	sem_t					thread_sync;
} rcv_context_t;

typedef struct local_context_tag {
	FILE *fin;
	st_svc_channel_t *channel_list;
	unsigned int num_channels;
	unsigned int write_output_to_file;
	char src_ip[256];
	int src_port;

	char **destination;
	int *dst_port;

	stRtpIORcvConfig_t		rcv_cfg;
	stRtpIORcvHandle_t		rcv_chan; 
	pthread_t				rcv_thread;
	pthread_t				*snd_thread;

	stRtpIOSndConfig_t		snd_cfg;
	st_svc_channel_config_t		*channel_config;
} local_context_t;


static void fast_memcpy(void *dst, void *src, int size)
{
	unsigned long *dst_long = (unsigned long *)dst;
	unsigned long *src_long = (unsigned long *)src;

	if ((((int)dst & 0x3) == 0) && (((int)src & 0x3) == 0)) {
		while (size >= 4) {
			*dst_long++ = *src_long++;
			size -= 4;
		}

		memcpy((void *)dst_long, (void *)src_long, size);
	} else
		memcpy((void *)dst_long, (void *)src_long, size);
}

/**** Local Function Prototype ****/
static void svc_light_extractor_test(int argc, char *argv[] );									 
static int parse_config( int argc, char *argv[], local_context_t *local_context );
static void print_help(char *argv[]);
static st_svc_extractor_buffer_t *svcex_buffer_alloc( size_t num_bytes_to_alloc );
static void create_output_buffers ( local_context_t *local_context, st_svc_extractor_t	extractor_handle );
static void get_output_full ( local_context_t *local_context, st_svc_extractor_t	extractor_handle );
static void *extractor_thread_func(void *arg);
static void strip_address( char *ip_port, int *port);
static void create_extractor_thread( local_context_t *local_context, st_svc_extractor_t extractor_handle );
static void generate_sdp(char *sdpStr, char *ipAddr, int port);
static void create_sender_thread(	local_context_t *local_context, st_svc_channel_t channel_handle,
									unsigned int channel_count);
static void *sender_thread_func(void *arg);
static void close_all( local_context_t *p_local_context );

static void print_help(char *argv[])
{
	printf("Usage: %s <src_ip_addr:port> [-f] <arguments> \n", argv[0]);
	printf("    [-f] - write bitstreams to local file \n");
    printf("    <arguments> \n");
    printf("        -c  <cfg_file> : Configuration file(s)\n");     
};

static void strip_address( char *ip_port, int *port)
{
	int k = 0;
	while (ip_port[k] != ':' && ip_port[k] != '\0') {
		k++;
	}
	if( ip_port[k] == '\0') {
		assert(0);
	}

	ip_port[k] = '\0'; // Terminate the string
	*port = atoi((char *)&ip_port[k+1]);
}

static int parse_config( int argc, char *argv[], local_context_t *local_context )
{
	int remain_argc, i;
	int switch_found = 0;
	unsigned int channel_count = 0; 

#define NUM_BASIC_CFG_ELEMENTS 4
	int channel_elements[NUM_BASIC_CFG_ELEMENTS] = { 0, 0, 0 };
	char destination[256]= "222.222.22.22:8000";
	char temp_destination[256]= "222.222.22.22:8000";
	cfg_entry_t cfg_entries[NUM_BASIC_CFG_ELEMENTS] = {
		{"avc",							CFG_TYPE_INT32, &channel_elements[0],      0,		1 }, 
		{"desired_spatial_layer",       CFG_TYPE_INT32, &channel_elements[1],      0,      255}, 
		{"desired_temporal_layer",      CFG_TYPE_INT32, &channel_elements[2],      0,      255},
		{"destination",					CFG_TYPE_STR,	&destination,      0,      0},
	};


	remain_argc = argc - 1;
    if (argc < 2) {
        print_help(argv);
        return -1;
    } else {
		unsigned int channel_num; 

		memcpy(&local_context->src_ip, argv[1], strlen( argv[1]) + 1);
		printf("RTP source IP %s \n", local_context->src_ip);
		strip_address( (char * )local_context->src_ip, &local_context->src_port );

		if (argv[2][0] == '-' && argv[2][1] == 'f') {
			remain_argc -= 2;
			local_context->write_output_to_file = 1;
			i = 3;
		} else {			 
			local_context->write_output_to_file = 0;
			remain_argc--;
			i = 2;
		}

		local_context->channel_list = 0;
		local_context->num_channels = remain_argc - 1;

		/* memory for channel handles */
		local_context->channel_list = ( st_svc_channel_t *) malloc( local_context->num_channels * sizeof( st_svc_channel_t ) );
		assert( local_context->channel_list );		

		/* memory for channel config, one per channel */
		local_context->channel_config = ( st_svc_channel_config_t * ) malloc ( local_context->num_channels * sizeof( st_svc_channel_config_t ) );
		assert( local_context->channel_config );

		/* memory for sender thread context. one per channel */
		local_context->snd_thread = ( pthread_t * ) malloc ( local_context->num_channels * sizeof( pthread_t ) );
		assert( local_context->snd_thread );

		local_context->dst_port = ( int * ) malloc( local_context->num_channels * sizeof( int ) );
		assert( local_context->dst_port );

		local_context->destination =  ( char ** ) malloc( local_context->num_channels * sizeof( char * ) );
		assert( local_context->destination );

		for( channel_num = 0; channel_num < local_context->num_channels ; channel_num++ ) {
			local_context->destination[channel_num] =  \
				( char * ) malloc( 256 * sizeof( char ) );
			assert( local_context->destination[channel_num] );
		}

        while (remain_argc) {
            if (argv[i][0] != '-' && !switch_found) {
                print_help(argv);
                return -1;
            } else {					
                switch (argv[i][1]) {
				case 'c':
						switch_found = 1;
						remain_argc--;
						i++;    
				default:
					if( !switch_found ) {
						print_help(argv);
						return -1;
					}

                    if (remain_argc) {
                        if (cfg_read(cfg_entries, NUM_BASIC_CFG_ELEMENTS, argv[i]) != 0) {
                            printf("Problem reading cfg file %s \n", argv[i]);
                            exit(-1);
                        }
						
						local_context->channel_config[channel_count].avc					= channel_elements[0];
						local_context->channel_config[channel_count].desired_spatial_layer	= channel_elements[1];
						local_context->channel_config[channel_count].desired_temporal_layer	= channel_elements[2];
						local_context->channel_config[channel_count].insert_dummy_frames	= FORCE_DUMMY_FRAMES;

						printf("RTP Destination for channel %d IP %s \n", channel_count, destination);
						strip_address( (char * )destination, &local_context->dst_port[channel_count] );
						memcpy( local_context->destination[channel_count], (char *)destination, 256 * sizeof( unsigned char ));
						memcpy( destination, (char *)temp_destination, 256 * sizeof( unsigned char ));

						channel_count++;
                    } else {
                        print_help(argv);
                        return -1;
                    }
                    break;
                }

				memcpy(  destination, temp_destination, sizeof( temp_destination));

                i++;
                remain_argc--;
            }
        }
    }


	assert( local_context->num_channels == channel_count );

	return ST_SVCEX_NO_ERROR;
}

static st_svc_extractor_buffer_t *svcex_buffer_alloc( size_t num_bytes_to_alloc )
{
	st_svc_extractor_buffer_t *buf = NULL;

    buf = (st_svc_extractor_buffer_t *) malloc( sizeof(st_svc_extractor_buffer_t) );
    memset(buf, 0, sizeof(st_svc_extractor_buffer_t));

    if (buf) {
        buf->buffer_orig = (unsigned char *) malloc( num_bytes_to_alloc + 4 );
        buf->buffer = (sx_uint8 *) (((long)buf->buffer_orig + 3) & 0xfffffffc);
        if (buf->buffer) {
			buf->actual_buffer_size = (unsigned int) num_bytes_to_alloc;
        } else {
            free(buf);
            buf = NULL;
        }
    }

    return buf;
}

static void create_output_buffers ( local_context_t *local_context, st_svc_extractor_t	extractor_handle )
{
	unsigned int num_output_buffers = NUM_OUTPUT_BUFFERS;
	unsigned int i = 0, channel_count = 0;
	st_svc_extractor_buffer_t *buf;
	unsigned int actual_buffer_size = MAX_SLICE_NAL_SIZE;
	st_svc_channel_t channel_handle;

	for ( channel_count = 0 ; channel_count < local_context->num_channels ; channel_count++ ) {
		channel_handle = local_context->channel_list[channel_count];
		if( channel_handle ) {
			for ( i = 0 ; i < num_output_buffers ; i++ ) {
				buf = svcex_buffer_alloc( actual_buffer_size );
				if( !buf ) {
					printf( "Unable to allocate memory for input buffers\n" );
					exit( -1 );
				}
				st_svc_channel_out_put_empty( channel_handle, buf) ;
			}
		}
	}
}

static void generate_sdp(char *sdpStr, char *ipAddr, int port)
{
	char temp[256];
	if (sdpStr == NULL) {
		printf("Fail generating sdp descriptor \n");
		exit(-1);
	}

	sdpStr[0] = '\0';
	sprintf(temp, "v=0\n");
	strcat(sdpStr, temp);
	sprintf(temp, "c=IN IP4 %s\n", ipAddr);
	strcat(sdpStr, temp);
	sprintf(temp, "m=video %d RTP/AVP 36\n", port);
	strcat(sdpStr, temp);
	sprintf(temp, "a=rtpmap:36 H264/90000\n");
	strcat(sdpStr, temp);
	sprintf(temp, "a=control:trackID=1\n");
	strcat(sdpStr, temp);
	sprintf(temp, "a=cliprect:0,0,576,704\n");
	strcat(sdpStr, temp);
	sprintf(temp, "a=framesize:36 704-576\n");
	strcat(sdpStr, temp);
	sprintf(temp, "a=fmtp:36 packetization-mode=1:profile-level-id=420028\n");
	strcat(sdpStr, temp);

#if 0
	printf("%s", sdpStr);
#endif
}

static void create_sender_thread(	local_context_t *local_context, st_svc_channel_t channel_handle,
									unsigned int channel_count)
{
	snd_context_t	*thread_ctx;
	int rcode;
	int temp_id = local_context->channel_config[channel_count].desired_temporal_layer;
	int divi[3] = {4,2,1};

	thread_ctx = (snd_context_t *)malloc(sizeof(snd_context_t));
	if (thread_ctx == NULL) {
		printf("Fail allocating rcv thread context \n");
		exit(-1);
	}
	memset(thread_ctx, 0, sizeof(snd_context_t));

	/* Open RTP snd/rcv Channel */
	local_context->snd_cfg.bufferCapacity	= SND_BUF_SIZE / 
		(1 << (MAX_NUM_SPATIAL - 1 - local_context->channel_config[channel_count].desired_spatial_layer));
	local_context->snd_cfg.numBuffers		= NUM_SND_BUF;
	if( local_context->channel_config[channel_count].insert_dummy_frames ) {
		// 25 fps always
		local_context->snd_cfg.timestampInc		= 160000/(1 << 2); 
	} else {
		local_context->snd_cfg.timestampInc		= 160000/(1 << temp_id); 	
	}
	
	local_context->snd_cfg.outputIpAddr		= ( char *)local_context->destination[channel_count];
	local_context->snd_cfg.outputPort		= local_context->dst_port[channel_count];
	rcode = stRtpIOSndCreate(&thread_ctx->snd_chan, &local_context->snd_cfg);
	if (rcode) {
		printf("Fail creating rcv channel (err %d) \n", rcode);
		exit(-1);
	}

	/* Create thread */
	thread_ctx->channel_handle	= channel_handle;
	thread_ctx->spatial_id		= local_context->channel_config[channel_count].desired_spatial_layer;
	thread_ctx->temporal_id		= local_context->channel_config[channel_count].desired_temporal_layer;
	thread_ctx->avc				= local_context->channel_config[channel_count].avc;
	thread_ctx->channel_count	= channel_count;
	thread_ctx->write_output_to_file = local_context->write_output_to_file;
	
	rcode = sem_init(&thread_ctx->thread_sync, 0, 0);
	if (rcode == -1) {
		exit(-1);
	}
	
	rcode = pthread_create(&local_context->snd_thread[channel_count], \
							NULL, sender_thread_func, (void *)thread_ctx);
	if (rcode) {
		printf("Fail creating receive thread \n");
		exit(-1);
	}

	sem_wait(&thread_ctx->thread_sync);

	rcode = stRtpIOSndStart(thread_ctx->snd_chan);
	if (rcode) {
		printf("Fail starting rcv channel \n");
		exit(-1);
	}

}

static void create_extractor_thread( local_context_t *local_context, st_svc_extractor_t extractor_handle )
{
	rcv_context_t	*thread_ctx;
	int rcode;

	thread_ctx = (rcv_context_t *)malloc(sizeof(rcv_context_t));
	if (thread_ctx == NULL) {
		printf("Fail allocating rcv thread context \n");
		exit(-1);
	}
	memset(thread_ctx, 0, sizeof(rcv_context_t));

	/* Open RTP snd/rcv Channel */
	local_context->rcv_cfg.bufferCapacity	= RCV_BUF_SIZE;
	local_context->rcv_cfg.numBuffers		= NUM_RCV_BUF;
	local_context->rcv_cfg.nonblocking		= 0;
	generate_sdp(local_context->rcv_cfg.inputSdpDesc, (char * )local_context->src_ip, local_context->src_port);
	rcode = stRtpIORcvCreate(&local_context->rcv_chan, &local_context->rcv_cfg);
	if (rcode) {
		printf("Fail creating rcv channel (err %d) \n", rcode);
		exit(-1);
	}

	/* Create thread */
	thread_ctx->rcv_chan			= local_context->rcv_chan;
	thread_ctx->extractor_handle	= extractor_handle;

	rcode = sem_init(&thread_ctx->thread_sync, 0, 0);
	if (rcode == -1) {
		exit(-1);
	}

	rcode = pthread_create(&local_context->rcv_thread, NULL, extractor_thread_func, (void *)thread_ctx);
	if (rcode) {
		printf("Fail creating receive thread \n");
		exit(-1);
	}

	sem_wait(&thread_ctx->thread_sync);

	rcode = stRtpIORcvStart(local_context->rcv_chan);
	if (rcode) {
		printf("Fail starting rcv channel \n");
		exit(-1);
	}
}

static void *extractor_thread_func(void *arg)
{
	rcv_context_t			*rcv_ctx = (rcv_context_t *) arg;
	stRtpIORcvHandle_t		rcv_chan = (stRtpIORcvHandle_t) rcv_ctx->rcv_chan;
	st_svc_extractor_buffer_t inbuf;
	int						rcode = 0;
	st_svc_extractor_t		extractor_handle = rcv_ctx->extractor_handle;
	st_svc_extractor_error_e	err;
	int						count = 0;

	sem_post(&rcv_ctx->thread_sync);

	while (1) {
		char	*rcv_buf;
		int		data_size;

		stRtpIORcvGetFullBuf(rcv_chan, &rcv_buf, &data_size); 
		/*{
			if( rcv_buf ) {
				FILE *fp; 
				char tmp[] = {0,0,0,1};
				fp = fopen ("test.264", "ab+");
				fwrite(&tmp, 4, 1, fp);
				fwrite(&rcv_buf, data_size, 1, fp);
				fclose(fp);
			}
		}*/

		/*st_svc_extractor_in_get_empty( extractor_handle, &inbuf);
		if (!inbuf) {
			 printf("st_svc_extractor_in_get_empty err(0x%x) \n", rcode);
			 exit(-1);
		}*/

		inbuf.buffer = ( unsigned char *) rcv_buf;
		inbuf.stored_data_size = data_size;		
		err = st_svc_extractor_in_put_full ( extractor_handle, &inbuf);
		if( err != ST_SVCEX_NO_ERROR ) {
			st_svc_extractor_delete( extractor_handle );
			printf( "st_svc_extractor_in_put_full returned %d\n", err );
			exit( -1 );
		}					
		
		err = st_svc_extract ( extractor_handle );
		if( err < ST_SVCEX_NO_ERROR ) {
			st_svc_extractor_delete( extractor_handle );
			printf( "st_svc_extract returned %d\n", err );
			exit( -1 );
		}

		stRtpIORcvPutEmptyBuf(rcv_chan, rcv_buf); 

	}

	return 0;
}

static void *sender_thread_func(void *arg)
{
	snd_context_t		*snd_ctx = (snd_context_t *)arg;
	stRtpIOSndHandle_t	snd_chan = (stRtpIOSndHandle_t)snd_ctx->snd_chan;
	int					rcode = 0;
	char				*snd_buf;
	st_svc_extractor_buffer_t	*outbuf;
	st_svc_channel_t			channel_handle = snd_ctx->channel_handle;
	char						file_name[256];
	FILE						*fp = NULL;

	if (snd_ctx->write_output_to_file) {
		sprintf( file_name, "out_s%d_t%d_%s_%d.264", snd_ctx->spatial_id, \
				 snd_ctx->temporal_id, snd_ctx->avc?"avc":"svc", \
				 snd_ctx->channel_count );

		printf("Open output file %s \n", file_name);
		fp = fopen ( file_name, "wb" );
		assert( fp );
	}

	sem_post(&snd_ctx->thread_sync);

	while (1) {
		rcode = st_svc_channel_out_get_full(channel_handle, &outbuf);
		if (rcode == 0) {
			if (!snd_ctx->write_output_to_file) {
				stRtpIOSndGetEmptyBuf(snd_chan, &snd_buf);
				fast_memcpy(snd_buf, outbuf->buffer, outbuf->stored_data_size);
			
				/* empty used up buffers*/
				st_svc_channel_out_put_empty(channel_handle, outbuf);

				/* into RTP's full buffer */
				stRtpIOSndPutFullBuf(snd_chan, snd_buf, outbuf->stored_data_size);

				if ((snd_ctx->cnt % 300) == 0) {
					printf("[%d][%d]:%d\n", snd_ctx->spatial_id, snd_ctx->temporal_id, snd_ctx->cnt);
				}
				snd_ctx->cnt++;

			} else {
				char start_code[4] = {0,0,0,1};
#if 0
				printf(" ** writing to file out_s%d_t%d_%s_%d cnt(%d) size(%d) ** \n", outbuf->spatial_layer, \
						outbuf->temporal_layer, snd_ctx->avc?"avc":"svc", \
						snd_ctx->channel_count, snd_ctx->cnt, outbuf->stored_data_size );
#else
				static long print_cnt = 0;

				if (print_cnt == 0) {
					printf("Start writing to files \n");
					print_cnt = 1;
				} 
#endif

				fwrite(start_code, 4 * sizeof(char), 1, fp);
				fwrite(outbuf->buffer, sizeof(char), outbuf->stored_data_size, fp);

				st_svc_channel_out_put_empty(channel_handle, outbuf);

				snd_ctx->cnt++;		
			}
		} else {
			Sleep(1);
		}
	} 

	if (snd_ctx->write_output_to_file) {
		if( fp ) {
			fclose(fp);
		}
	}

	return 0;
}

static void close_all( local_context_t *p_local_context ) 
{
	if( p_local_context->channel_list ) {
		free (p_local_context->channel_list );
	}
	if( p_local_context->snd_thread ) {
		free( p_local_context->snd_thread );
	}
	if( p_local_context->channel_config ) {
		free( p_local_context->channel_config );
	}
	if( p_local_context->dst_port ) {
		free( p_local_context->dst_port );
	}
	if( p_local_context->destination ) {
		free( p_local_context->destination );
	}
	if( p_local_context ) {
		free( p_local_context );
	}	
}

static void svc_rtp_extractor_test(int argc, char *argv[])
{
	st_svc_extractor_t			extractor_handle = NULL;
	st_svc_extractor_error_e	err;
	int							parse_error;
	local_context_t				*p_local_context = NULL;
	unsigned int i;	
	unsigned int channel_num;

	SetPriorityClass(GetCurrentProcess, HIGH_PRIORITY_CLASS);

	p_local_context = (local_context_t *) malloc( sizeof( local_context_t ) );
	assert( p_local_context );

	/* Parse Configuration. */
	parse_error = parse_config( argc, argv, p_local_context );
	if( parse_error != ST_SVCEX_NO_ERROR ) {
		printf( "parse_configuration returned %d\n", parse_error );
		exit( -1 );
	}

	/* Create extractor instance. 
	NOTE: This can happen even before parse_config. There is no dependency on 
	the configuration desired as long as the desired input buffers are emptied 
	into the extractors input buffers. */
	err = st_svc_extractor_create(&extractor_handle);
	if( err != ST_SVCEX_NO_ERROR ) {
		printf( "st_svc_extractor_create returned %d\n", err );
		exit( -1 );
	}

#ifdef PROFILE_EXTRACTOR
	{
		st_svc_extractor_config_t config;

		config.debug = 0;
		config.time_extractor = 1;
		err = st_svc_extractor_config(extractor_handle, &config);
		if( err != ST_SVCEX_NO_ERROR ) {
			printf( "st_svc_extractor_config returned %d\n", err );
			exit( -1 );
		}
	}
#endif

	for( channel_num = 0; channel_num < p_local_context->num_channels ; channel_num++ ) {
		err = st_svc_channel_create(&p_local_context->channel_list[channel_num], &p_local_context->channel_config[channel_num]);
		if( err ) {
			printf( "Error creating channel. st_svc_channel_create returned %d\n", err);
			st_svc_extractor_delete( extractor_handle );
			close_all( p_local_context );
			exit( -1 );
		} else {
			printf( "Succesfully Created Channel %d\n", channel_num);							
		}

		/***
		NOTE: Attaching to an extractor need not happen immediately. 
		***/
		err = st_svc_extractor_attach_channel( extractor_handle, p_local_context->channel_list[channel_num] );
		if( err ) {
			printf( "Error attaching chann      el. st_svc_extractor_attach_channel returned %d\n", err);
			st_svc_extractor_delete( extractor_handle );
			close_all( p_local_context );
			exit( -1 );
		} else {
			printf( "Succesfully attached channel %d\n", channel_num);							
		}
	}

	/* Create Output Buffers */
	create_output_buffers( p_local_context, extractor_handle );

	for( channel_num = 0; channel_num < p_local_context->num_channels ; channel_num++ ) {
		create_sender_thread(	p_local_context, 
								p_local_context->channel_list[channel_num], channel_num);
	}

	/* Start the extractor thread */
	create_extractor_thread( p_local_context, extractor_handle );	

	/* Wait for all target threads to complete. */
	{
		int snd_error = 0, rcv_error = 0;
		/* Wait for all threads to complete. */
		while(1) {
			rcv_error = pthread_join(p_local_context->rcv_thread, NULL);
			if( rcv_error ) {
					printf( "Rcv thread returned will error\n", rcv_error);
					break;
				}
			for( i = 0 ; i < p_local_context->num_channels ; i++ ) {
				snd_error |= pthread_join(p_local_context->snd_thread[i], NULL);
				if( snd_error ) {
					printf( "Send thread returned will error\n", snd_error);
					break;
				}
			}
		}
	}

	/* Destroy all allocated memories */
	for ( i = 0 ; i < p_local_context->num_channels ; i++ ) {
		/* NOTE: Must detach channel before deleting it. The extractor has no 
		other way to know if a channel has been deleted. */
		st_svc_extractor_detach_channel( extractor_handle, p_local_context->channel_list[i] ); 		
		st_svc_channel_delete( p_local_context->channel_list[i] );
	}

	/* destroy extractor */
	st_svc_extractor_delete( extractor_handle );	

	/* free locals */
	if( p_local_context->channel_list ) {
		free (p_local_context->channel_list );
	}
	if( p_local_context->snd_thread ) {
		free( p_local_context->snd_thread );
	}
	if( p_local_context->channel_config ) {
		free( p_local_context->channel_config );
	}
	if( p_local_context->dst_port ) {
		free( p_local_context->dst_port );
	}
	if( p_local_context->destination ) {
		free( p_local_context->destination );
	}
	if( p_local_context ) {
		free( p_local_context );
	}	

	printf("Extraction Done\n");
}

void main(int argc, char *argv[])
{	
	svc_rtp_extractor_test( argc, argv );
}
