/************************************************************************
 * Copyright 2007 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.
 *
 ************************************************************************/

#ifndef __SVC_EXTRACTOR_H__
#define __SVC_EXTRACTOR_H__

/**********************************************************************************
	PACKAGE: svc_extractor -- SVC Sub-bitstream extractor and Rewriter

	DESCRIPTION:

	SECTION: Include
	{
	#include "svc_extractor.h"
	}

	SECTION: Introduction

	@svc_extractor@ is a SVC sub-bitstream extractor and rewriter designed to extract 
	(and rewrite) sub-bitstreams from an SVC bitstream.  It produces either H.264 
	AVC- or H.264 SVC-compliant bitstreams as output. One instance of an extractor is 
	tied to one SVC source. 

	SECTION: Summary of features

	The following features are supported:

		Spatial Scalability upto three layers.

		Temporal Scalability upto three layers.

		SVC to AVC rewriting.

		SVC to SVC rewriting.

		The extractor supports only SVC streams encoded using Stretch's SVC encoder.

	Each extractor can produce output streams, for several output channels.  Each
	channel has its own configuration to specify which spatial or temporal layer it
	desires. Channels that require SVC and AVC outputs can be attached to the same extractor.

	SECTION: Usage model

	SUBSECTION:  Creating and Deleting an Extractor

	An extractor instance must be created and configured before channels can be
	attached to start extraction. st_svc_extractor_create() is used to create an
	extractor instance. Once an extractor is created, st_svc_extractor_config()
	must be called to configure the parameters for the extractor. Once the extraction
	process is done, st_svc_extractor_delete() destroys the extractor instance.

	SUBSECTION:  Creating and Deleting a Channel

	Channels receive extracted data, and are associated with a single extractor 
	instance.  The extractor services its channels based on their configurations.	
	Channels are created with st_svc_channel_create() and destroyed with 
	st_svc_channel_delete().

	SUBSECTION:  Attaching a Channel to an Extractor

	Once a channel is created and configured, it is attached to the desired extractor
	with st_svc_extractor_attach_channel() and may be detacted on the fly with
	st_svc_extractor_detach_channel().

	SUBSECTION:  Input Buffers for the Extractor

	st_svc_extractor_in_put_full() provides the extractor with buffers that it needs
	to process. These buffers are allocated by the caller and must contain complete
	NAL packets with no start codes.

	SUBSECTION:  Output Buffers for the Channel

	st_svc_channel_out_put_empty() provides buffers to the associated channel. These
	buffers must be allocated by the caller. st_svc_channel_out_get_full() gets
	output data for the channel.  The output buffers must be at least as large as
	the input buffers provided to st_svc_extractor_in_put_full().

	SUBSECTION:  Extraction Process

	st_svc_extract() processes the extractor's input buffers to produce the output
	requested by each channel.  Because it requires a scalable SEI packet to 
	understand the structure of its input, it discards all input buffers it 
	receives before the first scalable SEI.

	SUBSECTION:  Pseudo Code
	{
		#include "svc_extractor.h"

		st_svc_extractor_t			extractor_handle;
		st_svc_channel_t			channel_handle[NUM_CLIENTS];
		st_svc_extractor_config_t   extractor_config;
		// number of channels = NUM_CLIENTS
		st_svc_channel_config_t		channel_config[NUM_CLIENTS];
		st_svc_extractor_buffer_t	*output_buffer;
		st_svc_extractor_buffer_t	*input_buffer;
		unsigned int				num_out_buffers_to_allocate = NUM_OUTPUT_BUFFERS;
		unsigned int				buf_num, channel_num;

		// Create extractor instance.
		st_svc_extractor_create(&extractor_handle);

		// Config the extractor.
		extractor_config.debug = 0;
		st_svc_extractor_config(extractor_handle, &extractor_config);

		for( channel_num = 0 ; channel_num < NUM_CLIENTS ; channel_num++ ) {

			// Create and Configure a Channel.
			channel_config[channel_num].avc						= 1;	// = 0 if SVC output is desired
			channel_config[channel_num].desired_spatial_layer		= <spatial_layer_desired_for_this_channels>;
			channel_config[channel_num].desired_temporal_layer		= <temporal_layer_desired_for_this_channels>;
			st_svc_channel_create(&channel_handle[channel_num], &channel_config[channel_num]);

			// Attach the Channel to the Extractor.
			st_svc_extractor_attach_channel( extractor_handle, channel_handle[channel_num] );

			// Allocate Output Buffers. Notice that they are attached to the
			// channels they belong to.
			for( buf_num = 0 ; buf_num < num_out_buffers_to_allocate ; buf_num++ ) {
				output_buffer = <Allocate Output Buffer>;
				st_svc_channel_out_put_empty( channel_handle[channel_num], output_buffer );
			}
		}

		while( <input is available> ) {

			// Get the Input (ex. file or RTP )
			input_buffer = <Get the input buffer>;
			// Notice the input buffer is attached to the Extractor to which the input
			// is associated.
			st_svc_extractor_in_put_full ( extractor_handle, input_buffer);

			// Extraction Process
			st_svc_extract ( extractor_handle );

			// Store or Stream out the output buffers. Once they are used up, return them
			// to the Channel Output Buffer List.
			for( channel_num = 0 ; channel_num < NUM_CLIENTS ; channel_num++ ) {
				output_buffer = NULL;
				st_svc_channel_out_get_full(channel_handle[channel_num], &output_buffer, 0);
				if( output_buffer ) {
					... <consume output> ... ( write to file or stream out )
					st_svc_channel_out_put_empty(channel_handle[channel_num], output_buffer);
				}
			}
		}

		// Detach and Delete Channel Instance(s) from Extractor Instance
		for( channel_num = 0 ; channel_num < NUM_CLIENTS ; channel_num++ ) {
			st_svc_extractor_detach_channel( extractor_handle, channel_handle );
			st_svc_channel_delete( channel_handle[channel_num] );
		}

		// Delete Extractor Instance
		st_svc_extractor_delete( extractor_handle );
	}

 ************************************************************************************/


/**********************************************************************************
	Error code used by the Stretch SVC Extractor. The negative codes are errors and
	the positive ones are just informing the used of specific conditions.

		@ST_SVCEX_NOT_ALLOWED@ - Operation not allowed.

		@ST_SVCEX_CHANNEL_NOT_FOUND@ - Unable to find the channel when trying to
		detach it from the extractor.

		@ST_SVCEX_NO_EMPTY_INBUFFER@ - No empty input buffer.

		@ST_SVCEX_NO_FULL_OUTBUFFER@ - No full output buffer.

		@ST_SVCEX_NO_EMPTY_OUTBUFFER@ - No empty output buffer.

		@ST_SVCEX_NO_INPUT@ - No Input is available for processing.

		@ST_SVCEX_WAITING_FOR_SEI@ - Still waiting for the first SEI to begin extraction.

		@ST_SVCEX_NO_ACTIVE_CHANNEL@ - No active channels to process.

		@ST_SVCEX_NO_ERROR@ - Success.

		@ST_SVCEX_OUT_OF_MEMORY@ - Unable to allocate memory.

		@ST_SVCEX_INVALID_POINTER@ - Invalid pointer is passed to the API.

		@ST_SVCEX_UNSUPPORTED_SEI@ - Only Scalable SEI messages are recognised. All other
		messages are not recognised by the extraction process.

		@ST_SVCEX_UNSUPPORTED_NAL@ - Nal type is not supported.

		@ST_SVCEX_DID_EXCEEDS_RANGE@ - Dependency ID is more than supported range (0-2)

		@ST_SVCEX_TID_EXCEEDS_RANGE@ - Temporal ID is more than supported range (0-3)

		@ST_SVCEX_LAYERS_EXCEED_RANGE@ - Layer ID is more than supported range (0-12)
**********************************************************************************/

typedef enum st_svc_extractor_error_tag {
	ST_SVCEX_NOT_ALLOWED			= 8,
	ST_SVCEX_CHANNEL_NOT_FOUND		= 7,
	ST_SVCEX_NO_EMPTY_INBUFFER		= 6,
	ST_SVCEX_NO_FULL_OUTBUFFER		= 5,
	ST_SVCEX_NO_EMPTY_OUTBUFFER		= 4,
	ST_SVCEX_NO_INPUT				= 3,
	ST_SVCEX_WAITING_FOR_SEI		= 2,
	ST_SVCEX_NO_ACTIVE_CHANNEL		= 1,
	ST_SVCEX_NO_ERROR				= 0,
	ST_SVCEX_OUT_OF_MEMORY			= -1,
	ST_SVCEX_INVALID_POINTER		= -2,
	//ST_SVCEX_SEM_INIT_FAILED		= -3,
	ST_SVCEX_UNSUPPORTED_SEI		= -4,
	ST_SVCEX_UNSUPPORTED_NAL		= -5,
	ST_SVCEX_DID_EXCEEDS_RANGE		= -6,
	ST_SVCEX_TID_EXCEEDS_RANGE		= -7,
	ST_SVCEX_LAYERS_EXCEED_RANGE	= -8,
} st_svc_extractor_error_e;

/**********************************************************************************
	VISIBLE: Data Structure for the input and output buffers

	@Fields@:

		"buffer" - Pointer to the original allocated pointer to be used for
		extraction process.

		"actual_buffer_size" - Data Size of the original buffer upon allocation.

		"stored_data_size" - Data size of a full buffer ( after extraction and
		rewriting)

		"spatial_layer" - Spatial Layer to which this buffer belongs.

		"temporal_layer" - Temporal Layer to which this buffer belongs.

**********************************************************************************/
typedef struct st_svc_extractor_buffer_tag {
	unsigned char	*buffer;
	unsigned int    actual_buffer_size;
	unsigned int    stored_data_size;
	unsigned int    spatial_layer;
	unsigned int    temporal_layer;
} st_svc_extractor_buffer_t;

/**********************************************************************************
	VISIBLE: Configuration data for a Channel instance.

	@Fields@:

		"desired_spatial_layer" - The spatial scalability layer desired by the
		channel.

		"desired_temporal_layer" - The temporal scalability layer desired by the
		channel.

		"avc" - Setting this to zero produces SVC output from the extractor, otherwise
		the output is AVC

		"insert_dummy_frames" - When this flag is set to one, the extractor will
		insert dummy frames when gaps in frame num are found. This feature enables
		decoders that do not support subsequences to play the extracted bitstream.
		This field is only valid when "avc" is set to 1.

		"debug" - This field is only for debugging. Will produce a lot of intrusive
		prints.

**********************************************************************************/
typedef struct st_channel_config_tag {
	unsigned int desired_spatial_layer;
	unsigned int desired_temporal_layer;
	unsigned int avc;

	unsigned int insert_dummy_frames;
	unsigned int debug;
} st_svc_channel_config_t;

/**********************************************************************************
	VISIBLE: Configuration data for a Channel instance.

	@Fields@:

		"debug" - This field is only for debugging. Will produce a lot of intrusive
		prints.

		"time_extractor" - Clock cycle measurement for performance.

**********************************************************************************/
typedef struct st_extractor_config_tag {
	unsigned int debug;
	unsigned int time_extractor;
} st_svc_extractor_config_t;

/**********************************************************************************
	The channel context is stored in @st_svc_channel_t@. 
**********************************************************************************/
typedef struct	st_svc_channel_tag				*st_svc_channel_t;

/**********************************************************************************
	The extractor context is stored in @st_svc_extractor_t@. 
**********************************************************************************/
typedef struct	st_svc_extractor_tag			*st_svc_extractor_t;

extern st_svc_extractor_error_e st_svc_extractor_create(st_svc_extractor_t *handle);
extern st_svc_extractor_error_e st_svc_extractor_config(st_svc_extractor_t handle,
														st_svc_extractor_config_t *config);
extern st_svc_extractor_error_e st_svc_channel_create(st_svc_channel_t *handle, st_svc_channel_config_t *config);
extern st_svc_extractor_error_e st_svc_modify_channel (	st_svc_channel_t channel,
														st_svc_channel_config_t *config);
extern st_svc_extractor_error_e st_svc_extractor_attach_channel (st_svc_extractor_t extractor,
																st_svc_channel_t channel);
extern st_svc_extractor_error_e st_svc_extractor_detach_channel (st_svc_extractor_t extractor,
																st_svc_channel_t channel);
extern st_svc_extractor_error_e st_svc_extract ( st_svc_extractor_t extractor );
extern st_svc_extractor_error_e  st_svc_extractor_in_get_empty (st_svc_extractor_t extractor,
																st_svc_extractor_buffer_t **buf);
extern st_svc_extractor_error_e  st_svc_extractor_in_put_full (	st_svc_extractor_t extractor,
																st_svc_extractor_buffer_t *buf);
extern st_svc_extractor_error_e  st_svc_extractor_in_put_empty (st_svc_extractor_t extractor,
																st_svc_extractor_buffer_t *buf);
extern st_svc_extractor_error_e  st_svc_channel_out_put_empty (	st_svc_channel_t channel,
																st_svc_extractor_buffer_t *buf);
extern st_svc_extractor_error_e  st_svc_channel_out_get_full (	st_svc_channel_t channel,
																st_svc_extractor_buffer_t **buf );
extern unsigned int st_svc_channel_is_active (	st_svc_channel_t channel );
extern void st_svc_channel_get_config (	st_svc_channel_t channel, st_svc_channel_config_t **config );
extern void st_svc_extractor_delete(st_svc_extractor_t handle);
extern void st_svc_channel_delete(st_svc_channel_t handle);

#endif  // #ifndef __SVC_EXTRACTOR_H__
