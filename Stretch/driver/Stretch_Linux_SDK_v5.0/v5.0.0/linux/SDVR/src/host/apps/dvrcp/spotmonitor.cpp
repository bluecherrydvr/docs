/****************************************************************************\
*  Copyright C 2007 Stretch, Inc. All rights reserved. Stretch products are  *
*  protected under numerous U.S. and foreign patents, maskwork rights,       *
*  copyrights and other intellectual property laws.                          *
*                                                                            *
*  This source code and the related tools, software code and documentation,  *
*  and your use thereof, are subject to and governed by the terms and        *
*  conditions of the applicable Stretch IDE or SDK and RDK License Agreement *
*  (either as agreed by you or found at www.stretchinc.com). By using these  *
*  items, you indicate your acceptance of such terms and conditions between  *
*  you and Stretch, Inc. In the event that you do not agree with such terms  *
*  and conditions, you may not use any of these items and must immediately   *
*  destroy any copies you have made.                                         *
\****************************************************************************/

#include "spotmonitor.h"
#include "viewsettings.h"
#include "dvrsystem.h"
#include "dvrboard.h"
#include "camera.h"
#include "player.h"
#include "sdvr.h"
#include "sdvr_sdk.h"

extern DVRSystem *g_DVR;


SpotMonitor::SpotMonitor(DVRBoard *board, int port)
{
    m_board = board;
    m_port = port;
    m_view = NULL;
    m_haveAttributes = false;

    for (int i = 0; i < SDVR_MAX_OSD; ++i)
    {
        m_osdEnabled[i] = false;
        m_osdConfig[i].text_len = 0;
        m_osdConfig[i].location_ctrl = SDVR_LOC_TOP_LEFT;
        m_osdConfig[i].top_left_x = m_osdConfig[i].top_left_y = 0;
        m_osdConfig[i].dts_format = SDVR_OSD_DTS_MDY_12H;
        m_osdConfig[i].translucent = 0;
    }
}

sdvr_err_e SpotMonitor::getAttributes(sdvr_smo_attribute_t *attr)
{
    if (!m_haveAttributes)
    {
        sdvr_err_e err;
        err = sdvr_get_smo_attributes(m_board->index(), m_port, &m_attributes);
        if (err != SDVR_ERR_NONE)
            return err;

        m_haveAttributes = true;
    }

    if (attr != NULL)
        *attr = m_attributes;

    return SDVR_ERR_NONE;
}

bool SpotMonitor::setView(ViewSettings *vs)
{
    if (vs == NULL || !vs->isSmo() || vs->smoBoardIndex() != m_board->index())
        return false;

    clear();

    QSize size = sdvr::frameSize(g_DVR->videoStandard(), vs->decimation());
    m_rectVector = vs->layout(size.width(), size.height(),
        vs->smoLeftMargin(), vs->smoTopMargin(), vs->smoSpacing());
    m_view = vs;

    int count = vs->displayCount();
    for (int i = 0; i < count; ++i)
        enableGrid(i,true);

    return true;
}

bool SpotMonitor::enableGrid(int display, bool bEnable)
{
    CPlayer *player;
    Camera *cam;
    bool ret = false;

    if (m_view == NULL)
        return ret;

    if (m_view->displayChannel(display) == INVALID_CHAN_HANDLE)
        return ret;

    switch (sdvr_get_chan_type(m_view->displayChannel(display)))
    {
    case SDVR_CHAN_TYPE_DECODER:
        player = g_DVR->findDecoderByHandle(m_view->displayChannel(display));
        if (player != NULL)
        {
            ret = player->setSmoGrid(
    		    m_rectVector[display].x(), m_rectVector[display].y(),
    		    m_view->decimation(), 0, bEnable) == SDVR_ERR_NONE;
        }
        break;
    case SDVR_CHAN_TYPE_ENCODER:
    case SDVR_CHAN_TYPE_NONE:
        cam = g_DVR->findCameraByHandle(m_view->displayChannel(display));
        if (cam != NULL)
        {
            ret = cam->setSmoGrid(
    		    m_rectVector[display].x(), m_rectVector[display].y(),
    		    m_view->decimation(), 0, bEnable) == SDVR_ERR_NONE;
        }
        break;
    default:
        ret = false;
    }
    return ret;
}


void SpotMonitor::clear()
{
    if (m_view == NULL)
        return;

    int count = m_view->displayCount();
    for (int i = 0; i < count; ++i)
        enableGrid(i,false);

    m_rectVector.clear();
    m_view = NULL;
}
