#include	<isil5864/isil_common.h>

isil_audio_config_descriptor_t    default_audio_encode_config_descriptor = {
    .type = ISIL_AUDIO_ADPCM_32K,
    .sample_rate = ISIL_AUDIO_8000,
    .bit_wide = ISIL_AUDIO_16BIT,
};

isil_audio_config_descriptor_t    default_audio_decode_config_descriptor = {
    .type = ISIL_AUDIO_ADPCM_32K,
    .sample_rate = ISIL_AUDIO_8000,
    .bit_wide = ISIL_AUDIO_16BIT,
};

int  get_power_base(u32 value)
{
    int position, number;
    position = number = 0;
    while(value){
        if(value&1){
            number++;
        }
        position++;
        value >>= 1;
    }
    if(number==1){
        position--;
    }
    return position;
}

/****************************************************************************************/
/*              audio interface buf descriptor operation table                          */
/****************************************************************************************/
static void audio_section_descriptor_init_audio_descriptor(isil_audio_descriptor_t *audio_descriptor, void *descriptor_addr)
{
    if(descriptor_addr != NULL){
        audio_descriptor->descriptor = (audio_section_descriptor_t*)descriptor_addr;
    } else {
        audio_descriptor->descriptor = NULL;
    }
}

static u32  audio_section_descriptor_get_audio_descriptor_timestamp(isil_audio_descriptor_t *audio_descriptor)
{
    if(audio_descriptor->descriptor != NULL){
        return audio_descriptor->descriptor->timestamp;
    } else {
        return 0;
    }
}

static void audio_section_descriptor_set_audio_descriptor_timestamp(isil_audio_descriptor_t *audio_descriptor, u32 timestamp)
{
    if(audio_descriptor->descriptor != NULL){
        audio_descriptor->descriptor->timestamp = timestamp;
    }
}

static int  audio_section_descriptor_get_audio_descriptor_sample_rate(isil_audio_descriptor_t *audio_descriptor)
{
    if(audio_descriptor->descriptor != NULL){
        return audio_descriptor->descriptor->sample_rate;
    } else {
        return ISIL_AUDIO_8000;
    }
}

static void audio_section_descriptor_set_audio_descriptor_sample_rate(isil_audio_descriptor_t *audio_descriptor, int sample_rate)
{
    if(audio_descriptor->descriptor != NULL){
        audio_descriptor->descriptor->sample_rate = sample_rate;
    }
}

static int  audio_section_descriptor_get_audio_descriptor_type(isil_audio_descriptor_t *audio_descriptor)
{
    if(audio_descriptor->descriptor != NULL){
        return audio_descriptor->descriptor->type;
    } else {
        return ISIL_AUDIO_ADPCM_32K;
    }
}

static void audio_section_descriptor_set_audio_descriptor_type(isil_audio_descriptor_t *audio_descriptor, int type)
{
    if(audio_descriptor->descriptor != NULL){
        audio_descriptor->descriptor->type= type;
    }
}

static int  audio_section_descriptor_get_audio_descriptor_bit_wide(isil_audio_descriptor_t *audio_descriptor)
{
    if(audio_descriptor->descriptor != NULL){
        return audio_descriptor->descriptor->bit_wide;
    } else {
        return ISIL_AUDIO_16BIT;
    }
}

static void audio_section_descriptor_set_audio_descriptor_bit_wide(isil_audio_descriptor_t *audio_descriptor, int bit_wide)
{
    if(audio_descriptor->descriptor != NULL){
        audio_descriptor->descriptor->bit_wide = bit_wide;
    }
}

static u32  audio_section_descriptor_get_audio_descriptor_valid_len(isil_audio_descriptor_t *audio_descriptor)
{
    if(audio_descriptor->descriptor != NULL){
        return audio_descriptor->descriptor->valid_len;
    } else {
        return 0;
    }
}

static void audio_section_descriptor_set_audio_descriptor_valid_len(isil_audio_descriptor_t *audio_descriptor, u32 valid_len)
{
    if(audio_descriptor->descriptor != NULL){
        audio_descriptor->descriptor->valid_len = valid_len;
    }
}

static struct isil_audio_section_descriptor_operation audio_section_descriptor_op = {
    .init_audio_descriptor = audio_section_descriptor_init_audio_descriptor,
    .get_audio_descriptor_timestamp = audio_section_descriptor_get_audio_descriptor_timestamp,
    .set_audio_descriptor_timestamp = audio_section_descriptor_set_audio_descriptor_timestamp,
    .get_audio_descriptor_sample_rate = audio_section_descriptor_get_audio_descriptor_sample_rate,
    .set_audio_descriptor_sample_rate = audio_section_descriptor_set_audio_descriptor_sample_rate,
    .get_audio_descriptor_type = audio_section_descriptor_get_audio_descriptor_type,
    .set_audio_descriptor_type = audio_section_descriptor_set_audio_descriptor_type,
    .get_audio_descriptor_bit_wide = audio_section_descriptor_get_audio_descriptor_bit_wide,
    .set_audio_descriptor_bit_wide = audio_section_descriptor_set_audio_descriptor_bit_wide,
    .get_audio_descriptor_valid_len = audio_section_descriptor_get_audio_descriptor_valid_len,
    .set_audio_descriptor_valid_len = audio_section_descriptor_set_audio_descriptor_valid_len,
};

void	init_isil_audio_descriptor(isil_audio_descriptor_t *audio_descriptor, void *audio_descriptor_addr)
{
    if(audio_descriptor != NULL){
        audio_descriptor->op = &audio_section_descriptor_op;
        audio_descriptor->op->init_audio_descriptor(audio_descriptor, audio_descriptor_addr);
    }
}


/****************************************************************************************/
/*              audio encode property operation table                                   */
/****************************************************************************************/
static void audio_encode_config_param_update_init(isil_audio_param_t *running_param, isil_audio_config_descriptor_t *param)
{
    if(param == NULL){
        param = &default_audio_encode_config_descriptor;
    }
    running_param->property.type = param->type;
    running_param->property.sample_rate = param->sample_rate;
    running_param->property.bit_wide = param->bit_wide;
}

static int  audio_encode_config_param_update_update_sample_rate(isil_audio_param_t *running_param, int sample_rate)
{
    chip_audio_encode_t     *audio_encode;
    audio_encode_control_t  *audio_control;
    chip_audio_t    *chip_audio;
    isil_chip_t     *chip;

    if((sample_rate!=ISIL_AUDIO_8000) && (sample_rate!=ISIL_AUDIO_16000)){
        ISIL_DBG(ISIL_DBG_ERR, "only support 8K, 16K\n");
        return ISIL_ERR;
    }
    running_param->property.sample_rate = sample_rate;
    audio_encode = to_get_chip_audio_encode_with_running_param(running_param);
    chip_audio = to_get_chip_audio_with_audio_encode(audio_encode);
    chip = chip_audio->chip;
    audio_control = &audio_encode->audio_control;
    if(audio_control->op != NULL){
        audio_control->op->set_audio_sample_rate(audio_control, sample_rate, chip);
    }
    return ISIL_OK;
}

static int  audio_encode_config_param_update_get_sample_rate(isil_audio_param_t *running_param)
{
    return running_param->property.sample_rate;
}

static int  audio_encode_config_param_update_update_bit_wide(isil_audio_param_t *running_param, int bit_wide)
{
    chip_audio_encode_t     *audio_encode;
    audio_encode_control_t  *audio_control;
    chip_audio_t    *chip_audio;
    isil_chip_t     *chip;

    if((bit_wide!=ISIL_AUDIO_16BIT) && (bit_wide!=ISIL_AUDIO_8BIT)){
        ISIL_DBG(ISIL_DBG_ERR, "only support 8bit or 16bit\n");
        return ISIL_ERR;
    }
    running_param->property.bit_wide = bit_wide;
    audio_encode = to_get_chip_audio_encode_with_running_param(running_param);
    chip_audio = to_get_chip_audio_with_audio_encode(audio_encode);
    chip = chip_audio->chip;
    audio_control = &audio_encode->audio_control;
    if(audio_control->op != NULL){
        audio_control->op->set_bit_wide(audio_control, bit_wide, chip);
    }
    return ISIL_OK;
}

static int  audio_encode_config_param_update_get_bit_wide(isil_audio_param_t *running_param)
{
    return running_param->property.bit_wide;
}

static int  audio_encode_config_param_update_update_type(isil_audio_param_t *running_param, int type)
{
    chip_audio_encode_t     *audio_encode;
    audio_encode_control_t  *audio_control;
    chip_audio_t    *chip_audio;
    isil_chip_t     *chip;

    if((type != ISIL_AUDIO_ADPCM_32K) && (type != ISIL_AUDIO_PCM)){
        ISIL_DBG(ISIL_DBG_ERR, "only support ISIL_AUDIO_ADPCM_32K and ISIL_AUDIO_PCM\n");
        return ISIL_ERR;
    }
    running_param->property.type = type;
    audio_encode = to_get_chip_audio_encode_with_running_param(running_param);
    chip_audio = to_get_chip_audio_with_audio_encode(audio_encode);
    chip = chip_audio->chip;
    audio_control = &audio_encode->audio_control;
    if(audio_control->op != NULL){
        audio_control->op->set_audio_type(audio_control, type, chip);
    }
    return ISIL_OK;
}

static int  audio_encode_config_param_update_get_type(isil_audio_param_t *running_param)
{
    return running_param->property.type;
}

static struct audio_property_update_operation   audio_encode_running_param_update_op = {
    .init = audio_encode_config_param_update_init,
    .update_sample_rate = audio_encode_config_param_update_update_sample_rate,
    .get_sample_rate = audio_encode_config_param_update_get_sample_rate,
    .update_bit_wide = audio_encode_config_param_update_update_bit_wide,
    .get_bit_wide = audio_encode_config_param_update_get_bit_wide,
    .update_type = audio_encode_config_param_update_update_type,
    .get_type = audio_encode_config_param_update_get_type,
};

void    init_audio_encode_property(isil_audio_param_t *running_param, isil_audio_config_descriptor_t *config)
{
    if(running_param != NULL){
        running_param->op = &audio_encode_running_param_update_op;
        running_param->op->init(running_param, config);
    }
}

/****************************************************************************************/
/*                          audio decode property operation table                       */
/****************************************************************************************/
static void audio_decode_config_param_update_init(isil_audio_param_t *running_param, isil_audio_config_descriptor_t *param)
{
    if(param == NULL){
        param = &default_audio_decode_config_descriptor;
    }
    if(running_param->op != NULL){
        running_param->op->update_bit_wide(running_param, (int)param->bit_wide);
        running_param->op->update_sample_rate(running_param, (int)param->sample_rate);
        running_param->op->update_type(running_param, (int)param->type);
    } else {
        running_param->property.type = param->type;
        running_param->property.sample_rate = param->sample_rate;
        running_param->property.bit_wide = param->bit_wide;
    }
}

static int  audio_decode_config_param_update_update_sample_rate(isil_audio_param_t *running_param, int sample_rate)
{
    if((sample_rate!=ISIL_AUDIO_8000) && (sample_rate!=ISIL_AUDIO_16000)){
        ISIL_DBG(ISIL_DBG_ERR, "only support 8K, 16K\n");
        return ISIL_ERR;
    }
    if(running_param == NULL){
        return ISIL_ERR;
    }
    running_param->property.sample_rate = sample_rate;
    return ISIL_OK;
}

static int  audio_decode_config_param_update_get_sample_rate(isil_audio_param_t *running_param)
{
    return running_param->property.sample_rate;
}

static int  audio_decode_config_param_update_update_bit_wide(isil_audio_param_t *running_param, int bit_wide)
{
    chip_audio_decode_t     *audio_decode;
    audio_decode_control_t  *audio_control;
    chip_audio_t    *chip_audio;
    isil_chip_t     *chip;

    if((bit_wide!=ISIL_AUDIO_16BIT) && (bit_wide!=ISIL_AUDIO_8BIT)){
        ISIL_DBG(ISIL_DBG_ERR, "only support 8bit or 16bit\n");
        return ISIL_ERR;
    }
    running_param->property.bit_wide = bit_wide;
    audio_decode = to_get_chip_audio_decode_with_running_param(running_param);
    chip_audio = to_get_chip_audio_with_audio_decode(audio_decode);
    chip = chip_audio->chip;
    audio_control = &audio_decode->audio_control;
    if(audio_control->op != NULL){
        audio_control->op->set_decode_chan_bit_wide(audio_control, ISIL_AUDIO_OUT_SPEAKER_ID, bit_wide, chip);
        audio_control->op->set_decode_chan_bit_wide(audio_control, ISIL_AUDIO_OUT_PLAYBACK_ID, bit_wide, chip);
    }
    return ISIL_OK;
}

static int  audio_decode_config_param_update_get_bit_wide(isil_audio_param_t *running_param)
{
    return running_param->property.bit_wide;
}

static int  audio_decode_config_param_update_update_type(isil_audio_param_t *running_param, int type)
{
    if((type != ISIL_AUDIO_ADPCM_32K)  && (type != ISIL_AUDIO_PCM)){
        ISIL_DBG(ISIL_DBG_ERR, "only support ISIL_AUDIO_ADPCM_32K and ISIL_AUDIO_PCM\n");
        return ISIL_ERR;
    }
    running_param->property.type = type;
    return ISIL_OK;
}

static int  audio_decode_config_param_update_get_type(isil_audio_param_t *running_param)
{
    return running_param->property.type;
}

static struct audio_property_update_operation   audio_decode_running_param_update_op = {
    .init = audio_decode_config_param_update_init,
    .update_sample_rate = audio_decode_config_param_update_update_sample_rate,
    .get_sample_rate = audio_decode_config_param_update_get_sample_rate,
    .update_bit_wide = audio_decode_config_param_update_update_bit_wide,
    .get_bit_wide = audio_decode_config_param_update_get_bit_wide,
    .update_type = audio_decode_config_param_update_update_type,
    .get_type = audio_decode_config_param_update_get_type,
};

void    init_audio_decode_property(isil_audio_param_t *running_param, isil_audio_config_descriptor_t *config)
{
    if(running_param != NULL){
        running_param->op = &audio_decode_running_param_update_op;
        running_param->op->init(running_param, config);
    }
}


/****************************************************************************************/
/*                  audio enable control operation table                                */
/****************************************************************************************/
static int  audio_enable_get_adpcm_decode_state(audio_enable_control_t *audio_en)
{
    return audio_en->value.bit_value.adpcm_decode_enable;
}

static void audio_enable_update_enable_adpcm_decode(audio_enable_control_t *audio_en, int en)
{
    if(en == AUDIO_DISABLE){
        chip_audio_t        *chip_aduio = to_get_chip_audio_with_audio_enable(audio_en);
        chip_audio_decode_t *audio_decode = &chip_aduio->audio_decode;
        audio_decode_control_t  *audio_control = &audio_decode->audio_control;
        if((audio_control->value.bit_value.audio_out_chan0_enable == AUDIO_DISABLE) &&
                (audio_control->value.bit_value.audio_out_chan1_enable == AUDIO_DISABLE)){
            audio_en->value.bit_value.adpcm_decode_enable = AUDIO_DISABLE;
        }
    } else {
        audio_en->value.bit_value.adpcm_decode_enable = AUDIO_ENABLE;
    }
}

static void audio_enable_set_enable_adpcm_decode(audio_enable_control_t *audio_en, int en, isil_chip_t *chip)
{
    if(audio_en->op != NULL){
        audio_en->op->update_enable_adpcm_decode(audio_en, en);
    } else {
        audio_enable_update_enable_adpcm_decode(audio_en, en);    
    }
    if(chip != NULL){
        chip->io_op->chip_write32(chip, audio_en->reg_offset, audio_en->value.value);
    }
}

static int  audio_enable_get_pcm_encode_state(audio_enable_control_t *audio_en)
{
    return audio_en->value.bit_value.pcm_encode_enable;
}

static void audio_enable_update_enable_pcm_encode(audio_enable_control_t *audio_en, int en)
{
    chip_audio_t            *chip_audio = to_get_chip_audio_with_audio_enable(audio_en);
    chip_audio_encode_t     *audio_encode = &chip_audio->audio_encode;
    audio_encode_control_t  *audio_control = &audio_encode->audio_control;

    if(en == AUDIO_DISABLE){
        if((audio_control->pcm_value.bit_value.audio_chan0_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan1_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan2_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan3_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan4_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan5_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan6_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan7_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan8_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan9_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan10_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan11_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan12_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan13_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan14_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan15_en == AUDIO_DISABLE) &&
                (audio_control->pcm_value.bit_value.audio_chan16_en == AUDIO_DISABLE)){
            audio_en->value.bit_value.pcm_encode_enable = AUDIO_DISABLE; 
            audio_en->value.bit_value.adpcm_encode_enable = AUDIO_DISABLE;
        }
    } else {
        audio_en->value.bit_value.pcm_encode_enable = AUDIO_ENABLE;
    } 
}

static void audio_enable_set_enable_pcm_encode(audio_enable_control_t *audio_en, int en, isil_chip_t *chip)
{
    if(audio_en->op != NULL){
        audio_en->op->update_enable_pcm_encode(audio_en, en);
    } else {
        audio_enable_update_enable_pcm_encode(audio_en, en);    
    }
    if(chip != NULL){
        chip->io_op->chip_write32(chip, audio_en->reg_offset, audio_en->value.value);
    }
}

static int  audio_enable_get_adpcm_encode_state(audio_enable_control_t *audio_en)
{
    return audio_en->value.bit_value.adpcm_encode_enable;
}

static void audio_enable_update_enable_adpcm_encode(audio_enable_control_t *audio_en, int en)
{
    chip_audio_t            *chip_audio = to_get_chip_audio_with_audio_enable(audio_en);
    chip_audio_encode_t     *audio_encode = &chip_audio->audio_encode;
    audio_encode_control_t  *audio_control = &audio_encode->audio_control;

    if(en == AUDIO_DISABLE){
        if((audio_control->adpcm_value.bit_value.audio_chan0_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan1_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan2_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan3_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan4_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan5_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan6_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan7_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan8_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan9_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan10_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan11_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan12_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan13_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan14_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan15_en == AUDIO_DISABLE) &&
                (audio_control->adpcm_value.bit_value.audio_chan16_en == AUDIO_DISABLE)){
            audio_en->value.bit_value.adpcm_encode_enable = AUDIO_DISABLE;
        }
    } else {
        audio_en->value.bit_value.adpcm_encode_enable = AUDIO_ENABLE;
    } 
}

static void audio_enable_set_enable_adpcm_encode(audio_enable_control_t *audio_en, int en, isil_chip_t *chip)
{
    if(audio_en->op != NULL){
        audio_en->op->update_enable_adpcm_encode(audio_en, en);
    } else {
        audio_enable_update_enable_adpcm_encode(audio_en, en);    
    }
    if(chip != NULL){
        chip->io_op->chip_write32(chip, audio_en->reg_offset, audio_en->value.value);
    }
}

static void audio_enable_init_enable_control_reg(audio_enable_control_t *audio_en)
{
    audio_en->reg_offset = AUDIO_EN_CONTROL_REG;
    audio_en->value.value = 0;
}

static void audio_enable_get_enable_control_param(audio_enable_control_t *audio_en, isil_chip_t *chip)
{
    if(chip != NULL){
        audio_en->value.value = chip->io_op->chip_read32(chip, audio_en->reg_offset);
    }
}

static void audio_enable_set_enable_control_param(audio_enable_control_t *audio_en, isil_chip_t *chip)
{
    if(chip != NULL){
        chip->io_op->chip_write32(chip, audio_en->reg_offset, audio_en->value.value);
    }
}

static struct audio_enable_control_operation	audio_enable_op = {
    .get_adpcm_decode_state = audio_enable_get_adpcm_decode_state,
    .update_enable_adpcm_decode = audio_enable_update_enable_adpcm_decode,
    .set_enable_adpcm_decode = audio_enable_set_enable_adpcm_decode,

    .get_pcm_encode_state = audio_enable_get_pcm_encode_state,
    .update_enable_pcm_encode = audio_enable_update_enable_pcm_encode,
    .set_enable_pcm_encode = audio_enable_set_enable_pcm_encode,

    .get_adpcm_encode_state = audio_enable_get_adpcm_encode_state,
    .update_enable_adpcm_encode = audio_enable_update_enable_adpcm_encode,
    .set_enable_adpcm_encode = audio_enable_set_enable_adpcm_encode,

    .init = audio_enable_init_enable_control_reg,
    .get_enable_control_param = audio_enable_get_enable_control_param,
    .set_enable_control_param = audio_enable_set_enable_control_param,
};

void    init_audio_enable_control(audio_enable_control_t *audio_en)
{
    audio_en->op = &audio_enable_op;
    audio_en->op->init(audio_en);
}

/****************************************************************************************/
/*              pcm audio encode read&write ptr operation table                         */
/****************************************************************************************/
static void audio_pcm_encode_wr_rd_ptr_init(audio_pcm_encode_ptr_control_t *pcm_encode_ptr)
{
    pcm_encode_ptr->ch0_ch7_wr_ptr_value.value = 0;
    pcm_encode_ptr->ch0_ch7_wr_ptr_reg_offset = PCM_ENCODE_WR_PTR_CH0_CH7_REG;
    pcm_encode_ptr->ch8_ch15_wr_ptr_value.value = 0;
    pcm_encode_ptr->ch8_ch15_wr_ptr_reg_offset = PCM_ENCODE_WR_PTR_CH8_CH15_REG;
    pcm_encode_ptr->ch16_wr_ptr_value.value = 0;
    pcm_encode_ptr->ch16_wr_ptr_reg_offset = PCM_ENCODE_WR_PTR_CH16_REG;
    pcm_encode_ptr->ch0_ch7_rd_ptr_value.value = 0;
    pcm_encode_ptr->ch0_ch7_rd_ptr_reg_offset = PCM_ENCODE_RD_PTR_CH0_CH7_REG;
    pcm_encode_ptr->ch8_ch15_rd_ptr_value.value = 0;
    pcm_encode_ptr->ch8_ch15_rd_ptr_reg_offset = PCM_ENCODE_RD_PTR_CH8_CH15_REG;
    pcm_encode_ptr->ch16_rd_ptr_value.value = 0;
    pcm_encode_ptr->ch16_rd_ptr_reg_offset = PCM_ENCODE_RD_PTR_CH16_REG;
    pcm_encode_ptr->rd_finish_number_reg_offset = HOST_BLOCK_RD_PCM_FINISH_REG;
    pcm_encode_ptr->section_number = AUDIO_IN_PCM_CHAN_SECTION_NUMBER;
}

static void audio_pcm_encode_wr_rd_ptr_get_pcm_wr_rd_ptr(audio_pcm_encode_ptr_control_t *pcm_encode_ptr, isil_chip_t *chip)
{
    if(chip != NULL){
        pcm_encode_ptr->ch0_ch7_wr_ptr_value.value = chip->io_op->chip_read32(chip, pcm_encode_ptr->ch0_ch7_wr_ptr_reg_offset);
        pcm_encode_ptr->ch8_ch15_wr_ptr_value.value = chip->io_op->chip_read32(chip, pcm_encode_ptr->ch8_ch15_wr_ptr_reg_offset);
        pcm_encode_ptr->ch16_wr_ptr_value.value = chip->io_op->chip_read32(chip, pcm_encode_ptr->ch16_wr_ptr_reg_offset);

        pcm_encode_ptr->ch0_ch7_rd_ptr_value.value = chip->io_op->chip_read32(chip, pcm_encode_ptr->ch0_ch7_rd_ptr_reg_offset);
        pcm_encode_ptr->ch8_ch15_rd_ptr_value.value = chip->io_op->chip_read32(chip, pcm_encode_ptr->ch8_ch15_rd_ptr_reg_offset);
        pcm_encode_ptr->ch16_rd_ptr_value.value = chip->io_op->chip_read32(chip, pcm_encode_ptr->ch16_rd_ptr_reg_offset);

        //ISIL_DBG(ISIL_DBG_FATAL, "%08x, %08x\n", pcm_encode_ptr->ch0_ch7_wr_ptr_value.value, pcm_encode_ptr->ch0_ch7_rd_ptr_value.value);
    }
}

static int  audio_pcm_encode_wr_rd_ptr_get_chan_wr_ptr(audio_pcm_encode_ptr_control_t *pcm_encode_ptr, int chan_id, int *ext_flag)
{
    int	ret = INVALID_ISIL_AUDIO_SECTION_ID;
    switch(chan_id){
        case ISIL_AUDIO_IN_CHAN0_ID:
            ret = pcm_encode_ptr->ch0_ch7_wr_ptr_value.bit_value.audio_chan0_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN1_ID:
            ret = pcm_encode_ptr->ch0_ch7_wr_ptr_value.bit_value.audio_chan1_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN2_ID:
            ret = pcm_encode_ptr->ch0_ch7_wr_ptr_value.bit_value.audio_chan2_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN3_ID:
            ret = pcm_encode_ptr->ch0_ch7_wr_ptr_value.bit_value.audio_chan3_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN4_ID:
            ret = pcm_encode_ptr->ch0_ch7_wr_ptr_value.bit_value.audio_chan4_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN5_ID:
            ret = pcm_encode_ptr->ch0_ch7_wr_ptr_value.bit_value.audio_chan5_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN6_ID:
            ret = pcm_encode_ptr->ch0_ch7_wr_ptr_value.bit_value.audio_chan6_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN7_ID:
            ret = pcm_encode_ptr->ch0_ch7_wr_ptr_value.bit_value.audio_chan7_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN8_ID:
            ret = pcm_encode_ptr->ch8_ch15_wr_ptr_value.bit_value.audio_chan08_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN9_ID:
            ret = pcm_encode_ptr->ch8_ch15_wr_ptr_value.bit_value.audio_chan09_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN10_ID:
            ret = pcm_encode_ptr->ch8_ch15_wr_ptr_value.bit_value.audio_chan10_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN11_ID:
            ret = pcm_encode_ptr->ch8_ch15_wr_ptr_value.bit_value.audio_chan11_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN12_ID:
            ret = pcm_encode_ptr->ch8_ch15_wr_ptr_value.bit_value.audio_chan12_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN13_ID:
            ret = pcm_encode_ptr->ch8_ch15_wr_ptr_value.bit_value.audio_chan13_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN14_ID:
            ret = pcm_encode_ptr->ch8_ch15_wr_ptr_value.bit_value.audio_chan14_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN15_ID:
            ret = pcm_encode_ptr->ch8_ch15_wr_ptr_value.bit_value.audio_chan15_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN16_ID:
            ret = pcm_encode_ptr->ch16_wr_ptr_value.bit_value.audio_chan16_ptr;
            break;
        default:
            printk("%s.%d: err audio chan id %d\n", __FUNCTION__, __LINE__, chan_id);
            break;
    }
    *ext_flag = ret>>(get_power_base((u32)AUDIO_IN_PCM_CHAN_SECTION_NUMBER));
    ret &= ((1<<(get_power_base((u32)AUDIO_IN_PCM_CHAN_SECTION_NUMBER)))-1);
    return ret;
}

static int  audio_pcm_encode_wr_rd_ptr_get_chan_rd_ptr(audio_pcm_encode_ptr_control_t *pcm_encode_ptr, int chan_id, int *ext_flag)
{
    int	ret = INVALID_ISIL_AUDIO_SECTION_ID;
    switch(chan_id){
        case ISIL_AUDIO_IN_CHAN0_ID:
            ret = pcm_encode_ptr->ch0_ch7_rd_ptr_value.bit_value.audio_chan0_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN1_ID:
            ret = pcm_encode_ptr->ch0_ch7_rd_ptr_value.bit_value.audio_chan1_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN2_ID:
            ret = pcm_encode_ptr->ch0_ch7_rd_ptr_value.bit_value.audio_chan2_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN3_ID:
            ret = pcm_encode_ptr->ch0_ch7_rd_ptr_value.bit_value.audio_chan3_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN4_ID:
            ret = pcm_encode_ptr->ch0_ch7_rd_ptr_value.bit_value.audio_chan4_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN5_ID:
            ret = pcm_encode_ptr->ch0_ch7_rd_ptr_value.bit_value.audio_chan5_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN6_ID:
            ret = pcm_encode_ptr->ch0_ch7_rd_ptr_value.bit_value.audio_chan6_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN7_ID:
            ret = pcm_encode_ptr->ch0_ch7_rd_ptr_value.bit_value.audio_chan7_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN8_ID:
            ret = pcm_encode_ptr->ch8_ch15_rd_ptr_value.bit_value.audio_chan08_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN9_ID:
            ret = pcm_encode_ptr->ch8_ch15_rd_ptr_value.bit_value.audio_chan09_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN10_ID:
            ret = pcm_encode_ptr->ch8_ch15_rd_ptr_value.bit_value.audio_chan10_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN11_ID:
            ret = pcm_encode_ptr->ch8_ch15_rd_ptr_value.bit_value.audio_chan11_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN12_ID:
            ret = pcm_encode_ptr->ch8_ch15_rd_ptr_value.bit_value.audio_chan12_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN13_ID:
            ret = pcm_encode_ptr->ch8_ch15_rd_ptr_value.bit_value.audio_chan13_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN14_ID:
            ret = pcm_encode_ptr->ch8_ch15_rd_ptr_value.bit_value.audio_chan14_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN15_ID:
            ret = pcm_encode_ptr->ch8_ch15_rd_ptr_value.bit_value.audio_chan15_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN16_ID:
            ret = pcm_encode_ptr->ch16_rd_ptr_value.bit_value.audio_chan16_ptr;
            break;
        default:
            printk("%s.%d: err audio chan id %d\n", __FUNCTION__, __LINE__, chan_id);
            break;
    }
    *ext_flag = ret>>(get_power_base((u32)AUDIO_IN_PCM_CHAN_SECTION_NUMBER));
    ret &= ((1<<(get_power_base((u32)AUDIO_IN_PCM_CHAN_SECTION_NUMBER)))-1);
    return ret;
}

static int  audio_pcm_encode_wr_rd_get_chan_section_number(audio_pcm_encode_ptr_control_t *pcm_encode_ptr)
{
    return pcm_encode_ptr->section_number;
}

static void audio_pcm_encode_wr_rd_ptr_set_rd_finish_number(audio_pcm_encode_ptr_control_t *pcm_encode_ptr, int section_id, isil_chip_t *chip)
{
    chip->io_op->chip_write32(chip, pcm_encode_ptr->rd_finish_number_reg_offset, section_id);
}

static struct audio_pcm_encode_wr_rd_ptr_operation  audio_pcm_encode_wr_rd_ptr_op = {
    .init = audio_pcm_encode_wr_rd_ptr_init,
    .get_pcm_wr_rd_ptr = audio_pcm_encode_wr_rd_ptr_get_pcm_wr_rd_ptr,
    .get_chan_wr_ptr = audio_pcm_encode_wr_rd_ptr_get_chan_wr_ptr,
    .get_chan_rd_ptr = audio_pcm_encode_wr_rd_ptr_get_chan_rd_ptr,
    .get_chan_section_number = audio_pcm_encode_wr_rd_get_chan_section_number,
    .set_rd_finish_number = audio_pcm_encode_wr_rd_ptr_set_rd_finish_number,
};

void    init_audio_pcm_encode_ptr_control(audio_pcm_encode_ptr_control_t *pcm_encode_ptr)
{
    pcm_encode_ptr->op = &audio_pcm_encode_wr_rd_ptr_op;
    pcm_encode_ptr->op->init(pcm_encode_ptr);
}

/****************************************************************************************/
/*              pcm encode audio chip end addr calcuate operation table                 */
/****************************************************************************************/
static void audio_pcm_encode_data_addr_set_reg_bit_section_offset(audio_pcm_encode_data_addr_t *pcm_encode_data_addr, u32 section_offset)
{
    pcm_encode_data_addr->value.bit_value.section_offset = section_offset;
}

static u32  audio_pcm_encode_data_addr_get_reg_bit_section_offset(audio_pcm_encode_data_addr_t *pcm_encode_data_addr)
{
    return pcm_encode_data_addr->value.bit_value.section_offset;
}

static void audio_pcm_encode_data_addr_set_reg_bit_section_id(audio_pcm_encode_data_addr_t *pcm_encode_data_addr, u32 section_id)
{
    pcm_encode_data_addr->value.bit_value.section_id = section_id;
}

static u32  audio_pcm_encode_data_addr_get_reg_bit_section_id(audio_pcm_encode_data_addr_t *pcm_encode_data_addr)
{
    return pcm_encode_data_addr->value.bit_value.section_id;
}

static void audio_pcm_encode_data_addr_set_reg_bit_chan_id(audio_pcm_encode_data_addr_t *pcm_encode_data_addr, u32 chan_id)
{
    pcm_encode_data_addr->value.bit_value.chan_id = chan_id;
}

static u32  audio_pcm_encode_data_addr_get_reg_bit_chan_id(audio_pcm_encode_data_addr_t *pcm_encode_data_addr)
{
    return pcm_encode_data_addr->value.bit_value.chan_id;
}

static void audio_pcm_encode_data_addr_set_reg_bit_pcm_type(audio_pcm_encode_data_addr_t *pcm_encode_data_addr)
{
    pcm_encode_data_addr->value.bit_value.pcm_type = ADUIO_DATA_ADDR_PCM_ENCODE_TYPE;
}

static u32  audio_pcm_encode_data_addr_get_reg_bit_pcm_type(audio_pcm_encode_data_addr_t *pcm_encode_data_addr)
{
    return ADUIO_DATA_ADDR_PCM_ENCODE_TYPE;
}

static u32  audio_pcm_encode_data_addr_get_pcm_encode_data_addr(audio_pcm_encode_data_addr_t *pcm_encode_data_addr)
{
    return pcm_encode_data_addr->value.value;
}

static int  audio_pcm_encode_data_addr_get_buf_page_id(audio_pcm_encode_data_addr_t *pcm_encode_data_addr)
{
    return pcm_encode_data_addr->buf_page_id;
}

static int  audio_pcm_encode_data_addr_get_buf_in_chip_a_or_b(audio_pcm_encode_data_addr_t *pcm_encode_data_addr)
{
    return pcm_encode_data_addr->chip_a_or_b;
}

static u32  audio_pcm_encode_data_addr_get_pcm_encode_data_addr_in_host_end_offset(audio_pcm_encode_data_addr_t *pcm_encode_data_addr)
{
    u32 host_end_offset;
    host_end_offset = DDRBASE;
    host_end_offset |= (pcm_encode_data_addr->value.value<<2);
    return host_end_offset;
}

static u32  audio_pcm_encode_data_addr_get_pcm_encode_data_addr_in_ddr_end_offset(audio_pcm_encode_data_addr_t *pcm_encode_data_addr)
{
    u32	ddr_end_offset;
    ddr_end_offset = (pcm_encode_data_addr->buf_page_id<<19);
    ddr_end_offset |= (pcm_encode_data_addr->value.value<<2);
    return ddr_end_offset;
}

static void audio_pcm_encode_data_addr_init(audio_pcm_encode_data_addr_t *pcm_encode_data_addr, int page_id, int chip_a_or_b)
{
    pcm_encode_data_addr->value.value = 0;
    pcm_encode_data_addr->value.bit_value.pcm_type = ADUIO_DATA_ADDR_PCM_ENCODE_TYPE;
    pcm_encode_data_addr->buf_page_id = page_id;
    pcm_encode_data_addr->chip_a_or_b = chip_a_or_b;
}

static struct audio_pcm_encode_data_addr_operation  audio_pcm_encode_data_addr_op = {
    .set_reg_bit_section_offset = audio_pcm_encode_data_addr_set_reg_bit_section_offset,
    .get_reg_bit_section_offset = audio_pcm_encode_data_addr_get_reg_bit_section_offset,
    .set_reg_bit_section_id = audio_pcm_encode_data_addr_set_reg_bit_section_id,
    .get_reg_bit_section_id = audio_pcm_encode_data_addr_get_reg_bit_section_id,
    .set_reg_bit_chan_id = audio_pcm_encode_data_addr_set_reg_bit_chan_id,
    .get_reg_bit_chan_id = audio_pcm_encode_data_addr_get_reg_bit_chan_id,
    .set_reg_bit_pcm_type = audio_pcm_encode_data_addr_set_reg_bit_pcm_type,
    .get_reg_bit_pcm_type = audio_pcm_encode_data_addr_get_reg_bit_pcm_type,
    .get_pcm_encode_data_addr = audio_pcm_encode_data_addr_get_pcm_encode_data_addr,
    .get_buf_page_id = audio_pcm_encode_data_addr_get_buf_page_id,
    .get_buf_in_chip_a_or_b = audio_pcm_encode_data_addr_get_buf_in_chip_a_or_b,
    .get_pcm_encode_data_addr_in_host_end_offset = audio_pcm_encode_data_addr_get_pcm_encode_data_addr_in_host_end_offset,
    .get_pcm_encode_data_addr_in_ddr_end_offset = audio_pcm_encode_data_addr_get_pcm_encode_data_addr_in_ddr_end_offset,
    .init = audio_pcm_encode_data_addr_init,
};

void	init_audio_pcm_encode_data_addr(audio_pcm_encode_data_addr_t *pcm_encode_data_addr)
{
    if(pcm_encode_data_addr != NULL){
        pcm_encode_data_addr->op = &audio_pcm_encode_data_addr_op;
        pcm_encode_data_addr->op->init(pcm_encode_data_addr, ISIL_AUDIO_BUF_PAGE_ID, ISIL_AUDIO_BUF_OF_DDR_ID);
    }
}


/****************************************************************************************/
/*                  adpcm audio encode read&write ptr operation table                   */
/****************************************************************************************/
static void audio_adpcm_encode_wr_rd_ptr_init(audio_adpcm_encode_ptr_control_t *adpcm_encode_ptr)
{
    adpcm_encode_ptr->ch0_ch9_wr_ptr_value.value = 0;
    adpcm_encode_ptr->ch0_ch9_wr_ptr_reg_offset = ADPCM_ENCODE_WR_PTR_CH0_CH9_REG;
    adpcm_encode_ptr->ch10_ch16_wr_ptr_value.value = 0;
    adpcm_encode_ptr->ch10_ch16_wr_ptr_reg_offset = ADPCM_ENCODE_WR_PTR_CH10_CH16_REG;

    adpcm_encode_ptr->ch0_ch9_rd_ptr_value.value = 0;
    adpcm_encode_ptr->ch0_ch9_rd_ptr_reg_offset = ADPCM_ENCODE_RD_PTR_CH0_CH9_REG;
    adpcm_encode_ptr->ch10_ch16_rd_ptr_value.value = 0;
    adpcm_encode_ptr->ch10_ch16_rd_ptr_reg_offset = ADPCM_ENCODE_RD_PTR_CH10_CH16_REG;

    adpcm_encode_ptr->rd_finish_number_reg_offset = HOST_BLOCK_RD_ADPCM_FINISH_REG;
    adpcm_encode_ptr->section_number = AUDIO_IN_ADPCM_CHAN_SECTION_NUMBER;
}

static void audio_adpcm_encode_wr_rd_ptr_get_adpcm_wr_rd_ptr(audio_adpcm_encode_ptr_control_t *adpcm_encode_ptr, isil_chip_t *chip)
{
    if(chip != NULL){
        adpcm_encode_ptr->ch0_ch9_wr_ptr_value.value = chip->io_op->chip_read32(chip, adpcm_encode_ptr->ch0_ch9_wr_ptr_reg_offset);
        adpcm_encode_ptr->ch10_ch16_wr_ptr_value.value = chip->io_op->chip_read32(chip, adpcm_encode_ptr->ch10_ch16_wr_ptr_reg_offset);

        adpcm_encode_ptr->ch0_ch9_rd_ptr_value.value = chip->io_op->chip_read32(chip, adpcm_encode_ptr->ch0_ch9_rd_ptr_reg_offset);
        adpcm_encode_ptr->ch10_ch16_rd_ptr_value.value = chip->io_op->chip_read32(chip, adpcm_encode_ptr->ch10_ch16_rd_ptr_reg_offset);
    }
}

static int  audio_adpcm_encode_wr_rd_ptr_get_chan_wr_ptr(audio_adpcm_encode_ptr_control_t *adpcm_encode_ptr, int chan_id, int *ext_flag)
{
    int	ret = INVALID_ISIL_AUDIO_SECTION_ID;
    switch(chan_id){
        case ISIL_AUDIO_IN_CHAN0_ID:
            ret = adpcm_encode_ptr->ch0_ch9_wr_ptr_value.bit_value.audio_chan0_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN1_ID:
            ret = adpcm_encode_ptr->ch0_ch9_wr_ptr_value.bit_value.audio_chan1_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN2_ID:
            ret = adpcm_encode_ptr->ch0_ch9_wr_ptr_value.bit_value.audio_chan2_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN3_ID:
            ret = adpcm_encode_ptr->ch0_ch9_wr_ptr_value.bit_value.audio_chan3_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN4_ID:
            ret = adpcm_encode_ptr->ch0_ch9_wr_ptr_value.bit_value.audio_chan4_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN5_ID:
            ret = adpcm_encode_ptr->ch0_ch9_wr_ptr_value.bit_value.audio_chan5_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN6_ID:
            ret = adpcm_encode_ptr->ch0_ch9_wr_ptr_value.bit_value.audio_chan6_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN7_ID:
            ret = adpcm_encode_ptr->ch0_ch9_wr_ptr_value.bit_value.audio_chan7_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN8_ID:
            ret = adpcm_encode_ptr->ch0_ch9_wr_ptr_value.bit_value.audio_chan8_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN9_ID:
            ret = adpcm_encode_ptr->ch0_ch9_wr_ptr_value.bit_value.audio_chan9_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN10_ID:
            ret = adpcm_encode_ptr->ch10_ch16_wr_ptr_value.bit_value.audio_chan10_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN11_ID:
            ret = adpcm_encode_ptr->ch10_ch16_wr_ptr_value.bit_value.audio_chan11_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN12_ID:
            ret = adpcm_encode_ptr->ch10_ch16_wr_ptr_value.bit_value.audio_chan12_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN13_ID:
            ret = adpcm_encode_ptr->ch10_ch16_wr_ptr_value.bit_value.audio_chan13_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN14_ID:
            ret = adpcm_encode_ptr->ch10_ch16_wr_ptr_value.bit_value.audio_chan14_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN15_ID:
            ret = adpcm_encode_ptr->ch10_ch16_wr_ptr_value.bit_value.audio_chan15_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN16_ID:
            ret = adpcm_encode_ptr->ch10_ch16_wr_ptr_value.bit_value.audio_chan16_ptr;
            break;
        default:
            printk("%s.%d: err audio chan id %d\n", __FUNCTION__, __LINE__, chan_id);
            break;
    }
    *ext_flag = ret>>(get_power_base((u32)AUDIO_IN_ADPCM_CHAN_SECTION_NUMBER));
    ret &= ((1<<(get_power_base((u32)AUDIO_IN_ADPCM_CHAN_SECTION_NUMBER)))-1);
    return ret;
}

static int  audio_adpcm_encode_wr_rd_ptr_get_chan_rd_ptr(audio_adpcm_encode_ptr_control_t *adpcm_encode_ptr, int chan_id, int *ext_flag)
{
    int	ret = INVALID_ISIL_AUDIO_SECTION_ID;
    switch(chan_id){
        case ISIL_AUDIO_IN_CHAN0_ID:
            ret = adpcm_encode_ptr->ch0_ch9_rd_ptr_value.bit_value.audio_chan0_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN1_ID:
            ret = adpcm_encode_ptr->ch0_ch9_rd_ptr_value.bit_value.audio_chan1_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN2_ID:
            ret = adpcm_encode_ptr->ch0_ch9_rd_ptr_value.bit_value.audio_chan2_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN3_ID:
            ret = adpcm_encode_ptr->ch0_ch9_rd_ptr_value.bit_value.audio_chan3_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN4_ID:
            ret = adpcm_encode_ptr->ch0_ch9_rd_ptr_value.bit_value.audio_chan4_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN5_ID:
            ret = adpcm_encode_ptr->ch0_ch9_rd_ptr_value.bit_value.audio_chan5_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN6_ID:
            ret = adpcm_encode_ptr->ch0_ch9_rd_ptr_value.bit_value.audio_chan6_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN7_ID:
            ret = adpcm_encode_ptr->ch0_ch9_rd_ptr_value.bit_value.audio_chan7_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN8_ID:
            ret = adpcm_encode_ptr->ch0_ch9_rd_ptr_value.bit_value.audio_chan8_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN9_ID:
            ret = adpcm_encode_ptr->ch0_ch9_rd_ptr_value.bit_value.audio_chan9_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN10_ID:
            ret = adpcm_encode_ptr->ch10_ch16_rd_ptr_value.bit_value.audio_chan10_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN11_ID:
            ret = adpcm_encode_ptr->ch10_ch16_rd_ptr_value.bit_value.audio_chan11_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN12_ID:
            ret = adpcm_encode_ptr->ch10_ch16_rd_ptr_value.bit_value.audio_chan12_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN13_ID:
            ret = adpcm_encode_ptr->ch10_ch16_rd_ptr_value.bit_value.audio_chan13_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN14_ID:
            ret = adpcm_encode_ptr->ch10_ch16_rd_ptr_value.bit_value.audio_chan14_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN15_ID:
            ret = adpcm_encode_ptr->ch10_ch16_rd_ptr_value.bit_value.audio_chan15_ptr;
            break;
        case ISIL_AUDIO_IN_CHAN16_ID:
            ret = adpcm_encode_ptr->ch10_ch16_rd_ptr_value.bit_value.audio_chan16_ptr;
            break;
        default:
            printk("%s.%d: err audio chan id %d\n", __FUNCTION__, __LINE__, chan_id);
            break;
    }
    *ext_flag = ret>>(get_power_base((u32)AUDIO_IN_ADPCM_CHAN_SECTION_NUMBER));
    ret &= ((1<<(get_power_base((u32)AUDIO_IN_ADPCM_CHAN_SECTION_NUMBER)))-1);
    return ret;
}

static int  audio_adpcm_encode_wr_rd_ptr_get_chan_section_number(audio_adpcm_encode_ptr_control_t *adpcm_encode_ptr)
{
    return adpcm_encode_ptr->section_number;
}

static void audio_adpcm_encode_wr_rd_ptr_set_rd_finish_number(audio_adpcm_encode_ptr_control_t *adpcm_encode_ptr, int section_id, isil_chip_t *chip)
{
    if(chip != NULL){
        chip->io_op->chip_write32(chip, adpcm_encode_ptr->rd_finish_number_reg_offset, section_id);
    }
}

static struct audio_adpcm_encode_wr_rd_ptr_operation	audio_adpcm_encode_wr_rd_ptr_op = {
    .init = audio_adpcm_encode_wr_rd_ptr_init,
    .get_adpcm_wr_rd_ptr = audio_adpcm_encode_wr_rd_ptr_get_adpcm_wr_rd_ptr,

    .get_chan_wr_ptr = audio_adpcm_encode_wr_rd_ptr_get_chan_wr_ptr,
    .get_chan_rd_ptr = audio_adpcm_encode_wr_rd_ptr_get_chan_rd_ptr,
    .get_chan_section_number = audio_adpcm_encode_wr_rd_ptr_get_chan_section_number,
    .set_rd_finish_number = audio_adpcm_encode_wr_rd_ptr_set_rd_finish_number,
};

void	init_audio_adpcm_encode_ptr_control(audio_adpcm_encode_ptr_control_t *adpcm_encode_ptr)
{
    adpcm_encode_ptr->op = &audio_adpcm_encode_wr_rd_ptr_op;
    adpcm_encode_ptr->op->init(adpcm_encode_ptr);
}

/****************************************************************************************/
/*              adpcm encode audio chip end addr calcuate operation table               */
/****************************************************************************************/
static void audio_adpcm_encode_data_addr_set_reg_bit_section_offset(audio_adpcm_encode_data_addr_t *adpcm_encode_data_addr, u32 section_offset)
{
    adpcm_encode_data_addr->value.bit_value.section_offset = section_offset;
}

static u32  audio_adpcm_encode_data_addr_get_reg_bit_section_offset(audio_adpcm_encode_data_addr_t *adpcm_encode_data_addr)
{
    return adpcm_encode_data_addr->value.bit_value.section_offset;
}

static void audio_adpcm_encode_data_addr_set_reg_bit_section_id(audio_adpcm_encode_data_addr_t *adpcm_encode_data_addr, u32 section_id)
{
    adpcm_encode_data_addr->value.bit_value.section_id = section_id;
}

static u32  audio_adpcm_encode_data_addr_get_reg_bit_section_id(audio_adpcm_encode_data_addr_t *adpcm_encode_data_addr)
{
    return adpcm_encode_data_addr->value.bit_value.section_id;
}

static void audio_adpcm_encode_data_addr_set_reg_bit_chan_id(audio_adpcm_encode_data_addr_t *adpcm_encode_data_addr, u32 chan_id)
{
    adpcm_encode_data_addr->value.bit_value.chan_id = chan_id;
}

static u32  audio_adpcm_encode_data_addr_get_reg_bit_chan_id(audio_adpcm_encode_data_addr_t *adpcm_encode_data_addr)
{
    return adpcm_encode_data_addr->value.bit_value.section_id;
}

static void audio_adpcm_encode_data_addr_set_reg_bit_adpcm_type(audio_adpcm_encode_data_addr_t *adpcm_encode_data_addr)
{
    adpcm_encode_data_addr->value.bit_value.adpcm_encode_type = ADUIO_DATA_ADDR_ADPCM_ENCODE_TYPE;
}

static u32  audio_adpcm_encode_data_addr_get_reg_bit_adpcm_type(audio_adpcm_encode_data_addr_t *adpcm_encode_data_addr)
{
    return adpcm_encode_data_addr->value.bit_value.adpcm_encode_type;
}

static u32  audio_adpcm_encode_data_addr_get_adpcm_encode_data_addr(audio_adpcm_encode_data_addr_t *adpcm_encode_data_addr)
{
    return adpcm_encode_data_addr->value.value;
}

static int  audio_adpcm_encode_data_addr_get_buf_page_id(audio_adpcm_encode_data_addr_t *adpcm_encode_data_addr)
{
    return adpcm_encode_data_addr->buf_page_id;
}

static int  audio_adpcm_encode_data_addr_get_buf_in_chip_a_or_b(audio_adpcm_encode_data_addr_t *adpcm_encode_data_addr)
{
    return adpcm_encode_data_addr->chip_a_or_b;
}

static u32  audio_adpcm_encode_data_addr_get_adpcm_encode_data_addr_in_host_end_offset(audio_adpcm_encode_data_addr_t *adpcm_encode_data_addr)
{
    u32	host_end_offset;
    host_end_offset = DDRBASE;
    host_end_offset |= (adpcm_encode_data_addr->value.value<<2);
    return host_end_offset;
}

static u32  audio_adpcm_encode_data_addr_get_adpcm_encode_data_addr_in_ddr_end_offset(audio_adpcm_encode_data_addr_t *adpcm_encode_data_addr)
{
    u32	ddr_end_offset;
    ddr_end_offset = (adpcm_encode_data_addr->buf_page_id<<19);
    ddr_end_offset |= (adpcm_encode_data_addr->value.value<<2);
    return ddr_end_offset;
}

static void audio_adpcm_encode_data_addr_init(audio_adpcm_encode_data_addr_t *adpcm_encode_data_addr, int page_id, int chip_a_or_b)
{
    adpcm_encode_data_addr->value.value = 0;
    adpcm_encode_data_addr->value.bit_value.adpcm_encode_type = ADUIO_DATA_ADDR_ADPCM_ENCODE_TYPE;
    adpcm_encode_data_addr->buf_page_id = page_id;
    adpcm_encode_data_addr->chip_a_or_b = chip_a_or_b;
}

static struct audio_adpcm_encode_data_addr_operation	audio_adpcm_encode_data_addr_op = {
    .set_reg_bit_section_offset = audio_adpcm_encode_data_addr_set_reg_bit_section_offset,
    .get_reg_bit_section_offset = audio_adpcm_encode_data_addr_get_reg_bit_section_offset,
    .set_reg_bit_section_id = audio_adpcm_encode_data_addr_set_reg_bit_section_id,
    .get_reg_bit_section_id = audio_adpcm_encode_data_addr_get_reg_bit_section_id,
    .set_reg_bit_chan_id = audio_adpcm_encode_data_addr_set_reg_bit_chan_id,
    .get_reg_bit_chan_id = audio_adpcm_encode_data_addr_get_reg_bit_chan_id,
    .set_reg_bit_adpcm_type = audio_adpcm_encode_data_addr_set_reg_bit_adpcm_type,
    .get_reg_bit_adpcm_type = audio_adpcm_encode_data_addr_get_reg_bit_adpcm_type,
    .get_adpcm_encode_data_addr = audio_adpcm_encode_data_addr_get_adpcm_encode_data_addr,
    .get_buf_page_id = audio_adpcm_encode_data_addr_get_buf_page_id,
    .get_buf_in_chip_a_or_b = audio_adpcm_encode_data_addr_get_buf_in_chip_a_or_b,
    .get_adpcm_encode_data_addr_in_host_end_offset = audio_adpcm_encode_data_addr_get_adpcm_encode_data_addr_in_host_end_offset,
    .get_adpcm_encode_data_addr_in_ddr_end_offset = audio_adpcm_encode_data_addr_get_adpcm_encode_data_addr_in_ddr_end_offset,
    .init = audio_adpcm_encode_data_addr_init,
};

void    init_audio_adpcm_encode_data_addr(audio_adpcm_encode_data_addr_t *adpcm_encode_data_addr)
{
    adpcm_encode_data_addr->op = &audio_adpcm_encode_data_addr_op;
    adpcm_encode_data_addr->op->init(adpcm_encode_data_addr, ISIL_AUDIO_BUF_PAGE_ID, ISIL_AUDIO_BUF_OF_DDR_ID);
}

/****************************************************************************************/
/*              encode audio work mode control operation table                          */
/****************************************************************************************/
static void audio_encode_control_enable_pcm_chan(audio_encode_control_t *encode_control, int phy_chan_id)
{
    if(phy_chan_id < ISIL_PHY_AUDIO_IN_CHAN_NUMBER){
        encode_control->pcm_value.value &= ~(AUDIO_DISABLE<<phy_chan_id);
        encode_control->pcm_value.value |= (AUDIO_ENABLE<<phy_chan_id);
    }
}

static void audio_encode_control_disable_pcm_chan(audio_encode_control_t *encode_control, int phy_chan_id)
{
    if(phy_chan_id < ISIL_PHY_AUDIO_IN_CHAN_NUMBER){
        encode_control->pcm_value.value &= ~(AUDIO_ENABLE<<phy_chan_id);
        encode_control->pcm_value.value |= (AUDIO_DISABLE<<phy_chan_id);
    }
}

static void audio_encode_control_set_enable_pcm_chan(audio_encode_control_t *encode_control, int phy_chan_id, int en, int mode, isil_chip_t *chip)
{
    if(phy_chan_id < ISIL_PHY_AUDIO_IN_CHAN_NUMBER){
        if(en == AUDIO_ENABLE){
            encode_control->pcm_value.value &= ~(AUDIO_DISABLE<<phy_chan_id);
            encode_control->pcm_value.value |= (AUDIO_ENABLE<<phy_chan_id);
        } else {
            encode_control->pcm_value.value &= ~(AUDIO_ENABLE<<phy_chan_id);
            encode_control->pcm_value.value |= (AUDIO_DISABLE<<phy_chan_id);
        }
        if(mode == READ_BITSTREAM_BY_PCI){
            encode_control->pcm_value.bit_value.arm_mode_or_pci_mod = 1;
        } else {
            encode_control->pcm_value.bit_value.arm_mode_or_pci_mod = 0;
        }
        if(chip != NULL){
            chip->io_op->chip_write32(chip, encode_control->pcm_reg_offset, encode_control->pcm_value.value);
        }
    }
}

static int  audio_encode_control_get_pcm_chan_state(audio_encode_control_t *encode_control, int phy_chan_id)
{
    int	ret = AUDIO_DISABLE;
    if(phy_chan_id < ISIL_PHY_AUDIO_IN_CHAN_NUMBER){
        if(phy_chan_id>=0){
            ret = (((encode_control->pcm_value.value)>>phy_chan_id)&0x1);
        } else {
            printk("%s.%d: impossible, err chan_id = %d\n", __FILE__, __LINE__, phy_chan_id);
        }
    }
    return ret;
}

static void audio_encode_control_init_audio_pcm(audio_encode_control_t *encode_control)
{
    encode_control->pcm_value.value = 0;
    encode_control->pcm_reg_offset = AUDIO_IN_ORIG_EN_CONTROL_REG;
}

static void audio_encode_control_enable_adpcm_chan(audio_encode_control_t *encode_control, int logic_chan_id)
{
    if(logic_chan_id <= ISIL_AUDIO_IN_CHAN16_ID){
        encode_control->adpcm_value.value &= ~(AUDIO_DISABLE<<logic_chan_id);
        encode_control->adpcm_value.value |= (AUDIO_ENABLE<<logic_chan_id);
    }
}

static void audio_encode_control_disable_adpcm_chan(audio_encode_control_t *encode_control, int logic_chan_id)
{
    if(logic_chan_id <= ISIL_AUDIO_IN_CHAN16_ID){
        encode_control->adpcm_value.value &= ~(AUDIO_ENABLE<<logic_chan_id);
        encode_control->adpcm_value.value |= (AUDIO_DISABLE<<logic_chan_id);
    }
}

static void audio_encode_control_set_enable_adpcm_chan(audio_encode_control_t *encode_control, int logic_chan_id, int en, isil_chip_t *chip)
{
    if(logic_chan_id <= ISIL_AUDIO_IN_CHAN16_ID){
        if(en == AUDIO_ENABLE){
            encode_control->adpcm_value.value &= ~(AUDIO_DISABLE<<logic_chan_id);
            encode_control->adpcm_value.value |= (AUDIO_ENABLE<<logic_chan_id);
        } else {
            encode_control->adpcm_value.value &= ~(AUDIO_ENABLE<<logic_chan_id);
            encode_control->adpcm_value.value |= (AUDIO_DISABLE<<logic_chan_id);
        }
        if(chip != NULL){
            chip->io_op->chip_write32(chip, encode_control->adpcm_reg_offset, encode_control->adpcm_value.value);
        }
    }
}

static int  audio_encode_control_get_adpcm_chan_state(audio_encode_control_t *encode_control, int logic_chan_id)
{
    int	ret = AUDIO_DISABLE;
    if(logic_chan_id <= ISIL_AUDIO_IN_CHAN16_ID){
        if(logic_chan_id>=0){
            ret = (((encode_control->adpcm_value.value)>>logic_chan_id)&0x1);
        } else {
            printk("%s.%d: impossible, err chan_id = %d\n", __FILE__, __LINE__, logic_chan_id);
        }
    }
    return ret;
}

static void audio_encode_control_init_audio_adpcm(audio_encode_control_t *encode_control)
{
    encode_control->adpcm_value.value = 0;
    encode_control->adpcm_reg_offset = AUDIO_IN_ADPCM_EN_CONTROL_REG;
}

static void audio_encode_control_update_audio_sample_rate(audio_encode_control_t *encode_control, int sample_rate)
{
    encode_control->pcm_value.bit_value.audio_sample_rate = sample_rate;
}

static int  audio_encode_control_get_audio_sample_rate(audio_encode_control_t *encode_control)
{
    return encode_control->pcm_value.bit_value.audio_sample_rate;
}

static void audio_encode_control_set_audio_sample_rate(audio_encode_control_t *encode_control, int sample_rate, isil_chip_t *chip)
{
    encode_control->pcm_value.bit_value.audio_sample_rate = sample_rate;
    if(chip != NULL){
        chip->io_op->chip_write32(chip, encode_control->pcm_reg_offset, encode_control->pcm_value.value);
    }
}

static void audio_encode_control_update_audio_type(audio_encode_control_t *encode_control, int audio_type)
{
    encode_control->pcm_value.bit_value.audio_type = audio_type;
}

static int  audio_encode_control_get_audio_type(audio_encode_control_t *encode_control)
{
    return encode_control->pcm_value.bit_value.audio_type;
}

static void audio_encode_control_set_audio_type(audio_encode_control_t *encode_control, int audio_type, isil_chip_t *chip)
{
    encode_control->pcm_value.bit_value.audio_type = audio_type;
    if(chip != NULL){
        chip->io_op->chip_write32(chip, encode_control->pcm_reg_offset, encode_control->pcm_value.value);
    }
}

static void audio_encode_control_update_bit_wide(audio_encode_control_t *encode_control, int bit_wide)
{
    encode_control->pcm_value.bit_value.AD_BIT_WIDE = bit_wide;
}

static int  audio_encode_control_get_bit_wide(audio_encode_control_t *encode_control)
{
    return encode_control->pcm_value.bit_value.AD_BIT_WIDE;
}

static void audio_encode_control_set_bit_wide(audio_encode_control_t *encode_control, int bit_wide, isil_chip_t *chip)
{
    encode_control->pcm_value.bit_value.AD_BIT_WIDE = bit_wide;
    if(chip != NULL){
        chip->io_op->chip_write32(chip, encode_control->pcm_reg_offset, encode_control->pcm_value.value);
    }
}

static void	audio_encode_control_pci_sel_pcm_or_adpcm(audio_encode_control_t *encode_control, int logic_chan_id, int type)
{
    if(type == ISIL_AUDIO_PCM){
        encode_control->sel_pcm_or_adpcm_based_pci_push.value &= ~(0xffff);
    } else {
        encode_control->sel_pcm_or_adpcm_based_pci_push.value |= (0xffff);
    }
}

static void	audio_encode_control_set_pci_sel_pcm_or_adpcm(audio_encode_control_t *encode_control, int logic_chan_id, int type, isil_chip_t *chip)
{
    if(type == ISIL_AUDIO_PCM){
        encode_control->sel_pcm_or_adpcm_based_pci_push.value &= ~(0xffff);
    } else {
        encode_control->sel_pcm_or_adpcm_based_pci_push.value |= (0xffff);
    }

    if(chip != NULL){
        chip->io_op->chip_write32(chip, encode_control->sel_pci_pcm_adpcm_reg_offset, encode_control->sel_pcm_or_adpcm_based_pci_push.value);
    }
}

static void     audio_encode_control_set_toggle_pci_flowcn(audio_encode_control_t *encode_control, isil_chip_t *chip)
{
    if(chip != NULL){
        encode_control->sel_pcm_or_adpcm_based_pci_push.bit_value.pci_audio_flow_en = 1;
        encode_control->sel_pcm_or_adpcm_based_pci_push.bit_value.pci_audio_frame_toggle = 1;
        chip->io_op->chip_write32(chip, encode_control->sel_pci_pcm_adpcm_reg_offset, encode_control->sel_pcm_or_adpcm_based_pci_push.value);
        encode_control->sel_pcm_or_adpcm_based_pci_push.bit_value.pci_audio_frame_toggle = 0;
        chip->io_op->chip_write32(chip, encode_control->sel_pci_pcm_adpcm_reg_offset, encode_control->sel_pcm_or_adpcm_based_pci_push.value);
    }
}

static void	audio_encode_control_init_audio_pci_sel(audio_encode_control_t *encode_control)
{
    encode_control->sel_pci_pcm_adpcm_reg_offset = PCI_AUDIO_CHAN_SELECT_REG;
    encode_control->sel_pcm_or_adpcm_based_pci_push.value = 0xffff;
    encode_control->sel_pcm_or_adpcm_based_pci_push.bit_value.pci_audio_flow_en = 1;
}

static void audio_encode_control_set_audio_param(audio_encode_control_t *encode_control, isil_chip_t *chip)
{
    if(chip != NULL){
        chip->io_op->chip_write32(chip, encode_control->pcm_reg_offset, encode_control->pcm_value.value);
        chip->io_op->chip_write32(chip, encode_control->adpcm_reg_offset, encode_control->adpcm_value.value);
        chip->io_op->chip_write32(chip, encode_control->sel_pci_pcm_adpcm_reg_offset, encode_control->sel_pcm_or_adpcm_based_pci_push.value);
    }
}

static void audio_encode_control_get_audio_param(audio_encode_control_t *encode_control, isil_chip_t *chip)
{
    if(chip != NULL){
        //encode_control->pcm_value.value = chip->io_op->chip_read32(chip, encode_control->pcm_reg_offset);
        //encode_control->adpcm_value.value = chip->io_op->chip_read32(chip, encode_control->adpcm_reg_offset);
    }
}

static struct audio_encode_control_operation    audio_encode_control_op = {
    .enable_pcm_chan = audio_encode_control_enable_pcm_chan,
    .disable_pcm_chan = audio_encode_control_disable_pcm_chan,
    .set_enable_pcm_chan = audio_encode_control_set_enable_pcm_chan,
    .get_pcm_chan_state = audio_encode_control_get_pcm_chan_state,
    .init_audio_pcm = audio_encode_control_init_audio_pcm,

    .enable_adpcm_chan = audio_encode_control_enable_adpcm_chan,
    .disable_adpcm_chan = audio_encode_control_disable_adpcm_chan,
    .set_enable_adpcm_chan = audio_encode_control_set_enable_adpcm_chan,
    .get_adpcm_chan_state = audio_encode_control_get_adpcm_chan_state,
    .init_audio_adpcm = audio_encode_control_init_audio_adpcm,

    .update_audio_sample_rate = audio_encode_control_update_audio_sample_rate,
    .get_audio_sample_rate = audio_encode_control_get_audio_sample_rate,
    .set_audio_sample_rate = audio_encode_control_set_audio_sample_rate,
    .update_audio_type = audio_encode_control_update_audio_type,
    .get_audio_type = audio_encode_control_get_audio_type,
    .set_audio_type = audio_encode_control_set_audio_type,
    .update_bit_wide = audio_encode_control_update_bit_wide,
    .get_bit_wide= audio_encode_control_get_bit_wide,
    .set_bit_wide = audio_encode_control_set_bit_wide,

    .pci_sel_pcm_or_adpcm = audio_encode_control_pci_sel_pcm_or_adpcm,
    .set_enable_pci_sel_pcm_or_adpcm = audio_encode_control_set_pci_sel_pcm_or_adpcm,
    .toggle_pci_flowcn = audio_encode_control_set_toggle_pci_flowcn,
    .init_audio_pci_sel = audio_encode_control_init_audio_pci_sel,

    .set_audio_param = audio_encode_control_set_audio_param,
    .get_audio_param = audio_encode_control_get_audio_param,
};

void	init_audio_encode_control(audio_encode_control_t *encode_control)
{
    encode_control->op = &audio_encode_control_op;
    encode_control->op->init_audio_pcm(encode_control);
    encode_control->op->init_audio_adpcm(encode_control);
    encode_control->op->init_audio_pci_sel(encode_control);
}

/****************************************************************************************/
/*              decode audio work mode control operation table                          */
/****************************************************************************************/
static void audio_decode_control_init(audio_decode_control_t *decode_control)
{
    decode_control->value.value = 0;
    decode_control->reg_offset = AUDIO_OUT_CONTROL_REG;
}

static void audio_decode_control_update_decode_chan_mute(audio_decode_control_t *decode_control, int chan_id, int mute)
{
    switch(chan_id){
        case ISIL_AUDIO_OUT_SPEAKER_ID:
            decode_control->value.bit_value.audio_out_chan0_mute = mute;
            break;
        case ISIL_AUDIO_OUT_PLAYBACK_ID:
            decode_control->value.bit_value.audio_out_chan1_mute = mute;
            break;
        default:
            break;
    }
}

static int  audio_decode_control_get_decode_chan_mute(audio_decode_control_t *decode_control, int chan_id)
{
    int	ret = AUDIO_MUTE_ENABLE;
    switch(chan_id){
        case ISIL_AUDIO_OUT_SPEAKER_ID:
            ret = decode_control->value.bit_value.audio_out_chan0_mute;
            break;
        case ISIL_AUDIO_OUT_PLAYBACK_ID:
            ret = decode_control->value.bit_value.audio_out_chan1_mute;
            break;
        default:
            break;
    }
    return ret;
}

static void audio_decode_control_set_decode_chan_mute(audio_decode_control_t *decode_control, int chan_id, int mute, isil_chip_t *chip)
{
    if(chip != NULL){
        switch(chan_id){
            case ISIL_AUDIO_OUT_SPEAKER_ID:
                decode_control->value.bit_value.audio_out_chan0_mute = mute;
                break;
            case ISIL_AUDIO_OUT_PLAYBACK_ID:
                decode_control->value.bit_value.audio_out_chan1_mute = mute;
                break;
            default:
                return;
        }
        chip->io_op->chip_write32(chip, decode_control->reg_offset, decode_control->value.value);
    }
}

static void audio_decode_control_update_decode_chan_bit_wide(audio_decode_control_t *decode_control, int chan_id, int bit_wide)
{
    switch(chan_id){
        case ISIL_AUDIO_OUT_SPEAKER_ID:
            decode_control->value.bit_value.audio_out_chan0_DA_BIT_WIDE = bit_wide;
            break;
        case ISIL_AUDIO_OUT_PLAYBACK_ID:
            decode_control->value.bit_value.audio_out_chan1_DA_BIT_WIDE = bit_wide;
            break;
        default:
            break;
    }
}

static int  audio_decode_control_get_decode_chan_bit_wide(audio_decode_control_t *decode_control, int chan_id)
{
    int	ret = ISIL_AUDIO_16BIT;
    switch(chan_id){
        case ISIL_AUDIO_OUT_SPEAKER_ID:
            ret = decode_control->value.bit_value.audio_out_chan0_DA_BIT_WIDE;
            break;
        case ISIL_AUDIO_OUT_PLAYBACK_ID:
            ret = decode_control->value.bit_value.audio_out_chan1_DA_BIT_WIDE;
            break;
        default:
            break;
    }
    return ret;
}

static void audio_decode_control_set_decode_chan_bit_wide(audio_decode_control_t *decode_control, int chan_id, int bit_wide, isil_chip_t *chip)
{
    if(chip != NULL){
        switch(chan_id){
            case ISIL_AUDIO_OUT_SPEAKER_ID:
                decode_control->value.bit_value.audio_out_chan0_DA_BIT_WIDE = bit_wide;
                break;
            case ISIL_AUDIO_OUT_PLAYBACK_ID:
                decode_control->value.bit_value.audio_out_chan1_DA_BIT_WIDE = bit_wide;
                break;
            default:
                return;
        }
        chip->io_op->chip_write32(chip, decode_control->reg_offset, decode_control->value.value);
    }
}

static void audio_decode_control_update_decode_chan_enable(audio_decode_control_t *decode_control, int chan_id, int enable)
{
    switch(chan_id){
        case ISIL_AUDIO_OUT_SPEAKER_ID:
            decode_control->value.bit_value.audio_out_chan0_enable = enable;
            break;
        case ISIL_AUDIO_OUT_PLAYBACK_ID:
            decode_control->value.bit_value.audio_out_chan1_enable = enable;
            break;
        default:
            break;
    }
}

static int  audio_decode_control_get_decode_chan_enable(audio_decode_control_t *decode_control, int chan_id)
{
    int	ret = AUDIO_DISABLE;
    switch(chan_id){
        case ISIL_AUDIO_OUT_SPEAKER_ID:
            ret = decode_control->value.bit_value.audio_out_chan0_enable;
            break;
        case ISIL_AUDIO_OUT_PLAYBACK_ID:
            ret = decode_control->value.bit_value.audio_out_chan1_enable;
            break;
        default:
            break;
    }
    return ret;
}

static void audio_decode_control_set_decode_chan_enable(audio_decode_control_t *decode_control, int chan_id, int enable, isil_chip_t *chip)
{
    if(chip != NULL){
        switch(chan_id){
            case ISIL_AUDIO_OUT_SPEAKER_ID:
                decode_control->value.bit_value.audio_out_chan0_enable = enable;
                break;
            case ISIL_AUDIO_OUT_PLAYBACK_ID:
                decode_control->value.bit_value.audio_out_chan1_enable = enable;
                break;
            default:
                return;
        }
        chip->io_op->chip_write32(chip, decode_control->reg_offset, decode_control->value.value);
    }
}

static void audio_decode_control_set_decode_param(audio_decode_control_t *decode_control, isil_chip_t *chip)
{
    if(chip != NULL){
        chip->io_op->chip_write32(chip, decode_control->reg_offset, decode_control->value.value);
    }
}

static void audio_decode_control_get_decode_param(audio_decode_control_t *decode_control, isil_chip_t *chip)
{
    if(chip != NULL){
        //decode_control->value.value = chip->io_op->chip_read32(chip, decode_control->reg_offset);
    }
}

static struct audio_decode_control_operation	audio_decode_control_op = {
    .init = audio_decode_control_init,
    .update_decode_chan_mute = audio_decode_control_update_decode_chan_mute,
    .get_decode_chan_mute = audio_decode_control_get_decode_chan_mute,
    .set_decode_chan_mute = audio_decode_control_set_decode_chan_mute,
    .update_decode_chan_bit_wide = audio_decode_control_update_decode_chan_bit_wide,
    .get_decode_chan_bit_wide = audio_decode_control_get_decode_chan_bit_wide,
    .set_decode_chan_bit_wide = audio_decode_control_set_decode_chan_bit_wide,
    .update_decode_chan_enable = audio_decode_control_update_decode_chan_enable,
    .get_decode_chan_enable = audio_decode_control_get_decode_chan_enable,
    .set_decode_chan_enable = audio_decode_control_set_decode_chan_enable,

    .set_decode_param = audio_decode_control_set_decode_param,
    .get_decode_param = audio_decode_control_get_decode_param,
};

void	init_audio_decode_control(audio_decode_control_t *decode_control)
{
    decode_control->op = &audio_decode_control_op;
    decode_control->op->init(decode_control);
}

/****************************************************************************************/
/*                  decode audio read&write ptr operation table                         */
/****************************************************************************************/
static void audio_decode_ptr_control_init(audio_decode_ptr_control_t *decode_ptr)
{
    decode_ptr->wr_rd_value.value = 0;
    decode_ptr->wr_rd_reg_offset = ADPCM_DECODE_RD_WR_PTR_REG;
    decode_ptr->section_number = AUDIO_OUT_CHAN_SECTION_NUMBER;
}

static void audio_decode_ptr_control_get_adpcm_wr_rd_ptr(audio_decode_ptr_control_t *decode_ptr, isil_chip_t *chip)
{
    if(chip != NULL){
        decode_ptr->wr_rd_value.value = chip->io_op->chip_read32(chip, decode_ptr->wr_rd_reg_offset);

        //ISIL_DBG(ISIL_DBG_FATAL, "%08x\n", decode_ptr->wr_rd_value.value);
    }
}

static int  audio_decode_ptr_control_get_chan_wr_ptr(audio_decode_ptr_control_t *decode_ptr, int chan_id, int *ext_flag)
{
    int	ret = INVALID_ISIL_AUDIO_SECTION_ID;
    switch(chan_id){
        case ISIL_AUDIO_OUT_SPEAKER_ID:
            ret = decode_ptr->wr_rd_value.bit_value.chan0_wr_ptr;
            break;
        case ISIL_AUDIO_OUT_PLAYBACK_ID:
            ret = decode_ptr->wr_rd_value.bit_value.chan1_wr_ptr;
            break;
        default:
            printk("%s.%d: err audio chan id %d\n", __FUNCTION__, __LINE__, chan_id);
            break;
    }
    *ext_flag = ret>>(get_power_base((u32)AUDIO_OUT_CHAN_SECTION_NUMBER));
    ret &= ((1<<(get_power_base((u32)AUDIO_OUT_CHAN_SECTION_NUMBER)))-1);
    return ret;
}

static int  audio_decode_ptr_control_get_chan_rd_ptr(audio_decode_ptr_control_t *decode_ptr, int chan_id, int *ext_flag)
{
    int	ret = INVALID_ISIL_AUDIO_SECTION_ID;
    switch(chan_id){
        case ISIL_AUDIO_OUT_SPEAKER_ID:
            ret = decode_ptr->wr_rd_value.bit_value.chan0_rd_ptr;
            break;
        case ISIL_AUDIO_OUT_PLAYBACK_ID:
            ret = decode_ptr->wr_rd_value.bit_value.chan1_rd_ptr;
            break;
        default:
            printk("%s.%d: err audio chan id %d\n", __FUNCTION__, __LINE__, chan_id);
            break;
    }
    *ext_flag = ret>>(get_power_base((u32)AUDIO_OUT_CHAN_SECTION_NUMBER));    
    ret &= ((1<<(get_power_base((u32)AUDIO_OUT_CHAN_SECTION_NUMBER)))-1);
    return ret;
}

static int  audio_decode_ptr_control_get_chan_section_number(audio_decode_ptr_control_t *decode_ptr)
{
    return decode_ptr->section_number;
}

static void audio_decode_ptr_control_set_finish_wr_ptr(audio_decode_ptr_control_t *decode_ptr, int finish_id, isil_chip_t *chip, int chan_id)
{
    if(chip != NULL){
        finish_id++;
        if(finish_id >= (decode_ptr->section_number<<1)){
            finish_id = 0;
        }
        if(chan_id == ISIL_AUDIO_OUT_SPEAKER_ID){
            decode_ptr->wr_rd_value.bit_value.chan0_wr_ptr = finish_id;
        } else {
            decode_ptr->wr_rd_value.bit_value.chan1_wr_ptr = finish_id;       
        }
        chip->io_op->chip_write32(chip, decode_ptr->wr_rd_reg_offset, decode_ptr->wr_rd_value.value);
    }
}

struct audio_decode_ptr_control_operation   audio_decode_ptr_control_op = {
    .init = audio_decode_ptr_control_init,
    .get_adpcm_wr_rd_ptr = audio_decode_ptr_control_get_adpcm_wr_rd_ptr,

    .get_chan_wr_ptr = audio_decode_ptr_control_get_chan_wr_ptr,
    .get_chan_rd_ptr = audio_decode_ptr_control_get_chan_rd_ptr,
    .get_chan_section_number = audio_decode_ptr_control_get_chan_section_number,
    .set_finish_wr_ptr = audio_decode_ptr_control_set_finish_wr_ptr,
};

void	init_audio_decode_ptr_control(audio_decode_ptr_control_t *decode_ptr)
{
    decode_ptr->op = &audio_decode_ptr_control_op;
    decode_ptr->op->init(decode_ptr);
}

/****************************************************************************************/
/*              decode audio chip end addr calcuate operation table                     */
/****************************************************************************************/
static void audio_decode_data_addr_set_reg_bit_section_offset(audio_decode_data_addr_t *decode_data_addr, u32 section_offset)
{
    decode_data_addr->value.bit_value.section_offset = section_offset;
}

static u32  audio_decode_data_addr_get_reg_bit_section_offset(audio_decode_data_addr_t *decode_data_addr)
{
    return decode_data_addr->value.bit_value.section_offset;
}

static void audio_decode_data_addr_set_reg_bit_section_id(audio_decode_data_addr_t *decode_data_addr, u32 section_id)
{
    decode_data_addr->value.bit_value.section_id = section_id;
}

static u32  audio_decode_data_addr_get_reg_bit_section_id(audio_decode_data_addr_t *decode_data_addr)
{
    return decode_data_addr->value.bit_value.section_id;
}

static void audio_decode_data_addr_set_reg_bit_chan_id(audio_decode_data_addr_t *decode_data_addr, u32 chan_id)
{
    decode_data_addr->value.bit_value.chan_id = chan_id;
}

static u32  audio_decode_data_addr_get_reg_bit_chan_id(audio_decode_data_addr_t *decode_data_addr)
{
    return decode_data_addr->value.bit_value.chan_id;
}

static void audio_decode_data_addr_set_reg_bit_decode_type(audio_decode_data_addr_t *decode_data_addr)
{
    decode_data_addr->value.bit_value.decode_type = AUDIO_DATA_ADDR_ADPCM_DECODE_TYPE;
}

static u32  audio_decode_data_addr_get_reg_bit_decode_type(audio_decode_data_addr_t *decode_data_addr)
{
    return decode_data_addr->value.bit_value.decode_type;
}

static u32  audio_decode_data_addr_get_decode_data_addr(audio_decode_data_addr_t *decode_data_addr)
{
    return decode_data_addr->value.value;
}

static int  audio_decode_data_addr_get_buf_page_id(audio_decode_data_addr_t *decode_data_addr)
{
    return decode_data_addr->buf_page_id;
}

static int  audio_decode_data_addr_get_buf_in_chip_a_or_b(audio_decode_data_addr_t *decode_data_addr)
{
    return decode_data_addr->chip_a_or_b;
}

static u32  audio_decode_data_addr_get_adpcm_decode_data_addr_in_host_end_offset(audio_decode_data_addr_t *decode_data_addr)
{
    u32	host_end_offset;
    host_end_offset = DDRBASE;
    host_end_offset |= (decode_data_addr->value.value<<2);
    return host_end_offset;
}

static u32  audio_decode_data_addr_get_adpcm_decode_data_addr_in_ddr_end_offset(audio_decode_data_addr_t *decode_data_addr)
{
    u32	ddr_end_offset;
    ddr_end_offset = (decode_data_addr->buf_page_id<<19);
    ddr_end_offset |= (decode_data_addr->value.value<<2);

    return ddr_end_offset;
}

static void audio_decode_data_addr_init(audio_decode_data_addr_t *decode_data_addr)
{
    if(decode_data_addr != NULL){
        decode_data_addr->value.value = 0;
        decode_data_addr->value.bit_value.decode_type = AUDIO_DATA_ADDR_ADPCM_DECODE_TYPE;
        decode_data_addr->buf_page_id = ISIL_AUDIO_BUF_PAGE_ID;
        decode_data_addr->chip_a_or_b = ISIL_AUDIO_BUF_OF_DDR_ID;
    }
}

static struct audio_decode_data_addr_operation	audio_decode_data_addr_op = {
    .set_reg_bit_section_offset = audio_decode_data_addr_set_reg_bit_section_offset,
    .get_reg_bit_section_offset = audio_decode_data_addr_get_reg_bit_section_offset,
    .set_reg_bit_section_id = audio_decode_data_addr_set_reg_bit_section_id,
    .get_reg_bit_section_id = audio_decode_data_addr_get_reg_bit_section_id,
    .set_reg_bit_chan_id = audio_decode_data_addr_set_reg_bit_chan_id,
    .get_reg_bit_chan_id = audio_decode_data_addr_get_reg_bit_chan_id,
    .set_reg_bit_decode_type = audio_decode_data_addr_set_reg_bit_decode_type,
    .get_reg_bit_decode_type = audio_decode_data_addr_get_reg_bit_decode_type,
    .get_decode_data_addr = audio_decode_data_addr_get_decode_data_addr,
    .get_buf_page_id = audio_decode_data_addr_get_buf_page_id,
    .get_buf_in_chip_a_or_b = audio_decode_data_addr_get_buf_in_chip_a_or_b,
    .get_adpcm_decode_data_addr_in_host_end_offset = audio_decode_data_addr_get_adpcm_decode_data_addr_in_host_end_offset,
    .get_adpcm_decode_data_addr_in_ddr_end_offset = audio_decode_data_addr_get_adpcm_decode_data_addr_in_ddr_end_offset,
    .init = audio_decode_data_addr_init,
};

void	init_audio_decode_data_addr(audio_decode_data_addr_t *decode_data_addr)
{
    decode_data_addr->op = &audio_decode_data_addr_op;
    decode_data_addr->op->init(decode_data_addr);
}

/****************************************************************************************/
/*              chip level audio encode interface operation table                       */
/****************************************************************************************/
static void chip_audio_encode_init(chip_audio_encode_t *audio_encode)
{
    if(audio_encode != NULL){
        atomic_set(&audio_encode->need_sync_param, 1);
        memcpy(&audio_encode->config_param, &default_audio_encode_config_descriptor, sizeof(isil_audio_config_descriptor_t));
        init_audio_encode_property(&audio_encode->running_param, &default_audio_encode_config_descriptor);
        init_audio_encode_control(&audio_encode->audio_control);
        init_audio_pcm_encode_ptr_control(&audio_encode->pcm_audio_ptr);
        init_audio_pcm_encode_data_addr(&audio_encode->pcm_audio_data_base);
        init_audio_adpcm_encode_ptr_control(&audio_encode->adpcm_audio_ptr);
        init_audio_adpcm_encode_data_addr(&audio_encode->adpcm_audio_data_base);
    }
}

static void chip_audio_encode_set_enable_audio_encode_chan(chip_audio_encode_t *audio_encode, int chan_id, int enable, isil_chip_t *chip)
{
    audio_encode_control_t  *audio_control;
    isil_audio_param_t      *running_param;
    chip_audio_t            *chip_audio;
    isil_audio_driver_t     *audio_driver;
    int     phy_chan_id;

    audio_control = &audio_encode->audio_control;
    chip_audio = to_get_chip_audio_with_audio_encode(audio_encode);
    chip_audio->op->get_audio_chan_driver(chip_audio, chan_id, &audio_driver);
    if(audio_driver != NULL){
        phy_chan_id = audio_driver->audio_phy_chan_id;
    } else {
        phy_chan_id = chan_id;
    }
    running_param = &audio_encode->running_param;
    if(running_param->op != NULL){
        audio_enable_control_t  *audio_enable = &chip_audio->audio_enable;
        if((audio_enable->op!=NULL) && (running_param->op!=NULL)){
            switch(running_param->op->get_type(running_param)){
                case ISIL_AUDIO_PCM:
                    if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_PCI){
                        audio_control->op->set_enable_pci_sel_pcm_or_adpcm(audio_control, chan_id, ISIL_AUDIO_PCM, chip);
                        if(enable == AUDIO_ENABLE){
                            audio_enable->op->set_enable_pcm_encode(audio_enable, AUDIO_ENABLE, chip);
                            audio_control->op->set_enable_pcm_chan(audio_control, phy_chan_id, AUDIO_ENABLE, audio_driver->read_bitstream_mode, chip);
                            audio_enable->op->set_enable_adpcm_encode(audio_enable, AUDIO_ENABLE, chip);
                            audio_control->op->set_enable_adpcm_chan(audio_control, chan_id, AUDIO_ENABLE, chip);
                        } else if(enable == AUDIO_DISABLE){
                            audio_control->op->set_enable_adpcm_chan(audio_control, chan_id, AUDIO_DISABLE, chip);
                            audio_enable->op->set_enable_adpcm_encode(audio_enable, AUDIO_DISABLE, chip);
                            audio_control->op->set_enable_pcm_chan(audio_control, phy_chan_id, AUDIO_DISABLE, audio_driver->read_bitstream_mode, chip);
                            audio_enable->op->set_enable_pcm_encode(audio_enable, AUDIO_DISABLE, chip);
                        }
                    } else {
                        audio_enable->op->get_enable_control_param(audio_enable, chip);
                        audio_enable->op->set_enable_adpcm_encode(audio_enable, AUDIO_DISABLE, chip);
                        audio_control->op->set_enable_adpcm_chan(audio_control, chan_id, AUDIO_DISABLE, chip);
                        if(enable == AUDIO_ENABLE){
                            audio_enable->op->set_enable_pcm_encode(audio_enable, AUDIO_ENABLE, chip);
                            audio_control->op->set_enable_pcm_chan(audio_control, phy_chan_id, AUDIO_ENABLE, audio_driver->read_bitstream_mode, chip);
                        } else if(enable == AUDIO_DISABLE){
                            audio_control->op->set_enable_pcm_chan(audio_control, phy_chan_id, audio_driver->read_bitstream_mode, AUDIO_DISABLE, chip);
                            audio_enable->op->set_enable_pcm_encode(audio_enable, AUDIO_DISABLE, chip);
                        }                    
                    }
                    break;
                case ISIL_AUDIO_ADPCM_32K:
                    if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_PCI){
                        audio_control->op->set_enable_pci_sel_pcm_or_adpcm(audio_control, chan_id, ISIL_AUDIO_ADPCM_32K, chip);
                    }
                    if(enable == AUDIO_ENABLE){
                        audio_enable->op->set_enable_pcm_encode(audio_enable, AUDIO_ENABLE, chip);
                        audio_control->op->set_enable_pcm_chan(audio_control, phy_chan_id, AUDIO_ENABLE, audio_driver->read_bitstream_mode, chip);
                        audio_enable->op->set_enable_adpcm_encode(audio_enable, AUDIO_ENABLE, chip);
                        audio_control->op->set_enable_adpcm_chan(audio_control, chan_id, AUDIO_ENABLE, chip);
                    } else if(enable == AUDIO_DISABLE){
                        audio_control->op->set_enable_adpcm_chan(audio_control, chan_id, AUDIO_DISABLE, chip);
                        audio_enable->op->set_enable_adpcm_encode(audio_enable, AUDIO_DISABLE, chip);
                        audio_control->op->set_enable_pcm_chan(audio_control, phy_chan_id, AUDIO_DISABLE, audio_driver->read_bitstream_mode, chip);
                        audio_enable->op->set_enable_pcm_encode(audio_enable, AUDIO_DISABLE, chip);
                    }
                    break;
                default:
                    printk("%s.%d: no surpport %d type\n", __FUNCTION__, __LINE__, running_param->op->get_type(running_param));
                    break;
            }
        }
    }
}

static void chip_audio_encode_update_enable_audio_encode_chan(chip_audio_encode_t *audio_encode, int chan_id, int enable)
{
    audio_encode_control_t  *audio_control;
    isil_audio_param_t      *running_param;
    chip_audio_t            *chip_audio;
    isil_audio_driver_t     *audio_driver;
    int     phy_chan_id;

    audio_control = &audio_encode->audio_control;
    chip_audio = to_get_chip_audio_with_audio_encode(audio_encode);
    chip_audio->op->get_audio_chan_driver(chip_audio, chan_id, &audio_driver);
    if(audio_driver != NULL){
        phy_chan_id = audio_driver->audio_phy_chan_id;
    } else {
        phy_chan_id = chan_id;
    }
    running_param = &audio_encode->running_param;
    if(audio_control->op != NULL){
        audio_enable_control_t  *audio_enable = &chip_audio->audio_enable;
        if((audio_enable->op!=NULL) && (running_param->op!=NULL)){
            switch(running_param->op->get_type(running_param)){
                case ISIL_AUDIO_PCM:
                    if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_PCI){
                        audio_control->op->pci_sel_pcm_or_adpcm(audio_control, chan_id, ISIL_AUDIO_PCM);
                        if(enable == AUDIO_ENABLE){
                            audio_control->op->enable_pcm_chan(audio_control, phy_chan_id);
                            audio_enable->op->update_enable_pcm_encode(audio_enable, AUDIO_ENABLE);
                            audio_control->op->enable_adpcm_chan(audio_control, chan_id);
                            audio_enable->op->update_enable_adpcm_encode(audio_enable, AUDIO_ENABLE);
                        } else if(enable == AUDIO_DISABLE){
                            audio_control->op->disable_adpcm_chan(audio_control, chan_id);
                            audio_enable->op->update_enable_adpcm_encode(audio_enable, AUDIO_DISABLE);
                            audio_control->op->disable_pcm_chan(audio_control, phy_chan_id);
                            audio_enable->op->update_enable_pcm_encode(audio_enable, AUDIO_DISABLE);
                        }
                    } else {
                        audio_control->op->disable_adpcm_chan(audio_control, phy_chan_id);
                        audio_enable->op->update_enable_adpcm_encode(audio_enable, AUDIO_DISABLE);
                        if(enable == AUDIO_ENABLE){
                            audio_control->op->enable_pcm_chan(audio_control, phy_chan_id);
                            audio_enable->op->update_enable_pcm_encode(audio_enable, AUDIO_ENABLE);
                        } else if(enable == AUDIO_DISABLE){
                            audio_control->op->disable_pcm_chan(audio_control, phy_chan_id);
                            audio_enable->op->update_enable_pcm_encode(audio_enable, AUDIO_DISABLE);
                        }                    
                    }
                    break;
                case ISIL_AUDIO_ADPCM_32K:
                    if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_PCI){
                        audio_control->op->pci_sel_pcm_or_adpcm(audio_control, chan_id, ISIL_AUDIO_ADPCM_32K);
                    }
                    if(enable == AUDIO_ENABLE){
                        audio_control->op->enable_pcm_chan(audio_control, phy_chan_id);
                        audio_enable->op->update_enable_pcm_encode(audio_enable, AUDIO_ENABLE);
                        audio_control->op->enable_adpcm_chan(audio_control, chan_id);
                        audio_enable->op->update_enable_adpcm_encode(audio_enable, AUDIO_ENABLE);
                    } else if(enable == AUDIO_DISABLE){
                        audio_control->op->disable_adpcm_chan(audio_control, chan_id);
                        audio_enable->op->update_enable_adpcm_encode(audio_enable, AUDIO_DISABLE);
                        audio_control->op->disable_pcm_chan(audio_control, phy_chan_id);
                        audio_enable->op->update_enable_pcm_encode(audio_enable, AUDIO_DISABLE);
                    }
                    break;
                default:
                    printk("%s.%d: no surpport %d type\n", __FUNCTION__, __LINE__, running_param->op->get_type(running_param));
                    break;
            }
        } 
    }
}

static int  chip_audio_encode_get_audio_encode_chan_enable_state(chip_audio_encode_t *audio_encode, int chan_id)
{
    audio_encode_control_t  *audio_control;
    isil_audio_param_t      *running_param;
    int	ret = AUDIO_DISABLE;

    audio_control = &audio_encode->audio_control;
    running_param = &audio_encode->running_param;
    if(audio_control->op != NULL){
        chip_audio_t    *chip_audio = to_get_chip_audio_with_audio_encode(audio_encode);
        audio_enable_control_t  *audio_enable = &chip_audio->audio_enable;
        if((audio_enable->op!=NULL) && (running_param->op!=NULL)){
            switch(running_param->op->get_type(running_param)){
                case ISIL_AUDIO_PCM:
                    ret = audio_enable->op->get_pcm_encode_state(audio_enable);
                    break;
                case ISIL_AUDIO_ADPCM_32K:
                    ret = audio_enable->op->get_adpcm_encode_state(audio_enable);
                    break;
                default:
                    printk("%s.%d: no surpport %d type\n", __FUNCTION__, __LINE__, running_param->op->get_type(running_param));
                    break;
            }
        } 
    }
    return ret;
}

static int  chip_audio_encode_get_audio_encode_chan_rd_ptr(chip_audio_encode_t *audio_encode, int chan_id, int *ext_flag, int *queue_size)
{
    audio_adpcm_encode_ptr_control_t    *adpcm_audio_ptr;
    audio_pcm_encode_ptr_control_t      *pcm_audio_ptr;
    isil_audio_param_t                  *running_param;
    int	ret = INVALID_ISIL_AUDIO_SECTION_ID;

    running_param = &audio_encode->running_param;
    *ext_flag = 0;
    *queue_size = 0;
    if(running_param->op != NULL){
        switch(running_param->op->get_type(running_param)){
            case ISIL_AUDIO_PCM:
                pcm_audio_ptr = &audio_encode->pcm_audio_ptr;
                if(pcm_audio_ptr->op != NULL){
                    ret = pcm_audio_ptr->op->get_chan_rd_ptr(pcm_audio_ptr, chan_id, ext_flag);
                    *queue_size = pcm_audio_ptr->op->get_chan_section_number(pcm_audio_ptr);
                }
                break;
            case ISIL_AUDIO_ADPCM_32K:
                adpcm_audio_ptr = &audio_encode->adpcm_audio_ptr;
                if(adpcm_audio_ptr->op != NULL){
                    ret = adpcm_audio_ptr->op->get_chan_rd_ptr(adpcm_audio_ptr, chan_id, ext_flag);
                    *queue_size = adpcm_audio_ptr->op->get_chan_section_number(adpcm_audio_ptr);
                }
                break;
            default:
                printk("%s.%d: no surpport %d type\n", __FUNCTION__, __LINE__, running_param->op->get_type(running_param));
                break;
        }
    } 
    return ret;
}

static int  chip_audio_encode_get_audio_encode_chan_wr_ptr(chip_audio_encode_t *audio_encode, int chan_id, int *ext_flag, int *queue_size)
{
    audio_adpcm_encode_ptr_control_t    *adpcm_audio_ptr;
    audio_pcm_encode_ptr_control_t      *pcm_audio_ptr;
    isil_audio_param_t                  *running_param;
    int	ret = INVALID_ISIL_AUDIO_SECTION_ID;

    running_param = &audio_encode->running_param;
    *ext_flag = 0;
    *queue_size = 0;
    if(running_param->op != NULL){
        switch(running_param->op->get_type(running_param)){
            case ISIL_AUDIO_PCM:
                pcm_audio_ptr = &audio_encode->pcm_audio_ptr;
                if(pcm_audio_ptr->op != NULL){
                    ret = pcm_audio_ptr->op->get_chan_wr_ptr(pcm_audio_ptr, chan_id, ext_flag);
                    *queue_size = pcm_audio_ptr->op->get_chan_section_number(pcm_audio_ptr);
                }
                break;
            case ISIL_AUDIO_ADPCM_32K:
                adpcm_audio_ptr = &audio_encode->adpcm_audio_ptr;
                if(adpcm_audio_ptr->op != NULL){
                    ret = adpcm_audio_ptr->op->get_chan_wr_ptr(adpcm_audio_ptr, chan_id, ext_flag);
                    *queue_size = adpcm_audio_ptr->op->get_chan_section_number(adpcm_audio_ptr);
                }
                break;
            default:
                printk("%s.%d: no surpport %d type\n", __FUNCTION__, __LINE__, running_param->op->get_type(running_param));
                break;
        }
    } 
    return ret;
}

static void chip_audio_encode_get_audio_encode_param(chip_audio_encode_t *audio_encode, isil_chip_t *chip)
{
    if(chip != NULL){
        audio_encode->audio_control.op->get_audio_param(&audio_encode->audio_control, chip);
        audio_encode->pcm_audio_ptr.op->get_pcm_wr_rd_ptr(&audio_encode->pcm_audio_ptr, chip);
        audio_encode->adpcm_audio_ptr.op->get_adpcm_wr_rd_ptr(&audio_encode->adpcm_audio_ptr, chip);
    }
}

static void chip_audio_encode_set_audio_encode_param(chip_audio_encode_t *audio_encode, isil_chip_t *chip)
{
    if(chip != NULL){
        audio_encode->audio_control.op->set_audio_param(&audio_encode->audio_control, chip);
    }
}

static void chip_audio_encode_set_finish_rd_ptr(chip_audio_encode_t *audio_encode, int finish_rd_ptr, isil_chip_t *chip)
{
    audio_adpcm_encode_ptr_control_t    *adpcm_audio_ptr;
    audio_pcm_encode_ptr_control_t      *pcm_audio_ptr;
    isil_audio_param_t                  *running_param;

    running_param = &audio_encode->running_param;
    finish_rd_ptr &= 0x1f;
    if(running_param->op != NULL){
        switch(running_param->op->get_type(running_param)){
            case ISIL_AUDIO_PCM:
                pcm_audio_ptr = &audio_encode->pcm_audio_ptr;
                if(pcm_audio_ptr->op != NULL){
                    pcm_audio_ptr->op->set_rd_finish_number(pcm_audio_ptr, finish_rd_ptr, chip);
                }
                break;
            case ISIL_AUDIO_ADPCM_32K:
                adpcm_audio_ptr = &audio_encode->adpcm_audio_ptr;
                if(adpcm_audio_ptr->op != NULL){
                    adpcm_audio_ptr->op->set_rd_finish_number(adpcm_audio_ptr, finish_rd_ptr, chip);
                }
                break;
            default:
                printk("%s.%d: no surpport %d type\n", __FUNCTION__, __LINE__, running_param->op->get_type(running_param));
                break;
        }
    } 
}

static struct chip_audio_encode_operation   chip_audio_encode_op = {
    .init = chip_audio_encode_init,

    .set_enable_audio_encode_chan = chip_audio_encode_set_enable_audio_encode_chan,
    .update_enable_audio_encode_chan = chip_audio_encode_update_enable_audio_encode_chan,
    .get_audio_encode_chan_enable_state = chip_audio_encode_get_audio_encode_chan_enable_state,

    .get_audio_encode_chan_rd_ptr = chip_audio_encode_get_audio_encode_chan_rd_ptr,
    .get_audio_encode_chan_wr_ptr = chip_audio_encode_get_audio_encode_chan_wr_ptr,

    .get_audio_encode_param = chip_audio_encode_get_audio_encode_param,
    .set_audio_encode_param = chip_audio_encode_set_audio_encode_param,
    .set_finish_rd_ptr = chip_audio_encode_set_finish_rd_ptr,
};

void	init_chip_audio_encode(chip_audio_encode_t *audio_encode)
{
    audio_encode->op = &chip_audio_encode_op;
    audio_encode->op->init(audio_encode);
}

/****************************************************************************************/
/*                  chip level audio decode interface operation table                   */
/****************************************************************************************/
static void chip_audio_decode_init(chip_audio_decode_t *audio_decode)
{
    atomic_set(&audio_decode->need_sync_param, 1);
    memcpy(&audio_decode->config_param, &default_audio_encode_config_descriptor, sizeof(isil_audio_config_descriptor_t));
    init_audio_decode_property(&audio_decode->running_param, &default_audio_encode_config_descriptor);
    init_audio_decode_control(&audio_decode->audio_control);
    init_audio_decode_ptr_control(&audio_decode->audio_ptr);
    init_audio_decode_data_addr(&audio_decode->audio_data_base);
}

static void chip_audio_decode_update_enable_audio_decode_chan(chip_audio_decode_t *audio_decode, int chan_id, int enable)
{
    audio_decode_control_t  *audio_control = &audio_decode->audio_control;
    chip_audio_t            *chip_audio = to_get_chip_audio_with_audio_decode(audio_decode);
    audio_enable_control_t  *audio_enable = &chip_audio->audio_enable;
    if(audio_control->op != NULL){
        if(enable == AUDIO_ENABLE){
            audio_enable->op->update_enable_adpcm_decode(audio_enable, AUDIO_ENABLE);
            audio_control->op->update_decode_chan_enable(audio_control, chan_id, AUDIO_ENABLE);
        }else {
            audio_control->op->update_decode_chan_enable(audio_control, chan_id, AUDIO_DISABLE);
            audio_enable->op->update_enable_adpcm_decode(audio_enable, AUDIO_DISABLE);
        }
    }
}

static int  chip_audio_decode_get_audio_decode_chan_state(chip_audio_decode_t *audio_decode, int chan_id)
{
    int	ret = AUDIO_DISABLE;
    audio_decode_control_t  *audio_control = &audio_decode->audio_control;
    if(audio_control->op != NULL){
        ret = audio_control->op->get_decode_chan_enable(audio_control, chan_id);
    }
    return ret;
}

static void chip_audio_decode_set_enable_audio_decode_chan(chip_audio_decode_t *audio_decode, int chan_id, int enable, isil_chip_t *chip)
{
    audio_decode_control_t  *audio_control = &audio_decode->audio_control;
    chip_audio_t            *chip_audio = to_get_chip_audio_with_audio_decode(audio_decode);
    audio_enable_control_t  *audio_enable = &chip_audio->audio_enable;

    if(audio_control->op != NULL){
        if(enable == AUDIO_ENABLE){
            audio_enable->op->set_enable_adpcm_decode(audio_enable, AUDIO_ENABLE, chip);
            audio_control->op->set_decode_chan_enable(audio_control, chan_id, AUDIO_ENABLE, chip);
        }else if(enable == AUDIO_DISABLE){
            audio_control->op->set_decode_chan_enable(audio_control, chan_id, AUDIO_DISABLE, chip);
            audio_enable->op->set_enable_adpcm_decode(audio_enable, AUDIO_DISABLE, chip);
        }
    }
}

static void chip_audio_decode_update_mute_audio_decode_chan(chip_audio_decode_t *audio_decode, int chan_id, int mute)
{
    audio_decode_control_t  *audio_control = &audio_decode->audio_control;
    if(audio_control->op != NULL){
        if(mute == AUDIO_MUTE_ENABLE){
            audio_control->op->update_decode_chan_mute(audio_control, chan_id, AUDIO_MUTE_ENABLE);
        } else {
            audio_control->op->update_decode_chan_mute(audio_control, chan_id, AUDIO_MUTE_DISABLE);
        }
    }
}

static int  chip_audio_decode_get_audio_decode_mute_state(chip_audio_decode_t *audio_decode, int chan_id)
{
    int	ret = AUDIO_MUTE_ENABLE;
    audio_decode_control_t  *audio_control = &audio_decode->audio_control;
    if(audio_control->op != NULL){
        ret = audio_control->op->get_decode_chan_mute(audio_control, chan_id);
    }
    return ret;
}

static void chip_audio_decode_update_set_mute_audio_decode_chan(chip_audio_decode_t *audio_decode, int chan_id, int mute, isil_chip_t *chip)
{
    audio_decode_control_t  *audio_control = &audio_decode->audio_control;
    if(audio_control->op != NULL){
        if(mute == AUDIO_MUTE_ENABLE){
            audio_control->op->set_decode_chan_mute(audio_control, chan_id, AUDIO_MUTE_ENABLE, chip);
        }else if(mute == AUDIO_MUTE_DISABLE){
            audio_control->op->set_decode_chan_mute(audio_control, chan_id, AUDIO_MUTE_DISABLE, chip);
        }
    }
}

static int  chip_audio_decode_get_audio_decode_chan_rd_ptr(chip_audio_decode_t *audio_decode, int chan_id, int *ext_flag, int *queue_size)
{
    int	ret = INVALID_ISIL_AUDIO_SECTION_ID;
    isil_audio_param_t          *running_param;
    audio_decode_ptr_control_t  *audio_ptr;

    running_param = &audio_decode->running_param;
    *ext_flag = 0;
    *queue_size = 0;
    if(running_param->op != NULL){
        switch(running_param->op->get_type(running_param)){
            case ISIL_AUDIO_PCM:
            case ISIL_AUDIO_ADPCM_32K:
                audio_ptr = &audio_decode->audio_ptr;
                if(audio_ptr->op != NULL){
                    ret = audio_ptr->op->get_chan_rd_ptr(audio_ptr, chan_id, ext_flag);
                    *queue_size = audio_ptr->op->get_chan_section_number(audio_ptr);
                }
                break;
            default:
                printk("%s.%d: no surpport %d type\n", __FUNCTION__, __LINE__, running_param->op->get_type(running_param));
                break;
        }
    } 
    return ret;
}

static int  chip_audio_decode_get_audio_decode_chan_wr_ptr(chip_audio_decode_t *audio_decode, int chan_id, int *ext_flag, int *queue_size)
{
    int	ret = INVALID_ISIL_AUDIO_SECTION_ID;
    isil_audio_param_t          *running_param;
    audio_decode_ptr_control_t  *audio_ptr;

    running_param = &audio_decode->running_param;
    *ext_flag = 0;
    *queue_size = 0;
    if(running_param->op != NULL){
        switch(running_param->op->get_type(running_param)){
            case ISIL_AUDIO_PCM:
            case ISIL_AUDIO_ADPCM_32K:
                audio_ptr = &audio_decode->audio_ptr;
                if(audio_ptr->op != NULL){
                    ret = audio_ptr->op->get_chan_wr_ptr(audio_ptr, chan_id, ext_flag);
                    *queue_size = audio_ptr->op->get_chan_section_number(audio_ptr);
                }
                break;
            default:
                printk("%s.%d: no surpport %d type\n", __FUNCTION__, __LINE__, running_param->op->get_type(running_param));
                break;
        }
    } 
    return ret;
}

static void chip_audio_decode_get_audio_decode_param(chip_audio_decode_t *audio_decode, isil_chip_t *chip)
{
    if(chip != NULL){
        audio_decode->audio_control.op->get_decode_param(&audio_decode->audio_control, chip);
        audio_decode->audio_ptr.op->get_adpcm_wr_rd_ptr(&audio_decode->audio_ptr, chip);
    }
}

static void chip_audio_decode_set_audio_decode_param(chip_audio_decode_t *audio_decode, isil_chip_t *chip)
{
    if(chip != NULL){
        audio_decode_control_t	*audio_control = &audio_decode->audio_control;;
        audio_control->op->set_decode_param(audio_control, chip);
    }
}

static struct chip_audio_decode_operation   chip_audio_decode_op = {
    .init = chip_audio_decode_init,

    .update_enable_audio_decode_chan = chip_audio_decode_update_enable_audio_decode_chan,
    .get_audio_decode_chan_enable_state = chip_audio_decode_get_audio_decode_chan_state,
    .set_enable_audio_decode_chan = chip_audio_decode_set_enable_audio_decode_chan,
    .update_mute_audio_decode_chan = chip_audio_decode_update_mute_audio_decode_chan,
    .get_audio_decode_mute_state = chip_audio_decode_get_audio_decode_mute_state,
    .set_mute_audio_decode_chan = chip_audio_decode_update_set_mute_audio_decode_chan,
    .get_audio_decode_chan_rd_ptr = chip_audio_decode_get_audio_decode_chan_rd_ptr,
    .get_audio_decode_chan_wr_ptr = chip_audio_decode_get_audio_decode_chan_wr_ptr,
    .get_audio_decode_param = chip_audio_decode_get_audio_decode_param,
    .set_audio_decode_param = chip_audio_decode_set_audio_decode_param,
};

void	init_chip_audio_decode(chip_audio_decode_t *audio_decode)
{
    audio_decode->op = &chip_audio_decode_op;
    audio_decode->op->init(audio_decode);
}
/****************************************************************************************/
/*                  chip level audio interface operation table                          */
/****************************************************************************************/
static int chip_audio_init(chip_audio_t *chip_audio)
{
    init_audio_enable_control(&chip_audio->audio_enable);
    init_chip_audio_encode(&chip_audio->audio_encode);
    init_chip_audio_decode(&chip_audio->audio_decode);

    init_register_table_node(&chip_audio->logic_chan_table);
    init_register_table_node(&chip_audio->opened_logic_chan_table);
    return ISIL_OK;
}


static int chip_audio_reset(chip_audio_t *chip_audio)
{
    return ISIL_OK;
}


static int chip_audio_release(chip_audio_t *chip_audio)
{
    return ISIL_OK;
}

static void chip_audio_set_encode_audio_chan_type(chip_audio_t *chip_audio, int type)
{
    chip_audio_encode_t *audio_encode = &chip_audio->audio_encode;
    isil_audio_param_t	*running_param = &audio_encode->running_param;
    if((type!=ISIL_AUDIO_ADPCM_32K) && (type!=ISIL_AUDIO_PCM)){
        printk("%s.%d: type=%d, we only surpport adpcm and pcm mode\n", __FUNCTION__, __LINE__, type);
        type = ISIL_AUDIO_ADPCM_32K;
    }
    if(running_param->op != NULL){
        running_param->op->update_type(running_param, type);
    }
}

static void chip_audio_update_encode_audio_chan_type(chip_audio_t *chip_audio, int type)
{
    chip_audio_encode_t *audio_encode = &chip_audio->audio_encode;
    isil_audio_param_t	*running_param = &audio_encode->running_param;
    if((type!=ISIL_AUDIO_ADPCM_32K) && (type!=ISIL_AUDIO_PCM)){
        ISIL_DBG(ISIL_DBG_ERR, "type=%d, we only surpport adpcm and pcm mode\n", type);
        type = ISIL_AUDIO_ADPCM_32K;
    }
    if(running_param->op != NULL){
        running_param->op->update_type(running_param, type);
    }
}

static int  chip_audio_get_encode_audio_chan_type(chip_audio_t *chip_audio)
{
    int	ret = ISIL_AUDIO_ADPCM_32K;
    chip_audio_encode_t *audio_encode = &chip_audio->audio_encode;
    isil_audio_param_t	*running_param = &audio_encode->running_param;
    if(running_param->op != NULL) {
        ret = running_param->op->get_type(running_param);
    }
    return ret;
}

static void chip_audio_set_decode_audio_chan_type(chip_audio_t *chip_audio, int type)
{
    chip_audio_decode_t *audio_decode = &chip_audio->audio_decode;
    isil_audio_param_t	*running_param = &audio_decode->running_param;

    if((type!=ISIL_AUDIO_ADPCM_32K) && (type!=ISIL_AUDIO_PCM)){
        ISIL_DBG(ISIL_DBG_ERR, "type=%d, we only surpport adpcm and pcm mode\n", type);
        type = ISIL_AUDIO_ADPCM_32K;
    }
    if(running_param->op != NULL){
        running_param->op->update_type(running_param, type);
    }
}

static void chip_audio_update_decode_audio_chan_type(chip_audio_t *chip_audio, int type)
{
    chip_audio_decode_t *audio_decode = &chip_audio->audio_decode;
    isil_audio_param_t	*running_param = &audio_decode->running_param;

    if((type!=ISIL_AUDIO_ADPCM_32K) && (type!=ISIL_AUDIO_PCM)){
        ISIL_DBG(ISIL_DBG_ERR, "type=%d, we only surpport adpcm and pcm mode\n", type);
        type = ISIL_AUDIO_ADPCM_32K;
    }
    if(running_param->op != NULL){
        running_param->op->update_type(running_param, type);
    }
}

static int  chip_audio_get_decode_audio_chan_type(chip_audio_t *chip_audio)
{
    int	ret = ISIL_AUDIO_ADPCM_32K;
    chip_audio_decode_t *audio_decode = &chip_audio->audio_decode;
    isil_audio_param_t  *running_param = &audio_decode->running_param;

    if(running_param->op != NULL){
        ret = running_param->op->get_type(running_param);
    }
    return ret;
}

static void chip_audio_set_enable_audio_chan(chip_audio_t *chip_audio, int chan_id, int enable)
{
    if(chan_id < ISIL_AUDIO_IN_CHAN_NUMBER){
        chip_audio_encode_t *audio_encode = &chip_audio->audio_encode;
        if(audio_encode->op != NULL){
            audio_encode->op->set_enable_audio_encode_chan(audio_encode, chan_id, enable, chip_audio->chip);
        }
    } else if(chan_id < ISIL_AUDIO_CHAN_NUMBER){
        chip_audio_decode_t *audio_decode = &chip_audio->audio_decode;
        if(audio_decode->op != NULL){
            audio_decode->op->set_enable_audio_decode_chan(audio_decode, chan_id, enable, chip_audio->chip);
        }
    }
}

static void chip_audio_update_enable_audio_chan(chip_audio_t *chip_audio, int chan_id, int enable)
{
    if(chan_id < ISIL_AUDIO_IN_CHAN_NUMBER){
        chip_audio_encode_t *audio_encode = &chip_audio->audio_encode;
        if(audio_encode->op != NULL){
            audio_encode->op->update_enable_audio_encode_chan(audio_encode, chan_id, enable);
        }
    } else if(chan_id < ISIL_AUDIO_CHAN_NUMBER){
        chip_audio_decode_t *audio_decode = &chip_audio->audio_decode;
        if(audio_decode->op != NULL){
            audio_decode->op->update_enable_audio_decode_chan(audio_decode, chan_id, enable);
        }
    }
}

static int  chip_audio_get_audio_chan_enable_state(chip_audio_t *chip_audio, int chan_id)
{
    int	ret = AUDIO_DISABLE;
    if(chip_audio != NULL){
        if(chan_id < ISIL_AUDIO_IN_CHAN_NUMBER){
            chip_audio_encode_t *audio_encode = &chip_audio->audio_encode;
            if(audio_encode->op != NULL){
                ret = audio_encode->op->get_audio_encode_chan_enable_state(audio_encode, chan_id);
            }
        } else {
            chip_audio_decode_t *audio_decode = &chip_audio->audio_decode;
            if(audio_decode->op != NULL){
                ret = audio_decode->op->get_audio_decode_chan_enable_state(audio_decode, chan_id);
            }
        }
    }
    return ret;
}

static void chip_audio_set_mute_audio_chan(chip_audio_t *chip_audio, int chan_id, int mute)
{
    if(chan_id < ISIL_AUDIO_IN_CHAN_NUMBER){

    } else if(chan_id < ISIL_AUDIO_CHAN_NUMBER){
        chip_audio_decode_t *audio_decode = &chip_audio->audio_decode;
        if(audio_decode->op != NULL){
            audio_decode->op->set_mute_audio_decode_chan(audio_decode, chan_id, mute, chip_audio->chip);
        }
    }
}

static void chip_audio_update_mute_audio_chan(chip_audio_t *chip_audio, int chan_id, int mute)
{
    if(chan_id < ISIL_AUDIO_IN_CHAN_NUMBER){

    } else if(chan_id < ISIL_AUDIO_CHAN_NUMBER){
        chip_audio_decode_t *audio_decode = &chip_audio->audio_decode;
        if(audio_decode->op != NULL){
            audio_decode->op->update_mute_audio_decode_chan(audio_decode, chan_id, mute);
        }
    }
}

static int  chip_audio_get_audio_chan_mute_state(chip_audio_t *chip_audio, int chan_id)
{
    int	ret = AUDIO_MUTE_ENABLE;
    if(chan_id < ISIL_AUDIO_IN_CHAN_NUMBER){

    } else if(chan_id < ISIL_AUDIO_CHAN_NUMBER){
        chip_audio_decode_t *audio_decode = &chip_audio->audio_decode;
        if(audio_decode->op != NULL){
            ret = audio_decode->op->get_audio_decode_mute_state(&chip_audio->audio_decode, chan_id);
        }
    }
    return ret;
}

static void chip_audio_get_chip_audio_param(chip_audio_t *chip_audio)
{
    isil_chip_t *chip = chip_audio->chip;
    chip_audio->audio_enable.op->get_enable_control_param(&chip_audio->audio_enable, chip);
    chip_audio->audio_encode.op->get_audio_encode_param(&chip_audio->audio_encode, chip);
    chip_audio->audio_decode.op->get_audio_decode_param(&chip_audio->audio_decode, chip);
}

static void chip_audio_set_chip_audio_param(chip_audio_t *chip_audio)
{
    isil_chip_t *chip = chip_audio->chip;
    chip_audio->audio_enable.op->set_enable_control_param(&chip_audio->audio_enable, chip);
    chip_audio->audio_encode.op->set_audio_encode_param(&chip_audio->audio_encode, chip);
    chip_audio->audio_decode.op->set_audio_decode_param(&chip_audio->audio_decode, chip);
}

static int  audio_encode_chan_response_ddr_burst_done_isr(int irq, void *context)
{
    isil_audio_driver_t *audio_chan_driver = (isil_audio_driver_t*)context;
    int ret = 1;
    if(audio_chan_driver != NULL){
        isil_audio_packet_queue_t   *audio_packet_queue = &audio_chan_driver->audio_packet_queue;
        isil_chip_t                 *chip = audio_chan_driver->chip_audio->chip;
        chip_audio_encode_t         *audio_encode = &audio_chan_driver->chip_audio->audio_encode;
        audio_section_ptr_info_t    *audio_section_param = &audio_chan_driver->audio_section_param;
        dpram_control_t             *chip_dpram_controller;
        chip_ddr_burst_engine_t     *chip_ddr_burst_interface;
        ddr_burst_interface_t       *burst_interface;
        isil_timestamp_t            *timestamp = &audio_chan_driver->timestamp;
        dpram_page_node_t           *dpram_page;
        audio_section_descriptor_t  descriptor;
        u32 dpram_base;
        int finish_id;
        unsigned long   flags;

        spin_lock_irqsave(&audio_chan_driver->fsm_matrix_call_lock, flags);
        get_isil_dpram_controller(&chip_dpram_controller);
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        chip_ddr_burst_interface->op->get_ddr_burst_interface(chip_ddr_burst_interface, &burst_interface);
        burst_interface->op->clear_burst_done(burst_interface);
        dpram_page = audio_chan_driver->dpram_page;
        dpram_base = dpram_page->op->get_page_base(dpram_page);
        dpram_base += AUDIO_SECTION_SIZE;
        dpram_base -= sizeof(audio_section_descriptor_t);
        *((u32*)&descriptor) = chip->io_op->chip_read32(chip, dpram_base);
        timestamp->op->set_timestamp(timestamp, descriptor.timestamp, __FILE__);

        audio_packet_queue->op->try_get_curr_producer_from_pool(audio_packet_queue, &audio_chan_driver->audio_buf_pool);
        flush_audio_encode_curr_producer_frame(audio_chan_driver);
        if(audio_packet_queue->curr_producer != NULL){
            isil_audio_packet_section_t   *audio_section = audio_packet_queue->curr_producer;
            isil_audio_descriptor_t       *audio_descriptor = &audio_section->descriptor;

            *((u32*)audio_descriptor->descriptor) = *((u32*)&descriptor);
            //printk("descriptor = 0x%08x\n", *((u32*)&descriptor));
            audio_section->payload_len  = audio_descriptor->op->get_audio_descriptor_valid_len(audio_descriptor);
            audio_section->sample_rate = audio_descriptor->op->get_audio_descriptor_sample_rate(audio_descriptor);
            audio_section->type = audio_descriptor->op->get_audio_descriptor_type(audio_descriptor);
            audio_section->bit_wide = audio_descriptor->op->get_audio_descriptor_bit_wide(audio_descriptor);
            audio_section->timestamp = timestamp->op->get_timestamp(timestamp);
            audio_section->frameSerial = audio_chan_driver->i_frame_serial++;

            audio_section->payload_len = H264_MIN((audio_section->section_size-sizeof(ext_h264_nal_bitstream_t)), audio_section->payload_len);
#if defined(DMA_INTERFACE)
            audio_section->op->dma_map(audio_section, DMA_FROM_DEVICE);
            burst_interface->op->dma_host_to_sram_read(burst_interface, audio_chan_driver->dpram_page, audio_section->dma_addr+sizeof(ext_h264_nal_bitstream_t), audio_section->payload_len, 0);
            audio_section->op->dma_unmap(audio_section, DMA_FROM_DEVICE);
#else
            burst_interface->op->pio_host_to_sram_read(burst_interface, audio_chan_driver->dpram_page, (u32 *)(audio_section->data + sizeof(ext_h264_nal_bitstream_t)), audio_section->payload_len, 0);
#endif
            audio_section->payload_len = gen_ext_nal_audio_header(audio_section->data, audio_section->type, audio_section->payload_len);
            audio_packet_queue->op->put_curr_producer_into_queue(audio_packet_queue);
        } else {
            audio_chan_driver->discard_number++;
            //printk("%s.%d:audio_chan_%d, discard\n", __FUNCTION__, __LINE__, audio_chan_driver->audio_logic_chan_id);
        }

        finish_id = audio_section_param->op->get_update_finish_id(audio_section_param);
        audio_encode->op->set_finish_rd_ptr(audio_encode, finish_id, chip);

        //notify data move req queue to update
        chip_free_irq(chip, IRQ_BURST_TYPE_INTR, audio_chan_driver);
        chip_dpram_controller->op->ack_read_audio_req(chip_dpram_controller, audio_chan_driver);
        driver_gen_deliver_event(&audio_chan_driver->opened_logic_chan_ed, 1);
        spin_unlock_irqrestore(&audio_chan_driver->fsm_matrix_call_lock, flags);
    }
    return ret;
}

static int audio_encode_chan_start_read_bitstream(dpram_request_t *dpram_req, void *context)
{
    int ret=1;
    if((dpram_req!=NULL) && (context!=NULL)){
        isil_audio_driver_t         *audio_chan_driver = (isil_audio_driver_t*)context;
        isil_chip_t                 *chip = audio_chan_driver->chip_audio->chip;
        audio_section_ptr_info_t    *audio_section_param = &audio_chan_driver->audio_section_param;
        dpram_control_t             *chip_dpram_controller;
        chip_ddr_burst_engine_t     *chip_ddr_burst_interface;
        ddr_burst_interface_t       *burst_interface;

        get_isil_dpram_controller(&chip_dpram_controller);
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        chip_ddr_burst_interface->op->get_ddr_burst_interface(chip_ddr_burst_interface, &burst_interface);
        if(chip_dpram_controller->op->is_can_submit_move_data_from_ddr_to_dpram_service_req(chip_dpram_controller, &audio_chan_driver->dpram_page, chip)){
            driver_gen_deliver_event(&audio_chan_driver->opened_logic_chan_ed, 1);
            chip_request_irq(chip, IRQ_BURST_TYPE_INTR, audio_encode_chan_response_ddr_burst_done_isr, "BURST", (void*)audio_chan_driver);
            burst_interface->op->start_nonblock_transfer_from_ddr_to_sram(burst_interface, audio_chan_driver->dpram_page, audio_section_param->ddr_end_addr, audio_section_param->page_id, AUDIO_SECTION_SIZE, audio_section_param->chip_a_or_b);
            driver_gen_deliver_event(&audio_chan_driver->opened_logic_chan_ed, 1);
        } else {
            dpram_request_queue_t   *request_queue;
            request_queue = &chip_dpram_controller->request_queue;
            request_queue->op->release_curr_consumer(request_queue);
            driver_gen_deliver_event(&audio_chan_driver->opened_logic_chan_ed, 1);
            driver_gen_deliver_event(&audio_chan_driver->opened_logic_chan_ed, 1);
            driver_gen_deliver_event(&audio_chan_driver->opened_logic_chan_ed, 1);
            printk("%s.%d: retrigger\n", __FUNCTION__, __LINE__);
        }
    }
    return ret;
}

static int  audio_encode_chan_delete_start_read_bitstream(dpram_request_t *dpram_req, void *context)
{
    int ret=1;
    if((dpram_req!=NULL) && (context!=NULL)){

        printk("%s.%d: retrigger\n", __FUNCTION__, __LINE__);
    }
    return ret;
}

void init_submit_recv_audio_bitstream_service(dpram_request_t *dpram_req, isil_audio_driver_t *audio_chan_driver)
{
    if((dpram_req!=NULL) || (audio_chan_driver!=NULL)){
        dpram_req->chip = audio_chan_driver->chip_audio->chip;
        dpram_req->type = DPRAM_NONBLOCK_TRANSFER_REQUEST;
        dpram_req->context = (void*)audio_chan_driver;
        dpram_req->req_callback = audio_encode_chan_start_read_bitstream;
        dpram_req->delete_req_notify = audio_encode_chan_delete_start_read_bitstream;
    }
}

static int  audio_decode_chan_response_ddr_burst_done_isr(int irq, void *context)
{
    isil_audio_driver_t   *audio_chan_driver = (isil_audio_driver_t*)context;
    int ret = 0;
    if(audio_chan_driver != NULL){
        chip_audio_decode_t         *audio_decode = &audio_chan_driver->chip_audio->audio_decode;
        audio_decode_ptr_control_t  *audio_decode_ptr = &audio_decode->audio_ptr;
        audio_section_ptr_info_t    *audio_section_param = &audio_chan_driver->audio_section_param;
        isil_chip_t                  *chip = audio_chan_driver->chip_audio->chip;
        dpram_control_t             *chip_dpram_controller;
        chip_ddr_burst_engine_t     *chip_ddr_burst_interface;
        ddr_burst_interface_t       *burst_interface;
        int finish_id;
        unsigned long   flags;

        spin_lock_irqsave(&audio_chan_driver->fsm_matrix_call_lock, flags);
        get_isil_dpram_controller(&chip_dpram_controller);
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        chip_ddr_burst_interface->op->get_ddr_burst_interface(chip_ddr_burst_interface, &burst_interface);
        burst_interface->op->clear_burst_done(burst_interface);
        finish_id = audio_section_param->op->get_update_finish_id(audio_section_param);
        audio_decode_ptr->op->set_finish_wr_ptr(audio_decode_ptr, finish_id, chip, audio_chan_driver->audio_logic_chan_id);
        audio_chan_driver->i_frame_serial++;
        //notify data move req queue to update
        chip_free_irq(chip, IRQ_BURST_TYPE_INTR, audio_chan_driver);
        chip_dpram_controller->op->ack_write_audio_req(chip_dpram_controller, audio_chan_driver);
        driver_gen_deliver_event(&audio_chan_driver->opened_logic_chan_ed, 1);
        spin_unlock_irqrestore(&audio_chan_driver->fsm_matrix_call_lock, flags);
        ret = 1;
    }
    return ret;
}

static int audio_decode_chan_start_send_bitstream(dpram_request_t *dpram_req, void *context)
{
    int ret=1;
    if((dpram_req!=NULL) && (context!=NULL)){
        isil_audio_driver_t           *audio_chan_driver = (isil_audio_driver_t*)context;
        isil_chip_t                  *chip = audio_chan_driver->chip_audio->chip;
        audio_section_ptr_info_t    *audio_section_param = &audio_chan_driver->audio_section_param;
        isil_audio_packet_queue_t     *audio_packet_queue = &audio_chan_driver->audio_packet_queue;
        isil_audio_chan_buf_pool_t    *audio_buf_pool = &audio_chan_driver->audio_buf_pool;
        dpram_control_t             *chip_dpram_controller;
        chip_ddr_burst_engine_t     *chip_ddr_burst_interface;
        ddr_burst_interface_t       *burst_interface;
        ed_tcb_t                    *ed_tcb = &audio_chan_driver->opened_logic_chan_ed;

        get_isil_dpram_controller(&chip_dpram_controller);
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        chip_ddr_burst_interface->op->get_ddr_burst_interface(chip_ddr_burst_interface, &burst_interface);
        audio_packet_queue->op->try_get_curr_consumer_from_queue(audio_packet_queue);
        if(audio_packet_queue->curr_consumer != NULL){
            isil_audio_packet_section_t   *audio_section = audio_packet_queue->curr_consumer;
            isil_audio_descriptor_t       *audio_decode_descriptor = &audio_section->descriptor;

            driver_gen_deliver_event(ed_tcb, 1);
            if(chip_dpram_controller->op->is_can_submit_move_data_from_host_to_dpram_service_req(chip_dpram_controller, &audio_chan_driver->dpram_page, chip)){
                dpram_page_node_t   *dpram_page;
                u32 dpram_base;

#if defined(PIO_INTERFACE)
                burst_interface->op->pio_host_to_sram_write(burst_interface, audio_chan_driver->dpram_page, (u32 *)(audio_section->data/* + sizeof(ext_h264_nal_bitstream_t)*/), audio_section->payload_len, 0);
#endif
#if defined(DMA_INTERFACE)
                audio_section->op->dma_map(audio_section, DMA_TO_DEVICE);
                burst_interface->op->dma_host_to_sram_write(burst_interface, audio_chan_driver->dpram_page, audio_section->dma_addr/*+sizeof(ext_h264_nal_bitstream_t)*/, audio_section->payload_len, 0);
                audio_section->op->dma_unmap(audio_section, DMA_TO_DEVICE);
#endif

                dpram_page = audio_chan_driver->dpram_page;
                dpram_base = dpram_page->op->get_page_base(dpram_page);
                dpram_base += audio_section->section_size;
                dpram_base -= sizeof(audio_section_descriptor_t);
                audio_decode_descriptor->op->set_audio_descriptor_valid_len(audio_decode_descriptor, audio_section->payload_len);
                audio_decode_descriptor->op->set_audio_descriptor_sample_rate(audio_decode_descriptor, audio_section->sample_rate);
                audio_decode_descriptor->op->set_audio_descriptor_type(audio_decode_descriptor, audio_section->type);
                audio_decode_descriptor->op->set_audio_descriptor_bit_wide(audio_decode_descriptor, audio_section->bit_wide);
                audio_decode_descriptor->op->set_audio_descriptor_timestamp(audio_decode_descriptor, audio_section->timestamp&0xffff);
                chip->io_op->chip_write32(chip, dpram_base, (*((u32*)audio_decode_descriptor->descriptor)));
                chip_request_irq(chip, IRQ_BURST_TYPE_INTR, audio_decode_chan_response_ddr_burst_done_isr, "BURST", (void*)audio_chan_driver);
                burst_interface->op->start_nonblock_transfer_from_sram_to_ddr(burst_interface, audio_chan_driver->dpram_page, audio_section_param->ddr_end_addr, audio_section_param->page_id, AUDIO_SECTION_SIZE, audio_section_param->chip_a_or_b);
                audio_packet_queue->op->release_curr_consumer(audio_packet_queue, audio_buf_pool);
            } else {

                dpram_request_queue_t   *request_queue;
                request_queue = &chip_dpram_controller->request_queue;
                request_queue->op->release_curr_consumer(request_queue);
                audio_chan_driver->discard_number++;
                driver_gen_deliver_event(ed_tcb, 1);
                driver_gen_deliver_event(ed_tcb, 1);
                chip_dpram_controller->op->ack_read_audio_req(chip_dpram_controller, audio_chan_driver);
                printk("%s.%d: retrigger\n", __FUNCTION__, __LINE__);
            }
            driver_gen_deliver_event(ed_tcb, 1);
        } else {
            ed_tcb->ed_fsm.op->change_state(&ed_tcb->ed_fsm, ISIL_ED_IDLE);
            chip_dpram_controller->op->ack_read_audio_req(chip_dpram_controller, audio_chan_driver);
        }
    }
    return ret;
}

static int  audio_decode_chan_delete_start_send_bitstream(dpram_request_t *dpram_req, void *context)
{
    int ret=1;
    if((dpram_req!=NULL) && (context!=NULL)){

        printk("%s.%d: retrigger\n", __FUNCTION__, __LINE__);
    }
    return ret;
}

void init_submit_send_audio_bitstream_service(dpram_request_t *dpram_req, isil_audio_driver_t *audio_chan_driver)
{
    if((dpram_req!=NULL) || (audio_chan_driver!=NULL)){
        dpram_req->chip = audio_chan_driver->chip_audio->chip;
        dpram_req->type = DPRAM_NONBLOCK_TRANSFER_REQUEST;
        dpram_req->context = (void*)audio_chan_driver;
        dpram_req->req_callback = audio_decode_chan_start_send_bitstream;
        dpram_req->delete_req_notify = audio_decode_chan_delete_start_send_bitstream;
    }
}

static void chip_audio_process_audio_encode(chip_audio_t *chip_audio)
{
    if(chip_audio != NULL) {
        isil_audio_driver_t *audio_chan_driver;
        isil_audio_chan_buf_pool_t  *audio_buf_pool;
        ed_tcb_t            *opened_logic_chan_ed;
        isil_ed_fsm_t       *ed_fsm;
        unsigned long   flags;
        int i;

        for(i=0; i<ISIL_AUDIO_IN_CHAN_NUMBER; i++){
            chip_audio->op->get_audio_chan_driver(chip_audio, i, &audio_chan_driver);
            if(audio_chan_driver == NULL){
                continue;
            }
            if(atomic_read(&audio_chan_driver->opened_flag) == 0){
                continue;
            }
            spin_lock_irqsave(&audio_chan_driver->fsm_matrix_call_lock, flags);
            opened_logic_chan_ed = &audio_chan_driver->opened_logic_chan_ed;
            ed_fsm = &opened_logic_chan_ed->ed_fsm;
            if(isil_chip_audio_is_in_robust_processing(chip_audio)){
                ed_fsm->op->feed_state_watchdog(ed_fsm);
                spin_unlock_irqrestore(&audio_chan_driver->fsm_matrix_call_lock, flags);        
                ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_chan_driver->audio_logic_chan_id);
                continue;
            }
            switch(ed_fsm->op->get_curr_state(ed_fsm)){
                case ISIL_ED_IDLE:
                    driver_gen_deliver_event(opened_logic_chan_ed, 1);
                    break;
                case ISIL_ED_SUSPEND:
                case ISIL_ED_UNREGISTER:
                    ed_fsm->op->feed_state_watchdog(ed_fsm);
                default:
                    break;
            }
            audio_buf_pool = &audio_chan_driver->audio_buf_pool;
            if(audio_buf_pool->op->get_audio_section_tcb_pool_entry_number(audio_buf_pool)){
                ed_fsm->op->update_state_timeout_counter(ed_fsm);
            } else {
                ed_fsm->op->feed_state_watchdog(ed_fsm);    
            }
            spin_unlock_irqrestore(&audio_chan_driver->fsm_matrix_call_lock, flags);        
        }
    }
}

static void chip_audio_process_audio_decode(chip_audio_t *chip_audio)
{
    if(chip_audio != NULL){
        isil_audio_driver_t *audio_chan_driver;
        isil_audio_chan_buf_pool_t  *audio_buf_pool;
        ed_tcb_t            *opened_logic_chan_ed;
        isil_ed_fsm_t       *ed_fsm;
        unsigned long   flags;
        int i;

        for(i=ISIL_AUDIO_OUT_SPEAKER_ID; i<=ISIL_AUDIO_OUT_PLAYBACK_ID; i++){
            chip_audio->op->get_audio_chan_driver(chip_audio, i, &audio_chan_driver);
            if(audio_chan_driver == NULL){
                continue;
            }
            if(atomic_read(&audio_chan_driver->opened_flag) == 0){
                continue;
            }
            spin_lock_irqsave(&audio_chan_driver->fsm_matrix_call_lock, flags);
            opened_logic_chan_ed = &audio_chan_driver->opened_logic_chan_ed;
            ed_fsm = &opened_logic_chan_ed->ed_fsm;
            if(isil_chip_audio_is_in_robust_processing(chip_audio)){
                ed_fsm->op->feed_state_watchdog(ed_fsm);
                spin_unlock_irqrestore(&audio_chan_driver->fsm_matrix_call_lock, flags);        
                ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_chan_driver->audio_logic_chan_id);
                continue;
            }
            switch(ed_fsm->op->get_curr_state(ed_fsm)){
                case ISIL_ED_IDLE:
                    driver_gen_deliver_event(opened_logic_chan_ed, 1);
                    break;
                case ISIL_ED_SUSPEND:
                case ISIL_ED_UNREGISTER:
                    ed_fsm->op->feed_state_watchdog(ed_fsm);
                default:
                    break;
            }
            audio_buf_pool = &audio_chan_driver->audio_buf_pool;
            if(audio_buf_pool->op->get_audio_section_tcb_pool_entry_number(audio_buf_pool)){
                ed_fsm->op->update_state_timeout_counter(ed_fsm);
            } else {
                ed_fsm->op->feed_state_watchdog(ed_fsm);    
            }
            spin_unlock_irqrestore(&audio_chan_driver->fsm_matrix_call_lock, flags);        
        }
    }
}

static void chip_audio_get_audio_chan_driver(chip_audio_t *chip_audio, int chan_id, isil_audio_driver_t **ptr_audio_driver)
{
    if((chip_audio!=NULL) && (ptr_audio_driver!=NULL)){
        *ptr_audio_driver = NULL;
        if((chan_id>=ISIL_AUDIO_IN_CHAN0_ID) && (chan_id<=ISIL_AUDIO_OUT_PLAYBACK_ID)){
            *ptr_audio_driver = &chip_audio->chip->audio_chan_driver[chan_id];
        }
    }
}

static struct chip_audio_operation	chip_audio_op = {
    .init = chip_audio_init,
    .reset = chip_audio_reset,
    .release = chip_audio_release,

    .set_encode_audio_chan_type = chip_audio_set_encode_audio_chan_type,
    .update_encode_audio_chan_type = chip_audio_update_encode_audio_chan_type,
    .get_encode_audio_chan_type = chip_audio_get_encode_audio_chan_type,

    .set_decode_audio_chan_type = chip_audio_set_decode_audio_chan_type,
    .update_decode_audio_chan_type = chip_audio_update_decode_audio_chan_type,
    .get_decode_audio_chan_type = chip_audio_get_decode_audio_chan_type,

    .set_enable_audio_chan = chip_audio_set_enable_audio_chan,
    .update_enable_audio_chan = chip_audio_update_enable_audio_chan,
    .get_audio_chan_enable_state = chip_audio_get_audio_chan_enable_state,

    .set_mute_audio_chan = chip_audio_set_mute_audio_chan,
    .update_mute_audio_chan = chip_audio_update_mute_audio_chan,
    .get_audio_chan_mute_state = chip_audio_get_audio_chan_mute_state,

    .get_chip_audio_param = chip_audio_get_chip_audio_param,
    .set_chip_audio_param = chip_audio_set_chip_audio_param,

    .process_audio_encode = chip_audio_process_audio_encode,
    .process_audio_decode = chip_audio_process_audio_decode,
    .get_audio_chan_driver = chip_audio_get_audio_chan_driver,
};

static int audio_proc_read(struct seq_file *seq, void *data)
{
    int i = 0;
    chip_audio_t *chip_audio = (chip_audio_t *)seq->private;
    isil_audio_driver_t           *audio_chan_driver;
    isil_audio_param_t            *running_param;


    running_param = &chip_audio->audio_encode.running_param;
    seq_printf(seq, "%8s %8s %8s\n", "BIT", "RATE", "TYPE");
    switch(running_param->op->get_bit_wide(running_param)) {
        case ISIL_AUDIO_8BIT:
            seq_printf(seq, "%8s ", "8bit");
            break;
        case ISIL_AUDIO_16BIT:
            seq_printf(seq, "%8s ", "16bit");
            break;
        default:
            seq_printf(seq, "%8s ", "N/A");
    }
    switch(running_param->op->get_sample_rate(running_param)) {
        case ISIL_AUDIO_8000:
            seq_printf(seq, "%8s ", "8K");
            break;
        case ISIL_AUDIO_16000:
            seq_printf(seq, "%8s ", "16K");
            break;
        case ISIL_AUDIO_32000:
            seq_printf(seq, "%8s ", "32K");
            break;
        case ISIL_AUDIO_44100:
            seq_printf(seq, "%8s ", "44.1K");
            break;
        case ISIL_AUDIO_48000:
            seq_printf(seq, "%8s ", "48K");
            break;
        default:
            seq_printf(seq, "%8s ", "N/A");
    }
    switch(running_param->op->get_type(running_param)) {
        case ISIL_AUDIO_PCM:
            seq_printf(seq, "%8s ", "PCM");
            break;
        case ISIL_AUDIO_ALAW:
            seq_printf(seq, "%8s ", "ALAW");
            break;
        case ISIL_AUDIO_ULAW:
            seq_printf(seq, "%8s ", "ULAW");
            break;
        case ISIL_AUDIO_ADPCM_32K:
            seq_printf(seq, "%8s ", "ADPCM_32K");
            break;
        case ISIL_AUDIO_ADPCM_16K:
            seq_printf(seq, "%8s ", "ADPCM_16K");
            break;
        case ISIL_AUDIO_ADPCM_48K:
            seq_printf(seq, "%8s ", "ADPCM_48K");
            break;
        default:
            seq_printf(seq, "%8s ", "N/A");
    }
    seq_printf(seq, "\nchannel state:\n");
    seq_printf(seq, "%8s %8s %8s %8s %8s %8s %8s\n", "logic_id", "type", "encode", "discard", "state", "queue", "pool");
    for(i = 0; i <= ISIL_AUDIO_OUT_PLAYBACK_ID; i++) {
        isil_audio_packet_queue_t     *audio_packet_queue;
        isil_audio_chan_buf_pool_t    *audio_buf_pool;
        chip_audio->op->get_audio_chan_driver(chip_audio, i, &audio_chan_driver);

        if(!audio_chan_driver || !atomic_read(&audio_chan_driver->opened_flag)) {
            continue;
        }
        audio_packet_queue = &audio_chan_driver->audio_packet_queue;
        audio_buf_pool     = &audio_chan_driver->audio_buf_pool;
        seq_printf(seq, "%8d %8d %8d %8d %8d %8d %8d\n", audio_chan_driver->audio_logic_chan_id, 
                audio_chan_driver->type, audio_chan_driver->i_frame_serial,
                audio_chan_driver->discard_number,
                audio_chan_driver->opened_logic_chan_ed.ed_fsm.op->get_curr_state(&audio_chan_driver->opened_logic_chan_ed.ed_fsm), 
                audio_packet_queue->op->get_curr_queue_entry_number(audio_packet_queue),
                audio_buf_pool->op->get_audio_section_tcb_pool_entry_number(audio_buf_pool));
    }

    return 0;
}

static int audio_proc_write(struct file *file, const char __user *buffer,
        size_t count, loff_t *offset)
{
    chip_audio_t *chip_audio = (chip_audio_t *)isil_file_get_seq_private(file);
    isil_audio_driver_t         *audio_chan_driver;
    isil_audio_driver_t         *audio_decoder_driver;
    isil_audio_packet_queue_t   *audio_packet_queue;
    isil_audio_packet_section_t *audio_encode_section;
    isil_audio_packet_queue_t   *audio_decoder_packet_queue;
    isil_audio_packet_section_t *audio_decoder_section;
    isil_chip_t                 *chip;
    struct file *audio_file;
    char name[128];
    mm_segment_t fs; 
    char cmdbuf[128];
    char **argv;
    int argc, i;
    u32 loop = 0;

    if(count > 128){
        if(copy_from_user(cmdbuf, buffer, 128) != 0){
            printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);                    
        }
    }
    else{
        if(copy_from_user(cmdbuf, buffer, count) != 0){
            printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);                    
        }
    }
    argv = argv_split(GFP_KERNEL, cmdbuf, &argc);
    for(i = 0; i < (argc - 1); i++){
        ISIL_DBG(ISIL_DBG_INFO, "cmd %d: %s\n", i, argv[i]);
    }
    if(!chip_audio){
        goto write_release;
    }
    if((argc > 3) && (strcmp(argv[2], "loop") == 0)) {
        loop = 1;
    }

    chip = chip_audio->chip;

    if(argv[0] != NULL) {
        ed_tcb_t *encoder_ed = NULL;
        ed_tcb_t *decoder_ed = NULL;
        memset(name, 0, 128);
        sprintf(name, "%s/audio_chn%d.pcm", argv[1], atoi(argv[0]));
        audio_file = isil_kernel_file_open(name);
        fs=get_fs();
        set_fs(KERNEL_DS);
        chip_audio->op->get_audio_chan_driver(chip_audio, atoi(argv[0]), &audio_chan_driver);
        chip_audio->op->get_audio_chan_driver(chip_audio, ISIL_AUDIO_OUT_SPEAKER_ID, &audio_decoder_driver);
        if(!audio_chan_driver || !audio_decoder_driver) {
            ISIL_DBG(ISIL_DBG_ERR, "no such driver\n");
            goto audio_end;
        }
        encoder_ed = &audio_chan_driver->opened_logic_chan_ed;
        decoder_ed = &audio_decoder_driver->opened_logic_chan_ed;
        if(!encoder_ed->op) {
            init_isil_audio_encode_chan(audio_chan_driver, chip_audio, 1, 0, 0);
        }
        if(!decoder_ed->op) {
            init_isil_audio_decode_chan(audio_decoder_driver, chip_audio, 1, 0, ISIL_AUDIO_OUT_SPEAKER_ID);
        }
        encoder_ed->op->open(encoder_ed);
        driver_gen_resume_event(encoder_ed, 1);
        decoder_ed->op->open(decoder_ed);
        driver_gen_resume_event(decoder_ed, 1);
        audio_packet_queue = &audio_chan_driver->audio_packet_queue;
        audio_decoder_packet_queue = &audio_decoder_driver->audio_packet_queue;
        while(1) {
            schedule();
            if (signal_pending(current)) {
                goto audio_end;
            }
            audio_packet_queue->op->try_get_curr_consumer_from_queue(audio_packet_queue);
            audio_encode_section = audio_packet_queue->curr_consumer;
            //audio_encode_section->op->reference(audio_encode_section, &audio_encode_section);
            if(audio_encode_section){
                audio_file->f_op->write(audio_file, audio_encode_section->data + sizeof(ext_h264_nal_bitstream_t), audio_encode_section->payload_len - sizeof(ext_h264_nal_bitstream_t), &(audio_file->f_pos));
                /*ai to ao*/
                if(loop) {
                    audio_decoder_packet_queue->op->try_get_curr_producer_from_pool(audio_decoder_packet_queue, &audio_decoder_driver->audio_buf_pool);
                    audio_decoder_section = audio_decoder_packet_queue->curr_producer;
                    if(audio_decoder_section) {
                        audio_decoder_section->payload_len = audio_encode_section->payload_len;
                        audio_decoder_section->sample_rate = audio_encode_section->sample_rate;
                        audio_decoder_section->type        = audio_encode_section->type;
                        audio_decoder_section->bit_wide    = audio_encode_section->bit_wide;
                        audio_decoder_section->timestamp   = audio_encode_section->timestamp;

                        memcpy(audio_decoder_section->data, audio_encode_section->data, audio_encode_section->section_size);
                        audio_decoder_packet_queue->op->put_curr_producer_into_queue(audio_decoder_packet_queue);
                    }else{
                        printk("%d:audio decoder discard\n", __LINE__);
                    }

                }
                audio_packet_queue->op->release_curr_consumer(audio_packet_queue, &audio_chan_driver->audio_buf_pool);
            }

        }
audio_end:
        if(encoder_ed) {
            encoder_ed->op->close(encoder_ed);
        }
        if(decoder_ed) {
            decoder_ed->op->close(decoder_ed);
        }
        set_fs(fs);
        isil_kernel_file_close(audio_file);
    }

write_release:
    argv_free(argv);
    return count;
}

static int chip_audio_top_level_irq_handler(int irq, void *context)
{
    isil_chip_t		*chip;
    chip_audio_t	*chip_audio;
    chip_audio_push_isr_interface_t *audio_push_isr_interface;
    isil_audio_driver_t	*audio_chan_driver;
    ed_tcb_t            *ed_tcb;
    isil_audio_chan_push_interface_t  *audio_chan_push_interface;
    chip_audio_encode_t *audio_encode;
    audio_encode_control_t  *audio_control;
    unsigned long   isr_flags, ping_pong_flags, chan_id_flags, chan_id, ping_or_pong, gen_deliver_event;
    u8  *src_buf;

    chip = (isil_chip_t*)context;
    chip_audio = &chip->chip_audio;
    audio_push_isr_interface = &chip_audio->audio_push_interface;
    audio_encode = &chip_audio->audio_encode;
    audio_control = &audio_encode->audio_control;

    isr_flags = chip->io_op->chip_read32(chip, INTERRUPT_FLAGS_EXT);
    if(isr_flags & (1 << IRQ_EXT_AUDIO_EOF_INTR)) {
        chan_id_flags = chip->io_op->chip_read32(chip, PCI_INTR_CHAN_SOURCE);
        chan_id_flags >>= PCI_AUDIO_CHAN_OFFSET_BASE;
        while(chan_id_flags){
            ping_pong_flags = chip->io_op->chip_read32(chip, PCI_PREVIEW_AUDIO_AB);
            ping_pong_flags >>= PCI_AUDIO_CHAN_OFFSET_BASE;
            for(chan_id=0; chan_id<16; chan_id++){
                gen_deliver_event = 0;
                if(chan_id_flags & (1<<chan_id)){
                    ping_or_pong = ping_pong_flags&(1<<chan_id);
                    if(ping_or_pong){
                        if(chan_id < 4){
                            dma_sync_single_for_cpu(NULL, audio_push_isr_interface->chan0_chan3_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                            src_buf = &audio_push_isr_interface->chan0_chan3_push_pong_addr[(chan_id&3)*AUDIO_SECTION_SIZE];
                        } else if(chan_id < 8){
                            dma_sync_single_for_cpu(NULL, audio_push_isr_interface->chan4_chan7_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                            src_buf = &audio_push_isr_interface->chan4_chan7_push_pong_addr[(chan_id&3)*AUDIO_SECTION_SIZE];
                        } else if(chan_id < 12){
                            dma_sync_single_for_cpu(NULL, audio_push_isr_interface->chan8_chan11_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                            src_buf = &audio_push_isr_interface->chan8_chan11_push_pong_addr[(chan_id&3)*AUDIO_SECTION_SIZE];
                        } else {
                            dma_sync_single_for_cpu(NULL, audio_push_isr_interface->chan12_chan15_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                            src_buf = &audio_push_isr_interface->chan12_chan15_push_pong_addr[(chan_id&3)*AUDIO_SECTION_SIZE];
                        }
                    } else {
                        if(chan_id < 4){
                            dma_sync_single_for_cpu(NULL, audio_push_isr_interface->chan0_chan3_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                            src_buf = &audio_push_isr_interface->chan0_chan3_push_ping_addr[(chan_id&3)*AUDIO_SECTION_SIZE];
                        } else if(chan_id < 8){
                            dma_sync_single_for_cpu(NULL, audio_push_isr_interface->chan4_chan7_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                            src_buf = &audio_push_isr_interface->chan4_chan7_push_ping_addr[(chan_id&3)*AUDIO_SECTION_SIZE];
                        } else if(chan_id < 12){
                            dma_sync_single_for_cpu(NULL, audio_push_isr_interface->chan8_chan11_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                            src_buf = &audio_push_isr_interface->chan8_chan11_push_ping_addr[(chan_id&3)*AUDIO_SECTION_SIZE];
                        } else {
                            dma_sync_single_for_cpu(NULL, audio_push_isr_interface->chan12_chan15_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                            src_buf = &audio_push_isr_interface->chan12_chan15_push_ping_addr[(chan_id&3)*AUDIO_SECTION_SIZE];
                        }
                    }

                    chip_audio->op->get_audio_chan_driver(chip_audio, chan_id, &audio_chan_driver);
                    if(audio_chan_driver != NULL){
                        audio_chan_push_interface = &audio_chan_driver->audio_chan_push_interface;
                        ed_tcb = &audio_chan_driver->opened_logic_chan_ed;
                        if(audio_chan_push_interface->op != NULL){
                            if(audio_chan_push_interface->op->chan_is_open(audio_chan_push_interface, audio_chan_driver) == ISIL_OK){
                                audio_chan_push_interface->op->copy_data(audio_chan_push_interface, audio_chan_driver, src_buf, AUDIO_SECTION_SIZE);
                                gen_deliver_event = 1;
                            }
                        } 
                    } else {
                        ed_tcb = NULL;
                    }

                    if(ping_or_pong){
                        if(chan_id < 4){
                            dma_sync_single_for_device(NULL, audio_push_isr_interface->chan0_chan3_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                        } else if(chan_id < 8){
                            dma_sync_single_for_device(NULL, audio_push_isr_interface->chan4_chan7_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                        } else if(chan_id < 12){
                            dma_sync_single_for_device(NULL, audio_push_isr_interface->chan8_chan11_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                        } else {
                            dma_sync_single_for_device(NULL, audio_push_isr_interface->chan12_chan15_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                        }
                    } else {
                        if(chan_id < 4){
                            dma_sync_single_for_device(NULL, audio_push_isr_interface->chan0_chan3_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                        } else if(chan_id < 8){
                            dma_sync_single_for_device(NULL, audio_push_isr_interface->chan4_chan7_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                        } else if(chan_id < 12){
                            dma_sync_single_for_device(NULL, audio_push_isr_interface->chan8_chan11_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                        } else {
                            dma_sync_single_for_device(NULL, audio_push_isr_interface->chan12_chan15_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
                        }
                    }

                    chip->io_op->chip_write32(chip, PCI_INTR_CHAN_SOURCE, ((1<<chan_id)<<PCI_AUDIO_CHAN_OFFSET_BASE));
                    audio_control->op->toggle_pci_flowcn(audio_control, chip);
                    if(gen_deliver_event && (ed_tcb!=NULL)){
                        driver_gen_deliver_event(ed_tcb, 1);
                    }
                }
            }
            chan_id_flags = chip->io_op->chip_read32(chip, PCI_INTR_CHAN_SOURCE);
            chan_id_flags >>= PCI_AUDIO_CHAN_OFFSET_BASE;
        }
        chip->io_op->chip_write32(chip, INTERRUPT_FLAGS_EXT, (1<<IRQ_EXT_AUDIO_EOF_INTR));
    }

    return 0;
}

static void	chip_audio_push_isr_interface_init(chip_audio_push_isr_interface_t *audio_push_isr_interface, int irq, void *context)
{
    atomic_set(&audio_push_isr_interface->counter, 0);
    spin_lock_init(&audio_push_isr_interface->lock);
    audio_push_isr_interface->chip_audio_isr = chip_audio_top_level_irq_handler;
    audio_push_isr_interface->context = context;
    audio_push_isr_interface->irq = irq;
}

static void	chip_audio_push_isr_interface_open_chip_audio(chip_audio_push_isr_interface_t *audio_push_isr_interface, int logic_chan_id)
{
    isil_chip_t     *chip;
    chip_audio_t    *chip_audio;
    chip_audio_encode_t     *audio_encode;
    audio_encode_control_t  *audio_control;
    unsigned long   flags, value;

    chip = (isil_chip_t *)audio_push_isr_interface->context;
    chip_audio = &chip->chip_audio;
    audio_encode = &chip_audio->audio_encode;
    audio_control = &audio_encode->audio_control;

    spin_lock_irqsave(&audio_push_isr_interface->lock, flags);
    if(atomic_read(&audio_push_isr_interface->counter) == 0){
        audio_push_isr_interface->chan0_chan3_push_ping_addr = get_push_buf(BUF_TYPE_AUDIO, 0, chip);
        audio_push_isr_interface->chan4_chan7_push_ping_addr = get_push_buf(BUF_TYPE_AUDIO, 0, chip);
        audio_push_isr_interface->chan8_chan11_push_ping_addr = get_push_buf(BUF_TYPE_AUDIO, 0, chip);
        audio_push_isr_interface->chan12_chan15_push_ping_addr = get_push_buf(BUF_TYPE_AUDIO, 0, chip);
        audio_push_isr_interface->chan0_chan3_push_pong_addr = get_push_buf(BUF_TYPE_AUDIO, 0, chip);
        audio_push_isr_interface->chan4_chan7_push_pong_addr = get_push_buf(BUF_TYPE_AUDIO, 0, chip);
        audio_push_isr_interface->chan8_chan11_push_pong_addr = get_push_buf(BUF_TYPE_AUDIO, 0, chip);
        audio_push_isr_interface->chan12_chan15_push_pong_addr = get_push_buf(BUF_TYPE_AUDIO, 0, chip);

        audio_push_isr_interface->chan0_chan3_push_ping_dma_addr = dma_map_single(NULL, audio_push_isr_interface->chan0_chan3_push_ping_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
        audio_push_isr_interface->chan0_chan3_push_pong_dma_addr = dma_map_single(NULL, audio_push_isr_interface->chan0_chan3_push_pong_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
        audio_push_isr_interface->chan4_chan7_push_ping_dma_addr = dma_map_single(NULL, audio_push_isr_interface->chan4_chan7_push_ping_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
        audio_push_isr_interface->chan4_chan7_push_pong_dma_addr = dma_map_single(NULL, audio_push_isr_interface->chan4_chan7_push_pong_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
        audio_push_isr_interface->chan8_chan11_push_ping_dma_addr = dma_map_single(NULL, audio_push_isr_interface->chan8_chan11_push_ping_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
        audio_push_isr_interface->chan8_chan11_push_pong_dma_addr = dma_map_single(NULL, audio_push_isr_interface->chan8_chan11_push_pong_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
        audio_push_isr_interface->chan12_chan15_push_ping_dma_addr = dma_map_single(NULL, audio_push_isr_interface->chan12_chan15_push_ping_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
        audio_push_isr_interface->chan12_chan15_push_pong_dma_addr = dma_map_single(NULL, audio_push_isr_interface->chan12_chan15_push_pong_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);

        dma_sync_single_for_device(NULL, audio_push_isr_interface->chan0_chan3_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
        dma_sync_single_for_device(NULL, audio_push_isr_interface->chan0_chan3_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
        dma_sync_single_for_device(NULL, audio_push_isr_interface->chan4_chan7_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
        dma_sync_single_for_device(NULL, audio_push_isr_interface->chan4_chan7_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
        dma_sync_single_for_device(NULL, audio_push_isr_interface->chan8_chan11_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
        dma_sync_single_for_device(NULL, audio_push_isr_interface->chan8_chan11_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
        dma_sync_single_for_device(NULL, audio_push_isr_interface->chan12_chan15_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
        dma_sync_single_for_device(NULL, audio_push_isr_interface->chan12_chan15_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);

        //add audio push base reg setting
        chip->io_op->chip_write32(chip, PCI_AUDIO_CHAN00_CHAN03_PING_BASE_ADDR_REG, audio_push_isr_interface->chan0_chan3_push_ping_dma_addr);
        chip->io_op->chip_write32(chip, PCI_AUDIO_CHAN00_CHAN03_PONG_BASE_ADDR_REG, audio_push_isr_interface->chan0_chan3_push_pong_dma_addr);
        chip->io_op->chip_write32(chip, PCI_AUDIO_CHAN04_CHAN07_PING_BASE_ADDR_REG, audio_push_isr_interface->chan4_chan7_push_ping_dma_addr);
        chip->io_op->chip_write32(chip, PCI_AUDIO_CHAN04_CHAN07_PONG_BASE_ADDR_REG, audio_push_isr_interface->chan4_chan7_push_pong_dma_addr);
        chip->io_op->chip_write32(chip, PCI_AUDIO_CHAN08_CHAN11_PING_BASE_ADDR_REG, audio_push_isr_interface->chan8_chan11_push_ping_dma_addr);
        chip->io_op->chip_write32(chip, PCI_AUDIO_CHAN08_CHAN11_PONG_BASE_ADDR_REG, audio_push_isr_interface->chan8_chan11_push_pong_dma_addr);
        chip->io_op->chip_write32(chip, PCI_AUDIO_CHAN12_CHAN15_PING_BASE_ADDR_REG, audio_push_isr_interface->chan12_chan15_push_ping_dma_addr);
        chip->io_op->chip_write32(chip, PCI_AUDIO_CHAN12_CHAN15_PONG_BASE_ADDR_REG, audio_push_isr_interface->chan12_chan15_push_pong_dma_addr);

        if(chip_request_irq(chip, audio_push_isr_interface->irq, audio_push_isr_interface->chip_audio_isr, "audio_push", (void*)chip) != ISIL_OK){
            printk("%s, %d: register audio push top isr err\n\r", __FUNCTION__, __LINE__);
        } else {
            value = chip->io_op->chip_read32(chip, PCI_MASTER_CONTROL);
            value &= 0xff0000ff;
            chip->io_op->chip_write32(chip, PCI_MASTER_CONTROL, value);

            value = chip->io_op->chip_read32(chip, PCI_MASTER_INTR_ENABLE_REG);
            value |= (1<<IRQ_EXT_AUDIO_EOF_INTR);
            chip->io_op->chip_write32(chip, PCI_MASTER_INTR_ENABLE_REG, value);
        }
    }
    atomic_inc(&audio_push_isr_interface->counter);
    spin_unlock_irqrestore(&audio_push_isr_interface->lock, flags);
}

static void	chip_audio_push_isr_interface_close_chip_audio(chip_audio_push_isr_interface_t *audio_push_isr_interface, int logic_chan_id)
{
    isil_chip_t *chip;
    unsigned long   flags, value;

    chip = (isil_chip_t *)audio_push_isr_interface->context;
    spin_lock_irqsave(&audio_push_isr_interface->lock, flags);
    if(atomic_read(&audio_push_isr_interface->counter)){
        if(atomic_dec_and_test(&audio_push_isr_interface->counter)){
            value = chip->io_op->chip_read32(chip, PCI_MASTER_INTR_ENABLE_REG);
            value &= ~(1<<IRQ_EXT_AUDIO_EOF_INTR);
            chip->io_op->chip_write32(chip, PCI_MASTER_INTR_ENABLE_REG, value);

            value = chip->io_op->chip_read32(chip, PCI_MASTER_CONTROL);
            value &= 0xff0000ff;
            chip->io_op->chip_write32(chip, PCI_MASTER_CONTROL, value);

            dma_unmap_single(NULL, audio_push_isr_interface->chan0_chan3_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
            dma_unmap_single(NULL, audio_push_isr_interface->chan0_chan3_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
            dma_unmap_single(NULL, audio_push_isr_interface->chan4_chan7_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
            dma_unmap_single(NULL, audio_push_isr_interface->chan4_chan7_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
            dma_unmap_single(NULL, audio_push_isr_interface->chan8_chan11_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
            dma_unmap_single(NULL, audio_push_isr_interface->chan8_chan11_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
            dma_unmap_single(NULL, audio_push_isr_interface->chan12_chan15_push_ping_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);
            dma_unmap_single(NULL, audio_push_isr_interface->chan12_chan15_push_pong_dma_addr, AUDIO_BUF_SIZE, DMA_FROM_DEVICE);

            release_push_buf(BUF_TYPE_AUDIO, 0, audio_push_isr_interface->chan0_chan3_push_ping_addr, chip);
            release_push_buf(BUF_TYPE_AUDIO, 0, audio_push_isr_interface->chan4_chan7_push_ping_addr, chip);
            release_push_buf(BUF_TYPE_AUDIO, 0, audio_push_isr_interface->chan8_chan11_push_ping_addr, chip);
            release_push_buf(BUF_TYPE_AUDIO, 0, audio_push_isr_interface->chan12_chan15_push_ping_addr, chip);
            release_push_buf(BUF_TYPE_AUDIO, 0, audio_push_isr_interface->chan0_chan3_push_pong_addr, chip);
            release_push_buf(BUF_TYPE_AUDIO, 0, audio_push_isr_interface->chan4_chan7_push_pong_addr, chip);
            release_push_buf(BUF_TYPE_AUDIO, 0, audio_push_isr_interface->chan8_chan11_push_pong_addr, chip);
            release_push_buf(BUF_TYPE_AUDIO, 0, audio_push_isr_interface->chan12_chan15_push_pong_addr, chip);

            chip_free_irq(chip, audio_push_isr_interface->irq, (void*)chip);
        }
    }
    spin_unlock_irqrestore(&audio_push_isr_interface->lock, flags);
}

static void chip_audio_push_isr_interface_disable_chip_audio_chan(chip_audio_push_isr_interface_t *audio_push_isr_interface, int logic_chan_id)
{
    isil_chip_t     *chip;
    chip_audio_t    *chip_audio;
    unsigned long   flags, value;

    spin_lock_irqsave(&audio_push_isr_interface->lock, flags);

    chip = (isil_chip_t *)audio_push_isr_interface->context;
    value = chip->io_op->chip_read32(chip, PCI_MASTER_CONTROL);
    value &= ~(0x100<<logic_chan_id);
    chip->io_op->chip_write32(chip, PCI_MASTER_CONTROL, value);

    chip_audio = &chip->chip_audio;
    chip_audio->op->set_enable_audio_chan(chip_audio, logic_chan_id, AUDIO_DISABLE);

    spin_unlock_irqrestore(&audio_push_isr_interface->lock, flags);
}

static void chip_audio_push_isr_interface_enable_chip_audio_chan(chip_audio_push_isr_interface_t *audio_push_isr_interface, int logic_chan_id)
{
    isil_chip_t     *chip;
    chip_audio_t    *chip_audio;
    unsigned long   flags, value;

    spin_lock_irqsave(&audio_push_isr_interface->lock, flags);
    chip = (isil_chip_t *)audio_push_isr_interface->context;
    chip_audio = &chip->chip_audio;
    chip_audio->op->set_enable_audio_chan(chip_audio, logic_chan_id, AUDIO_ENABLE);

    value = chip->io_op->chip_read32(chip, PCI_MASTER_CONTROL);
    value |= (0x100<<logic_chan_id);
    chip->io_op->chip_write32(chip, PCI_MASTER_CONTROL, value);

    if(atomic_read(&audio_push_isr_interface->counter) == 1){
        chip_audio_encode_t     *audio_encode;
        audio_encode_control_t  *audio_control;

        audio_encode = &chip_audio->audio_encode;
        audio_control = &audio_encode->audio_control;
        audio_control->op->toggle_pci_flowcn(audio_control, chip);
    }
    spin_unlock_irqrestore(&audio_push_isr_interface->lock, flags);
}

static int  chip_audio_push_isr_interface_curr_open_chip_audio_chan_number(chip_audio_push_isr_interface_t *audio_push_isr_interface)
{
    return atomic_read(&audio_push_isr_interface->counter);
}

static struct chip_audio_push_isr_interface_operation    chip_audio_push_isr_interface_op = {
    .init = chip_audio_push_isr_interface_init,
    .open_chip_audio = chip_audio_push_isr_interface_open_chip_audio,
    .close_chip_audio = chip_audio_push_isr_interface_close_chip_audio,
    .disable_chip_audio_chan = chip_audio_push_isr_interface_disable_chip_audio_chan,
    .enable_chip_audio_chan = chip_audio_push_isr_interface_enable_chip_audio_chan,
    .curr_open_chip_audio_chan_number = chip_audio_push_isr_interface_curr_open_chip_audio_chan_number,
};

static void	init_chip_audio_push_isr_interface(chip_audio_push_isr_interface_t *audio_push_isr_interface, void *context)
{
    audio_push_isr_interface->op = &chip_audio_push_isr_interface_op;
    audio_push_isr_interface->op->init(audio_push_isr_interface, IRQ_AUDIO_EOF_TYPE_INTR, context);
}

void    start_chip_audio_robust_process(chip_audio_t *chip_audio)
{
    robust_processing_control_t *chip_audio_robust;

    start_chip_robust_process(chip_audio->chip);
    chip_audio_robust = &chip_audio->chip_audio_robust;
    chip_audio_robust->op->start_robust_processing(chip_audio_robust);
}

void    isil_chip_audio_robust_process_done(chip_audio_t *chip_audio)
{
    robust_processing_control_t *chip_audio_robust;
    chip_audio_robust = &chip_audio->chip_audio_robust;
    chip_audio_robust->op->robust_processing_done(chip_audio_robust);
}

void    isil_chip_audio_wait_robust_process_done(chip_audio_t *chip_audio)
{
    robust_processing_control_t *chip_audio_robust;

    chip_wait_robust_process_done(chip_audio->chip);
    chip_audio_robust = &chip_audio->chip_audio_robust;
    chip_audio_robust->op->wait_robust_processing_done(chip_audio_robust);
}

int isil_chip_audio_is_in_robust_processing(chip_audio_t *chip_audio)
{
    robust_processing_control_t *chip_audio_robust;
    int ret = 0;

    chip_audio_robust = &chip_audio->chip_audio_robust;
    if(chip_audio_robust->op->is_in_robust_processing(chip_audio_robust) ||
            chip_is_in_robust_processing(chip_audio->chip)){
        ret = 1;
    }
    return ret;
}

int init_chip_audio(chip_audio_t *chip_audio, isil_chip_t *chip, int bus_id)
{
    int	ret = ISIL_ERR;
    if((chip_audio!=NULL) && (chip!=NULL)){
        isil_proc_register_s *audio_proc;
        chip_audio->chip = chip;
        chip_audio->op = &chip_audio_op;
        ret = chip_audio->op->init(chip_audio);
        if(ret) {
            ISIL_DBG(ISIL_DBG_ERR, "init chip_audio failed %d\n", ret);
            return ret;
        }
        chip_audio->op->set_chip_audio_param(chip_audio);

        init_robust_processing_control(&chip_audio->chip_audio_robust);
        init_chip_audio_push_isr_interface(&chip_audio->audio_push_interface, chip);

        audio_proc = &chip->audio_proc;
        strcpy(audio_proc->name, "audio");
        audio_proc->read  = audio_proc_read;
        audio_proc->write = audio_proc_write;
        audio_proc->private = chip_audio;
        isil_module_register(chip, audio_proc);
        ret = ISIL_OK;
    }
    return ret;
}

void	remove_chip_audio(chip_audio_t *chip_audio)
{
    if(chip_audio != NULL){
        isil_module_unregister(chip_audio->chip, &chip_audio->chip->audio_proc);
        chip_audio->audio_enable.op->update_enable_pcm_encode(&chip_audio->audio_enable, AUDIO_DISABLE);
        chip_audio->audio_enable.op->update_enable_adpcm_encode(&chip_audio->audio_enable, AUDIO_DISABLE);
        chip_audio->audio_enable.op->update_enable_adpcm_decode(&chip_audio->audio_enable, AUDIO_DISABLE);
        chip_audio->op->set_chip_audio_param(chip_audio);
    }
}


