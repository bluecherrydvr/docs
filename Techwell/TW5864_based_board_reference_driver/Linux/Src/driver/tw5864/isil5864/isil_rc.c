#include	<isil5864/isil_common.h>



static isil_rc_driver_op    rc_driver_op;
static void isil_rc_init(isil_h264_logic_encode_chan_t *encoder)
{
    isil_rc_driver_t	*rc_driver;

    rc_driver = &encoder->rc_driver;
    rc_driver->discard_frame_number_by_level = 0;
    rc_driver->rc_op = &rc_driver_op;
}

static void isil264_update_rc(isil_h264_logic_encode_chan_t	*encoder, isil_video_frame_tcb_t *video_frame)
{
    isil_rc_driver_t	*rc_driver;
    isil_h264_encode_property_t	*encode_property;
    isil_h264_encode_control_t	*encode_control;
    isil_h264_encode_rc_t       *encode_rc;

    rc_driver = &encoder->rc_driver;
    encode_property = &encoder->encode_property;
    encode_rc      = &encode_property->encode_rc;
    encode_control = &encoder->encode_control;
    switch(encode_rc->op->get_rc_type(encode_rc)){
       default:
            if((video_frame->frame_type==H264_FRAME_TYPE_IDR) || (video_frame->frame_type==H264_FRAME_TYPE_I)){
                encode_control->i_curr_qp = encode_rc->op->get_qpi(encode_rc);
            } else if(video_frame->frame_type==H264_FRAME_TYPE_P){
                encode_control->i_curr_qp = encode_rc->op->get_qpp(encode_rc);
            } else {
                encode_control->i_curr_qp = encode_rc->op->get_qpb(encode_rc);
            }
            break;
    }
}

static void	isil264_jump_rc(isil_h264_logic_encode_chan_t *encoder)
{
}

static void  isil264_update_rc_para(isil_h264_logic_encode_chan_t *encoder)
{
    isil_rc_driver_t	*rc_driver;
    isil_h264_encode_property_t	*encode_property;
    isil_h264_encode_control_t	*encode_control;
    isil_h264_encode_rc_t		*encode_rc;

    rc_driver = &encoder->rc_driver;
    encode_property = &encoder->encode_property;
    encode_rc      = &encode_property->encode_rc;
    encode_control = &encoder->encode_control;
    switch(encode_rc->op->get_rc_type(encode_rc)){
        default:
            rc_driver->rc_op->jump_rc(encoder);
            break;
    }
}

static isil_rc_driver_op	rc_driver_op =
{
    .init_rc = isil_rc_init,
    .set_qp = isil264_update_rc,
    .update_rc = isil264_update_rc_para,
    .jump_rc = isil264_jump_rc,
};

void    init_rc_driver(isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    if(h264_logic_encode_chan != NULL){
        isil_rc_driver_t	*rc_driver = &h264_logic_encode_chan->rc_driver;
        rc_driver->rc_op = &rc_driver_op;
        rc_driver->rc_op->init_rc(h264_logic_encode_chan);
    }
}


