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
#include <QVariant>
#include "viewsettings.h"
#include "dvrsystem.h"
#include "player.h"
#include "camera.h"
#include "sdvr.h"
#include "log.h"

extern DVRSystem *g_DVR;

static const char *TN_MONITOR = "monitor";
static const char *AN_NAME = "name";
static const char *AN_ROWS = "rows";
static const char *AN_COLS = "cols";
static const char *AN_DECIMATION = "decimation";
static const char *AN_ENABLERAWAUDIO = "enable_raw_audio";

static const char *TN_SMO = "smo";
static const char *TN_SMOBOARD = "board";
static const char *TN_SMOLEFTMARGIN = "leftmargin";
static const char *TN_SMOTOPMARGIN = "topmargin";
static const char *TN_SMOSPACING = "spacing";

static const char *TN_SHOW = "show";
static const char *AN_DISPLAY = "display";
static const char *AN_CAMID = "camera";
static const char *AN_PLAYERID = "player";



ViewSettings::ViewSettings(const QString &name, int rows, int cols,
                           sdvr_video_res_decimation_e decimation,
                           bool bEnableRawAudio)
    : m_name(name)
{
    m_rows = rows;
    m_cols = cols;
    m_decimation = decimation;
    m_bEnableRawAudio = bEnableRawAudio;

    setSmo(false);
    setSmoLeftMargin(0);   	// Spacing from the left edge
    setSmoTopMargin(0);     // Spacing from the top edge
    setSmoSpacing(0);		// Grid spacing
    setSmoBoardIndex(0);

    m_selectedDisplay = 0;

    int nScreens = displayCount();
    // Initialize all the grids views in a display to no channel attached
    for (int i = 0; i < nScreens; ++i)
    {
        m_displayChannel.append(INVALID_CHAN_HANDLE);
        m_chanIdList.append(chanIdItem_t(SDVR_CHAN_TYPE_NONE,0));
    }
}

QVector<QRect> ViewSettings::layout(int hres, int vres, int leftMargin,
                                    int topMargin, int spacing) const
{
    QVector<QRect> vec;

    int count = displayCount();

    for (int i = 0; i < count; ++i)
        vec.append(QRect(leftMargin + (hres + spacing) * (i % cols()),
                         topMargin  + (vres + spacing) * (i / cols()),
                         hres, vres));

    return vec;
}

QSize ViewSettings::size(int hres, int vres, int leftMargin, int topMargin,
                         int spacing) const
{
    QSize size;

    size.setWidth(hres * cols() + spacing * (cols() - 1) + leftMargin * 2);
    size.setHeight(vres * rows() + spacing * (rows() - 1) + topMargin * 2);

    return size;
}

void ViewSettings::setDisplayChannel(int display, sdvr_chan_handle_t chan)
{
    if (0 <= display && display < m_displayChannel.size())
        m_displayChannel[display] = chan;
}

sdvr_chan_handle_t ViewSettings::displayChannel(int display)
{
    if (0 <= display && display < m_displayChannel.size())
        return m_displayChannel[display];

    return INVALID_CHAN_HANDLE;
}

//bool ViewSettings::displayEnabled(int display)
//{
//    if (0 <= display && display < m_displayChannel.size())
//        return m_displayChannel[display] != NULL;
//
//    return false;
//}

QVector<int> ViewSettings::displaysPlayingChannel(sdvr_chan_handle_t chan) const

{
	QVector<int> displayList;

    int count = displayCount();
    for (int display = 0; display < count; ++display)
        if (m_displayChannel[display] == chan)
            displayList.append(display);

    return displayList;
}

bool ViewSettings::playingChannel(sdvr_chan_handle_t chan) const
{
    if (chan !=  INVALID_CHAN_HANDLE)
    {
        int count = displayCount();
        for (int display = 0; display < count; ++display)
            if (m_displayChannel[display] == chan)
                return true;
    }

    return false;
}

void ViewSettings::setSelectedDisplay(int display)
{
    if (0 <= display && display < displayCount())
        m_selectedDisplay = display;
}

sdvr_chan_handle_t ViewSettings::selectedDisplayChannel()
{
    if (0 <= m_selectedDisplay && m_selectedDisplay < displayCount())
        return m_displayChannel[m_selectedDisplay];

    return 0;
}

QDomElement ViewSettings::toElement(QDomDocument &doc) const
{
	QDomElement vsElem = doc.createElement(TN_MONITOR);
	vsElem.setAttribute(AN_NAME, name());
    vsElem.setAttribute(AN_ROWS, rows());
    vsElem.setAttribute(AN_COLS, cols());
    vsElem.setAttribute(AN_DECIMATION,
        sdvr::decimationToString(decimation()));

    vsElem.setAttribute(AN_ENABLERAWAUDIO,QVariant(isEnableRawAudio()).toString());

    if (isSmo())
    {
        QDomElement smoElem = doc.createElement(TN_SMO);

        QDomElement smoBoardElem = doc.createElement(TN_SMOBOARD);
        smoBoardElem.appendChild(doc.createTextNode(QString::number(smoBoardIndex())));
        smoElem.appendChild(smoBoardElem);

        QDomElement smoLeftMarginElem = doc.createElement(TN_SMOLEFTMARGIN);
        smoLeftMarginElem.appendChild(doc.createTextNode(QString::number(smoLeftMargin())));
        smoElem.appendChild(smoLeftMarginElem);

        QDomElement smoTopMarginElem = doc.createElement(TN_SMOTOPMARGIN);
        smoTopMarginElem.appendChild(doc.createTextNode(QString::number(smoTopMargin())));
        smoElem.appendChild(smoTopMarginElem);

        QDomElement smoSpacingElem = doc.createElement(TN_SMOSPACING);
        smoSpacingElem.appendChild(doc.createTextNode(QString::number(smoSpacing())));
        smoElem.appendChild(smoSpacingElem);

        vsElem.appendChild(smoElem);
    }

    int count = displayCount();
    for (int screen = 0; screen < count; ++screen)
    {
    	
        if (m_displayChannel[screen] != INVALID_CHAN_HANDLE)
		{
			QDomElement playElem = doc.createElement(TN_SHOW);
			playElem.setAttribute(AN_DISPLAY, screen);
            if (sdvr_get_chan_type(m_displayChannel[screen]) == SDVR_CHAN_TYPE_DECODER)
            {
                CPlayer *player = g_DVR->findDecoderByHandle(m_displayChannel[screen]);
			    playElem.setAttribute(AN_PLAYERID, player->getId());
            }
            else
            {
                Camera *cam;
                cam = g_DVR->findCameraByHandle(m_displayChannel[screen]);
			    playElem.setAttribute(AN_CAMID, cam->id());
            }

			vsElem.appendChild(playElem);
		}
    }

    return vsElem;
}

ViewSettings *ViewSettings::fromElement(QDomElement vsElem)
{
	bool ok;

    if (vsElem.tagName() != TN_MONITOR)
    {
        Log::error(QString("'%1' element expected").arg(TN_MONITOR));
        return NULL;
    }

    QString name = vsElem.attribute(AN_NAME);
    if (name.isEmpty())
    {
        Log::error("Missing or empty view settings name");
        return NULL;
    }

    int rows = vsElem.attribute(AN_ROWS, "0").toInt();
    if (rows < 1 || rows > 8)
    {
        Log::error("Bad number of display rows");
        return NULL;
    }

    int cols = vsElem.attribute(AN_COLS, "0").toInt();
    if (cols < 1 || cols > 8)
    {
        Log::error("Bad number of display columns");
        return NULL;
    }

    sdvr_video_res_decimation_e decimation =
        sdvr::stringToDecimation(vsElem.attribute(AN_DECIMATION));
    if (decimation == SDVR_VIDEO_RES_DECIMATION_NONE)
    {
        Log::error("Bad or missing resolution decimation in view settings");
        return NULL;
    }

    bool bEnableRawAudio = QVariant(vsElem.attribute(AN_ENABLERAWAUDIO)).toBool();

    ViewSettings *vs = new ViewSettings(name, rows, cols, decimation, bEnableRawAudio);

    QDomElement smoElem = vsElem.firstChildElement(TN_SMO);
    if (!smoElem.isNull())
    {
        vs->setSmo(true);

        QDomElement smoBoardElem = smoElem.firstChildElement(TN_SMOBOARD);
        if (!smoBoardElem.isNull())
            vs->setSmoBoardIndex(smoBoardElem.text().toUInt());

        QDomElement smoLeftMarginElem = smoElem.firstChildElement(TN_SMOLEFTMARGIN);
        if (!smoLeftMarginElem.isNull())
            vs->setSmoLeftMargin(smoLeftMarginElem.text().toUInt());

        QDomElement smoTopMarginElem = smoElem.firstChildElement(TN_SMOTOPMARGIN);
        if (!smoTopMarginElem.isNull())
            vs->setSmoTopMargin(smoTopMarginElem.text().toUInt());

        QDomElement smoSpacingElem = smoElem.firstChildElement(TN_SMOSPACING);
        if (!smoSpacingElem.isNull())
            vs->setSmoSpacing(smoSpacingElem.text().toUInt());
    }

    QDomElement e = vsElem.firstChildElement(TN_SHOW);
	while (!e.isNull())
	{
        int disp = e.attribute(AN_DISPLAY).toInt(&ok);
        if (ok && 0 <= disp && disp < vs->displayCount())
        {
            int Id = e.attribute(AN_CAMID).toInt(&ok);
            if (ok)
            {
                // Cameras are not open yet. Save the id and type and later
                // set channels by calling setChannels()
                vs->m_chanIdList[disp].type = SDVR_CHAN_TYPE_ENCODER;
                vs->m_chanIdList[disp].id = Id;
            } else
            {
                Id = e.attribute(AN_PLAYERID).toInt(&ok);
                if (ok)
                {
                    // Players are not open yet. Save the id and type and later
                    // set channels by calling setChannels()
                    vs->m_chanIdList[disp].type = SDVR_CHAN_TYPE_DECODER;
                    vs->m_chanIdList[disp].id = Id;
                }
            }
        }

		e = e.nextSiblingElement(TN_SHOW);
	}

    return vs;
}

void ViewSettings::setChannels()
{
    int count = displayCount();
    int id;
    for (int disp = 0; disp < count; ++disp)
    {
        if ((id = m_chanIdList[disp].id) != 0)
        {
            if (m_chanIdList[disp].type == SDVR_CHAN_TYPE_DECODER)
            {
                CPlayer *player = g_DVR->findDecoderById(m_chanIdList[disp].id);
                if (player != NULL && player->isChannelOpen())
                    setDisplayChannel(disp, player->handle());

            }
            else
            {
                Camera *cam = g_DVR->findCameraById(m_chanIdList[disp].id);
                if (cam != NULL && cam->isChannelOpen())
                    setDisplayChannel(disp, cam->handle());
            }
        }
    }
}
