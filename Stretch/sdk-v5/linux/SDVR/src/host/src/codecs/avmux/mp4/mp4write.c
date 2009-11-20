/* mp4write.c */
#include <assert.h>
#include "internal.h"
#include "bytestrm.h"

static void mp4_write_ftyp_tag(byte_strm_t *bs, mp4_context_t *ctx);
static void mp4_write_smff_tag(byte_strm_t *bs, mp4_context_t *ctx); // Stretch box
static sx_int32 mp4_write_moov_tag(byte_strm_t *bs, mp4_context_t *ctx);
static sx_int32 put_avc_sps(mp4_track_t *track, mp4_packet_t *pkt);
static sx_int32 put_avc_pps(mp4_track_t *track, mp4_packet_t *pkt);
static sx_int32 put_mpeg4_config(mp4_track_t *track, mp4_packet_t *pkt);

#ifndef FRAGMENT_MODE
static sx_int32 mp4_write_mdat_tag(byte_strm_t *bs, mp4_context_t *ctx);
#else // #ifndef FRAGMENT_MODE
static sx_int32 mp4_mdat_open(mp4_context_t *ctx);
static sx_int32 mp4_mdat_close(mp4_context_t *ctx);
static sx_int32 mp4_write_sx_open(mp4_context_t *ctx);
static sx_int32 mp4_write_sx_close(mp4_context_t *ctx);
static sx_int32 mp4_write_moov_fragment_sx(mp4_context_t *ctx);
static sx_int32 mp4_write_mvex_sx(mp4_context_t *ctx);
static sx_int32 mp4_write_trex_sx(byte_strm_t *bs, mp4_track_t *track);
static sx_int32 mp4_write_moof_sx(mp4_context_t *ctx);
static sx_int32 mp4_write_mfhd_sx(mp4_context_t *ctx);
static sx_int32 mp4_write_traf_sx(byte_strm_t *bs, mp4_track_t *track);
static sx_int32 mp4_write_tfhd_sx(byte_strm_t *bs, mp4_track_t *track);
static sx_int32 mp4_write_trun_sx(byte_strm_t *bs, mp4_track_t *track);
static void rebuild_entries(mp4_context_t *ctx);
static sx_int32 mp4_find_next_moof(byte_strm_t *bs, mp4_atom_t *atom, mp4_atom_t *moof);
#endif // #ifdef FRAGMENT_MODE

extern const codec_tag_t mp4_obj_type[];
extern const codec_tag_t codec_video_tags[];
extern const codec_tag_t codec_audio_tags[];

/*
 * Create mp4 context
 *
 * Parameters:
 *   ctx(in): pointer of mp4 context
 * Return:
 *   Error code
 */
sx_int32 mp4_create(mp4_context_t *ctx)
{
    sx_int32 err = MP4MUX_ERR_OK;
    // create byte stream
    byte_strm_t *bs = malloc(sizeof(byte_strm_t));
    if (bs == NULL)
        return MP4MUX_ERR_MEMORY;
    if ((err = bs_init(bs, ctx->fd, BS_FLAG_READ | BS_FLAG_WRITE, ctx->bs_buf_size)))
        return err;
    ctx->stream = bs;

    // initialization
    ctx->time = ctx->sys_time + 0x7C25B080; // 1970 based -> 1904 based
    ctx->nb_tracks = 0;
	ctx->dts_factor = (double)MP4_TIMESCALE / (double)DTS_REF_CLOCK;
    ctx->init_dts = -1; // assume time stamp is positive value

    mp4_write_ftyp_tag(bs, ctx);
    mp4_write_smff_tag(bs, ctx);
#ifndef FRAGMENT_MODE
    mp4_write_mdat_tag(bs, ctx);
#endif
    return err;
}

/*
 * Open mp4 context for write
 *
 * Parameters:
 *   ctx(in): pointer of mp4 context
 * Return:
 *   Error code
 */
sx_int32 mp4_write_open(mp4_context_t *ctx)
{
    sx_int32 err = MP4MUX_ERR_OK;
    // create byte stream
    byte_strm_t *bs = malloc(sizeof(byte_strm_t));
    if (bs == NULL)
        return MP4MUX_ERR_MEMORY;
    if ((err = bs_init(bs, ctx->fd, BS_FLAG_READ | BS_FLAG_WRITE, ctx->bs_buf_size)))
        return err;
    ctx->stream = bs;
    return err;
}

/*
 * Close mp4 context
 *
 * Parameters:
 *   ctx(in): pointer of mp4 context
 * Return:
 *   Error code
 */
sx_int32 mp4_write_close(mp4_context_t *ctx)
{
    sx_int32 err = MP4MUX_ERR_OK;
    byte_strm_t *bs = ctx->stream;
    sx_int32 i, j;
    sx_int32 flags = 0;
    sx_uint64 limit = (sx_int64)UINT32_MAX - 8;
    sx_uint64 mdat_size;
    offset_t moov_pos;

#ifndef FRAGMENT_MODE

    moov_pos = bs_wtell(bs);

    // update mdat size
    if (ctx->mdat_size <= limit) {
        bs_wseek(bs, ctx->mdat_pos, SEEK_SET);
        bs_put_be32(bs, (sx_uint32)ctx->mdat_size + 8);
    } 
    else {
        // overwrite 'wide' placeholder for 64-bit size
        bs_wseek(bs, ctx->mdat_pos - 8, SEEK_SET);
        bs_put_be32(bs, 1); // 64-bit size: real atom size will be 64 bit value after tag field
        bs_put_tag(bs, (sx_uint8*)"mdat");
        bs_put_be64(bs, ctx->mdat_size + 16);
    }
    bs_wseek(bs, moov_pos, SEEK_SET);
    mp4_write_moov_tag(bs, ctx);

#else // #ifndef FRAGMENT_MODE

    // close mdat
    if (CHK_FLAGS(ctx->flags, MP4_FLAG_MDAT_OPEN)) {
        mp4_mdat_close(ctx);
        mp4_write_moof_sx(ctx);
    }
    // append moov header
    if (CHK_FLAGS(ctx->flags, MP4_FLAG_APPEND_MOOV)) {
        // keep moov pos
        moov_pos = bs_wtell(bs);
        // update mdat size
        mdat_size = moov_pos - ctx->init_mdat_pos;
        if (mdat_size <= limit) {
            bs_wseek(bs, ctx->init_mdat_pos, SEEK_SET);
            bs_put_be32(bs, (sx_uint32)mdat_size);
        } 
        else {
            // overwrite 'wide' placeholder for 64-bit size
            bs_wseek(bs, ctx->init_mdat_pos - 8, SEEK_SET);
            bs_put_be32(bs, 1); // 64-bit size: real atom size will be 64 bit value after tag field
            bs_put_tag(bs, (sx_uint8*)"mdat");
            bs_put_be64(bs, mdat_size + 8);
        }
        // restore moov pos
        bs_wseek(bs, moov_pos, SEEK_SET);
        // write moov header
        mp4_write_moov_tag(ctx->stream, ctx);
        SET_FLAGS(flags, SMFF_FLAG_CLOSING_MOOV);
    }
    // write smff flags
    SET_FLAGS(flags, SMFF_FLAG_CLOSED);
    bs_wseek(bs, ctx->smff_pos + 9, SEEK_SET);
    bs_put_be24(bs, flags); // flags

#endif // #ifndef FRAGMENT_MODE

    for (i=0; i<ctx->nb_tracks; i++) {
        mp4_track_t *trk = ctx->tracks[i];
        mp4_freep((void *)&trk->samples);
        mp4_freep((void *)&trk->samples_fr);
        for (j=0; j<(int)trk->stsd_count; j++) {
            mp4_freep((void *)&trk->stsd_data[j].sps_data);
            mp4_freep((void *)&trk->stsd_data[j].pps_data);
        }
        free(trk->stsd_data);
        mp4_freep((void *)&ctx->tracks[i]);
    }
    ctx->nb_tracks = 0;

    bs_flush_buffer(bs);
    bs_close(bs);
    mp4_freep((void *)&ctx->stream);
    return err;
}

/*
 * Add a track to mp4 file
 *
 * Parameters:
 *   ctx(in): mp4 context
 *   track_info(in/out): track info
 * Return:
 *   Error code
 */
sx_int32 mp4_add_track(mp4_context_t *ctx, mp4_track_info_t *track_info)
{
    mp4_track_t *track;
    if (ctx->nb_tracks >= MP4_MAX_TRACKS)
        return MP4MUX_ERR_TRACK_NUMBER;

#ifdef FRAGMENT_MODE
    // It's not allowed to add more tracks if moov header has been written.
    // mp4_add_track() should be called before any calls of mp4_put_packet()
    if (CHK_FLAGS(ctx->flags, MP4_FLAG_MOOV_WRITTEN))
        return MP4MUX_ERR_ADD_TRACK_NA;
#endif

    track = malloc(sizeof(mp4_track_t));
    if (track == NULL)
        return MP4MUX_ERR_MEMORY;
    memset(track, 0, sizeof(mp4_track_t));

    memcpy(&track->info, track_info, sizeof(mp4_track_info_t));
    track->time = ctx->time;
    track->mode = ctx->mode;
    ctx->tracks[ctx->nb_tracks++] = track;
    track->info.track_id = ctx->nb_tracks;
    track->info.duration = 0;

    if (track->info.codec_id == CODEC_ID_MPEG4
        || track->info.codec_id == CODEC_ID_H264)
        SET_FLAGS(track->flags, TRACK_FLAG_NEED_CONFIG);

    // allocate memory for stsd data
    track->stsd_data = (stsd_data_t *)malloc(sizeof(stsd_data_t) * MP4_STSD_SIZE_INC);
    if (track->stsd_data == NULL)
        return MP4MUX_ERR_MEMORY;
    memset(track->stsd_data, 0, sizeof(stsd_data_t) * MP4_STSD_SIZE_INC);
    track->stsd_count = MP4_STSD_SIZE_INC;

#ifdef FRAGMENT_MODE
    track->default_desc_idx = 1;
    track->default_flags = 0;
#endif

    // update track info
    memcpy(track_info, &track->info, sizeof(mp4_track_info_t));
    return MP4MUX_ERR_OK;
}

/*
 * Put a packet into mp4 file
 *
 * Parameters:
 *   ctx(in): pointer of mp4 context
 *   track_no(in): track id
 *   pkt(in): pointer of an a/v packet
 * Return:
 *   Error code
 */
sx_int32 mp4_put_packet(mp4_context_t *ctx, sx_uint32 track_no, mp4_packet_t *pkt)
{
    byte_strm_t *bs = ctx->stream;
    mp4_track_t *trk = ctx->tracks[track_no - 1];
    sx_uint32 samples_in_chunk = 0;
    sx_int32 size = pkt->size;
    sx_int32 size1;
    sx_int32 new_seg = 0;
	double dts_diff = (double)(pkt->dts - trk->prev_dts);

    if (trk == NULL)
        return MP4MUX_ERR_INVALID_TRACK;
    if (!size) 
        return MP4MUX_ERR_OK;

    // keep previous dts
	trk->prev_dts = pkt->dts;
    // samples in chunk
    if (trk->info.sample_size)
        samples_in_chunk = pkt->sample_count / trk->info.sample_size;
    else
        samples_in_chunk = 1;

    if (trk->info.codec_id == CODEC_ID_H264) {
        // here we assume 4-byte nal start code prefix 
        if (!(pkt->data[0] == 0 && pkt->data[1] == 0 && pkt->data[2] == 0 && pkt->data[3] == 1))
            return MP4MUX_ERR_START_CODE; // discard packet with invalide start code
        switch (pkt->data[4] & 0x1f) {
        case 7: // SPS
#ifdef FRAGMENT_MODE
            // check if mdat is open
            if (CHK_FLAGS(ctx->flags, MP4_FLAG_MDAT_OPEN)) {
                if (ctx->mdat_size > 0) {
                    mp4_mdat_close(ctx);
                    mp4_write_moof_sx(ctx);
                    mp4_mdat_open(ctx);
                }
            }
#endif
            return put_avc_sps(trk, pkt);
        case 8: // PPS
            return put_avc_pps(trk, pkt);
        case 5: // IDR
            new_seg = 1;
        case 1: // non-IDR
            // replace NAL start code with frame size
            size1 = pkt->size - 4;
            pkt->data[0] = (size1 >> 24) & 0xff;
            pkt->data[1] = (size1 >> 16) & 0xff;
            pkt->data[2] = (size1 >> 8) & 0xff;
            pkt->data[3] = size1 & 0xff;
            size = pkt->size;
            break;
        default:
            return MP4MUX_ERR_UNSUPPORTED_NAL_TYPE;
        }
    }
    else if (trk->info.codec_id == CODEC_ID_MPEG4) {
        // check start code prefix
        if (!(pkt->data[0] == 0 && pkt->data[1] == 0 && pkt->data[2] == 1))
            return MP4MUX_ERR_OK; // discard invalid packet
        switch (pkt->data[3]) {
        case 0xb0: // visual sequence start code
#ifdef FRAGMENT_MODE
            // check if mdat is open
            if (CHK_FLAGS(ctx->flags, MP4_FLAG_MDAT_OPEN)) {
                if (ctx->mdat_size > 0) {
                    mp4_mdat_close(ctx);
                    mp4_write_moof_sx(ctx);
                    mp4_mdat_open(ctx);
                }
            }
#endif
            return put_mpeg4_config(trk, pkt);
        case 0xb3: // group of vop start code
            new_seg = 1;
        case 0xb6: // vop start code
            break;
        default:
            return MP4MUX_ERR_OK;
        }
    }

#ifdef FRAGMENT_MODE
    // check if mdat is open
    if (!CHK_FLAGS(ctx->flags, MP4_FLAG_MDAT_OPEN)) {
        mp4_mdat_open(ctx);
    }
    else { // mdat is open
        if (new_seg && ctx->mdat_size > 0) {
            mp4_mdat_close(ctx);
            mp4_write_moof_sx(ctx);
            // flushing
            // ASSUME: we always have video track
            if (ctx->flush_period > 0 
                && (pkt->dts - ctx->last_flush_ts) >= ctx->flush_period) {
                bs_wflush(ctx->stream);
                ctx->last_flush_ts = pkt->dts;
            }
            mp4_mdat_open(ctx);
        }
    }
#endif

    // check fragment index array size
    if (!(trk->fr_entries % MP4_INDEX_CLUSTER_SIZE_FR)) {
        trk->samples_fr = realloc(trk->samples_fr, 
            (trk->fr_entries + MP4_INDEX_CLUSTER_SIZE_FR) * sizeof(sample_entry_t));
        if (!trk->samples_fr)
            return MP4MUX_ERR_MEMORY;
    }

    // update sample entry
    trk->samples_fr[trk->fr_entries].pos = bs_wtell(bs);
    trk->samples_fr[trk->fr_entries].size = size;
    trk->samples_fr[trk->fr_entries].samples = samples_in_chunk;
    trk->samples_fr[trk->fr_entries].dts = pkt->dts;
    trk->samples_fr[trk->fr_entries].sd_index = trk->stsd_idx_wr + 1;
    trk->samples_fr[trk->fr_entries].flags = 0;

    if (CHK_FLAGS(trk->flags, TRACK_FLAG_NEED_CONFIG)) {
        // check stsd validity
        if (!CHK_FLAGS(trk->stsd_data[trk->stsd_idx_wr].flags, CONFIG_FLAGS_VALID)) {
            // for some reason, we get video packets before we get valid sps/pps/vol
            // we still record video frames and keep stsd for later repair
            SET_FLAGS(trk->stsd_data[trk->stsd_idx_wr].flags, CONFIG_FLAGS_RESERVE);
        }
    }

    // update flags
    if (trk->info.type == MP4_TRACK_VIDEO) {
        // update cts
        if (pkt->dts != pkt->pts)
            SET_FLAGS(trk->flags, TRACK_FLAG_HAS_B_FRAME);
        trk->samples_fr[trk->fr_entries].cts = pkt->pts - pkt->dts;
        // update key frame flag
        if (CHK_FLAGS(pkt->flags, PKT_FLAG_KEY_FRAME)) {
            SET_FLAGS(trk->samples_fr[trk->fr_entries].flags, MP4_SAMPLE_KEYFRAME);
            trk->key_frames++;
        }
        // update frame count
		dts_diff = dts_diff * ctx->dts_factor / (double)trk->info.default_duration + 0.5;
		trk->accu_cnt += (sx_int64)dts_diff;
    }
    else if (trk->info.type == MP4_TRACK_AUDIO) {
		trk->accu_cnt += samples_in_chunk;
    }

    // copy fragment entries to track entries
    // Note: it costs more memory to save the track entries. But this saves time when
    // write closing moov box, because we don't need to parse the whole file again.
    if (CHK_FLAGS(ctx->flags, MP4_FLAG_APPEND_MOOV)) {
        // check index array size
        if (!(trk->wr_entries % MP4_INDEX_CLUSTER_SIZE)) {
            trk->samples = realloc(trk->samples, (trk->wr_entries + MP4_INDEX_CLUSTER_SIZE) * sizeof(sample_entry_t));
            if (!trk->samples)
                return MP4MUX_ERR_MEMORY;
        }
        memcpy(&trk->samples[trk->wr_entries], &trk->samples_fr[trk->fr_entries], sizeof(sample_entry_t));
        trk->wr_entries++;
    }

    trk->fr_entries++;
    trk->sample_count += samples_in_chunk;
    ctx->mdat_size += size;

    bs_put_buffer(bs, pkt->data, size);
    bs_flush_buffer(bs);
    return MP4MUX_ERR_OK;
}

sx_int32 mp4_av_sync(mp4_context_t *ctx, sx_uint32 track_no, mp4_packet_t *pkt)
{
    mp4_track_t *trk = ctx->tracks[track_no - 1];
    sx_uint32 samples_in_chunk = 0;
    sx_int32 pkt_size, in_size = pkt->size;
    mp4_packet_t pkt1;
    sx_int64 dts_diff;
	double factor, clk_factor, diff, alpha = 0.9;
    offset_t pos;
    byte_strm_t *bs = ctx->stream;
#ifdef DEBUG_AV_SYNC
    double time;
#endif

    if (trk == NULL)
        return MP4MUX_ERR_INVALID_TRACK;
    if (in_size <= 0) 
        return MP4MUX_ERR_OK;

#ifdef FRAGMENT_MODE
    // check if moov is written
    // we need to write an empty moov before writing the first packet into mdat
    if (!CHK_FLAGS(ctx->flags, MP4_FLAG_MOOV_WRITTEN)) {
        mp4_write_moov_fragment_sx(ctx);
        SET_FLAGS(ctx->flags, MP4_FLAG_MOOV_WRITTEN);
    }
#endif

	trk->curr_dts = pkt->dts;
	if (trk->info.type == MP4_TRACK_AUDIO) {
		samples_in_chunk = pkt->sample_count / trk->info.sample_size;
		if (!CHK_FLAGS(trk->flags, TRACK_FLAG_HAVE_SAMPLE)) { // first sample
#ifdef DEBUG_AV_SYNC
			mp4_log(ctx->log, "a0 0x%I64x\n", pkt->dts);
#endif
            // set movie initial dts
            if (ctx->init_dts < 0)
                ctx->init_dts = pkt->dts;
			trk->init_dts = pkt->dts; // save initial dts
            ctx->last_flush_ts = 0;
            
            // write elst
            pos = bs_wtell(bs);
            bs_wseek(bs, trk->elst_pos, SEEK_SET);
            bs_put_be32(bs, (sx_uint32)(trk->init_dts - ctx->init_dts));
            bs_wseek(bs, pos, SEEK_SET);

			trk->accu_cnt = 0; // reset accumulated sample count
			pkt->dts = pkt->pts = 0; // reset dts for track beginning
			trk->dts_thr = trk->info.default_duration / 2;
			SET_FLAGS(trk->flags, TRACK_FLAG_HAVE_SAMPLE);
			return MP4MUX_ERR_OK;
		}

		// detect drop packet
        diff = (double)(pkt->dts - trk->init_dts) / (double)MP4_TIMESCALE
             * (ctx->dts_factor * (double)trk->info.timescale) + 0.5;
        dts_diff = (sx_int64)diff - trk->accu_cnt;

        // padding drop packet
        if (dts_diff > trk->dts_thr) {
            sx_int32 padding = (sx_int32)(dts_diff + (trk->info.default_duration >> 1)) / trk->info.default_duration;
            sx_int32 i;

#ifdef DEBUG_AV_SYNC
		    mp4_log(ctx->log, "padding %d, %d\n", (int)dts_diff, padding);
#endif

            // default packet size
            pkt_size = trk->info.default_duration * trk->info.default_size;

            // reset pkt elements
            memset(&pkt1, 0, sizeof(mp4_packet_t));
            // allocate buffer
            pkt1.data = (sx_uint8 *)malloc(pkt_size);
            if (pkt1.data == NULL)
                return MP4MUX_ERR_MEMORY;
            memset(pkt1.data, 0, pkt_size);
            // set pkt elements
            pkt1.size = pkt1.sample_count = pkt_size;
		    // time stamp
		    diff = trk->info.default_duration * (double)MP4_TIMESCALE 
			     / (ctx->dts_factor * (double)trk->info.timescale) + 0.5;
            for (i=0; i<padding; i++) {
			    pkt1.dts = pkt1.pts = trk->prev_dts + (sx_int64)diff;
                mp4_put_packet(ctx, track_no, &pkt1);
            }
            trk->drop_cnt += padding;
            free(pkt1.data);
        }

		// update dts after packet padding
		clk_factor = (double)(pkt->dts - trk->init_dts) / (double)trk->accu_cnt;
		factor = (double)MP4_TIMESCALE / (clk_factor * (double)trk->info.timescale);
		ctx->dts_factor = ctx->dts_factor * alpha + (1.0 - alpha) * factor;
#ifdef DEBUG_AV_SYNC
		time = (double)trk->accu_cnt / trk->info.timescale;
		mp4_log(ctx->log, "%6.1f, %d, %8.6f, %8.6f, %d\n", 
			time, (int)pkt->size, ctx->dts_factor, clk_factor,
			(int)(pkt->dts - trk->init_dts - ctx->video_dts));
#endif
		pkt->dts = pkt->pts = trk->accu_cnt; // use sample count as ts for audio
	}
	else if (trk->info.type == MP4_TRACK_VIDEO) {
		int thr = trk->info.default_duration / 100;
		if (!CHK_FLAGS(trk->flags, TRACK_FLAG_HAVE_SAMPLE)) { // first sample
#ifdef DEBUG_AV_SYNC
			mp4_log(ctx->log, "v0 0x%I64x\n", pkt->dts);
#endif
            // set movie initial dts
            if (ctx->init_dts < 0)
                ctx->init_dts = pkt->dts;
			trk->init_dts = pkt->dts;
            ctx->last_flush_ts = 0;

            // write elst
            pos = bs_wtell(bs);
            bs_wseek(bs, trk->elst_pos, SEEK_SET);
            bs_put_be32(bs, (sx_uint32)(trk->init_dts - ctx->init_dts));
            bs_wseek(bs, pos, SEEK_SET);

			trk->accu_cnt = 0;
			pkt->dts = pkt->pts = 0; // reset ts for track beginning
			SET_FLAGS(trk->flags, TRACK_FLAG_HAVE_SAMPLE);
			return MP4MUX_ERR_OK;
		}
#ifdef DEBUG_AV_SYNC
		ctx->video_dts = pkt->dts - trk->init_dts;
#endif

#ifdef DEBUG_AV_SYNC
		// print video ts ratio
		if (trk->accu_cnt > 0) {
			factor = (double)(pkt->dts - trk->init_dts) / (double)trk->accu_cnt;
			if ((trk->accu_cnt % 100) == 0)
				mp4_log(ctx->log, "v f=%8.6f\n", factor);
		}
#endif

		// compensate dts drift
		diff = (double)(pkt->dts - trk->init_dts) * ctx->dts_factor + 0.5;
		pkt->dts = pkt->pts = (sx_int64)diff;

		// normalize
		dts_diff = pkt->dts - trk->prev_dts - trk->info.default_duration;
		if (dts_diff > -thr && dts_diff < thr)
			pkt->dts = pkt->pts = trk->prev_dts + trk->info.default_duration;
#ifdef DEBUG_AV_SYNC
		else
			mp4_log(ctx->log, "v %d\n", (int)(pkt->dts - trk->prev_dts));
#endif
	}

    return MP4MUX_ERR_OK;
}

sx_int32 put_avc_sps(mp4_track_t *track, mp4_packet_t *pkt)
{
    sx_int32 size = pkt->size - 4, wr_sps = 0;
    stsd_data_t *stsd;

    // make sure we have enough space
    track->stsd_data = mp4_fast_realloc2(track->stsd_data, &track->stsd_count, 
        track->stsd_idx_wr + 2, sizeof(stsd_data_t), MP4_STSD_SIZE_INC);
    if (track->stsd_data == NULL)
        return MP4MUX_ERR_MEMORY;

    // get current stsd
    stsd = &track->stsd_data[track->stsd_idx_wr];

    // check if we need to reserve current stsd
    if (CHK_FLAGS(stsd->flags, CONFIG_FLAGS_RESERVE)) {
        track->stsd_idx_wr++;
        stsd = &track->stsd_data[track->stsd_idx_wr];
        memset(stsd, 0, sizeof(stsd_data_t));
    }

    if (stsd->sps_data == NULL) { 
        // sps is empty, allocate memory
        stsd->sps_data = (sx_uint8 *)malloc(size);
        if (stsd->sps_data == NULL)
            return MP4MUX_ERR_MEMORY;
        wr_sps = 1;
    }
    else { 
        // sps is not empty, check if repeat sps
        if ((sx_int32)stsd->sps_len != size || memcmp(stsd->sps_data, &pkt->data[4], size)) {
            // different sps, move to the next entry
            track->stsd_idx_wr++;
            stsd = &track->stsd_data[track->stsd_idx_wr];
            memset(stsd, 0, sizeof(stsd_data_t));
            // allocate memory for sps
            stsd->sps_data = (sx_uint8 *)malloc(size);
            if (stsd->sps_data == NULL)
                return MP4MUX_ERR_MEMORY;
            wr_sps = 1;
        }
    }

    if (wr_sps) {
        memcpy(stsd->sps_data, &pkt->data[4], size);
        stsd->sps_len = size;
        SET_FLAGS(stsd->flags, CONFIG_FLAGS_HAS_SPS);
        if (CHK_FLAGS(stsd->flags, CONFIG_FLAGS_HAS_PPS))
            SET_FLAGS(stsd->flags, CONFIG_FLAGS_VALID);
        SET_FLAGS(track->flags, TRACK_FLAG_NEW_CONFIG);
    }
    return MP4MUX_ERR_OK;
}

sx_int32 put_avc_pps(mp4_track_t *track, mp4_packet_t *pkt)
{
    sx_int32 size = pkt->size - 4, wr_pps = 0;
    stsd_data_t *stsd;

    // make sure we have enough space
    track->stsd_data = mp4_fast_realloc2(track->stsd_data, &track->stsd_count, 
        track->stsd_idx_wr + 2, sizeof(stsd_data_t), MP4_STSD_SIZE_INC);
    if (track->stsd_data == NULL)
        return MP4MUX_ERR_MEMORY;

    // get current stsd
    stsd = &track->stsd_data[track->stsd_idx_wr];

    // check if we need to reserve current stsd
    if (CHK_FLAGS(stsd->flags, CONFIG_FLAGS_RESERVE)) {
        track->stsd_idx_wr++;
        stsd = &track->stsd_data[track->stsd_idx_wr];
        memset(stsd, 0, sizeof(stsd_data_t));
    }

    if (stsd->pps_data == NULL) {
        // empty pps, allocate memory
        stsd->pps_data = (sx_uint8 *)malloc(size);
        if (stsd->pps_data == NULL)
            return MP4MUX_ERR_MEMORY;
        wr_pps = 1;
    }
    else {
        // check if repeat pps
        if ((sx_int32)stsd->pps_len != size || memcmp(stsd->pps_data, &pkt->data[4], size)) {
            // different pps, move to the next entry
            track->stsd_idx_wr++;
            stsd = &track->stsd_data[track->stsd_idx_wr];
            memset(stsd, 0, sizeof(stsd_data_t));
            // allocate memory for pps
            stsd->pps_data = (sx_uint8 *)malloc(size);
            if (stsd->pps_data == NULL)
                return MP4MUX_ERR_MEMORY;
            wr_pps = 1;
        }
    }

    if (wr_pps) {
        memcpy(stsd->pps_data, &pkt->data[4], size);
        stsd->pps_len = size;
        SET_FLAGS(stsd->flags, CONFIG_FLAGS_HAS_PPS);
        if (CHK_FLAGS(stsd->flags, CONFIG_FLAGS_HAS_SPS))
            SET_FLAGS(stsd->flags, CONFIG_FLAGS_VALID);
        SET_FLAGS(track->flags, TRACK_FLAG_NEW_CONFIG);
    }
    return MP4MUX_ERR_OK;
}

sx_int32 put_mpeg4_config(mp4_track_t *track, mp4_packet_t *pkt)
{
    sx_int32 size = pkt->size, wr_vol = 0;
    stsd_data_t *stsd;

    // make sure we have enough space
    track->stsd_data = mp4_fast_realloc2(track->stsd_data, &track->stsd_count, 
        track->stsd_idx_wr + 2, sizeof(stsd_data_t), MP4_STSD_SIZE_INC);
    if (track->stsd_data == NULL)
        return MP4MUX_ERR_MEMORY;

    // get current stsd
    stsd = &track->stsd_data[track->stsd_idx_wr];

    // check if we need to reserve current stsd
    if (CHK_FLAGS(stsd->flags, CONFIG_FLAGS_RESERVE)) {
        track->stsd_idx_wr++;
        stsd = &track->stsd_data[track->stsd_idx_wr];
        memset(stsd, 0, sizeof(stsd_data_t));
    }

    if (stsd->sps_data == NULL) { 
        // empty stsd, allocate memory
        stsd->sps_data = (sx_uint8 *)malloc(size);
        if (stsd->sps_data == NULL)
            return MP4MUX_ERR_MEMORY;
        wr_vol = 1;
    }
    else { 
        // stsd not empty, check if repeat vol
        if ((sx_int32)stsd->sps_len != size || memcmp(stsd->sps_data, pkt->data, size)) {
            // different vol, move to the next entry
            track->stsd_idx_wr++;
            stsd = &track->stsd_data[track->stsd_idx_wr];
            memset(stsd, 0, sizeof(stsd_data_t));
            // allocate memory for vol
            stsd->sps_data = (sx_uint8 *)malloc(size);
            if (stsd->sps_data == NULL)
                return MP4MUX_ERR_MEMORY;
            wr_vol = 1;
        }
    }

    if (wr_vol) {
        memcpy(stsd->sps_data, pkt->data, size);
        stsd->sps_len = size;
        SET_FLAGS(stsd->flags, CONFIG_FLAGS_VALID);
        SET_FLAGS(track->flags, TRACK_FLAG_NEW_CONFIG);
    }
    return MP4MUX_ERR_OK;
}

static offset_t update_size(byte_strm_t *bs, offset_t pos)
{
    offset_t size;
    offset_t curpos = bs_wtell(bs);
    bs_wseek(bs, pos, SEEK_SET);
    size = curpos - pos;
    assert(size <= UINT32_MAX); // need to support 64-bit size?
    bs_put_be32(bs, (sx_uint32)size); // rewrite size 
    bs_wseek(bs, curpos, SEEK_SET);
    return size;
}

/* stco atom */
static sx_int32 mp4_write_stco_tag(byte_strm_t *bs, mp4_track_t* track)
{
    sx_int32 i;
    sx_int32 mode64 = 0; // use 32 bit size variant if possible
    offset_t pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size 
    if (pos > (sx_int64)UINT32_MAX) {
        mode64 = 1;
        bs_put_tag(bs, (sx_uint8*)"co64");
    } 
    else
        bs_put_tag(bs, (sx_uint8*)"stco");
    bs_put_be32(bs, 0); // version & flags
    bs_put_be32(bs, track->wr_entries); // entry count 
    for (i=0; i<track->wr_entries; i++) {
        if (mode64 == 1)
            bs_put_be64(bs, track->samples[i].pos);
        else
            bs_put_be32(bs, (sx_uint32)track->samples[i].pos);
    }
    return (sx_int32)update_size(bs, pos);
}

/* stsz atom */
static sx_int32 mp4_write_stsz_tag(byte_strm_t *bs, mp4_track_t* track)
{
    sx_int32 i, entries = 0;

    offset_t pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size 
    bs_put_tag(bs, (sx_uint8*)"stsz");
    bs_put_be32(bs, 0); // version & flags 

    if (track->info.sample_size > 0) {
        for (i=0; i<track->wr_entries; i++)
            entries += track->samples[i].size / track->info.sample_size;
        bs_put_be32(bs, track->info.sample_size); // sample size
        bs_put_be32(bs, entries); // sample count
    }
    else {
        bs_put_be32(bs, 0); // sample size
        bs_put_be32(bs, track->wr_entries); // sample count
        for (i=0; i<track->wr_entries; i++) {
            bs_put_be32(bs, track->samples[i].size);
        }
    }
    return (sx_int32)update_size(bs, pos);
}

/* stsc atom */
static sx_int32 mp4_write_stsc_tag(byte_strm_t *bs, mp4_track_t* track)
{
    sx_int32 index = 0, oldval = -1, i;
    offset_t entryPos, curpos;

    offset_t pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size 
    bs_put_tag(bs, (sx_uint8*)"stsc");
    bs_put_be32(bs, 0); // version & flags
    entryPos = bs_wtell(bs);
    bs_put_be32(bs, track->wr_entries); // entry count
    for (i=0; i<track->wr_entries; i++) {
        if (oldval != (sx_int32)track->samples[i].sd_index)
        {
            bs_put_be32(bs, i+1); // first chunk
            bs_put_be32(bs, track->samples[i].samples); // samples per chunk
            bs_put_be32(bs, track->samples[i].sd_index); // sample description index
            oldval = track->samples[i].sd_index;
            index++;
        }
    }
    curpos = bs_wtell(bs);
    bs_wseek(bs, entryPos, SEEK_SET);
    bs_put_be32(bs, index); // rewrite size
    bs_wseek(bs, curpos, SEEK_SET);

    return (sx_int32)update_size(bs, pos);
}

/* stss atom */
static sx_int32 mp4_write_stss_tag(byte_strm_t *bs, mp4_track_t* track)
{
    offset_t curpos, entryPos;
    sx_int32 i, index = 0;
    offset_t pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"stss");
    bs_put_be32(bs, 0); // version & flags
    entryPos = bs_wtell(bs);
    bs_put_be32(bs, track->wr_entries); // entry count
    for (i=0; i<track->wr_entries; i++) {
        if (CHK_FLAGS(track->samples[i].flags, MP4_SAMPLE_KEYFRAME)) {
            bs_put_be32(bs, i+1);
            index++;
        }
    }
    curpos = bs_wtell(bs);
    bs_wseek(bs, entryPos, SEEK_SET);
    bs_put_be32(bs, index); // rewrite size
    bs_wseek(bs, curpos, SEEK_SET);
    return (sx_int32)update_size(bs, pos);
}

static sx_uint32 descrLength(sx_uint32 len)
{
    sx_int32 i;
    for (i=1; len >> (7*i); i++);
    return len + 1 + i;
}

static void bs_putDescr(byte_strm_t *bs, sx_int32 tag, sx_int32 size)
{
    sx_int32 i= descrLength(size) - size - 2;
    bs_put_byte(bs, tag);
    for (; i>0; i--)
        bs_put_byte(bs, (size >> (7*i)) | 0x80);
    bs_put_byte(bs, size & 0x7F);
}

static sx_int32 mp4_write_esds_tag(byte_strm_t *bs, mp4_track_t* track, stsd_data_t *stsd)
{
    offset_t pos = bs_wtell(bs);
    sx_int32 decoderSpecificInfoLen = (stsd && stsd->sps_data) ? descrLength(stsd->sps_len) : 0;

    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"esds");
    bs_put_be32(bs, 0); // version + flags

    // ES_Descriptor
    bs_putDescr(bs, 0x03, 3 + descrLength(13 + decoderSpecificInfoLen) + descrLength(1));
    bs_put_be16(bs, track->info.track_id); // ES_ID
    bs_put_byte(bs, 0x00); // flags

    // DecoderConfigDescriptor
    bs_putDescr(bs, 0x04, 13 + decoderSpecificInfoLen);

    // objectTypeIndication
    bs_put_byte(bs, codec_get_tag(mp4_obj_type, track->info.codec_id));

    // 6-bit streamType + 1-bit upStream + 1-bit reserved(1)
    if (track->info.type == MP4_TRACK_AUDIO)
        bs_put_byte(bs, 0x15); // streamType = 0x5 (AudioStream)
    else
        bs_put_byte(bs, 0x11); // streamType = 0x4 (VisualStream)

    // 24-bit bufferSizdeDB
    bs_put_byte(bs, track->info.rc_buffer_size >> 16);
    bs_put_be16(bs, track->info.rc_buffer_size & 0xFFFF);

    // 32-bit maxBitRate
    bs_put_be32(bs, MAX(track->info.bit_rate, track->info.rc_max_rate));
    // 32-bit avgBitRate
    if (track->info.rc_max_rate != track->info.rc_min_rate || track->info.rc_min_rate == 0)
        bs_put_be32(bs, 0); // vbr
    else
        bs_put_be32(bs, track->info.rc_max_rate); // avg bitrate

    // DecoderSpecificInfo
    if (stsd != NULL && stsd->sps_len > 0 && stsd->sps_data) {
        bs_putDescr(bs, 0x05, stsd->sps_len); // DecoderSpecificInfo
        bs_put_buffer(bs, stsd->sps_data, stsd->sps_len);
    }

    // SLConfigDescriptor
    bs_putDescr(bs, 0x06, 1);
    bs_put_byte(bs, 0x02);
    return (sx_int32)update_size(bs, pos);
}

#if 0
static sx_int32 mp4_write_wave_tag(byte_strm_t *bs, mp4_track_t* track)
{
    offset_t pos = bs_wtell(bs);

    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"wave");

    // frma atom
    bs_put_be32(bs, 12); // size
    bs_put_tag(bs, (sx_uint8*)"frma");
    bs_put_le32(bs, track->tag);

    // mp4a atom
    if (track->info.codec_id == CODEC_ID_PCM_MULAW) {
        bs_put_be32(bs, 12); // size
        bs_put_tag(bs, (sx_uint8*)"mp4a");
        bs_put_be32(bs, 0);

        // esds atom
        mp4_write_esds_tag(bs, track, NULL);
    }

    // terminator atom
    bs_put_be32(bs, 8); // size
    bs_put_be32(bs, 0);

    return (sx_int32)update_size(bs, pos);
}
#endif

static sx_int32 mp4_write_audio_tag(byte_strm_t *bs, mp4_track_t* track)
{
    offset_t pos = bs_wtell(bs);
    sx_int32 version = 0;
    track->tag = codec_get_tag(codec_audio_tags, track->info.codec_id);

    bs_put_be32(bs, 0); // size
    bs_put_le32(bs, track->tag); // data format
    bs_put_be32(bs, 0); // reserved 
    bs_put_be16(bs, 0); // reserved 
    bs_put_be16(bs, 1); // data-reference-index

    // SoundDescription 
    bs_put_be16(bs, version); // mov: version, mp4: pre-defined = 0
    bs_put_be16(bs, 0); // mov: revision level, mp4: pre-defined = 0
    bs_put_be32(bs, 0); // mov: vender = 0, mp4: pre-defined = 0

    if (track->mode == MODE_MOV) { // mov mode
        bs_put_be16(bs, track->info.channels); // number of channels
        bs_put_be16(bs, 16); // uncompressed sample size
        bs_put_be16(bs, 0); // compress id = 0
    }
    else { // mp4 mode
        bs_put_be16(bs, 2); // pre-defined = 2
        bs_put_be16(bs, 16); // pre-defined = 16
        bs_put_be16(bs, 0); // pre-defined = 0
    }

    bs_put_be16(bs, 0); // mov: packet size = 0, mp4: pre-defined = 0
    bs_put_be16(bs, track->info.timescale); // mov: sample rate, mp4: time-scale
    bs_put_be16(bs, 0); // mov: sample rate, mp4: pre-defined = 0

    if (version == 1) { // mov: SoundDescription V1 extended info 
        bs_put_be32(bs, track->frame_size); // samples per packet 
        bs_put_be32(bs, track->info.sample_size / track->info.channels); // bytes per packet 
        bs_put_be32(bs, track->info.sample_size); // bytes per frame 
        bs_put_be32(bs, 2); // bytes per sample 
    }

    return (sx_int32)update_size(bs, pos);
}

static sx_int32 mp4_write_avcc_tag(byte_strm_t *bs, stsd_data_t *stsd)
{
    offset_t pos = bs_wtell(bs);
    sx_uint8 profile, profile_comp, level;

    bs_put_be32(bs, 0);
    bs_put_tag(bs, (sx_uint8*)"avcC");

    assert(stsd);
    if (stsd->sps_data && stsd->sps_len > 0) {
        profile = stsd->sps_data[0];
        profile_comp = stsd->sps_data[1];
        level = stsd->sps_data[2];

        bs_put_byte(bs, 1); // version 
        bs_put_byte(bs, profile); // profile 
        bs_put_byte(bs, profile_comp); // profile compat 
        bs_put_byte(bs, level); // level 
        bs_put_byte(bs, 0xff); // 6 bits reserved (111111) + 2 bits nal size length - 1 (11) 
        // put sps
        bs_put_byte(bs, 1 | 0xe0); // 3 bits reserved (111) + 5 bits number of sps 
        bs_put_be16(bs, stsd->sps_len);
        bs_put_buffer(bs, stsd->sps_data, stsd->sps_len);
    }
    else {
        // don't have valid sps, leave some space
        bs_put_byte(bs, 1); // version 
        bs_put_byte(bs, 0x67); // profile 
        bs_put_byte(bs, 0x42); // profile compat 
        bs_put_byte(bs, 0); // level 
        bs_put_byte(bs, 0xff); // 6 bits reserved (111111) + 2 bits nal size length - 1 (11) 
        // put sps
        bs_put_byte(bs, 1 | 0xe0); // 3 bits reserved (111) + 5 bits number of sps 
        bs_put_be16(bs, 32); // put 32 zero bytes
        bs_put_be32(bs, 0);
        bs_put_be32(bs, 0);
        bs_put_be32(bs, 0);
        bs_put_be32(bs, 0);
        bs_put_be32(bs, 0);
        bs_put_be32(bs, 0);
        bs_put_be32(bs, 0);
        bs_put_be32(bs, 0);
    }

    // put pps
    if (stsd->pps_data && stsd->pps_len > 0) {
        bs_put_byte(bs, 1); // number of pps 
        bs_put_be16(bs, stsd->pps_len);
        bs_put_buffer(bs, stsd->pps_data, stsd->pps_len);
    }
    else {
        // don't have valid pps, leave some space
        bs_put_byte(bs, 1); // number of pps 
        bs_put_be16(bs, 8); // put 8 zero bytes
        bs_put_be32(bs, 0);
        bs_put_be32(bs, 0);
    }

    return (sx_int32)update_size(bs, pos);
}

static sx_int32 mp4_write_video_tag(byte_strm_t *bs, mp4_track_t* track, stsd_data_t *stsd)
{
    offset_t pos = bs_wtell(bs);
    char compressor_name[32];
    sx_uint32 tag = codec_get_tag(codec_video_tags, track->info.codec_id);

    bs_put_be32(bs, 0); // size
    bs_put_le32(bs, tag); // format
    bs_put_be32(bs, 0); // reserved = 0
    bs_put_be16(bs, 0); // reserved = 0
    bs_put_be16(bs, 1); // data-reference-index

    bs_put_be16(bs, 0); // mov: version, mp4: reserved = 0
    bs_put_be16(bs, 0); // mov: revision, mp4: reserved = 0

    bs_put_be32(bs, 0); // mov: vendor, mp4: reserved = 0
    bs_put_be32(bs, 0); // mov: temporal quality, mp4: reserved = 0
    bs_put_be32(bs, 0); // mov: spatial quality, mp4: reserved = 0

    bs_put_be16(bs, track->info.width); // width
    bs_put_be16(bs, track->info.height); // height
    bs_put_be32(bs, 0x00480000); // mov: horizontal resolution 72dpi, mp4: pre-defined
    bs_put_be32(bs, 0x00480000); // mov: vertical resolution 72dpi, mp4: pre-defined
    bs_put_be32(bs, 0); // mov: data size, mp4: reserved = 0
    bs_put_be16(bs, 1); // mov: frame count, mp4: pre-defined = 1

    // mov: compressor name, mp4: pre-defined = 0
    memset(compressor_name, 0, 32);
    bs_put_byte(bs, (int)strlen(compressor_name));
    bs_put_buffer(bs, (sx_uint8*)compressor_name, 31);

    bs_put_be16(bs, 0x18); // mov: depth, mp4: pre-defined = 24
    bs_put_be16(bs, 0xffff); // mov: color table id, mp4: pre-defined = -1
    if (track->info.codec_id == CODEC_ID_MPEG4)
        mp4_write_esds_tag(bs, track, stsd);
    else if (track->info.codec_id == CODEC_ID_H264)
        mp4_write_avcc_tag(bs, stsd);

    return (sx_int32)update_size(bs, pos);
}

static sx_int32 mp4_write_stsd_tag(byte_strm_t *bs, mp4_track_t* track)
{
    sx_uint32 i, entries;
    offset_t pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"stsd");
    bs_put_be32(bs, 0); // version & flags
    entries = track->stsd_idx_wr + 1;
    bs_put_be32(bs, entries); // entry count
    for (i=0; i<entries; i++) {
        if (track->info.type == MP4_TRACK_VIDEO)
            mp4_write_video_tag(bs, track, &track->stsd_data[i]);
        else if (track->info.type == MP4_TRACK_AUDIO)
            mp4_write_audio_tag(bs, track);
    }
    return (sx_int32)update_size(bs, pos);
}

static sx_int32 mp4_write_ctts_tag(byte_strm_t *bs, mp4_track_t* track)
{
    mp4_stts_t *ctts_entries;
    sx_uint32 entries = 0;
    sx_uint32 atom_size;
    sx_int32 i;

    ctts_entries = malloc((track->wr_entries + 1) * sizeof(*ctts_entries));
    assert(ctts_entries != NULL);
    ctts_entries[0].count = 1;
    ctts_entries[0].duration = (sx_int32)track->samples[0].cts;
    for (i=1; i<track->wr_entries; i++) {
        if (track->samples[i].cts == ctts_entries[entries].duration) {
            ctts_entries[entries].count++;
        } 
        else {
            entries++;
            ctts_entries[entries].duration = (sx_int32)track->samples[i].cts;
            ctts_entries[entries].count = 1;
        }
    }
    entries++; // last one
    atom_size = 16 + (entries * 8);
    bs_put_be32(bs, atom_size); // size
    bs_put_tag(bs, (sx_uint8*)"ctts");
    bs_put_be32(bs, 0); // version & flags
    bs_put_be32(bs, entries); // entry count
    for (i=0; i<(sx_int32)entries; i++) {
        bs_put_be32(bs, ctts_entries[i].count);
        bs_put_be32(bs, ctts_entries[i].duration);
    }
    free(ctts_entries);
    return atom_size;
}

/* stts atom */
static sx_int32 mp4_write_stts_tag(byte_strm_t *bs, mp4_track_t* track)
{
    mp4_stts_t *stts_entries = NULL;
    sx_uint32 entries = 0;
    sx_uint32 atom_size;
    sx_int32 i;

    if (track->info.type == MP4_TRACK_AUDIO && !track->info.audio_vbr) {
        stts_entries = malloc(sizeof(*stts_entries)); // one entry
        assert(stts_entries != NULL);
        stts_entries[0].count = track->sample_count;
        stts_entries[0].duration = 1;
        entries = 1;
    } 
    else {
        if (track->wr_entries > 0) {
            sx_int64 duration;
            stts_entries = malloc(track->wr_entries * sizeof(*stts_entries)); // worst case
            assert(stts_entries != NULL);
            // first entry
            duration = track->wr_entries == 1 ? track->info.default_duration :
                    track->samples[1].dts - track->samples[0].dts;
            stts_entries[0].duration = (sx_int32)duration;
            stts_entries[0].count = 1;

            // loop for rest of entries
            for (i=1; i<track->wr_entries; i++) {
                duration = i + 1 == track->wr_entries ? track->info.default_duration : 
                    track->samples[i+1].dts - track->samples[i].dts;
                if (duration == stts_entries[entries].duration) {
                    stts_entries[entries].count++;
                } 
                else {
                    entries++;
                    stts_entries[entries].duration = (sx_int32)duration;
                    stts_entries[entries].count = 1;
                }
            }
            entries++; // last entry
        }
    }
    atom_size = 16 + (entries * 8);
    bs_put_be32(bs, atom_size); // size
    bs_put_tag(bs, (sx_uint8*)"stts");
    bs_put_be32(bs, 0); // version & flags
    bs_put_be32(bs, entries); // entry count
    for (i=0; i<(sx_int32)entries; i++) {
        bs_put_be32(bs, stts_entries[i].count);
        bs_put_be32(bs, stts_entries[i].duration);
    }
    if (stts_entries != NULL)
    free(stts_entries);
    return atom_size;
}

static sx_int32 mp4_write_dref_tag(byte_strm_t *bs)
{
    bs_put_be32(bs, 28); // size
    bs_put_tag(bs, (sx_uint8*)"dref");
    bs_put_be32(bs, 0); // version & flags
    bs_put_be32(bs, 1); // entry count

    bs_put_be32(bs, 0xc); // size
    bs_put_tag(bs, (sx_uint8*)"url ");
    bs_put_be32(bs, 1); // version & flags

    return 28;
}

static sx_int32 mp4_write_stbl_tag(byte_strm_t *bs, mp4_track_t* track)
{
    offset_t pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"stbl");
    mp4_write_stsd_tag(bs, track);
    mp4_write_stts_tag(bs, track);
    if (track->info.type == MP4_TRACK_VIDEO && track->key_frames < track->wr_entries)
        mp4_write_stss_tag(bs, track);
    if (track->info.type == MP4_TRACK_VIDEO && CHK_FLAGS(track->flags, TRACK_FLAG_HAS_B_FRAME))
        mp4_write_ctts_tag(bs, track);
    mp4_write_stsc_tag(bs, track);
    mp4_write_stsz_tag(bs, track);
    mp4_write_stco_tag(bs, track);
    return (sx_int32)update_size(bs, pos);
}

static sx_int32 mp4_write_dinf_tag(byte_strm_t *bs)
{
    offset_t pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"dinf");
    mp4_write_dref_tag(bs);
    return (sx_int32)update_size(bs, pos);
}

static sx_int32 mp4_write_smhd_tag(byte_strm_t *bs)
{
    bs_put_be32(bs, 16); // size
    bs_put_tag(bs, (sx_uint8*)"smhd");
    bs_put_be32(bs, 0); // version & flags
    bs_put_be16(bs, 0); // reserved (balance, normally = 0)
    bs_put_be16(bs, 0); // reserved
    return 16;
}

static sx_int32 mp4_write_vmhd_tag(byte_strm_t *bs)
{
    bs_put_be32(bs, 0x14); // size (always 0x14)
    bs_put_tag(bs, (sx_uint8*)"vmhd");
    bs_put_be32(bs, 0x01); // version & flags
    bs_put_be64(bs, 0); // reserved (graphics mode = copy)
    return 0x14;
}

static sx_int32 mp4_write_hdlr_tag(byte_strm_t *bs, mp4_track_t *track)
{
    const char *descr, *hdlr, *hdlr_type;
    offset_t pos = bs_wtell(bs);

    if (!track) { // no media -> data handler
        hdlr = "hdlr";
        hdlr_type = "url ";
        descr = "DataHandler";
    } else {
        hdlr = "\0\0\0\0";
        if (track->info.type == MP4_TRACK_VIDEO) {
            hdlr_type = "vide";
            descr = "VideoHandler";
        } else {
            hdlr_type = "soun";
            descr = "SoundHandler";
        }
    }

    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"hdlr");
    bs_put_be32(bs, 0); // version & flags
    bs_put_buffer(bs, (sx_uint8*)hdlr, 4); // handler
    bs_put_tag(bs, (sx_uint8*)hdlr_type); // handler type
    bs_put_be32(bs ,0); // reserved
    bs_put_be32(bs ,0); // reserved
    bs_put_be32(bs ,0); // reserved
    bs_put_byte(bs, (int)strlen(descr)); // string counter
    bs_put_buffer(bs, (sx_uint8*)descr, (int)strlen(descr)); // handler description
    return (sx_int32)update_size(bs, pos);
}

static sx_int32 mp4_write_minf_tag(byte_strm_t *bs, mp4_track_t *track)
{
    offset_t pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"minf");
    if (track->info.type == MP4_TRACK_VIDEO)
        mp4_write_vmhd_tag(bs);
    else
        mp4_write_smhd_tag(bs);
//    if (track->mode == MODE_MOV)
//        mp4_write_hdlr_tag(bs, NULL);
    mp4_write_dinf_tag(bs);
    mp4_write_stbl_tag(bs, track);
    return (sx_int32)update_size(bs, pos);
}

static sx_int32 mp4_write_mdhd_tag(byte_strm_t *bs, mp4_track_t *track)
{
    sx_int32 version = track->info.duration < INT32_MAX ? 0 : 1;

    (version == 1) ? bs_put_be32(bs, 44) : bs_put_be32(bs, 32); // size
    bs_put_tag(bs, (sx_uint8*)"mdhd");
    bs_put_byte(bs, version);
    bs_put_be24(bs, 0); // flags
    if (version == 1) {
        bs_put_be64(bs, track->time);
        bs_put_be64(bs, track->time);
    }
    else {
        bs_put_be32(bs, (sx_uint32)track->time); // creation time
        bs_put_be32(bs, (sx_uint32)track->time); // modification time
    }
    bs_put_be32(bs, track->info.timescale); // time scale
    (version == 1) ? bs_put_be64(bs, track->info.duration) 
        : bs_put_be32(bs, (sx_uint32)track->info.duration); // duration
    bs_put_be16(bs, track->info.language); // language
    bs_put_be16(bs, 0); // reserved (quality)

    return 32;
}

static sx_int32 mp4_write_mdia_tag(byte_strm_t *bs, mp4_track_t *track)
{
    offset_t pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"mdia");
    mp4_write_mdhd_tag(bs, track);
    mp4_write_hdlr_tag(bs, track);
    mp4_write_minf_tag(bs, track);
    return (sx_int32)update_size(bs, pos);
}

static sx_int32 mp4_write_edts_tag(byte_strm_t *bs, mp4_track_t *track)
{
    sx_int64 duration = rescale_rnd(track->info.duration, MP4_TIMESCALE, track->info.timescale);
    sx_int64 pts_offset, media_time;
    sx_int8 version = duration < INT32_MAX ? 0 : 1;
    offset_t pos = bs_wtell(bs);

	bs_put_be32(bs, version ? 64 : 48); // size
	bs_put_tag(bs, (sx_uint8*)"edts");
	bs_put_be32(bs, version ? 56 : 40); // size
	bs_put_tag(bs, (sx_uint8*)"elst");
    bs_put_byte(bs, version);
    bs_put_be24(bs, 0); // flags
	bs_put_be32(bs, 0x2); // entry count

    // calculate offset
    if (track->pts_offset > 0) {
        // starting time is positive, delay the whole track by offset
        pts_offset = track->pts_offset;
        media_time = 0;
    }
    else {
        // starting time is negtive, skip the beginning offset
        pts_offset = 0;
        duration += track->pts_offset; // reduce track duration
        media_time = (0 - track->pts_offset) 
            * track->info.timescale / MP4_TIMESCALE; // convert to track timescale
    }

	if (version == 1) {
		bs_put_be64(bs, pts_offset); // offset
		bs_put_be64(bs, (sx_uint64)-1); // media time = -1
		bs_put_be32(bs, 0x00010000); // media rate
		bs_put_be64(bs, duration); // duration
		bs_put_be64(bs, media_time); // media time
		bs_put_be32(bs, 0x00010000); // media rate
	}
	else {
		bs_put_be32(bs, (sx_uint32)pts_offset); // offset
		bs_put_be32(bs, (sx_uint32)-1); // media time = -1
		bs_put_be32(bs, 0x00010000); // media rate
		bs_put_be32(bs, (sx_uint32)duration); // duration
		bs_put_be32(bs, (sx_uint32)media_time); // media time
		bs_put_be32(bs, 0x00010000); // media rate
	}
	return (sx_int32)update_size(bs, pos);
}

static sx_int32 mp4_write_edts_tag_sx(byte_strm_t *bs, mp4_track_t *track)
{
    offset_t pos = bs_wtell(bs);

	bs_put_be32(bs, 36); // size
	bs_put_tag(bs, (sx_uint8*)"edts");
	bs_put_be32(bs, 28); // size
	bs_put_tag(bs, (sx_uint8*)"elst");
    bs_put_byte(bs, 0); // version
    bs_put_be24(bs, 0); // flags
	bs_put_be32(bs, 0x1); // entry count

    track->elst_pos = bs_wtell(bs); // keep elst pos
	bs_put_be32(bs, 0); // segment duration intialized as 0
	bs_put_be32(bs, (sx_uint32)-1); // media time = -1
	bs_put_be32(bs, 0x00010000); // media rate

    return (sx_int32)update_size(bs, pos);
}

static sx_int32 mp4_write_tkhd_tag(byte_strm_t *bs, mp4_track_t *track)
{
    sx_int64 duration = rescale_rnd(track->info.duration, MP4_TIMESCALE, track->info.timescale);
    sx_int32 version = duration < INT32_MAX ? 0 : 1;

    (version == 1) ? bs_put_be32(bs, 104) : bs_put_be32(bs, 92); // size
    bs_put_tag(bs, (sx_uint8*)"tkhd");
    bs_put_byte(bs, version);
    bs_put_be24(bs, 0xf); // flags
    if (version == 1) {
        bs_put_be64(bs, track->time);
        bs_put_be64(bs, track->time);
    } 
    else {
        bs_put_be32(bs, (sx_uint32)track->time); // creation time
        bs_put_be32(bs, (sx_uint32)track->time); // modification time
    }
    bs_put_be32(bs, track->info.track_id); // track-id
    bs_put_be32(bs, 0); // reserved = 0
    (version == 1) ? bs_put_be64(bs, duration) : bs_put_be32(bs, (sx_uint32)duration); // duration

    bs_put_be32(bs, 0); // reserved = 0
    bs_put_be32(bs, 0); // reserved = 0
    bs_put_be32(bs, 0x0); // layer & alternate group
    // volume, only for audio
    if (track->info.type == MP4_TRACK_AUDIO)
        bs_put_be16(bs, 0x0100);
    else
        bs_put_be16(bs, 0);
    bs_put_be16(bs, 0); // reserved = 0

    // unity matrix = {0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000}
    bs_put_be32(bs, 0x00010000);
    bs_put_be32(bs, 0x0);
    bs_put_be32(bs, 0x0);
    bs_put_be32(bs, 0x0);
    bs_put_be32(bs, 0x00010000);
    bs_put_be32(bs, 0x0);
    bs_put_be32(bs, 0x0);
    bs_put_be32(bs, 0x0);
    bs_put_be32(bs, 0x40000000);

    // width and height, visual only
    if(track->info.type == MP4_TRACK_VIDEO) {
        double sample_aspect_ratio = 1.0; // FIXME
        if (!sample_aspect_ratio) 
            sample_aspect_ratio = 1;
        bs_put_be32(bs, (sx_uint32)(sample_aspect_ratio * track->info.width * 0x10000));
        bs_put_be32(bs, (sx_uint32)(track->info.height * 0x10000));
    }
    else {
        bs_put_be32(bs, 0);
        bs_put_be32(bs, 0);
    }
    return 0x5c;
}

static sx_int32 mp4_write_trak_tag(byte_strm_t *bs, mp4_track_t *track)
{
    offset_t pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"trak");
    // write track header
    mp4_write_tkhd_tag(bs, track);
    // write edts
    if (!CHK_FLAGS(track->flags, TRACK_FLAG_HAVE_SAMPLE)) {
		mp4_write_edts_tag_sx(bs, track);
    }
	else if (track->pts_offset != 0) {
		mp4_write_edts_tag(bs, track);
	}
    // write mdia
    mp4_write_mdia_tag(bs, track);
    return (sx_int32)update_size(bs, pos);
}

static sx_int32 mp4_write_mvhd_tag(byte_strm_t *bs, mp4_context_t *ctx)
{
    sx_int32 maxTrackID = 1, i;
    sx_int64 maxTrackLenTemp, maxTrackLen = 0;
    sx_int32 version;
    mp4_track_t *track;

    // update track duration
    for (i=0; i<ctx->nb_tracks; i++) {
        track = ctx->tracks[i];
        track->info.duration = 0;
        if (track->wr_entries > 0) {
            track->info.duration = track->samples[track->wr_entries-1].dts - track->samples[0].dts
                                 + track->info.default_duration;
            maxTrackLenTemp = rescale_rnd(track->info.duration, MP4_TIMESCALE, track->info.timescale);
            if (maxTrackLen < maxTrackLenTemp)
                maxTrackLen = maxTrackLenTemp;
            if (maxTrackID < track->info.track_id)
                maxTrackID = track->info.track_id;
        }
    }

    version = maxTrackLen < (sx_int64)UINT32_MAX ? 0 : 1;
    if (version == 1) 
        bs_put_be32(bs, 120);
    else 
        bs_put_be32(bs, 108); // size
    bs_put_tag(bs, (sx_uint8*)"mvhd");
    bs_put_byte(bs, version);
    bs_put_be24(bs, 0); // flags
    if (version == 1) {
        bs_put_be64(bs, ctx->time);
        bs_put_be64(bs, ctx->time);
    } 
    else {
        bs_put_be32(bs, (sx_uint32)ctx->time); // creation time
        bs_put_be32(bs, (sx_uint32)ctx->time); // modification time
    }
    bs_put_be32(bs, ctx->timescale); // timescale
    if (version == 1) 
        bs_put_be64(bs, maxTrackLen);
    else 
        bs_put_be32(bs, (sx_uint32)maxTrackLen); // duration of longest track

    bs_put_be32(bs, 0x00010000); // rate (typically 1.0)
    bs_put_be16(bs, 0x0100); // volume (typically 1.0)
    bs_put_be16(bs, 0); // reserved = 0
    bs_put_be32(bs, 0); // reserved = 0
    bs_put_be32(bs, 0); // reserved = 0

    // unity matrix = {0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000}
    bs_put_be32(bs, 0x00010000);
    bs_put_be32(bs, 0x0);
    bs_put_be32(bs, 0x0);
    bs_put_be32(bs, 0x0);
    bs_put_be32(bs, 0x00010000);
    bs_put_be32(bs, 0x0);
    bs_put_be32(bs, 0x0);
    bs_put_be32(bs, 0x0);
    bs_put_be32(bs, 0x40000000);

    bs_put_be32(bs, 0); // mov: preview time, mp4: pre-defined = 0
    bs_put_be32(bs, 0); // mov: preview duration, mp4: pre-defined = 0
    bs_put_be32(bs, 0); // mov: poster time, mp4: pre-defined = 0
    bs_put_be32(bs, 0); // mov: selection time, mp4: pre-defined = 0
    bs_put_be32(bs, 0); // mov: selection duration, mp4: pre-defined = 0
    bs_put_be32(bs, 0); // mov: current time, mp4: pre-defined = 0
    bs_put_be32(bs, maxTrackID+1); // next track id
    return 0x6c;
}

static sx_int32 mp4_write_moov_tag(byte_strm_t *bs, mp4_context_t *ctx)
{
    sx_int32 i;
	sx_int64 atime = 0, vtime = 0;
    offset_t pos;
	mp4_track_t *trk, *atrk = NULL, *vtrk = NULL;

	pos = bs_wtell(bs);
    bs_put_be32(bs, 0); /* size placeholder*/
    bs_put_tag(bs, (sx_uint8*)"moov");
    ctx->timescale = MP4_TIMESCALE;

    for (i=0; i<ctx->nb_tracks; i++) {
		trk = ctx->tracks[i];
        if (trk->wr_entries <= 0) 
            continue;

		// set track time
        trk->time = ctx->time;

		// get initial time stamp
		if (trk->info.type == MP4_TRACK_AUDIO) {
			double temp = (double)trk->init_dts * ctx->dts_factor + 0.5; // convert to 90k
			atime = (sx_int64)temp;
			atrk = trk;
#ifdef DEBUG_AV_SYNC
			mp4_log(ctx->log, "atrk %d, %d\n", (int)trk->accu_cnt, (int)(trk->curr_dts - trk->init_dts));
#endif
		}
		else if (trk->info.type == MP4_TRACK_VIDEO) {
			double temp = (double)trk->init_dts * ctx->dts_factor + 0.5; // convert to 90k
			vtime = (sx_int64)temp;
			vtrk = trk;
#ifdef DEBUG_AV_SYNC
			mp4_log(ctx->log, "vtrk %d, %d\n", (int)trk->accu_cnt, (int)(trk->curr_dts - trk->init_dts));
#endif
		}
    }

    if (atrk != NULL && vtrk != NULL) {
	    atrk->pts_offset = atime - vtime;
#ifdef DEBUG_AV_SYNC
	    mp4_log(ctx->log, "a offset = %d\n", (int)atrk->pts_offset);
#endif
    }

    mp4_write_mvhd_tag(bs, ctx);
    for (i=0; i<ctx->nb_tracks; i++) {
        if (ctx->tracks[i]->wr_entries > 0) {
            mp4_write_trak_tag(bs, ctx->tracks[i]);
        }
    }

    return (sx_int32)update_size(bs, pos);
}

#ifndef FRAGMENT_MODE
static sx_int32 mp4_write_mdat_tag(byte_strm_t *bs, mp4_context_t *ctx)
{
    bs_put_be32(bs, 8); // placeholder for extended size field (64 bit)
    bs_put_tag(bs, (sx_uint8*)"wide");

    ctx->mdat_pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size placeholder
    bs_put_tag(bs, (sx_uint8*)"mdat");
    return 0;
}
#endif

static void mp4_write_ftyp_tag(byte_strm_t *bs, mp4_context_t *ctx)
{
    bs_put_be32(bs, 0x14); // size 
    bs_put_tag(bs, (sx_uint8*)"ftyp");

    // major brand
    if (ctx->mode == MODE_3GP)
        bs_put_tag(bs, (sx_uint8*)"3gp4");
    else if (ctx->mode == MODE_MP4)
        bs_put_tag(bs, (sx_uint8*)"isom");
    else
        bs_put_tag(bs, (sx_uint8*)"qt  ");

    // minor version
    bs_put_be32(bs, 0x200 );

    // compatible brands
    if (ctx->mode == MODE_3GP)
        bs_put_tag(bs, (sx_uint8*)"3gp4");
    else if (ctx->mode == MODE_MP4)
        bs_put_tag(bs, (sx_uint8*)"mp41");
    else
        bs_put_tag(bs, (sx_uint8*)"qt  ");
}

static void mp4_write_smff_tag(byte_strm_t *bs, mp4_context_t *ctx)
{
    bs_put_be32(bs, 20); // size, 20 bytes total
    bs_put_tag(bs, (sx_uint8*)"free");

    ctx->smff_pos = bs_wtell(bs);
    bs_put_be32(bs, 12); // size, total 12 bytes of smff
    bs_put_tag(bs, (sx_uint8*)"smff");

    bs_put_byte(bs, ctx->version); // version
    bs_put_be24(bs, 0); // flags
}

sx_int32 mp4_write_free_tag(mp4_context_t *ctx)
{
    byte_strm_t *bs = ctx->stream;
    offset_t pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"free");
    return (sx_int32)update_size(bs, pos);
}

#ifdef FRAGMENT_MODE
static sx_int32 mp4_mdat_open(mp4_context_t *ctx)
{
    byte_strm_t *bs = ctx->stream;
    int i;

    if (!CHK_FLAGS(ctx->flags, MP4_FLAG_INITIAL_MDAT)) {
        // 64-bit size placeholder
        bs_put_be32(bs, 8); 
        bs_put_tag(bs, (sx_uint8*)"wide");
        // mdat pos
        ctx->mdat_pos = bs_wtell(bs);
        ctx->init_mdat_pos = ctx->mdat_pos;
        // size placeholder
        bs_put_be32(bs, 0); 
        bs_put_tag(bs, (sx_uint8*)"mdat");
        SET_FLAGS(ctx->flags, MP4_FLAG_INITIAL_MDAT);
    }
    else {
        ctx->mdat_pos = bs_wtell(bs);
        bs_put_be32(bs, 0); // 32-bit size placeholder
        bs_put_tag(bs, (sx_uint8*)"mdat");
    }

    ctx->mdat_size = 0;
    SET_FLAGS(ctx->flags, MP4_FLAG_MDAT_OPEN);

    // set base data offset for all tracks
    for (i=0; i<ctx->nb_tracks; i++)
        ctx->tracks[i]->base_data_offset = ctx->mdat_pos + 8;

    return 0;
}

static sx_int32 mp4_mdat_close(mp4_context_t *ctx)
{
    byte_strm_t *bs = ctx->stream;
    sx_uint64 limit = (sx_int64)UINT32_MAX - 8;
    offset_t cur_pos = bs_wtell(bs);
    assert(ctx->mdat_size <= limit);

    // write size of mdat
    bs_wseek(bs, ctx->mdat_pos, SEEK_SET);
    bs_put_be32(bs, (sx_uint32)ctx->mdat_size + 8);

    bs_wseek(bs, cur_pos, SEEK_SET);
    CLR_FLAGS(ctx->flags, MP4_FLAG_MDAT_OPEN);
    return 0;
}

// beginning of Stretch box wrapper
static sx_int32 mp4_write_sx_open(mp4_context_t *ctx)
{
    byte_strm_t *bs = ctx->stream;
    ctx->sx_pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"free");
    // it's possible to put some Stretch identifier here
    return 0;
}

// end of Stretch box wrapper
static sx_int32 mp4_write_sx_close(mp4_context_t *ctx)
{
    byte_strm_t *bs = ctx->stream;
    offset_t cur_pos = bs_wtell(bs);
    sx_int64 size = cur_pos - ctx->sx_pos;
    assert(size <= UINT32_MAX - 8);

    // write size of sx
    bs_wseek(bs, ctx->sx_pos, SEEK_SET);
    bs_put_be32(bs, (sx_uint32)size);
    bs_wseek(bs, cur_pos, SEEK_SET);
    return 0;
}

// write moov box in fragment mode
// this function should be called only once
// when put_packet() is called the first time
static sx_int32 mp4_write_moov_fragment_sx(mp4_context_t *ctx)
{
    sx_int32 i;
    offset_t pos;
    byte_strm_t *bs = ctx->stream;
    mp4_write_sx_open(ctx); // sx wrapper

    pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"moov");
    ctx->timescale = MP4_TIMESCALE;

    for (i=0; i<ctx->nb_tracks; i++) {
        ctx->tracks[i]->time = ctx->time;
    }

    // mvhd
    mp4_write_mvhd_tag(bs, ctx);

    // trak
    for (i=0; i<ctx->nb_tracks; i++) {
        mp4_write_trak_tag(bs, ctx->tracks[i]);
    }

    // mvex
    mp4_write_mvex_sx(ctx);

    update_size(bs, pos);
    return mp4_write_sx_close(ctx); // sx wrapper
}

// write movie extension box
static sx_int32 mp4_write_mvex_sx(mp4_context_t *ctx)
{
    sx_int32 i;
    offset_t pos;
    byte_strm_t *bs = ctx->stream;
    pos = bs_wtell(bs);

    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"mvex");

    // mehd (omitted)

    // trex
    for (i=0; i<ctx->nb_tracks; i++) {
        mp4_write_trex_sx(bs, ctx->tracks[i]);
    }

    return (sx_int32)update_size(bs, pos);
}

// write track extension box
// set default values for movie fragments
static sx_int32 mp4_write_trex_sx(byte_strm_t *bs, mp4_track_t *track)
{
    offset_t pos;

    pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"trex");
    bs_put_byte(bs, 0); // version
    bs_put_be24(bs, 0); // flags 

    bs_put_be32(bs, track->info.track_id); // track id
    bs_put_be32(bs, track->default_desc_idx); // default_sample_description_index
    bs_put_be32(bs, track->info.default_duration); // default_duration
    bs_put_be32(bs, track->info.default_size); // default_size
    bs_put_be32(bs, track->default_flags); // default_flags

    return (sx_int32)update_size(bs, pos);
}

// ==============
// movie fragment
// ==============

// write movie fragment
static sx_int32 mp4_write_moof_sx(mp4_context_t *ctx)
{
    sx_int32 i;
    offset_t pos;
    byte_strm_t *bs = ctx->stream;
    mp4_write_sx_open(ctx); // sx wrapper

    pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"moof");

    // mfhd
    mp4_write_mfhd_sx(ctx);

    // traf
    for (i=0; i<ctx->nb_tracks; i++) {
        mp4_write_traf_sx(bs, ctx->tracks[i]);
    }

    update_size(bs, pos);
    return mp4_write_sx_close(ctx); // sx wrapper
}

// write movie fragment header
static sx_int32 mp4_write_mfhd_sx(mp4_context_t *ctx)
{
    offset_t pos;
    byte_strm_t *bs = ctx->stream;
    pos = bs_wtell(bs);

    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"mfhd");
    bs_put_byte(bs, 0); // version
    bs_put_be24(bs, 0); // flags 
    bs_put_be32(bs, ctx->seq_number++); // seq number

    return (sx_int32)update_size(bs, pos);
}

// write track fragment
static sx_int32 mp4_write_traf_sx(byte_strm_t *bs, mp4_track_t *track)
{
    offset_t pos;

    pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"traf");

    // tfhd
    mp4_write_tfhd_sx(bs, track);

    // trun (we support only one trun per traf)
    mp4_write_trun_sx(bs, track);

    // some clean-up of this track
    track->fr_entries = 0;

    return (sx_int32)update_size(bs, pos);
}

// write track fragment header
static sx_int32 mp4_write_tfhd_sx(byte_strm_t *bs, mp4_track_t *track)
{
    offset_t pos;
    sx_uint32 tf_flags = 0x0;

    pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"tfhd");
    bs_put_byte(bs, 0); // version

    // update flags
    tf_flags |= TF_FLAGS_BASE_DATA_OFFSET; // always use base_data_offset
    if (track->default_desc_idx != track->stsd_idx_wr + 1) {
        SET_FLAGS(tf_flags, TF_FLAGS_SAMPLE_DESC_INDEX);
        track->default_desc_idx = track->stsd_idx_wr + 1;
    }
    bs_put_be24(bs, tf_flags); // flags 

    bs_put_be32(bs, track->info.track_id); // track id
    // optional fields
    if (CHK_FLAGS(tf_flags, TF_FLAGS_BASE_DATA_OFFSET))
        bs_put_be64(bs, track->base_data_offset);
    if (CHK_FLAGS(tf_flags, TF_FLAGS_SAMPLE_DESC_INDEX))
        bs_put_be32(bs, track->stsd_idx_wr + 1);
    if (CHK_FLAGS(tf_flags, TF_FLAGS_DEFAULT_SAMPLE_DURATION))
        bs_put_be32(bs, track->info.default_duration);
    if (CHK_FLAGS(tf_flags, TF_FLAGS_DEFAULT_SAMPLE_SIZE))
        bs_put_be32(bs, track->info.default_size);
    if (CHK_FLAGS(tf_flags, TF_FLAGS_DEFAULT_SAMPLE_FLAGS))
        bs_put_be32(bs, track->default_flags);

    return (sx_int32)update_size(bs, pos);
}

// write track fragment run
static sx_int32 mp4_write_trun_sx(byte_strm_t *bs, mp4_track_t *track)
{
    offset_t pos;
    sx_uint32 tf_flags = 0x0;
    sx_int32 i, count = track->fr_entries;
    sx_int64 dts_offset, dts_diff;

    pos = bs_wtell(bs);
    bs_put_be32(bs, 0); // size
    bs_put_tag(bs, (sx_uint8*)"trun");
    bs_put_byte(bs, 0); // version

    // update flags
    if (track->fr_entries > 0) {
        // sample offset flag
        SET_FLAGS(tf_flags, TF_FLAGS_SAMPLE_OFFSET);
        // dts offset flag
        dts_offset = track->samples_fr[0].dts;
        SET_FLAGS(tf_flags, TF_FLAGS_DTS_OFFSET);
        // sample size flag
        //if (track->info.sample_size == 0)
            SET_FLAGS(tf_flags, TF_FLAGS_SAMPLE_SIZE);
        SET_FLAGS(tf_flags, TF_FLAGS_SAMPLE_DURATION);
        if (track->info.type == MP4_TRACK_VIDEO) {
            SET_FLAGS(tf_flags, TF_FLAGS_SAMPLE_FLAGS);
            SET_FLAGS(tf_flags, TF_FLAGS_SAMPLE_CAMERA_INFO);
            if (CHK_FLAGS(track->flags, TRACK_FLAG_NEW_CONFIG)) {
                SET_FLAGS(tf_flags, TF_FLAGS_SAMPLE_CONFIG_DATA);
                CLR_FLAGS(track->flags, TRACK_FLAG_NEW_CONFIG);
            }
        }
    }
    bs_put_be24(bs, tf_flags); // flags 
    bs_put_be32(bs, count); // sample count

    // optional fields
    if (CHK_FLAGS(tf_flags, TF_FLAGS_DTS_OFFSET))
        bs_put_be64(bs, dts_offset);
    if (CHK_FLAGS(tf_flags, TF_FLAGS_BASE_DATA_OFFSET))
        bs_put_be32(bs, 0);
    if (CHK_FLAGS(tf_flags, TF_FLAGS_FIRST_SAMPLE_FLAGS))
        bs_put_be32(bs, 0); // first_sample_flags
    if (CHK_FLAGS(tf_flags, TF_FLAGS_SAMPLE_CONFIG_DATA)) {
        stsd_data_t *stsd = &track->stsd_data[track->stsd_idx_wr];
        if (track->info.codec_id == CODEC_ID_H264) {
            bs_put_be32(bs, stsd->sps_len); // sps size
            bs_put_buffer(bs, stsd->sps_data, stsd->sps_len); // sps data
            bs_put_be32(bs, stsd->pps_len); // pps size
            bs_put_buffer(bs, stsd->pps_data, stsd->pps_len); // pps data
        }
        else if (track->info.codec_id == CODEC_ID_MPEG4) {
            bs_put_be32(bs, stsd->sps_len); // vol length
            bs_put_buffer(bs, stsd->sps_data, stsd->sps_len); // vol data
        }
    }

    // sample info
    for (i=0; i<count; i++) {
        // sample offset diff
        if (CHK_FLAGS(tf_flags, TF_FLAGS_SAMPLE_OFFSET)) {
            sx_int64 offset = track->samples_fr[i].pos - track->base_data_offset;
            bs_put_be32(bs, (sx_int32)offset);
        }
        // sample dts diff (not duration)
        if (CHK_FLAGS(tf_flags, TF_FLAGS_SAMPLE_DURATION)) {
            dts_diff = track->samples_fr[i].dts - dts_offset;
            bs_put_be32(bs, (sx_int32)dts_diff);
        }
        // sample size
        if (CHK_FLAGS(tf_flags, TF_FLAGS_SAMPLE_SIZE))
            bs_put_be32(bs, track->samples_fr[i].size);
        // sample flags
        if (CHK_FLAGS(tf_flags, TF_FLAGS_SAMPLE_FLAGS)) {
            if (CHK_FLAGS(track->samples_fr[i].flags, MP4_SAMPLE_KEYFRAME))
                bs_put_be32(bs, 1);
            else
                bs_put_be32(bs, 0);
        }
        // sample cts diff
        if (CHK_FLAGS(tf_flags, TF_FLAGS_SAMPLE_COMP_TIME_OFFSET)) {
            bs_put_be32(bs, 0);
        }
        // sample camera info (video only)
        if (CHK_FLAGS(tf_flags, TF_FLAGS_SAMPLE_CAMERA_INFO)) {
            bs_put_be32(bs, 0);
        }
    }

    return (sx_int32)update_size(bs, pos);
}

// rebuild sample entried for movie header
static void rebuild_entries(mp4_context_t *ctx)
{
    byte_strm_t *bs = ctx->stream;
    mp4_atom_t atom = {0, 0, 0};
    mp4_atom_t moof = {0, 0, 0};
    atom.size = bs_get_size(bs);

    bs_rseek(bs, 0, SEEK_SET);
    while (mp4_find_next_moof(bs, &atom, &moof)) {
        mp4_parse_moof(ctx, &moof);
    }
}

// find next moof box
static sx_int32 mp4_find_next_moof(byte_strm_t *bs, mp4_atom_t *atom, mp4_atom_t *moof)
{
    sx_int64 total_size = 0;
    mp4_atom_t a;

    a.offset = atom->offset;

    while (atom->size > 8) {
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
        } 
        else {
            moof->offset = bs_rtell(bs);
            moof->size = bs_get_be32(bs);
            moof->type = bs_get_le32(bs);
            if (moof->type == MKTAG( 'm', 'o', 'o', 'f' )) {
                atom->offset += a.size + 8;
                atom->size -= a.size + 8;
                return 1;
            }
            else // not a moof box, skip it
                bs_skip(bs, moof->size - 8);
        }

        a.offset += a.size;
        total_size += a.size;
        atom->offset += a.size + 8;
        atom->size -= a.size + 8;
    }

    return 0;
}

sx_int32 mp4_find_moov_sx(byte_strm_t *bs, mp4_atom_t *atom, mp4_atom_t *moov)
{
    sx_int64 total_size = 0;
    mp4_atom_t a;

    a.offset = atom->offset;

    while (atom->size > 8) {
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
        } 
        else {
            moov->offset = bs_rtell(bs);
            moov->size = bs_get_be32(bs);
            moov->type = bs_get_le32(bs);
            if (moov->type == MKTAG( 'm', 'o', 'o', 'v' )) {
                atom->offset += a.size + 8;
                atom->size -= a.size + 8;
                return 1;
            }
            else // not a moof box, skip it
                bs_skip(bs, moov->size - 8);
        }

        a.offset += a.size;
        total_size += a.size;
        atom->offset += a.size + 8;
        atom->size -= a.size + 8;
    }

    return 0;
}

sx_int32 mp4_authorize(mp4_context_t *ctx)
{
    sx_int32 i;
    byte_strm_t *bs = ctx->stream;
    offset_t pos;
    mp4_atom_t atom = {0, 0, 0};
    mp4_atom_t moov = {0, 0, 0};
    sx_uint32 flags = 0;

    SET_FLAGS(ctx->flags, MP4_FLAG_REBUILD);

    // search moov to reconstruct movie header
    pos = bs_get_size(bs);
    atom.size = pos;
    bs_rseek(bs, 0, SEEK_SET);
    if (mp4_find_moov_sx(bs, &atom, &moov)) {
        mp4_parse_moov_sx(ctx, &moov);
    }

    // rebuild entries
    rebuild_entries(ctx);

    // seek to the end of file
    bs_wseek(bs, pos, SEEK_SET);

    // write moov
    bs_put_be32(bs, 0); // size place holder
    bs_put_tag(bs, (sx_uint8*)"moov");
    ctx->timescale = MP4_TIMESCALE;

    // write mvhd
    mp4_write_mvhd_tag(bs, ctx);

    // write tracks
    for (i=0; i<ctx->nb_tracks; i++) {
        if (ctx->tracks[i]->wr_entries > 0) {
            mp4_write_trak_tag(bs, ctx->tracks[i]);
        }
    }

    update_size(bs, pos);

    // update smff flags
    if (ctx->smff_pos > 0) {
        SET_FLAGS(flags, SMFF_FLAG_CLOSING_MOOV);
        SET_FLAGS(flags, SMFF_FLAG_CLOSED);
        bs_wseek(bs, ctx->smff_pos + 9, SEEK_SET);
        bs_put_be24(bs, flags); // flags
        bs_flush_buffer(bs);
    }

	return MP4MUX_ERR_OK;
}

sx_int32 mp4_authorize_close(mp4_context_t *ctx)
{
    sx_int32 err = MP4MUX_ERR_OK;
    byte_strm_t *bs = ctx->stream;
    sx_int32 i, j;

    for (i=0; i<ctx->nb_tracks; i++) {
        mp4_track_t *trk = ctx->tracks[i];
        mp4_freep((void *)&trk->samples);
        mp4_freep((void *)&trk->samples_fr);
        for (j=0; j<(int)trk->stsd_count; j++) {
            mp4_freep((void *)&trk->stsd_data[j].sps_data);
            mp4_freep((void *)&trk->stsd_data[j].pps_data);
        }
        free(trk->stsd_data);
        mp4_freep((void *)&ctx->tracks[i]);
    }

    bs_flush_buffer(bs);
    bs_close(bs);
    mp4_freep((void *)&ctx->stream);
    return err;
}

#endif // #ifdef FRAGMENT_MODE
