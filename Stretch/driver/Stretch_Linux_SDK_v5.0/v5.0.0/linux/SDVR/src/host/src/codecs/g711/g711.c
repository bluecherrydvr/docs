/************************************************************************
Copyright (c) 2007 Stretch, Inc. All rights reserved.  Stretch products
are protected under numerous U.S. and foreign patents, maskwork rights,
copyrights and other intellectual property laws.

This source code and the related tools, software code and documentation,
and your use thereof, are subject to and governed by the terms and
conditions of the applicable Stretch IDE or SDK and RDK License Agreement
(either as agreed by you or found at www.stretchinc.com).  By using
these items, you indicate your acceptance of such terms and conditions
between you and Stretch, Inc.  In the event that you do not agree with
such terms and conditions, you may not use any of these items and must
immediately destroy any copies you have made.
************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "g711.h"

/*************************************************************************
    A g711 codec handle.  It can be used for encoding or decoding.
*************************************************************************/
struct g711_struct {
    g711_config_t config;
};

/*************************************************************************
    A-law encoding rule according ITU-T Rec. G.711.
*************************************************************************/
static short g711_linear_to_alaw(short in)
{
    short out, ex;

    out = in < 0 ? (~in) >> 4 : in >> 4;

    if (out > 15) {
        ex = 1;
        while (out > 16 + 15) {
            out >>= 1;
            ex++;
        }
        out -= 16;
        out += ex << 4;
    }

    if (in >= 0) {
        out |= (0x0080);
    }

    return out ^ (0x0055);
}

/*************************************************************************
    ALaw decoding rule according ITU-T Rec. G.711.
*************************************************************************/
static short g711_alaw_to_linear(short in)
{
    short tmp, mant, ex;

    tmp = in ^ 0x0055;
    tmp &= (0x007F);
    ex = tmp >> 4;
    mant = tmp & (0x000F);
    if (ex > 0) {
        mant = mant + 16;
    }

    mant = (mant << 4) + (0x0008);
    if (ex > 1) {
      mant = mant << (ex - 1);
    }

    return in > 127 ? mant : -mant;
}

/*************************************************************************
    Mu law encoding rule according ITU-T Rec. G.711.
*************************************************************************/
static short g711_linear_to_ulaw(short in)
{
    short i, out, absno, segno, low_nibble, high_nibble;

    absno = in < 0 ? ((~in) >> 2) + 33 : ((in) >> 2) + 33;

    if (absno > (0x1FFF)) {
      absno = (0x1FFF);
    }

    i = absno >> 6;
    segno = 1;
    while (i != 0)
    {
        segno++;
        i >>= 1;
    }

    high_nibble = (0x0008) - segno;

    low_nibble = (absno >> segno) & (0x000F);
    low_nibble = (0x000F) - low_nibble;

    out = (high_nibble << 4) | low_nibble;

    /* Add sign bit */
    return in >= 0 ? (out | 0x0080) : out;
}

/*************************************************************************
    Mu Law decoding rule according ITU-T Rec. G.711.
*************************************************************************/
static short g711_ulaw_to_linear(short in)
{
    short out, segment, mantissa, exponent, sign, step;

    sign = in < 0x0080 ? -1 : 1;

    mantissa = ~in;
    exponent = (mantissa >> 4) & (0x0007);
    segment = exponent + 1;
    mantissa = mantissa & (0x000F);

    step = (4) << segment;

    out = sign * (((0x0080) << exponent) + step * mantissa + step / 2 - 4 * 33);

    return out;
}

/*************************************************************************
    Initialize and return a g711 codec instance in "*g711".  The 
    instance can be used for encoding as well as decoding as long as
    the configuration parameters are the same.
*************************************************************************/
g711_status_e g711_open(g711_config_t *config, g711_t **g711)
{
    g711_t *hdl;
    hdl = (g711_t *) malloc(sizeof(g711_t)); 
    memcpy(&hdl->config, config, sizeof(g711_config_t));
    *g711 = hdl;

    return G711_OK;
}

/*************************************************************************
    Encode "num" input samples in "ibuf" and store the output in *obuf.
    "ibuf" must hold "num" of shorts and "obuf" "num" of bytes.
    In case of multi-channel data, the "ibuf" should contain "n * num"
    number of interleaved samples where "n" is the number of channels.
    "obuf" is assumed to be sufficient to hold the encoded data.
    If "g711" is configured to have multiple channels and encode
    only one channel, the encoding will only be performed on the
    specific channel and the output is stored in "obuf" contiguously.
*************************************************************************/
g711_status_e g711_encode(g711_t *g711, unsigned num, short *ibuf, unsigned char *obuf)
{
    int i, n, incr, init, size;

    incr = g711->config.channel_select < g711->config.num_channels ? g711->config.num_channels : 1;
    init = g711->config.channel_select;
    size = num * g711->config.num_channels;

    switch (g711->config.law) {
    case G711_ALAW:
        for (i = 0, n = init; n < size; n += incr, i++) {
            obuf[i] = (unsigned char) g711_linear_to_alaw(ibuf[n]);
        }
        break;
    case G711_ULAW:
        for (i = 0, n = init; n < size; n += incr, i++) {
            obuf[i] = (unsigned char) g711_linear_to_ulaw(ibuf[n]);
        }
        break;
    default:
        assert(0);
    }

    return G711_OK;
}
g711_status_e g711_encode_stereo(g711_t *g711, unsigned num, short *ibuf, unsigned char *obuf)
{
    int i, n, incr, init, size;

    incr = g711->config.channel_select < g711->config.num_channels ? g711->config.num_channels : 1;
    init = g711->config.channel_select;
    size = num * g711->config.num_channels;

    switch (g711->config.law) {
    case G711_ALAW:
        for (i = 0, n = init; n < size; n += incr, i += 2) {
            obuf[i] = (unsigned char) g711_linear_to_alaw(ibuf[n]);
            obuf[i+1] = obuf[i];

        }
        break;
    case G711_ULAW:
        for (i = 0, n = init; n < size; n += incr, i += 2) {
            obuf[i] = (unsigned char) g711_linear_to_ulaw(ibuf[n]);
            obuf[i+1] = obuf[i];

        }
        break;
    default:
        assert(0);
    }

    return G711_OK;
}
/*************************************************************************
    Decode "num" input samples in "ibuf" and store the output in *obuf.
    "ibuf" must hold "num" of shorts and "obuf" "num" of bytes.
    In case of multi-channel data, the "ibuf" should contain "n * num"
    number of interleaved samples where "n" is the number of channels.
    "obuf" is assumed to be sufficient to hold the encoded data.
    If "g711" is configured to have multiple channels and decode
    only one channel, the decoding will only be performed on the
    specific channel and the output is stored in "obuf" contiguously.
*************************************************************************/
g711_status_e g711_decode(g711_t *g711, unsigned num, short *ibuf, short *obuf)
{
    int i, n, incr, init, size;

    incr = g711->config.channel_select < g711->config.num_channels ? g711->config.num_channels : 1;
    init = g711->config.channel_select;
    size = g711->config.channel_select < g711->config.num_channels ? num : num * g711->config.num_channels;

    switch (g711->config.law) {
    case G711_ALAW:
        for (i = 0, n = init; n < size; n += incr, i++) {
            obuf[i] = g711_alaw_to_linear(ibuf[n]);
        }
        break;
    case G711_ULAW:
        for (i = 0, n = init; n < size; n += incr, i++) {
            obuf[i] = g711_ulaw_to_linear(ibuf[n]);
        }
        break;
    default:
        assert(0);
    }

    return G711_OK;
    return G711_OK;
}

g711_status_e g711_decode1(g711_t *g711, unsigned num, unsigned char *ibuf, short *obuf)
{
    int i, n, incr, init, size;

    incr = g711->config.channel_select < g711->config.num_channels ? g711->config.num_channels : 1;
    init = g711->config.channel_select;
    size = g711->config.channel_select < g711->config.num_channels ? num : num * g711->config.num_channels;

    switch (g711->config.law) {
    case G711_ALAW:
        for (i = 0, n = init; n < size; n += incr, i++) {
            obuf[i] = g711_alaw_to_linear(ibuf[n]);
        }
        break;
    case G711_ULAW:
        for (i = 0, n = init; n < size; n += incr, i++) {
            obuf[i] = g711_ulaw_to_linear(ibuf[n]);
        }
        break;
    default:
        assert(0);
    }

    return G711_OK;
    return G711_OK;
}

/*************************************************************************
    Free the g711 codec instance
*************************************************************************/
g711_status_e g711_close(g711_t *g711)
{
    free(g711);
    return G711_OK;
}
