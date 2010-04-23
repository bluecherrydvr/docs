/* mp4read.c */
#include <assert.h>
#include "internal.h"
#include "bytestrm.h"

// codec tags
extern const codec_tag_t codec_video_tags[];
extern const codec_tag_t codec_audio_tags[];
extern const codec_tag_t mp4_obj_type[];

// function prototypes
static void mp4_build_index(mp4_context_t *ctx, mp4_track_t *track);
static sx_int32 mp4_read_index_fragment_mode(mp4_context_t *ctx);
static sx_int32 mp4_build_index_fragment_mode(mp4_context_t *ctx);
static void mp4_build_moof_index(mp4_context_t *ctx);
static sx_int32 mp4_read_header(mp4_context_t *ctx);
static sx_int32 mp4_find_smff(mp4_context_t *ctx, mp4_atom_t *atom);
static sx_int32 mp4_read_smff(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_default(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_one_moof(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_ftyp(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_free(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_mdat(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_udta(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_moov(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_mvhd(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_trak(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_tkhd(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_mdhd(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_hdlr(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_stco(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_ctts(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_elst(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_avcc(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_stco(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_stsc(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_stsd(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_stss(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_stsz(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_stts(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_esds(mp4_context_t *ctx, mp4_atom_t atom);
// fragment mode
static sx_int32 mp4_read_mvex(mp4_context_t *ctx, mp4_atom_t atom);
static sx_int32 mp4_read_trex(mp4_context_t *ctx, mp4_atom_t atom);

// parse table
static const mp4_parse_table_t mp4_default_parse_table[] = {
    { MKTAG( 'a', 'v', 'c', 'C' ), mp4_read_avcc },
    { MKTAG( 'c', 'o', '6', '4' ), mp4_read_stco },
    { MKTAG( 'c', 't', 't', 's' ), mp4_read_ctts },
    { MKTAG( 'e', 'd', 't', 's' ), mp4_read_default },
    { MKTAG( 'e', 'l', 's', 't' ), mp4_read_elst },
    { MKTAG( 'e', 's', 'd', 's' ), mp4_read_esds },
    { MKTAG( 'f', 'r', 'e', 'e' ), mp4_read_free },
    { MKTAG( 'f', 't', 'y', 'p' ), mp4_read_ftyp },
    { MKTAG( 'h', 'd', 'l', 'r' ), mp4_read_hdlr },
    { MKTAG( 'm', 'd', 'a', 't' ), mp4_read_mdat },
    { MKTAG( 'm', 'd', 'h', 'd' ), mp4_read_mdhd },
    { MKTAG( 'm', 'd', 'i', 'a' ), mp4_read_default },
    { MKTAG( 'm', 'i', 'n', 'f' ), mp4_read_default },
    { MKTAG( 'm', 'o', 'o', 'v' ), mp4_read_moov },
    { MKTAG( 'm', 'v', 'h', 'd' ), mp4_read_mvhd },
    { MKTAG( 's', 't', 'b', 'l' ), mp4_read_default },
    { MKTAG( 's', 't', 'c', 'o' ), mp4_read_stco },
    { MKTAG( 's', 't', 's', 'c' ), mp4_read_stsc },
    { MKTAG( 's', 't', 's', 'd' ), mp4_read_stsd },
    { MKTAG( 's', 't', 's', 's' ), mp4_read_stss },
    { MKTAG( 's', 't', 's', 'z' ), mp4_read_stsz },
    { MKTAG( 's', 't', 't', 's' ), mp4_read_stts },
    { MKTAG( 't', 'k', 'h', 'd' ), mp4_read_tkhd },
    { MKTAG( 't', 'r', 'a', 'k' ), mp4_read_trak },
    { MKTAG( 'u', 'd', 't', 'a' ), mp4_read_udta },
    // fragment mode
    { MKTAG( 'm', 'v', 'e', 'x' ), mp4_read_mvex },
    { MKTAG( 't', 'r', 'e', 'x' ), mp4_read_trex },
    { 0, NULL }
};

// get sample size for basic audio codecs
int av_get_bits_per_sample(codec_id_t codec_id)
{
    switch (codec_id) {
    case CODEC_ID_PCM_ALAW:
    case CODEC_ID_PCM_MULAW:
    case CODEC_ID_PCM_S8:
    case CODEC_ID_PCM_U8:
        return 8;
    case CODEC_ID_PCM_S16BE:
    case CODEC_ID_PCM_S16LE:
    case CODEC_ID_PCM_U16BE:
    case CODEC_ID_PCM_U16LE:
        return 16;
    default:
        return 8;
    }
    return 8;
}

// open for read
sx_int32 mp4_read_open(mp4_context_t *ctx)
{
    sx_int32 err = MP4MUX_ERR_OK;
    double percent = 0.0;
    mp4_atom_t atom, moov;
    offset_t pos;
    sx_uint32 bs_flags = BS_FLAG_READ;

    // create byte stream
    byte_strm_t *bs = malloc(sizeof(byte_strm_t));
    if (bs == NULL)
        return MP4MUX_ERR_MEMORY;
    if (CHK_FLAGS(ctx->flags, MP4_FLAG_REPAIR))
        SET_FLAGS(bs_flags, BS_FLAG_WRITE);
    if ((err = bs_init(bs, ctx->fd, bs_flags, ctx->bs_buf_size)))
        return err;
    ctx->stream = bs;
    ctx->file_size = bs_get_size(bs);
    ctx->mdat_size = 0;

    // find smff
    atom.offset = 0;
    atom.size = ctx->file_size;
    bs_rseek(bs, 0, SEEK_SET);
    err = mp4_find_smff(ctx, &atom);

    if (err == MP4MUX_ERR_OK 
        && !CHK_FLAGS(ctx->smff_flags, SMFF_FLAG_CLOSING_MOOV)) {
        // found smff but not closing moov
        memset(&atom, 0, sizeof(mp4_atom_t));
        memset(&moov, 0, sizeof(mp4_atom_t));

        // search moov in free boxes to construct movie header
        pos = bs_get_size(bs);
        atom.size = pos;
        bs_rseek(bs, 0, SEEK_SET);
        if (mp4_find_moov_sx(bs, &atom, &moov)) {
            mp4_parse_moov_sx(ctx, &moov);
            if (ctx->mux_mode == MP4_READ_PLAY) {
                SET_FLAGS(ctx->flags, MP4_FLAG_FRAGMENT_PLAY);
            }
        }
        else
            return MP4MUX_ERR_PARSE_ERROR; // no moov found, return error
    }
    else {
        err = mp4_read_header(ctx);

        // check error
        if (err == MP4MUX_ERR_PARSE_ERROR)
            return err;
        // return if there is no mdat
        if (!CHK_FLAGS(ctx->flags, MP4_FLAG_FOUND_MDAT))
            return MP4MUX_ERR_PARSE_NO_MDAT;

        // print statistics
        if (ctx->file_size > 0)
            percent = (double)(ctx->file_size - ctx->mdat_size) / (double)ctx->file_size * 100.0;
        mp4_log(ctx->log, "file size = %I64d, ", ctx->file_size);
        mp4_log(ctx->log, "mdat size = %I64d, ", ctx->mdat_size);
        mp4_log(ctx->log, "overhead = %6.3f%%\n", percent);

        // moov box was not found, check if fragment mode
        if ((ctx->mux_mode == MP4_READ_PLAY || ctx->mux_mode == MP4_READ_INFO)
            && !CHK_FLAGS(ctx->flags, MP4_FLAG_FOUND_MOOV)) {
            memset(&atom, 0, sizeof(mp4_atom_t));
            memset(&moov, 0, sizeof(mp4_atom_t));

            // search moov in free boxes to construct movie header
            pos = bs_get_size(bs);
            atom.size = pos;
            bs_rseek(bs, 0, SEEK_SET);
            if (mp4_find_moov_sx(bs, &atom, &moov)) {
                mp4_parse_moov_sx(ctx, &moov);
                if (ctx->mux_mode == MP4_READ_PLAY) {
                    SET_FLAGS(ctx->flags, MP4_FLAG_FRAGMENT_PLAY);
                    SET_FLAGS(ctx->flags, MP4_FLAG_PARSE_MOOF);
                }
            }
            else
                return MP4MUX_ERR_PARSE_ERROR; // no moov found, return error
        }
    }

    return err;
}

// look for smff box
static sx_int32 mp4_find_smff(mp4_context_t *ctx, mp4_atom_t *atom)
{
    byte_strm_t *bs = ctx->stream;
    sx_int32 err = MP4MUX_ERR_PARSE_ERROR;
    mp4_atom_t a, smff;
    sx_int64 total_size = 0;
    sx_int32 box_cnt = 0;

    a.offset = atom->offset;

    // assume smff box is within first 10 boxes
    while (atom->size > 8 && box_cnt < 10) {
        a.size = bs_get_be32(bs);
        a.type = bs_get_le32(bs);

        total_size += 8;
        a.offset += 8;
        if (a.size == 1) { // 64 bit extended size 
            a.size = bs_get_be64(bs) - 8;
            a.offset += 8;
            total_size += 8;
        }
        if (a.size == 0) {
            a.size = atom->size - total_size;
            if (a.size <= 8)
                break;
        }
        a.size -= 8;
        if (a.size < 0)
            break;
        a.size = MIN(a.size, atom->size);

        // looking for 'free' box
        if (a.type != MKTAG( 'f', 'r', 'e', 'e' ) || a.size <= 8) {
            // not 'free' box, skip it
            bs_skip(bs, a.size);
            box_cnt++;
        } 
        else {
            smff.offset = bs_rtell(bs);
            smff.size = bs_get_be32(bs);
            smff.type = bs_get_le32(bs);
            if (smff.type == MKTAG( 's', 'm', 'f', 'f' )) {
                mp4_read_smff(ctx, a);
                atom->offset += a.size + 8;
                atom->size -= a.size + 8;
                return MP4MUX_ERR_OK;
            }
            else { // not a smff box, skip it
                bs_skip(bs, smff.size - 8);
                box_cnt++;
            }
        }

        a.offset += a.size;
        total_size += a.size;
        atom->offset += a.size + 8;
        atom->size -= a.size + 8;
    }

    return err;
}

// open for reading info
sx_int32 mp4_read_info(mp4_context_t *ctx)
{
    sx_int32 err = MP4MUX_ERR_OK;
    mp4_atom_t atom, moov;
    offset_t pos;

    // create byte stream
    byte_strm_t *bs = malloc(sizeof(byte_strm_t));
    if (bs == NULL)
        return MP4MUX_ERR_MEMORY;
    if ((err = bs_init(bs, ctx->fd, BS_FLAG_READ, ctx->bs_buf_size)))
        return err;
    ctx->stream = bs;
    ctx->file_size = bs_get_size(bs);
    ctx->mdat_size = 0;

    // find smff
    atom.offset = 0;
    atom.size = ctx->file_size;
    bs_rseek(bs, 0, SEEK_SET);
    err = mp4_find_smff(ctx, &atom);

    if (err == MP4MUX_ERR_OK 
        && !CHK_FLAGS(ctx->smff_flags, SMFF_FLAG_CLOSING_MOOV)) {
        // found smff but not closing moov
        memset(&atom, 0, sizeof(mp4_atom_t));
        memset(&moov, 0, sizeof(mp4_atom_t));

        // search moov in free boxes to construct movie header
        pos = bs_get_size(bs);
        atom.size = pos;
        bs_rseek(bs, 0, SEEK_SET);
        if (mp4_find_moov_sx(bs, &atom, &moov)) {
            mp4_parse_moov_sx(ctx, &moov);
            if (ctx->mux_mode == MP4_READ_PLAY) {
                SET_FLAGS(ctx->flags, MP4_FLAG_FRAGMENT_PLAY);
            }
        }
        else
            return MP4MUX_ERR_PARSE_ERROR; // no moov found, return error
    }
    else {
        err = mp4_read_header(ctx);

        // check error
        if (err == MP4MUX_ERR_PARSE_ERROR)
            return err;
        // return if there is no mdat
        if (!CHK_FLAGS(ctx->flags, MP4_FLAG_FOUND_MDAT))
            return MP4MUX_ERR_PARSE_NO_MDAT;

        // moov box was not found, check if fragment mode
        if ((ctx->mux_mode == MP4_READ_PLAY || ctx->mux_mode == MP4_READ_INFO)
            && !CHK_FLAGS(ctx->flags, MP4_FLAG_FOUND_MOOV)) {
            memset(&atom, 0, sizeof(mp4_atom_t));
            memset(&moov, 0, sizeof(mp4_atom_t));

            // search moov in free boxes to construct movie header
            pos = bs_get_size(bs);
            atom.size = pos;
            bs_rseek(bs, 0, SEEK_SET);
            if (mp4_find_moov_sx(bs, &atom, &moov)) {
                mp4_parse_moov_sx(ctx, &moov);
            }
            else
                return MP4MUX_ERR_PARSE_ERROR; // no moov found, return error
        }
    }

    return err;
}

// close for read
sx_int32 mp4_read_close(mp4_context_t *ctx)
{
    sx_int32 i, j;

    for (i=0; i<ctx->nb_tracks; i++) {
        mp4_track_t *trk = ctx->tracks[i];
        mp4_freep((void *)&trk->ctts_data);
        mp4_freep((void *)&trk->samples_rd);
        mp4_freep((void *)&trk->elst_data);
        // free stsd data
        for (j=0; j<(int)trk->stsd_count; j++) {
            mp4_freep((void *)&trk->stsd_data[j].sps_data);
            mp4_freep((void *)&trk->stsd_data[j].pps_data);
        }
        mp4_freep((void *)&trk->stsd_data);
        // free track
        mp4_freep((void *)&ctx->tracks[i]);
    }
    ctx->nb_tracks = 0;

    mp4_freep((void*)&ctx->moof_list);
    bs_close(ctx->stream);
    mp4_freep((void **)&ctx->stream);
    return MP4MUX_ERR_OK;
}

// read header
static sx_int32 mp4_read_header(mp4_context_t *ctx)
{
    sx_int32 err, i;
    mp4_atom_t atom = { 0, 0, 0 };
    byte_strm_t *bs = (byte_strm_t *)ctx->stream;

    if (CHK_FLAGS(ctx->flags, MP4_FLAG_FRAGMENT_PLAY)) {
        atom.offset = ctx->moof_list[ctx->moof_cnt - 1].offset - 8;
        atom.size = ctx->file_size - atom.offset;
        bs_rseek(bs, atom.offset, SEEK_SET);
    }
    else {
        atom.size = ctx->file_size;//(INT64_MAX);
        bs_rseek(bs, 0, SEEK_SET);
    }

    // parse file
    err = mp4_read_default(ctx, atom);

    // update tracks
    if (err != MP4MUX_ERR_PARSE_ERROR 
        && CHK_FLAGS(ctx->flags, MP4_FLAG_FOUND_MOOV)) {
        for (i=0; i<ctx->nb_tracks; i++) {
            mp4_track_t *trk = ctx->tracks[i];

            if (!trk->info.timescale)
                trk->info.timescale = ctx->timescale;

            if (!CHK_FLAGS(ctx->flags, MP4_FLAG_FRAGMENT_PLAY)) {
                // check sample count in the track
                if (!trk->stts_count || !trk->stco_count || !trk->stsc_count 
                    || (!trk->info.sample_size && !trk->sample_count)) {
                    // some sample tables are empty, ignore this track
                    trk->sample_count = 0;
                    continue;
                }
                // constant audio frame size, currently not used
                if (trk->info.type == MP4_TRACK_AUDIO && trk->stts_count == 1)
                    trk->frame_size = trk->stts_data[0].duration;
                // build index for playback
                if (CHK_FLAGS(ctx->flags, MP4_FLAG_PARSE_SAMPLE_TABLE)) {
                    trk->alloc_entries = 0;
                    mp4_build_index(ctx, trk);
                }
            }
        }
    }

    // free track buffers
    for (i=0; i<ctx->nb_tracks; i++) {
        mp4_track_t *trk = ctx->tracks[i];
        mp4_freep((void *)&trk->stco_data);
        mp4_freep((void *)&trk->stsc_data);
        mp4_freep((void *)&trk->stsz_data);
        mp4_freep((void *)&trk->stss_data);
        mp4_freep((void *)&trk->stts_data);
    }
    return err;
}

// read one movie fragment
static sx_int32 mp4_read_one_moof(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    sx_int64 parsed_size = 0;
    mp4_atom_t a;
    sx_int32 err = MP4MUX_ERR_OK;

    if (atom.size < 8)
        return MP4MUX_ERR_NOT_ENOUGH_DATA;

    a.offset = atom.offset;
    while (((parsed_size + 8) < atom.size) && !bs_eof(bs) && !err) {
		// read atom
        a.size = bs_get_be32(bs);
        a.type = bs_get_le32(bs);
        parsed_size += 8;
        a.offset += 8;
        if (a.size == 1) { // 64 bit extended size 
            a.size = bs_get_be64(bs) - 8;
            a.offset += 8;
            parsed_size += 8;
        }
        if (a.size == 0) {
			// atom size not updated, return error
			return MP4MUX_ERR_NOT_ENOUGH_DATA;
        }
        a.size -= 8;
        // a.size can be 0 (such as 'wide' box)
        if (a.size < 0)
			return MP4MUX_ERR_PARSE_ERROR;
		if (atom.size - parsed_size < a.size)
			return MP4MUX_ERR_NOT_ENOUGH_DATA;

        if (a.type == MKTAG( 'f', 'r', 'e', 'e' )) {
            // parse free box
            offset_t start_pos = bs_rtell(bs);
            sx_int32 moof_cnt = ctx->moof_cnt;
            sx_int64 left;
            // read free box
            err = mp4_read_free(ctx, a);
            left = a.size - bs_rtell(bs) + start_pos;
            // skip extra data
            if (left > 0)  
                bs_skip(bs, left);
            // return if found one or more moof
            if (ctx->moof_cnt > moof_cnt)
                break;
        }
		else if (a.type == MKTAG( 'm', 'o', 'o', 'v' )) {
			// found closing moov
			SET_FLAGS(ctx->flags, MP4_FLAG_WRITE_CLOSED);
            bs_skip(bs, a.size);
			break;
		}
        else {
            // skip the box
            bs_skip(bs, a.size);
        }

        a.offset += a.size;
        parsed_size += a.size;
    }

    return err;
}

// read index in fragment mode
static sx_int32 mp4_read_index_fragment_mode(mp4_context_t *ctx)
{
    sx_int32 err = MP4MUX_ERR_OK;
    mp4_atom_t atom = { 0, 0, 0 };
    byte_strm_t *bs = (byte_strm_t *)ctx->stream;

    if (ctx->moof_cnt == 0)
        atom.offset = 0;
    else
        atom.offset = ctx->moof_list[ctx->moof_cnt - 1].offset - 8;
    atom.size = ctx->file_size - atom.offset;
    bs_rseek(bs, atom.offset, SEEK_SET);

    // read one moof
    err = mp4_read_one_moof(ctx, atom);

    return err;
}

// get packet data
sx_int32 mp4_get_packet_data(byte_strm_t *bs, mp4_packet_t *pkt, sx_int32 size)
{
    sx_int32 ret = MP4MUX_ERR_OK;
    pkt->pos = bs_rtell(bs);
    ret = bs_get_buffer(bs, pkt->data, size);
    pkt->size = ret;
    return ret;
}

// get a packet
sx_int32 mp4_get_packet(mp4_context_t *ctx, mp4_packet_t *pkt)
{
    byte_strm_t *bs = ctx->stream;
    mp4_track_t *cur_trk = 0;
    sample_entry_t *sample = 0;
    sx_int64 best_dts = INT64_MAX;
    sx_int32 i, err;

    // buffer should be allocated by upper layer
    if (pkt->data == NULL || pkt->size <= 0)
        return MP4MUX_ERR_PACKET_BUFFER; 

    // force parsing if there is no more sample
    for (i=0; i<ctx->nb_tracks; i++) {
        mp4_track_t *trk = ctx->tracks[i];
        if (trk->current_sample >= trk->sample_count) {
            SET_FLAGS(ctx->flags, MP4_FLAG_FORCE_PARSE_MOOF);
            break;
        }
    }

    // build index in fragment mode
    if ((err = mp4_build_index_fragment_mode(ctx)) != MP4MUX_ERR_OK)
        return err;

    pkt->flags = 0; // clear flags
    for (i=0; i<ctx->nb_tracks; i++) {
        mp4_track_t *trk = ctx->tracks[i];
        if (!CHK_FLAGS(trk->flags, TRACK_FLAG_DISABLED) && trk->current_sample < trk->sample_count) {
            sample_entry_t *current_sample = &trk->samples_rd[trk->current_sample];
            sx_int64 dts = rescale_rnd(current_sample->dts, 
                (sx_int64)MP4_AV_TIME_BASE, (sx_int64)trk->info.timescale);
            //mp4_log(ctx->log, "stream %d, sample %d, dts %"PRId64"\n", i, trk->current_sample, dts);
            if (!sample || current_sample->pos < sample->pos) {
                sample = current_sample;
                best_dts = dts;
                cur_trk = trk;
            }
        }
    }
    if (!sample) {
        if (CHK_FLAGS(ctx->flags, MP4_FLAG_FRAGMENT_PLAY)) {
            if (CHK_FLAGS(ctx->flags, MP4_FLAG_WRITE_CLOSED))
                return MP4MUX_ERR_END_OF_FILE;
            else {
                SET_FLAGS(ctx->flags, MP4_FLAG_FORCE_PARSE_MOOF);
                return MP4MUX_ERR_NO_MORE_SAMPLES;
            }
        }
        else
            return MP4MUX_ERR_END_OF_FILE;
    }

    // check packet size
    if (pkt->size < (sx_int32)sample->size) {
        pkt->size = sample->size;
        return MP4MUX_ERR_PACKET_SIZE;
    }

    // check stsd index
    if (cur_trk->info.type == MP4_TRACK_VIDEO) {
        if (cur_trk->info.codec_id == CODEC_ID_H264) {
            if (cur_trk->stsd_idx_rd != sample->sd_index) {
                // copy sps/pps to packet
                sx_uint8 *p;
                sx_int32 size;
                stsd_data_t *stsd = &cur_trk->stsd_data[sample->sd_index - 1];
                assert(stsd->sps_data != NULL && stsd->sps_len != 0);
                assert(stsd->pps_data != NULL && stsd->pps_len != 0);
                size = stsd->sps_len + stsd->pps_len + 8;
                if (pkt->size < size) {
                    pkt->size = size;
                    return MP4MUX_ERR_PACKET_SIZE;
                }
                p = pkt->data;
                // sps
                *p++ = 0;
                *p++ = 0;
                *p++ = 0;
                *p++ = 1;
                memcpy(p, stsd->sps_data, stsd->sps_len);
                p += stsd->sps_len;
                // pps
                *p++ = 0;
                *p++ = 0;
                *p++ = 0;
                *p++ = 1;
                memcpy(p, stsd->pps_data, stsd->pps_len);
                p += stsd->sps_len;
                pkt->size = size;
                pkt->track_id = cur_trk->info.track_id;
                pkt->dts = pkt->dts = sample->dts;
                pkt->flags |= PKT_FLAG_CONFIG_DATA;
                cur_trk->stsd_idx_rd = sample->sd_index;
                return MP4MUX_ERR_OK;
            }
        }
        else if (cur_trk->info.codec_id == CODEC_ID_MPEG4) {
            if (cur_trk->stsd_idx_rd != sample->sd_index) {
                // copy sps/pps to packet
                sx_int32 size;
                stsd_data_t *stsd = &cur_trk->stsd_data[sample->sd_index - 1];
                size = stsd->sps_len;
                if (pkt->size < size) {
                    pkt->size = size;
                    return MP4MUX_ERR_PACKET_SIZE;
                }
                // config data
                memcpy(pkt->data, stsd->sps_data, stsd->sps_len);
                pkt->size = size;
                pkt->track_id = cur_trk->info.track_id;
                pkt->dts = pkt->dts = sample->dts;
                pkt->flags |= PKT_FLAG_CONFIG_DATA;
                cur_trk->stsd_idx_rd = sample->sd_index;
                return MP4MUX_ERR_OK;
            }
        }
    }

    cur_trk->current_sample++; // increase current sample index for next packet
    if (bs_rseek(bs, sample->pos, SEEK_SET) != sample->pos) {
        //mp4_log(ctx->log, "stream %d, offset 0x%"PRIx64": partial file\n", cur_trk->info.track_id, sample->pos);
        return MP4MUX_ERR_SEEK_SAMPLE;
    }

    mp4_get_packet_data(bs, pkt, sample->size);
    pkt->track_id = cur_trk->info.track_id;
    pkt->dts = sample->dts;
    // update pts
    if (cur_trk->ctts_data) {
        pkt->pts = pkt->dts + cur_trk->ctts_data[cur_trk->ctts_index].duration;
        /* update ctts context */
        cur_trk->ctts_sample++;
        if (cur_trk->ctts_index < (sx_int32)cur_trk->ctts_count 
            && cur_trk->ctts_data[cur_trk->ctts_index].count == cur_trk->ctts_sample) {
            cur_trk->ctts_index++;
            cur_trk->ctts_sample = 0;
        }
    } 
    else {
        pkt->pts = pkt->dts;
    }
    if (CHK_FLAGS(sample->flags, MP4_SAMPLE_KEYFRAME))
        SET_FLAGS(pkt->flags, PKT_FLAG_KEY_FRAME);
    pkt->pos = sample->pos;
    return MP4MUX_ERR_OK;
}

// default parsing function 
// return code: TBD
//   MP4MUX_ERR_PARSE_DONE(1): succeed
//   MP4MUX_ERR_OK(0): continue to parse next atom
//   MP4MUX_ERR_PARSE_ERROR(-1): error occured, exit
static sx_int32 mp4_read_default(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    sx_int64 total_size = 0;
    mp4_atom_t a;
    sx_int32 i;
    sx_int32 err = MP4MUX_ERR_OK;

    a.offset = atom.offset;

    if (atom.size < 0)
        atom.size = INT64_MAX;
    while (((total_size + 8) < atom.size) && !bs_eof(bs) && !err) {
        a.size = atom.size;
        a.type = 0;
        if (atom.size >= 8) {
            a.size = bs_get_be32(bs);
            a.type = bs_get_le32(bs);
        }
        total_size += 8;
        a.offset += 8;
        if (a.size == 1) { // 64 bit extended size 
            a.size = bs_get_be64(bs) - 8;
            a.offset += 8;
            total_size += 8;
        }
        if (a.size == 0) { // atom size is not set
            a.size = atom.size - total_size + 8;
            if (CHK_FLAGS(ctx->flags, MP4_FLAG_REPAIR)) {
                bs_wseek(bs, a.offset - 8, SEEK_SET);
                if (a.size < 8) {
                    a.size = 8;
                    bs_put_be32(bs, (sx_int32)a.size);
                    bs_put_tag(bs, (sx_uint8*)"free");
                    bs_flush_buffer(bs);
                }
                else if (a.size < INT32_MAX) {
                    bs_put_be32(bs, (sx_int32)a.size);
                    bs_flush_buffer(bs);
                }
            }
            else if (a.size <= 8)
                break;
        }
        a.size -= 8;
        if (a.size < 0) // atom size is wrong
            break;
        a.size = MIN(a.size, atom.size - total_size);

        // look for the parser function
        for (i = 0; mp4_default_parse_table[i].type != 0
             && mp4_default_parse_table[i].type != a.type; i++);

        if (mp4_default_parse_table[i].type == 0) { 
            // no parser function, skip the box
            bs_skip(bs, a.size);
        } 
        else {
            // parser function is found
            offset_t start_pos = bs_rtell(bs);
            sx_int64 left;
            err = mp4_default_parse_table[i].parse(ctx, a);
            if (!CHK_FLAGS(ctx->flags, MP4_FLAG_FRAGMENT_PLAY) 
                && CHK_FLAGS(ctx->flags, MP4_FLAG_FOUND_MOOV) 
                && CHK_FLAGS(ctx->flags, MP4_FLAG_FOUND_MDAT)) {
                err = MP4MUX_ERR_PARSE_DONE;
                break;
            }
            left = a.size - bs_rtell(bs) + start_pos;
            if (left > 0) // skip extra data 
                bs_skip(bs, left);
        }

        a.offset += a.size;
        total_size += a.size;
    }

    if (!err && total_size < atom.size && atom.size < 0x7ffff) {
        bs_skip(bs, atom.size - total_size);
    }

    return err;
}

// read smff box
static sx_int32 mp4_read_smff(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;

    mp4_log(ctx->log, "  smff [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);

    ctx->smff_pos = atom.offset - 8;
    ctx->version = bs_get_byte(bs);
    ctx->smff_flags = bs_get_be24(bs);
    atom.size -= 4;
    atom.offset += 4;
    mp4_log(ctx->log, "    version = %d\n", ctx->version);
    mp4_log(ctx->log, "    flags = 0x%x\n", ctx->smff_flags);
    if (!CHK_FLAGS(ctx->smff_flags, SMFF_FLAG_CLOSING_MOOV))
        SET_FLAGS(ctx->flags, MP4_FLAG_PARSE_MOOF);
    if (CHK_FLAGS(ctx->smff_flags, SMFF_FLAG_CLOSING_MOOV)
        || CHK_FLAGS(ctx->smff_flags, SMFF_FLAG_CLOSED))
        SET_FLAGS(ctx->flags, MP4_FLAG_WRITE_CLOSED);

    return MP4MUX_ERR_OK;
}

// read free box
static sx_int32 mp4_read_free(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    sx_int64 offset;
    mp4_atom_t a;

    mp4_log(ctx->log, "free [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);

    if (atom.size > 8) {
        a.size = bs_get_be32(bs) - 8;
        a.type = bs_get_le32(bs);
        atom.size -= 8;
        atom.offset += 8;
        a.offset = atom.offset;

        // 'smff' box should be at the beginning of the file
        if (a.type == MKTAG( 's', 'm', 'f', 'f' )) {
            mp4_read_smff(ctx, a);
            atom.offset += a.size;
            atom.size -= a.size;
        }
        else if (a.type == MKTAG( 'm', 'o', 'o', 'f' ) 
            && CHK_FLAGS(ctx->flags, MP4_FLAG_PARSE_MOOF)) {
            // last moof offset
            if (ctx->moof_cnt > 0)
                offset = ctx->moof_list[ctx->moof_cnt - 1].offset + 8;
            else
                offset = -1;

            if (atom.offset > offset) {
                mp4_log(ctx->log, "  moof [0x%08llx]", atom.offset - 8);
                mp4_log(ctx->log, " %d\n", atom.size + 8);
                // check list size
                ctx->moof_list = mp4_fast_realloc(ctx->moof_list, &ctx->alloc_moof_cnt,
                    ctx->moof_cnt + 1, sizeof(mp4_atom_t));
                if (ctx->moof_list == NULL)
                    return MP4MUX_ERR_PARSE_ERROR;
                ctx->moof_list[ctx->moof_cnt].type = a.type;
                ctx->moof_list[ctx->moof_cnt].size = atom.size + 8;
                ctx->moof_list[ctx->moof_cnt].offset = atom.offset - 8;
                //mp4_log(ctx->log, "  %d, 0x%x\n", ctx->moof_cnt, ctx->moof_list[ctx->moof_cnt].offset);
                ctx->moof_cnt++;
            }
        }
    }

    // skip rest of data
    bs_skip(bs, atom.size);
    return MP4MUX_ERR_OK;
}

// read ftyp box
static sx_int32 mp4_read_ftyp(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    sx_uint32 type = bs_get_le32(bs);
    sx_uint32 minor;

    if (type != MKTAG('q', 't', ' ', ' '))
        SET_FLAGS(ctx->flags, MP4_FLAG_ISOM);
    minor = bs_get_be32(bs); // minor version
    mp4_log(ctx->log, "ftyp [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);
    mp4_log(ctx->log, "  major brand = %.4s\n", (char *)&type);
    mp4_log(ctx->log, "  minor = d\n", (char *)&type);
    bs_skip(bs, atom.size - 8);
    return MP4MUX_ERR_OK;
}

// read mdat box
static sx_int32 mp4_read_mdat(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    mp4_log(ctx->log, "mdat [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %I64d\n", atom.size + 8);
    if (atom.size == 0) // found an empty mdat
        return MP4MUX_ERR_OK;
    ctx->mdat_size += atom.size;
    SET_FLAGS(ctx->flags, MP4_FLAG_FOUND_MDAT);
    bs_skip(bs, atom.size);
    return MP4MUX_ERR_OK;
}

// read moov box
static sx_int32 mp4_read_moov(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;

    mp4_log(ctx->log, "moov [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);

    // skip parsing moov in fragment playback mode
    if (CHK_FLAGS(ctx->flags, MP4_FLAG_FRAGMENT_PLAY)) {
        bs_skip(bs, atom.size);
        SET_FLAGS(ctx->flags, MP4_FLAG_WRITE_CLOSED);
    }

    if (mp4_read_default(ctx, atom) == MP4MUX_ERR_PARSE_ERROR)
        return MP4MUX_ERR_PARSE_ERROR;
    SET_FLAGS(ctx->flags, MP4_FLAG_FOUND_MOOV);
    return MP4MUX_ERR_OK;
}

// read mvhd box
static sx_int32 mp4_read_mvhd(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    sx_int32 version = bs_get_byte(bs); /* version */
    bs_get_be24(bs); // flags
    mp4_log(ctx->log, "mvhd [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);
    mp4_log(ctx->log, "  version = %d\n", version);

    if (version == 1) {
        ctx->time = bs_get_be64(bs);
        ctx->mod_time = bs_get_be64(bs);
    } 
    else {
        ctx->time = bs_get_be32(bs); // creation time
        ctx->mod_time = bs_get_be32(bs); // modification time
    }
    ctx->timescale = bs_get_be32(bs); // time scale
    ctx->duration = (version == 1) ? bs_get_be64(bs) : bs_get_be32(bs); // duration 
    bs_get_be32(bs); // preferred scale 
    bs_get_be16(bs); // preferred volume 
    bs_skip(bs, 10); // reserved 
    bs_skip(bs, 36); // display matrix 
    bs_get_be32(bs); // preview time 
    bs_get_be32(bs); // preview duration 
    bs_get_be32(bs); // poster time 
    bs_get_be32(bs); // selection time 
    bs_get_be32(bs); // selection duration 
    bs_get_be32(bs); // current time 
    bs_get_be32(bs); // next track ID 

    mp4_log(ctx->log, "  create time = %llx\n", ctx->time);
    mp4_log(ctx->log, "  modify time = %llx\n", ctx->mod_time);
    mp4_log(ctx->log, "  timescale = %d\n", ctx->timescale);
    mp4_log(ctx->log, "  duration = %llx\n", ctx->duration);
    return MP4MUX_ERR_OK;
}

// read trak box
static sx_int32 mp4_read_trak(mp4_context_t *ctx, mp4_atom_t atom)
{
    mp4_track_t *trk;
    mp4_log(ctx->log, "trak [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);

    if (ctx->nb_tracks >= MP4_MAX_TRACKS)
        return MP4MUX_ERR_TRACK_NUMBER;

    trk = (mp4_track_t *)malloc(sizeof(mp4_track_t));
    if (!trk) 
        return MP4MUX_ERR_MEMORY;
    memset(trk, 0, sizeof(mp4_track_t));

    // allocate memory for stsd data
    trk->stsd_data = (stsd_data_t *)malloc(sizeof(stsd_data_t) * MP4_STSD_SIZE_INC);
    if (trk->stsd_data == NULL)
        return MP4MUX_ERR_MEMORY;
    memset(trk->stsd_data, 0, sizeof(stsd_data_t) * MP4_STSD_SIZE_INC);
    trk->stsd_count = MP4_STSD_SIZE_INC;

    trk->info.type = MP4_TRACK_UNKNOWN;
    ctx->tracks[ctx->nb_tracks++] = trk;
    return mp4_read_default(ctx, atom);
}

// read tkhd box
static sx_int32 mp4_read_tkhd(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    mp4_track_t *trk = ctx->tracks[ctx->nb_tracks - 1];
    sx_int32 version = bs_get_byte(bs); // version
    sx_int64 duration;

    bs_get_be24(bs); // flags
    mp4_log(ctx->log, "tkhd [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);
    mp4_log(ctx->log, "  version = %d\n", version);

    if (version == 1) {
        bs_get_be64(bs);
        bs_get_be64(bs);
    } 
    else {
        bs_get_be32(bs); // creation time 
        bs_get_be32(bs); // modification time 
    }
    trk->info.track_id = (sx_int32)bs_get_be32(bs); // track id 
    mp4_log(ctx->log, "  track id = %d\n", trk->info.track_id);
    bs_get_be32(bs); // reserved
    if (version == 1) 
        duration = bs_get_be64(bs);
    else
        duration = bs_get_be32(bs); // duration 
    mp4_log(ctx->log, "  duration = %I64d\n", duration);
    bs_get_be32(bs); // reserved
    bs_get_be32(bs); // reserved
    bs_get_be16(bs); // mov: layer, mp4: pre-defined = 0
    bs_get_be16(bs); // mov: alternate group, mp4: pre-defined = 0
    bs_get_be16(bs); // mov: volume, mp4: pre-defined(audio=0x0100, other=0)
    bs_get_be16(bs); // reserved
    bs_skip(bs, 36); // mov: display matrix, mp4: pre-defined
    if (CHK_FLAGS(ctx->flags, MP4_FLAG_ISOM)) {
        bs_get_be32(bs); // mp4: pre-defined
        bs_get_be32(bs); // mp4: pre-defined
    }
    else {
        trk->info.width = bs_get_be32(bs) >> 16; // mov: width
        trk->info.height = bs_get_be32(bs) >> 16; // mov: height
        mp4_log(ctx->log, "  width = %d\n", trk->info.width);
        mp4_log(ctx->log, "  height = %d\n", trk->info.height);
    }

    return MP4MUX_ERR_OK;
}

// read mdhd box
static sx_int32 mp4_read_mdhd(mp4_context_t *ctx, mp4_atom_t atom)
{
    sx_int32 lang;
    byte_strm_t *bs = ctx->stream;
    mp4_track_t *trk = ctx->tracks[ctx->nb_tracks - 1];
    sx_int32 version = bs_get_byte(bs);
    bs_get_be24(bs); // flags

    mp4_log(ctx->log, "mdhd [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);
    mp4_log(ctx->log, "  version = %d\n", version);
    if (version > 1)
        return MP4MUX_ERR_PARSE_ERROR; // unsupported 

    if (version == 1) {
        bs_get_be64(bs);
        bs_get_be64(bs);
    } 
    else {
        bs_get_be32(bs); // creation time 
        bs_get_be32(bs); // modification time 
    }

    trk->info.timescale = bs_get_be32(bs);
    trk->info.duration = (version == 1) ? bs_get_be64(bs) : bs_get_be32(bs); // duration
    lang = bs_get_be16(bs); // language 
    mp4_lang_to_iso639(lang, (char *)&trk->info.language);
    bs_get_be16(bs); // mov: quality, mp4: pre-defined = 0

    mp4_log(ctx->log, "  timescale = %d\n", trk->info.timescale);
    mp4_log(ctx->log, "  duration = %I64d\n", trk->info.duration);
    return MP4MUX_ERR_OK;
}

// read hdlr box
static sx_int32 mp4_read_hdlr(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    mp4_track_t *trk = ctx->tracks[ctx->nb_tracks - 1];
    sx_uint32 type;
    sx_uint32 ctype;

    mp4_log(ctx->log, "hdlr [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);
    bs_get_be32(bs); // version/flags

    ctype = bs_get_le32(bs); // mov: component type, mp4: pre-defined = 0
    type = bs_get_le32(bs); // mov: component subtype, mp4: handler type

    if (!ctype)
        SET_FLAGS(ctx->flags, MP4_FLAG_ISOM);
    if (type == MKTAG('v', 'i', 'd', 'e'))
        trk->info.type = MP4_TRACK_VIDEO;
    else if (type == MKTAG('s', 'o', 'u', 'n'))
        trk->info.type = MP4_TRACK_AUDIO;
    mp4_log(ctx->log, "  type = %c%c%c%c\n", 
        *((char*)&type), ((char*)&type)[1], ((char*)&type)[2], ((char*)&type)[3]);

    bs_get_be32(bs); // mov: component  manufacture, mp4: reserved = 0
    bs_get_be32(bs); // mov: component flags, mp4: reserved = 0
    bs_get_be32(bs); // mov: component flags mask, mp4: reserved = 0

    if (atom.size > 24) {
        // component name
        bs_skip(bs, atom.size - (bs_rtell(bs) - atom.offset));
    }

    return MP4MUX_ERR_OK;
}

// read stco box
static sx_int32 mp4_read_stco(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    mp4_track_t *trk = ctx->tracks[ctx->nb_tracks - 1];
    sx_uint32 i, entries;

    if (atom.type == MKTAG('s', 't', 'c', 'o')) {
        mp4_log(ctx->log, "stco [0x%08llx]", atom.offset - 8);
        mp4_log(ctx->log, " %d\n", atom.size + 8);
    } 
    else if (atom.type == MKTAG('c', 'o', '6', '4')) {
        mp4_log(ctx->log, "co64 [0x%08llx]", atom.offset - 8);
        mp4_log(ctx->log, " %d\n", atom.size + 8);
    } 

    if (!CHK_FLAGS(ctx->flags, MP4_FLAG_PARSE_SAMPLE_TABLE)) {
        bs_skip(bs, atom.size);
        return MP4MUX_ERR_OK;
    }

    bs_get_be32(bs); // version/flags
    entries = bs_get_be32(bs);
    if (entries >= UINT32_MAX / sizeof(sx_int64))
        return MP4MUX_ERR_PARSE_ERROR;
    trk->stco_count = entries;
    trk->stco_data = malloc(entries * sizeof(sx_int64));
    if (!trk->stco_data)
        return MP4MUX_ERR_PARSE_ERROR;

    if (atom.type == MKTAG('s', 't', 'c', 'o')) {
        mp4_log(ctx->log, "  entries = %d\n", entries);
        for (i=0; i<entries; i++) {
            trk->stco_data[i] = bs_get_be32(bs);
            mp4_log(ctx->log, "    %d, 0x%x\n", i, trk->stco_data[i]);
        }
    } 
    else if (atom.type == MKTAG('c', 'o', '6', '4')) {
        mp4_log(ctx->log, "  entries = %d\n", entries);
        for (i=0; i<entries; i++) {
            trk->stco_data[i] = bs_get_be64(bs);
            mp4_log(ctx->log, "    %d, 0x%llx\n", i, trk->stco_data[i]);
        }
    } 
    else
        return MP4MUX_ERR_PARSE_ERROR;

    return MP4MUX_ERR_OK;
}

// read ctts box
static sx_int32 mp4_read_ctts(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    mp4_track_t *trk = ctx->tracks[ctx->nb_tracks - 1];
    sx_uint32 i, entries;

    mp4_log(ctx->log, "ctts [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);
    bs_get_be32(bs); // version/flags

    entries = bs_get_be32(bs);
    mp4_log(ctx->log, "  entries = %d\n", entries);
    if (entries >= UINT32_MAX / sizeof(mp4_stts_t))
        return MP4MUX_ERR_PARSE_ERROR;
    trk->ctts_count = entries;
    trk->ctts_data = malloc(entries * sizeof(mp4_stts_t));
    if (!trk->ctts_data)
        return MP4MUX_ERR_PARSE_ERROR;

    for (i=0; i<entries; i++) {
        sx_int32 count    = bs_get_be32(bs);
        sx_int32 duration = bs_get_be32(bs);

        if (duration < 0) {
            trk->ctts_count = 0;
            bs_skip(bs, 8 * (entries - i - 1));
            break;
        }
        trk->ctts_data[i].count = count;
        trk->ctts_data[i].duration = duration;
    }
    return MP4MUX_ERR_OK;
}

// read stsc box
static sx_int32 mp4_read_stsc(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    mp4_track_t *trk = ctx->tracks[ctx->nb_tracks - 1];
    sx_uint32 i, entries;

    mp4_log(ctx->log, "stsc [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);

    if (!CHK_FLAGS(ctx->flags, MP4_FLAG_PARSE_SAMPLE_TABLE)) {
        bs_skip(bs, atom.size);
        return MP4MUX_ERR_OK;
    }

    bs_get_be32(bs); // version/flags
    entries = bs_get_be32(bs);
    mp4_log(ctx->log, "  entries = %d\n", entries);
    if (entries >= UINT32_MAX / sizeof(mp4_stsc_t))
        return MP4MUX_ERR_PARSE_ERROR;
    trk->stsc_count = entries;
    trk->stsc_data = malloc(entries * sizeof(mp4_stsc_t));
    if (!trk->stsc_data)
        return MP4MUX_ERR_PARSE_ERROR;

    for (i=0; i<entries; i++) {
        trk->stsc_data[i].first = bs_get_be32(bs);
        trk->stsc_data[i].count = bs_get_be32(bs);
        trk->stsc_data[i].index = bs_get_be32(bs);
        mp4_log(ctx->log, "    %d, %d, %d\n", 
            trk->stsc_data[i].first, trk->stsc_data[i].count, trk->stsc_data[i].index);
    }
    return MP4MUX_ERR_OK;
}

// read stss box
static sx_int32 mp4_read_stss(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    mp4_track_t *trk = ctx->tracks[ctx->nb_tracks - 1];
    sx_uint32 i, entries;

    mp4_log(ctx->log, "stss [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);

    if (!CHK_FLAGS(ctx->flags, MP4_FLAG_PARSE_SAMPLE_TABLE)) {
        bs_skip(bs, atom.size);
        return MP4MUX_ERR_OK;
    }

    bs_get_be32(bs); // version/flags
    entries = bs_get_be32(bs);
    mp4_log(ctx->log, "  entries = %d\n", entries);
    if (entries >= UINT32_MAX / sizeof(sx_int32))
        return MP4MUX_ERR_PARSE_ERROR;
    trk->stss_count = entries;
    trk->stss_data = malloc(entries * sizeof(sx_int32));
    if (!trk->stss_data)
        return MP4MUX_ERR_PARSE_ERROR;

    for (i=0; i<entries; i++) {
        trk->stss_data[i] = bs_get_be32(bs);
        mp4_log(ctx->log, "    %d\n", trk->stss_data[i]);
    }
    return MP4MUX_ERR_OK;
}

// read stsz box
static sx_int32 mp4_read_stsz(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    mp4_track_t *trk = ctx->tracks[ctx->nb_tracks - 1];
    sx_uint32 i, entries, sample_size;

    mp4_log(ctx->log, "stsz [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);

    if (!CHK_FLAGS(ctx->flags, MP4_FLAG_PARSE_SAMPLE_TABLE)) {
        bs_skip(bs, atom.size);
        return MP4MUX_ERR_OK;
    }

    bs_get_be32(bs); // version/flags
    sample_size = bs_get_be32(bs);
    mp4_log(ctx->log, "  sample size = %d\n", sample_size);
    if (!trk->info.sample_size) /* do not overwrite value computed in stsd */
        trk->info.sample_size = sample_size;

    entries = bs_get_be32(bs);
    mp4_log(ctx->log, "  entries = %d\n", entries);
    if (entries >= UINT32_MAX / sizeof(sx_int32))
        return MP4MUX_ERR_PARSE_ERROR;
    trk->sample_count = entries;
    if (sample_size)
        return MP4MUX_ERR_OK;
    trk->stsz_data = malloc(entries * sizeof(sx_int32));
    if (!trk->stsz_data)
        return MP4MUX_ERR_PARSE_ERROR;

    for (i=0; i<entries; i++) {
        trk->stsz_data[i] = bs_get_be32(bs);
        mp4_log(ctx->log, "    %d\n", trk->stsz_data[i]);
    }
    return MP4MUX_ERR_OK;
}

// read stts box
static sx_int32 mp4_read_stts(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    mp4_track_t *trk = ctx->tracks[ctx->nb_tracks - 1];
    sx_uint32 i, entries;
    sx_int64 duration = 0;
    sx_int64 total_sample_count = 0;

    mp4_log(ctx->log, "stts [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);

    if (!CHK_FLAGS(ctx->flags, MP4_FLAG_PARSE_SAMPLE_TABLE)) {
        bs_skip(bs, atom.size);
        return MP4MUX_ERR_OK;
    }

    bs_get_be32(bs); // version/flags
    entries = bs_get_be32(bs);
    mp4_log(ctx->log, "  entries = %d\n", entries);
    if (entries >= UINT32_MAX / sizeof(mp4_stts_t))
        return MP4MUX_ERR_PARSE_ERROR;
    trk->stts_count = entries;
    trk->stts_data = malloc(entries * sizeof(mp4_stts_t));
    if (!trk->stts_data)
        return MP4MUX_ERR_PARSE_ERROR;

    for (i=0; i<entries; i++) {
        sx_int32 sample_duration;
        sx_int32 sample_cnt;

        sample_cnt = bs_get_be32(bs);
        sample_duration = bs_get_be32(bs);
        if (sample_duration <= 0) {
            sx_int32 new_duration = (i == 0) ? 1 : trk->stts_data[i-1].duration; // try to guess
            mp4_log(ctx->log, "    ERR: fix negtive/zero duration %d, %d\n", 
                sample_duration, new_duration);
            sample_duration = new_duration;
        }
        trk->stts_data[i].count = sample_cnt;
        trk->stts_data[i].duration = sample_duration;
        mp4_log(ctx->log, "    count = %d, duration = %d\n", 
            sample_cnt, sample_duration);
        duration += (sx_int64)sample_duration * sample_cnt;
        total_sample_count += sample_cnt;
    }

    if (duration)
        trk->info.duration = duration;
    return MP4MUX_ERR_OK;
}

// read avcc box
static sx_int32 mp4_read_avcc(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    mp4_track_t *trk = ctx->tracks[ctx->nb_tracks - 1];
    sx_uint8 count, i;

    mp4_log(ctx->log, "  avcc [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);
    bs_get_byte(bs); // version
    count = bs_get_byte(bs); // profile
    mp4_log(ctx->log, "    profile = %x\n", count);
    count = bs_get_byte(bs); // profile compatibility
    mp4_log(ctx->log, "    profile_comp = %x\n", count);
    count = bs_get_byte(bs); // level
    mp4_log(ctx->log, "    level = %x\n", count);
    count = (bs_get_byte(bs) & 0x3) + 1; // nal size
    mp4_log(ctx->log, "    nal size = %x\n", count);

    // sps
    count = bs_get_byte(bs) & 0x1f;
    assert(count == 1);
    for (i=0; i<count; i++) {
        stsd_data_t *stsd = &trk->stsd_data[trk->stsd_idx_rd];
        stsd->sps_len = bs_get_be16(bs);
        stsd->sps_data = malloc(stsd->sps_len);
        assert(stsd->sps_data);
        bs_get_buffer(bs, stsd->sps_data, stsd->sps_len);
        mp4_log(ctx->log, "    sps %d, len = %d\n", i, stsd->sps_len);
    }

    // pps
    count = bs_get_byte(bs);
    assert(count == 1);
    for (i=0; i<count; i++) {
        stsd_data_t *stsd = &trk->stsd_data[trk->stsd_idx_rd];
        stsd->pps_len = bs_get_be16(bs);
        stsd->pps_data = malloc(stsd->pps_len);
        assert(stsd->pps_data);
        bs_get_buffer(bs, stsd->pps_data, stsd->pps_len);
        mp4_log(ctx->log, "    pps %d, len = %d\n", i, stsd->pps_len);
    }

    return MP4MUX_ERR_OK;
}

// read stsd box
static sx_int32 mp4_read_stsd(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    mp4_track_t *trk = ctx->tracks[ctx->nb_tracks - 1];
    sx_int32 entries, frames_per_sample;
    sx_uint32 format;
    sx_uint8 codec_name[32];
    sx_int16 color_table_id;
    sx_int8 *pch;

    mp4_log(ctx->log, "stsd [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);

    /*
    if (!CHK_FLAGS(ctx->flags, MP4_FLAG_PARSE_SAMPLE_TABLE)) {
        bs_skip(bs, atom.size);
        return MP4MUX_ERR_OK;
    }
    */

    bs_get_be32(bs); // version/flags
    entries = bs_get_be32(bs);
    mp4_log(ctx->log, "  entries = %d\n", entries);
    trk->stsd_idx_rd = 0;

    // allocate more memory if not enough
    trk->stsd_data = mp4_fast_realloc1(trk->stsd_data, 
        &trk->stsd_count, entries, sizeof(stsd_data_t), 0);
    if (trk->stsd_data == NULL)
        return MP4MUX_ERR_MEMORY;
    memset(trk->stsd_data, 0, entries * sizeof(stsd_data_t));

    // read all entries
    while (entries--) {
        codec_id_t id;
        mp4_atom_t a = { 0, 0, 0 };
        offset_t start_pos = bs_rtell(bs);
        sx_int32 size = bs_get_be32(bs); // size
        format = bs_get_le32(bs); // data format
        pch = (char *)&format;
        if (pch[0] == 'm' && pch[1] == 's' && pch[2] == 0)
            mp4_log(ctx->log, "  format = %c%c%02x\n", pch[0], pch[1], pch[3]);
        else
            mp4_log(ctx->log, "  format = %c%c%c%c\n", pch[0], pch[1], pch[2], pch[3]);

        bs_get_be32(bs); // reserved
        bs_get_be16(bs); // reserved
        bs_get_be16(bs); // data reference index

        id = codec_get_id(codec_audio_tags, format);

        if (trk->info.type != MP4_TRACK_VIDEO && id > 0) {
            trk->info.type = MP4_TRACK_AUDIO;
        } 
        else if (trk->info.type != MP4_TRACK_AUDIO && format 
            && format != MKTAG('m', 'p', '4', 's')) {
            id = codec_get_id(codec_video_tags, format);
            if (id > 0)
                trk->info.type = MP4_TRACK_VIDEO;
        }

        if (trk->info.type == MP4_TRACK_VIDEO) {
            trk->info.codec_id = id;
            bs_get_be16(bs); // mov: version, mp4: pre-defined = 0
            bs_get_be16(bs); // mov: revision level, mp4: reserved = 0
            bs_get_be32(bs); // mov: vendor, mp4: reserved = 0
            bs_get_be32(bs); // mov: temporal quality, mp4: reserved = 0
            bs_get_be32(bs); // mov: spatial quality, mp4: reserved = 0

            trk->info.width = bs_get_be16(bs); // width 
            trk->info.height = bs_get_be16(bs); // height 
            mp4_log(ctx->log, "  width = %d\n", trk->info.width);
            mp4_log(ctx->log, "  height = %d\n", trk->info.height);

            bs_get_be32(bs); // mov: horiz resolution, mp4: pre-defined = 0x00480000
            bs_get_be32(bs); // mov: vert resolution, mp4: pre-defined = 0x00480000
            bs_get_be32(bs); // mov: data size = 0, mp4: reserved = 0
            frames_per_sample = bs_get_be16(bs); // mov: frames per samples, mp4: pre-defined = 1
            mp4_log(ctx->log, "  frames per sample = %d\n", frames_per_sample);

            bs_get_buffer(bs, codec_name, 32); // mov: codec name (pascal string), mp4: pre-defined = 0
            trk->bits_per_sample = bs_get_be16(bs); // mov: depth, mp4: pre-defined = 24
            color_table_id = bs_get_be16(bs); // mov: colortable id, mp4: pre-defined = -1
            assert(color_table_id == -1);
        } 
        else if (trk->info.type == MP4_TRACK_AUDIO) {
            sx_int32 bits_per_sample;
            bs_get_be16(bs); // mov: version, mp4: reserved = 0

            trk->info.codec_id = id;
            bs_get_be16(bs); // mov: revision level, mp4: reserved = 0
            bs_get_be32(bs); // mov: vendor, mp4: reserved = 0

            trk->info.channels = bs_get_be16(bs); // mov: channel count, mp4: pre-defined = 2
            mp4_log(ctx->log, "  channels = %d\n", trk->info.channels);
            trk->bits_per_sample = bs_get_be16(bs); // mov: sample size, mp4: pre-defined = 16
            mp4_log(ctx->log, "  sample size = %d\n", trk->bits_per_sample);
            bs_get_be16(bs); // mov: compression id = 0, mp4: pre-defined = 0
            bs_get_be16(bs); // mov: packet size = 0, mp4: reserved = 0
            trk->sample_rate = ((bs_get_be32(bs) >> 16)); // mp4: lower 16-bit pre-defined = 0
            mp4_log(ctx->log, "  sampling rate = %d\n", trk->sample_rate);

            switch (trk->info.codec_id) {
            case CODEC_ID_PCM_S8:
            case CODEC_ID_PCM_U8:
                if (trk->bits_per_sample == 16)
                    trk->info.codec_id = CODEC_ID_PCM_S16BE;
                break;
            case CODEC_ID_PCM_S16LE:
            case CODEC_ID_PCM_S16BE:
                if (trk->bits_per_sample == 8)
                    trk->info.codec_id = CODEC_ID_PCM_S8;
                break;
            default:
                break;
            }

            // read extra data for mov
            if (!CHK_FLAGS(ctx->flags, MP4_FLAG_ISOM)) {
                bs_skip(bs, size - (bs_rtell(bs) - start_pos));
            }

            // FIXME, check if code below is necessary
            bits_per_sample = av_get_bits_per_sample(trk->info.codec_id);
            if (bits_per_sample) {
                trk->bits_per_sample = bits_per_sample;
                trk->info.sample_size = (bits_per_sample >> 3) * trk->info.channels;
            }
        } 
        else {
            bs_skip(bs, size - (bs_rtell(bs) - start_pos));
        }

        // read extra atoms
        a.size = size - (bs_rtell(bs) - start_pos);
        if (a.size > 8) {
            if (mp4_read_default(ctx, a) == MP4MUX_ERR_PARSE_ERROR)
                return MP4MUX_ERR_PARSE_ERROR;
        } 
        else if (a.size > 0)
            bs_skip(bs, a.size);

        trk->stsd_idx_rd++;
    }

    if (trk->info.type == MP4_TRACK_AUDIO && trk->sample_rate == 0 && trk->info.timescale > 1) {
        trk->sample_rate = trk->info.timescale;
    }

    /* special codec parameters handling */
    switch (trk->info.codec_id) {
    case CODEC_ID_MPEG4:
        trk->info.width = 0; // let decoder init width/height
        trk->info.height = 0;
        break;
    default:
        break;
    }

    trk->stsd_idx_rd = 0;
    return MP4MUX_ERR_OK;
}

// read udta box
static sx_int32 mp4_read_udta(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    sx_uint64 end = bs_rtell(bs) + atom.size;

    mp4_log(ctx->log, "udta [%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);
    while (bs_rtell(bs) + 8 < (sx_int64)end) {
        sx_uint32 tag_size = bs_get_be32(bs);
        sx_uint32 tag = bs_get_le32(bs);
        sx_uint64 next = bs_rtell(bs) + tag_size - 8;
        tag = tag;

        if (next > end) // stop if tag_size is wrong
            break;

        bs_rseek(bs, next, SEEK_SET);
    }

    return MP4MUX_ERR_OK;
}

// read edit list box
static sx_int32 mp4_read_elst(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    mp4_track_t *trk = ctx->tracks[ctx->nb_tracks - 1];
    sx_uint32 i;
    sx_uint32 version, flags;

    mp4_log(ctx->log, "elst [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);
    version = bs_get_byte(bs); // version
    flags = bs_get_be24(bs); // flags
    mp4_log(ctx->log, "  version = %d\n", version);
    trk->edit_count = bs_get_be32(bs); // entries 
    mp4_log(ctx->log, "  entries = %d\n", trk->edit_count);

    // allocate memory for elst
    trk->elst_data = (mp4_elst_t *)malloc(trk->edit_count * sizeof(mp4_elst_t));
    if (trk->elst_data == NULL)
        return MP4MUX_ERR_MEMORY;

    for (i=0; i<trk->edit_count; i++) {
        if (CHK_FLAGS(ctx->flags, MP4_FLAG_ISOM)) { // mp4 mode
            if (version == 1) {
                trk->elst_data[i].duration = bs_get_be64(bs); // segment duration 
                trk->elst_data[i].media_time = bs_get_be64(bs); // media time 
            }
            else {
                trk->elst_data[i].duration = bs_get_be32(bs); // segment duration 
                trk->elst_data[i].media_time = bs_get_be32(bs); // media time 
            }
            trk->elst_data[i].media_rate = bs_get_be32(bs); // media rate 
        }
        else { // mov mode
            trk->elst_data[i].duration = bs_get_be32(bs); // segment duration 
            trk->elst_data[i].media_time = bs_get_be32(bs); // media time 
            trk->elst_data[i].media_rate = bs_get_be32(bs); // media rate 
        }
        mp4_log(ctx->log, "    %d, %I64d,", i, trk->elst_data[i].duration);
        mp4_log(ctx->log, " %I64d", trk->elst_data[i].media_time);
        mp4_log(ctx->log, " %4.2f\n", (double)trk->elst_data[i].media_rate / 0x10000);

        // we only support delay offset
        if (trk->elst_data[i].media_time < 0 && trk->elst_data[i].duration != 0)
            trk->info.init_offset = trk->elst_data[i].duration * trk->info.timescale / ctx->timescale;
        else if (trk->elst_data[i].media_time > 0 && trk->elst_data[i].duration > 0)
            trk->info.init_offset = 0 - trk->elst_data[i].media_time;
    }
    return MP4MUX_ERR_OK;
}

// read descriptor length
static sx_int32 mp4_read_descr_len(byte_strm_t *bs)
{
    sx_int32 len = 0;
    sx_int32 count = 4;
    while (count--) {
        sx_int32 c = bs_get_byte(bs);
        len = (len << 7) | (c & 0x7f);
        if (!(c & 0x80))
            break;
    }
    return len;
}

// read descriptor
static sx_int32 mp4_read_descr(mp4_context_t *ctx, sx_int32 *tag)
{
    sx_int32 len;
    byte_strm_t *bs = ctx->stream;
    *tag = bs_get_byte(bs);
    len = mp4_read_descr_len(bs);
    mp4_log(ctx->log, "    MPEG4 desc: tag=0x%02x len=%d\n", *tag, len);
    return len;
}

// read esds box
static sx_int32 mp4_read_esds(mp4_context_t *ctx, mp4_atom_t atom)
{
    byte_strm_t *bs = ctx->stream;
    mp4_track_t *trk = ctx->tracks[ctx->nb_tracks - 1];
    sx_int32 tag, len, temp;

    mp4_log(ctx->log, "  esds [0x%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);
    bs_get_be32(bs); // version + flags

    len = mp4_read_descr(ctx, &tag);
    if (tag == MP4ESDescrTag) {
        temp = bs_get_be16(bs); // ES_ID
        mp4_log(ctx->log, "     esid = %d\n", temp);
        temp = bs_get_byte(bs); // flags + priority
        mp4_log(ctx->log, "     flags = 0x%x\n", temp);
    } 
    else
        bs_get_be16(bs); // ID

    len = mp4_read_descr(ctx, &tag);
    if (tag == MP4DecConfigDescrTag) {
        sx_int32 object_type_id = bs_get_byte(bs);
        mp4_log(ctx->log, "     object type id = 0x%x\n", object_type_id);
        temp = bs_get_byte(bs); // stream type
        mp4_log(ctx->log, "     stream type = 0x%x\n", temp >> 2);
        temp = bs_get_be24(bs); // buffer size db
        mp4_log(ctx->log, "     buffer size = %d\n", temp);
        temp = bs_get_be32(bs); // max bitrate
        mp4_log(ctx->log, "     max br = %d\n", temp);
        temp = bs_get_be32(bs); // avg bitrate
        mp4_log(ctx->log, "     avg br = %d\n", temp);

        trk->info.codec_id = codec_get_id(mp4_obj_type, object_type_id);
        len = mp4_read_descr(ctx, &tag);
        if (tag == MP4DecSpecificDescrTag) {
            int i;
            stsd_data_t *stsd = &trk->stsd_data[trk->stsd_idx_rd];
            mp4_log(ctx->log, "    Specific MPEG4 header len=%d\n", len);
            stsd->sps_len = len;
            stsd->sps_data = malloc(stsd->sps_len);
            assert(stsd->sps_data);
            bs_get_buffer(bs, stsd->sps_data, stsd->sps_len);
            mp4_log(ctx->log, "    ");
            for (i=0; i<len; i++)
                mp4_log(ctx->log, "%02x", stsd->sps_data[i]);
            mp4_log(ctx->log, "\n");
        }
    }
    atom = atom;                // remove compiler warnning
    return MP4MUX_ERR_OK;
}

// search index with time stamp
// track(in): pointer of a track
// wanted_timestamp(in): time stamp to search
// flags(in): search flags
sx_int32 mp4_index_search_timestamp(mp4_track_t *track, sx_int64 wanted_timestamp, sx_uint32 flags)
{
    sample_entry_t *entries = track->samples_rd;
    int nb_entries = track->rd_entries;
    int a, b, m;
    sx_int64 timestamp;

    a = -1;
    b = nb_entries;

    // seek the time stamp
    while (b - a > 1) {
        m = (a + b) >> 1;
        timestamp = entries[m].dts;
        if (timestamp >= wanted_timestamp)
            b = m;
        if (timestamp <= wanted_timestamp)
            a = m;
    }
    m = CHK_FLAGS(flags, MP4_SEEK_FLAG_BACKWARD) ? a : b;

    // seek the nearest key frame
    if (!CHK_FLAGS(flags, MP4_SEEK_FLAG_ANY)) {
        while (m >= 0 && m < nb_entries 
            && !CHK_FLAGS(entries[m].flags, MP4_SAMPLE_KEYFRAME)) {
            m += CHK_FLAGS(flags, MP4_SEEK_FLAG_BACKWARD) ? -1 : 1;
        }
    }

    if (m == nb_entries)
        return -1;
    return m;
}

// search next key frame
// track(in): pointer of track
// flags(in): search flags
sx_int32 mp4_index_search_next_key_frame(mp4_track_t *track, sx_int32 flags)
{
    sample_entry_t *entries = track->samples_rd;
    int nb_entries = track->rd_entries;
    int m = track->current_sample + 1;

    while (m >= 0 && m < nb_entries 
        && !CHK_FLAGS(entries[m].flags, MP4_SAMPLE_KEYFRAME)) {
        m += CHK_FLAGS(flags, MP4_SEEK_FLAG_BACKWARD) ? -1 : 1;
    }

    if (m == nb_entries)
        return -1;
    return m;
}

// add index entry to a track
// track(in): pointer of a track
// pos(in): sample pos
// timestamp(in): sample time stamp
// size(in): sample size
// distance(in): distance to a key frame
// flags(in): sample flags
// sd_index(in): sample description index
int mp4_add_index_entry(mp4_track_t *track, 
                        sx_int64 pos, 
                        sx_int64 timestamp, 
                        sx_int32 size, 
                        sx_int32 distance, 
                        sx_int32 flags, 
                        sx_int32 sd_index)
{
    sample_entry_t *entries, *ie;
    int index;

    if ((unsigned)track->rd_entries >= UINT32_MAX / sizeof(sample_entry_t) - 1)
        return -1; // too many entries

    // increase one entry
    entries = mp4_fast_realloc(track->samples_rd, &track->alloc_entries,
                               track->rd_entries + 1, sizeof(sample_entry_t));
    if (!entries)
        return -1;

    track->samples_rd = entries;
#if 0 // to speed up, we assume all samples are in time order
    index = mp4_index_search_timestamp(track, timestamp, MP4_SEEK_FLAG_ANY);
#else
    index = -1;
#endif
    if (index < 0) {
        // time stamp not inside entries, append the sample
        index = track->rd_entries++;
        ie = &entries[index];
        assert(index == 0 || ie[-1].dts < timestamp);
    }
    else {
        // time stamp inside entries, insert the sample
        ie = &entries[index];
        if (ie->dts != timestamp) {
            if (ie->dts <= timestamp)
                return -1;
            memmove(entries + index + 1, entries + index, 
                sizeof(sample_entry_t) * (track->rd_entries - index));
            track->rd_entries++;
        }
        else if (ie->pos == pos && distance < ie->min_dist) //do not reduce the distance
            distance = ie->min_dist;
    }

    ie->pos = pos;
    ie->dts = timestamp;
    ie->min_dist = distance;
    ie->size = size;
    ie->flags = flags;
    ie->sd_index = sd_index;

    return index;
}

// build sample index of a track
static void mp4_build_index(mp4_context_t *ctx, mp4_track_t *track)
{
    offset_t current_offset;
    sx_int64 current_dts = 0;
    sx_uint32 stts_index = 0;
    sx_uint32 stsc_index = 0;
    sx_uint32 stss_index = 0;
    sx_int32 i, j;

    if (track->stsz_data || track->info.type == MP4_TRACK_VIDEO) {
        // variable sample size or video
        sx_uint32 current_sample = 0;
        sx_uint32 stts_sample = 0;
        sx_uint32 keyframe, sample_size;
        sx_uint32 distance = 0;

        // loop for all chunks
        for (i = 0; i < (sx_int32)track->stco_count; i++) {
            current_offset = track->stco_data[i]; // sample offset
            // update stsc index
            if ((stsc_index + 1 < track->stsc_count)
                && (i == track->stsc_data[stsc_index + 1].first - 1))
                stsc_index++;
            // loop for samples of current chunk
            for (j = 0; j < track->stsc_data[stsc_index].count; j++) {
                if (current_sample >= (sx_uint32)track->sample_count) {
                    mp4_log(ctx->log, "ERR: wrong sample count\n");
                    goto out;
                }
                // check if key frame
                keyframe = (!track->stss_count) 
                    || (current_sample == (sx_uint32)track->stss_data[stss_index] - 1);
                if (keyframe) {
                    distance = 0;
                    if (stss_index < track->stss_count - 1)
                        stss_index++;
                }
                // get sample size
                sample_size = track->info.sample_size > 0 ? track->info.sample_size 
                    : track->stsz_data[current_sample];
                mp4_add_index_entry(track, current_offset, current_dts, sample_size, 
                    distance, keyframe ? MP4_SAMPLE_KEYFRAME : 0, 
                    track->stsc_data[stsc_index].index);
                current_offset += sample_size;
                current_dts += track->stts_data[stts_index].duration;
                distance++;
                stts_sample++;
                current_sample++;
                // update stts index
                if (stts_index < track->stts_count - 1 
                    && stts_sample == (sx_uint32)track->stts_data[stts_index].count) {
                    stts_sample = 0;
                    stts_index++;
                }
            }
        }
    } 
    else { 
        // audio with constant sample size
        // read whole chunk
        sx_int64 chunk_samples, chunk_size, chunk_duration;
        // loop for all chunks
        for (i = 0; i < (sx_int32)track->stco_count; i++) {
            current_offset = track->stco_data[i]; // sample offset
            // update stsc index
            if (stsc_index < track->stsc_count - 1
                && i == track->stsc_data[stsc_index + 1].first - 1)
                stsc_index++;
            // samples in this chunk
            chunk_samples = track->stsc_data[stsc_index].count;
            // get chunk size
            if (track->info.sample_size > 1 
                || track->info.codec_id == CODEC_ID_PCM_U8 
                || track->info.codec_id == CODEC_ID_PCM_S8)
                // constant sample size
                chunk_size = chunk_samples * track->info.sample_size;
            else if (track->samples_per_frame > 0 
                && (chunk_samples * track->bytes_per_frame % track->samples_per_frame == 0))
                // one frame contains integer number of samples
                chunk_size = chunk_samples * track->bytes_per_frame / track->samples_per_frame;
            else { 
                chunk_size = chunk_samples * track->info.sample_size;
            }
            mp4_add_index_entry(track, current_offset, current_dts, (sx_int32)chunk_size, 
                0, MP4_SAMPLE_KEYFRAME, track->stsc_data[stsc_index].index);
            // get chunk duration
            chunk_duration = 0;
            while (chunk_samples > 0) {
                if (chunk_samples < track->stts_data[stts_index].count) {
                    chunk_duration += track->stts_data[stts_index].duration * chunk_samples;
                    track->stts_data[stts_index].count -= (sx_int32)chunk_samples;
                    break;
                } 
                else {
                    chunk_duration += track->stts_data[stts_index].duration * chunk_samples;
                    chunk_samples -= track->stts_data[stts_index].count;
                    if (stts_index < track->stts_count - 1)
                        stts_index++;
                }
            }
            //mp4_log(ctx->log, "AVIndex track %d, chunk %d, offset %"PRIx64", dts %"PRId64", size %d, duration %d\n",
            //        track->info.track_id, i, current_offset, current_dts, chunk_size, chunk_duration);

            // update dts
            current_dts += chunk_duration;
        } // loop for all chunks
    }
 out:
    // adjust sample count
    track->sample_count = track->rd_entries;
}

// build sample index in fragment mode playback
static sx_int32 mp4_build_index_fragment_mode(mp4_context_t *ctx)
{
    sx_int32 err = MP4MUX_ERR_OK;
    byte_strm_t *bs = ctx->stream;

    // do nothing if not in fragment play mode
    // or closing moov is already found
    if (!CHK_FLAGS(ctx->flags, MP4_FLAG_FRAGMENT_PLAY)
        || CHK_FLAGS(ctx->flags, MP4_FLAG_WRITE_CLOSED))
        return MP4MUX_ERR_OK;

    // update file size
    ctx->file_size = bs_get_size(bs);

    // do parsing only when file size increment is larger than threshold
    if (CHK_FLAGS(ctx->flags, MP4_FLAG_FORCE_PARSE_MOOF)
        || ctx->moof_cnt == 0 
        || ctx->file_size - ctx->moof_list[ctx->moof_cnt - 1].offset > MP4_FRAGMENT_SIZE_THR) {
        // clear flag
        CLR_FLAGS(ctx->flags, MP4_FLAG_FORCE_PARSE_MOOF);

        // read header
        err = mp4_read_index_fragment_mode(ctx);
        if (err == MP4MUX_ERR_PARSE_ERROR) {
            return err;
        }

        // build moof index
        mp4_build_moof_index(ctx);
    }

    return MP4MUX_ERR_OK;
}

// build moof index
static void mp4_build_moof_index(mp4_context_t *ctx)
{
    mp4_atom_t moof = {0, 0, 0};
    int i;

    if (ctx->next_moof_idx >= ctx->moof_cnt)
        return; // no new moof to parse
    SET_FLAGS(ctx->flags, MP4_FLAG_REBUILD);
    for (i=ctx->next_moof_idx; i<ctx->moof_cnt; i++) {
        moof.type = ctx->moof_list[i].type;
        moof.offset = ctx->moof_list[i].offset;
        moof.size = ctx->moof_list[i].size;
        mp4_parse_moof(ctx, &moof);
    }
    ctx->next_moof_idx = ctx->moof_cnt;
}

// seek inside a track
// trk(in): pointer of a track
// timestamp(in): time stamp to seek
// flags(in): seek flags
static sx_int32 mp4_seek_stream(mp4_track_t *trk, sx_int64 timestamp, sx_uint32 flags)
{
    sx_int32 sample, time_sample;
    sx_uint32 i;

    sample = mp4_index_search_timestamp(trk, timestamp, flags);
    //dprintf(st->codec, "stream %d, timestamp %"PRId64", sample %d\n", st->index, timestamp, sample);
    if (sample < 0)
        return -1;

    trk->current_sample = sample;
    // adjust ctts index 
    if (trk->ctts_data) {
        time_sample = 0;
        for (i = 0; i < trk->ctts_count; i++) {
            sx_int32 next = time_sample + trk->ctts_data[i].count;
            if (next > trk->current_sample) {
                trk->ctts_index = i;
                trk->ctts_sample = trk->current_sample - time_sample;
                break;
            }
            time_sample = next;
        }
    }
    return sample;
}

// seek next key frame inside a track
// trk(in): pointer of a track
// flags(in): seek flags
static sx_int32 mp4_seek_stream_next_key(mp4_track_t *trk, sx_uint32 flags)
{
    sx_int32 sample, time_sample;
    sx_uint32 i;

    CLR_FLAGS(flags, MP4_SEEK_FLAG_ANY);
    sample = mp4_index_search_next_key_frame(trk, flags);
    if (sample < 0)
        return -1;

    // adjust ctts index 
    if (trk->ctts_data) {
        time_sample = 0;
        for (i = 0; i < trk->ctts_count; i++) {
            sx_int32 next = time_sample + trk->ctts_data[i].count;
            if (next > trk->current_sample) {
                trk->ctts_index = i;
                trk->ctts_sample = trk->current_sample - time_sample;
                break;
            }
            time_sample = next;
        }
    }
    return sample;
}

// seek inside a track
// ctx(in): mp4 context
// track_id(in): track id
// sample_time(in): sample time stamp
// flags(in): seek flags
sx_int32 mp4_read_seek(mp4_context_t *ctx, sx_int32 track_id, sx_int64 sample_time, sx_uint32 flags)
{
    mp4_track_t *trk;
    sx_int64 seek_timestamp, timestamp;
    sx_int32 sample;
    sx_int32 i;

    if (track_id >= ctx->nb_tracks)
        return MP4MUX_ERR_INVALID_TRACK;

    trk = ctx->tracks[track_id];
    if (CHK_FLAGS(flags, MP4_SEEK_FLAG_NEXT_KEY))
        sample = mp4_seek_stream_next_key(trk, flags);
    else
        sample = mp4_seek_stream(trk, sample_time, flags);
    if (sample < 0)
        return MP4MUX_ERR_SEEK_SAMPLE;

    // adjust seek timestamp to found sample timestamp 
    seek_timestamp = trk->samples_rd[sample].dts;

    // sync all tracks to the seek point
    SET_FLAGS(flags, MP4_SEEK_FLAG_ANY);
    for (i = 0; i < ctx->nb_tracks; i++) {
        trk = ctx->tracks[i];
        if (track_id == i || CHK_FLAGS(trk->flags, TRACK_FLAG_DISABLED))
            continue;

        timestamp = rescale_rnd(seek_timestamp, ctx->tracks[track_id]->info.timescale, trk->info.timescale);
        mp4_seek_stream(trk, timestamp, flags);
    }
    return MP4MUX_ERR_OK;
}

// seek in trick mode
sx_int32 mp4_read_seek_trick_mode(mp4_context_t *ctx)
{
    mp4_track_t *trk = NULL;
    sx_int64 timestamp;
    sx_int32 sample, track_id;
    sx_int32 i;

    if (ctx->trick_track == NULL) {
        // search video track
        for (i=0; i<ctx->nb_tracks; i++) {
            if (ctx->tracks[i]->info.type == MP4_TRACK_VIDEO) {
                trk = ctx->tracks[i];
                track_id = trk->info.track_id;
                break;
            }
        }
        if (trk == NULL)
            return MP4MUX_ERR_NO_VIDEO_TRACK;
        else 
            ctx->trick_track = trk;
    }

    trk = ctx->trick_track;
    if (CHK_FLAGS(ctx->seek_flags, MP4_SEEK_INITIAL)) {
        // initial seek
        sample = mp4_seek_stream_next_key(trk, ctx->seek_flags);
        if (sample < 0)
            return -1;
        trk->current_sample = sample;
        trk->curr_key = sample;
        trk->cur_dts = (double)trk->samples_rd[sample].dts;
        CLR_FLAGS(ctx->seek_flags, MP4_SEEK_INITIAL);

        // seek next key frame
        trk->next_key = mp4_seek_stream_next_key(trk, ctx->seek_flags);
        if (trk->next_key < 0)
            return -1;
        return MP4MUX_ERR_OK;
    }

    trk->cur_dts += ctx->play_speed;
    timestamp = (sx_int64)trk->cur_dts;
    if (timestamp - trk->samples_rd[trk->curr_key].dts 
        > trk->samples_rd[trk->next_key].dts - trk->samples_rd[trk->curr_key].dts) {
        trk->current_sample = trk->next_key;
        trk->curr_key = trk->next_key;
        // seek next key frame
        trk->next_key = mp4_seek_stream_next_key(trk, ctx->seek_flags);
        if (trk->next_key < 0)
            return -1;
    }
    else {
        trk->current_sample = trk->curr_key;
    }

    return MP4MUX_ERR_OK;
}

// fragment mode
// parse moov for fragment mode
sx_int32 mp4_parse_moov_sx(mp4_context_t *ctx, mp4_atom_t *moov)
{
    int i;

    mp4_read_moov(ctx, *moov);

    for (i=0; i<ctx->nb_tracks; i++) {
        mp4_track_t *trk = ctx->tracks[i];

        if (!trk->info.timescale)
            trk->info.timescale = ctx->timescale;
    }

    return 0;
}

// read mvex box
static sx_int32 mp4_read_mvex(mp4_context_t *ctx, mp4_atom_t atom)
{
    mp4_log(ctx->log, "mvex [%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);
    SET_FLAGS(ctx->flags, MP4_FLAG_FOUND_MVEX);
    if (mp4_read_default(ctx, atom) < 0)
        return MP4MUX_ERR_PARSE_ERROR;
    return MP4MUX_ERR_OK;
}

// read trex box
static sx_int32 mp4_read_trex(mp4_context_t *ctx, mp4_atom_t atom)
{
    sx_int32 track_id;
    mp4_track_t *track;
    byte_strm_t *bs = ctx->stream;

    mp4_log(ctx->log, "trex [%08llx]", atom.offset - 8);
    mp4_log(ctx->log, " %d\n", atom.size + 8);

    bs_get_be32(bs); // version/flags
    track_id = bs_get_be32(bs); // track id
    mp4_log(ctx->log, "  track_id = %d\n", track_id);
    track = ctx->tracks[track_id - 1];
    track->default_desc_idx = bs_get_be32(bs); // default_sample_description_index
    track->info.default_duration = bs_get_be32(bs); // default_duration
    track->info.default_size = bs_get_be32(bs); // default_size
    track->default_flags = bs_get_be32(bs); // default_flags
    mp4_log(ctx->log, "  default desc = %d\n", track->default_desc_idx);
    mp4_log(ctx->log, "  default duration = %d\n", track->info.default_duration);
    mp4_log(ctx->log, "  default size = %d\n", track->info.default_size);
    mp4_log(ctx->log, "  default flags = %d\n", track->default_flags);

    return 0;
}

// parse a moof box
sx_int32 mp4_parse_moof(mp4_context_t *ctx, mp4_atom_t *moof)
{
    byte_strm_t *bs = ctx->stream;
    sx_int32 size, track_id, sample_cnt, i;
    sx_uint32 type, seq_no, temp, flags;
    sx_int64 base_data_offset, dts_offset;
    mp4_track_t *track;
    
    moof->size -= 8;
    moof->offset += 8;
    bs_rseek(bs, moof->offset, SEEK_SET);

    // mfhd
    size = bs_get_be32(bs); // size
    type = bs_get_le32(bs); // type
    assert(type == MKTAG( 'm', 'f', 'h', 'd' ));
    temp = bs_get_be32(bs); // version/flag
    seq_no = bs_get_be32(bs); // seq_no
    // FIXME, check validity of seq_no
    moof->size -= 16;

    // parse the content
    while (moof->size > 0) {
        // traf
        size = bs_get_be32(bs); // size
        type = bs_get_le32(bs); // type
        assert(type == MKTAG( 't', 'r', 'a', 'f' ));
        moof->size -= 8;

        // tfhd
        assert(moof->size > 0);
        size = bs_get_be32(bs); // size
        type = bs_get_le32(bs); // type
        assert(type == MKTAG( 't', 'f', 'h', 'd' ));
        temp = bs_get_byte(bs); // version
        flags = bs_get_be24(bs); // flags
        track_id = bs_get_be32(bs); // track_id
        track = ctx->tracks[track_id - 1];
        moof->size -= 16;
        // traf base data offset
        if (CHK_FLAGS(flags, TF_FLAGS_BASE_DATA_OFFSET)) {
            base_data_offset = bs_get_be64(bs);
            moof->size -= 8;
        }
        // traf sample descriptor index
        if (CHK_FLAGS(flags, TF_FLAGS_SAMPLE_DESC_INDEX)) {
            track->default_desc_idx = bs_get_be32(bs);
            moof->size -= 4;
        }
        // traf default sample duration
        if (CHK_FLAGS(flags, TF_FLAGS_DEFAULT_SAMPLE_DURATION)) {
            temp = bs_get_be32(bs);
            moof->size -= 4;
        }
        // traf default sample size
        if (CHK_FLAGS(flags, TF_FLAGS_DEFAULT_SAMPLE_SIZE)) {
            temp = bs_get_be32(bs);
            moof->size -= 4;
        }
        // traf default sample flags
        if (CHK_FLAGS(flags, TF_FLAGS_DEFAULT_SAMPLE_FLAGS)) {
            temp = bs_get_be32(bs);
            moof->size -= 4;
        }

        // trun
        assert(moof->size > 0);
        size = bs_get_be32(bs); // size
        type = bs_get_le32(bs); // type
        assert(type == MKTAG( 't', 'r', 'u', 'n' ));
        temp = bs_get_byte(bs); // version
        flags = bs_get_be24(bs); // flags
        sample_cnt = bs_get_be32(bs); // sample_count
        moof->size -= 16;

        // check sample array size
        track->samples = mp4_fast_realloc1(track->samples, &track->alloc_entries, 
            track->wr_entries + sample_cnt, sizeof(sample_entry_t), MP4_INDEX_CLUSTER_SIZE);
        if (!track->samples)
            return MP4MUX_ERR_MEMORY;

        // trun dts offset
        if (CHK_FLAGS(flags, TF_FLAGS_DTS_OFFSET)) {
            dts_offset = bs_get_be64(bs);
            moof->size -= 8;
        }
        // trun base data offset
        if (CHK_FLAGS(flags, TF_FLAGS_BASE_DATA_OFFSET)) {
            temp = bs_get_be32(bs);
            moof->size -= 4;
        }
        // first sample flags
        if (CHK_FLAGS(flags, TF_FLAGS_FIRST_SAMPLE_FLAGS)) {
            temp = bs_get_be32(bs);
            moof->size -= 4;
        }
        // trun config data
        if (CHK_FLAGS(flags, TF_FLAGS_SAMPLE_CONFIG_DATA)) {
            sx_int32 len;
            stsd_data_t *stsd;
            sx_uint8 *buf = malloc(256);
            assert(buf != NULL);

            // check if there is space in stsd data
            track->stsd_data = mp4_fast_realloc1(track->stsd_data, &track->stsd_count, 
                track->stsd_idx_rd1 + 1, sizeof(stsd_data_t), MP4_STSD_SIZE_INC);
            if (track->stsd_data == NULL)
                return MP4MUX_ERR_MEMORY;

            // read config data
            stsd = &track->stsd_data[track->stsd_idx_rd1];
            if (track->info.codec_id == CODEC_ID_H264) {
                len = bs_get_be32(bs); // sps size
                moof->size -= 4;
                bs_get_buffer(bs, buf, len); // sps data
                moof->size -= len;
                if (CHK_FLAGS(ctx->flags, MP4_FLAG_REBUILD)) {
                    stsd->sps_len = len;
                    stsd->sps_data = (sx_uint8 *)malloc(len);
                    assert(stsd->sps_data != NULL);
                    memcpy(stsd->sps_data, buf, len);
                }
                len = bs_get_be32(bs); // pps size
                moof->size -= 4;
                bs_get_buffer(bs, buf, len); // pps data
                moof->size -= len;
                if (CHK_FLAGS(ctx->flags, MP4_FLAG_REBUILD)) {
                    stsd->pps_len = len;
                    stsd->pps_data = (sx_uint8 *)malloc(len);
                    assert(stsd->pps_data != NULL);
                    memcpy(stsd->pps_data, buf, len);
                }
            }
            else if (track->info.codec_id == CODEC_ID_MPEG4) {
                len = bs_get_be32(bs); // sps size
                moof->size -= 4;
                bs_get_buffer(bs, buf, len); // sps data
                moof->size -= len;
            }
            free(buf);
            track->stsd_idx_rd1++;
        }

        // loop for samples of this trun
        for (i=0; i<sample_cnt; i++, track->wr_entries++) {
            track->samples[track->wr_entries].sd_index = track->default_desc_idx;
            track->samples[track->wr_entries].flags = 0;
            // sample offset
            if (CHK_FLAGS(flags, TF_FLAGS_SAMPLE_OFFSET)) {
                temp = bs_get_be32(bs);
                moof->size -= 4;
                track->samples[track->wr_entries].pos = base_data_offset + temp;
            }
            // sample duration
            if (CHK_FLAGS(flags, TF_FLAGS_SAMPLE_DURATION)) {
                temp = bs_get_be32(bs);
                moof->size -= 4;
                track->samples[track->wr_entries].dts = dts_offset + temp;
                track->samples[track->wr_entries].cts = track->samples[track->wr_entries].dts;
            }
            // sample size
            if (CHK_FLAGS(flags, TF_FLAGS_SAMPLE_SIZE)) {
                temp = bs_get_be32(bs);
                moof->size -= 4;
                track->samples[track->wr_entries].size = temp;
            }
            // sample flags
            if (CHK_FLAGS(flags, TF_FLAGS_SAMPLE_FLAGS)) {
                temp = bs_get_be32(bs);
                moof->size -= 4;
                track->samples[track->wr_entries].flags = temp ? 1 : 0;
                if (temp)
                    track->key_frames++;
            }
            // sample composition time
            if (CHK_FLAGS(flags, TF_FLAGS_SAMPLE_COMP_TIME_OFFSET)) {
                temp = bs_get_be32(bs);
                moof->size -= 4;
            }
            // sample camera info
            if (CHK_FLAGS(flags, TF_FLAGS_SAMPLE_CAMERA_INFO)) {
                temp = bs_get_be32(bs);
                moof->size -= 4;
            }
            // sample entries
            if (track->info.type == MP4_TRACK_VIDEO)
                track->samples[track->wr_entries].samples = 1;
            else {
                track->samples[track->wr_entries].samples = 
                    track->samples[track->wr_entries].size / track->info.default_size;
                track->sample_count += track->samples[track->wr_entries].samples;
            }
        } // loop for sample count

        // copy data for playback
        if (CHK_FLAGS(ctx->flags, MP4_FLAG_FRAGMENT_PLAY)) {
            track->sample_count = track->wr_entries;
            track->samples_rd = track->samples;
        }
    } // while loop

	return MP4MUX_ERR_OK;
}
