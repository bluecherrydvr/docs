#ifndef _H26L_ENCODE_H_
#define _H26L_ENCODE_H_

#ifndef _LIB
	#ifdef _DEBUG
		#pragma comment(lib, "h26lencoderD.lib")
	#else
		#pragma comment(lib, "h26lencoder.lib")
	#endif
#endif
//! all input parameters

typedef struct
{
	int no_frames;                //!< number of frames to be encoded
	int qp0;                      //!< QP of first frame
	int qpN;                      //!< QP of remaining frames
	int jumpd;                    //!< number of frames to skip in input sequence (e.g 2 takes frame 0,3,6,9...)
	int mv_res;                   //!< motion vector resolution: 0: 1/4-pel, 1: 1/8-pel
	int hadamard;                 /*!< 0: 'normal' SAD in 1/3 pixel search.  1: use 4x4 Haphazard transform and '
                                     Sum of absolute transform difference' in 1/3 pixel search                   */
	int search_range;             /*!< search range - integer pel search and 16x16 blocks.  The search window is
                                     generally around the predicted vector. Max vector is 2xmcrange.  For 8x8
                                     and 4x4 block sizes the search range is 1/2 of that for 16x16 blocks.       */
	int no_multpred;              /*!< 1: prediction from the last frame only. 2: prediction from the last or
                                     second last frame etc.  Maximum 5 frames                                    */

	int mult_ref_B;

	int img_width;                //!< GH: if CUSTOM image format is chosen, use this size
	int img_height;               //!< GH: width and height must be a multiple of 16 pels
	int yuv_format;               //!< GH: YUV format (0=4:0:0, 1=4:2:0, 2=4:2:2, 3=4:4:4,currently only 4:2:0 is supported)
	int color_depth;              //!< GH: YUV color depth per component in bit/pel (currently only 8 bit/pel is supported)
	int intra_upd;                /*!< For error robustness. 0: no special action. 1: One GOB/frame is intra coded
                                     as regular 'update'. 2: One GOB every 2 frames is intra coded etc.
                                     In connection with this intra update, restrictions is put on motion vectors
                                     to prevent errors to propagate from the past                                */
	int blc_size[8][2];           //!< array for different block sizes
	int slice_mode;               //!< Indicate what algorithm to use for setting slices
	int slice_argument;           //!< Argument to the specified slice algorithm
	int UseConstrainedIntraPred;  //!< 0: Inter MB pixels are allowed for intra prediction 1: Not allowed
	int  infile_header;           //!< If input file has a header set this to the length of the header
	char infile[100];             //!< YUV 4:2:0 input format
	char outfile[100];            //!< H.26L compressed output bitstream
	char ReconFile[100];          //!< Reconstructed Pictures
	char TraceFile[100];          //!< Trace Outputs
	int intra_period;             //!< Random Access period though intra

	// B pictures
	int successive_Bframe;        //!< number of B frames that will be used
	int qpB;                      //!< QP of B frames

	// SP Pictures
	int sp_periodicity;           //!< The periodicity of SP-pictures
	int qpsp;                     //!< SP Picture QP for prediction error
	int qpsp_pred;                //!< SP Picture QP for predicted block

	// Introduced by TOM
	int symbol_mode;              //!< Specifies the mode the symbols are mapped on bits
	int of_mode;                  //!< Specifies the mode of the output file
	int partition_mode;           //!< Specifies the mode of data partitioning

	int SequenceHeaderType;
	int TRModulus;
	int PicIdModulus;

	int InterSearch16x16;
	int InterSearch16x8;
	int InterSearch8x16;
	int InterSearch8x8;
	int InterSearch8x4;
	int InterSearch4x8;
	int InterSearch4x4;

	char PictureTypeSequence[100];

	int full_search;
	
	int last_frame;

	int qpN2, qpB2, qp2start;
  
	int rdopt;

	int add_ref_frame;

	int NumberLeakyBuckets;
	char LeakyBucketRateFile[100];
	char LeakyBucketParamFile[100];

	int LossRateA;
	int LossRateB;
	int LossRateC;
	int NoOfDecoders;
	int RestrictRef;

	int target_bitrate;
	int min_quantizer;
	int max_quantizer;
	int if_bitrate_control;

}InputParameters; 

#ifdef __cplusplus
extern "C"
{
#endif
int h26l_init_encoder(void *config);
int h26l_enc_frame(unsigned char *pSrc, unsigned char *bitstream, int *length);
int h26l_close_encoder(unsigned char *bitstream, int *length);
#ifdef __cplusplus
};
#endif
#endif
