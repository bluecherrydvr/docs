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

#ifndef VIEWSETTINGS_H
#define VIEWSETTINGS_H

#include <QVector>
#include <QRect>
#include <QSet>
#include <QDomElement>
#include "sdvr_sdk.h"


class Camera;

class ViewSettings
{

public:
    struct chanIdItem_t{
        chanIdItem_t(sdvr_chan_type_e t, int i)
        {
            type = t;
            id = i;
        }
        sdvr_chan_type_e type;
        int id;
    };
    typedef QList<chanIdItem_t> ChanIdList_t;


    ViewSettings(const QString &name, int rows, int cols,
                 sdvr_video_res_decimation_e decimation, bool bEnableRawAudio);

    const QString &name() const  { return m_name; }
    void setName(const QString &name) { m_name = name; }

    int rows() const         { return m_rows; }
    int cols() const         { return m_cols; }
    int displayCount() const { return m_rows * m_cols; }
    sdvr_video_res_decimation_e decimation() const { return m_decimation; }

    bool isEnableRawAudio() const { return m_bEnableRawAudio; }
    void setEnableRawAudio(bool bEnableRawAudio) { m_bEnableRawAudio = bEnableRawAudio; }

    bool isSmo() const { return m_smo; }
    void setSmo(bool enabled) { m_smo = enabled; }

    int smoSpacing() const { return m_smoSpacing; }
    void setSmoSpacing(int spacing) { m_smoSpacing = spacing; }

    int smoLeftMargin() const { return m_smoLeftMargin; }
    void setSmoLeftMargin(int margin) { m_smoLeftMargin = margin; }

    int smoTopMargin() const { return m_smoTopMargin; }
    void setSmoTopMargin(int margin) { m_smoTopMargin = margin; }

    int smoBoardIndex() const { return m_smoBoardIndex; }
    void setSmoBoardIndex(int index) { m_smoBoardIndex = index; }

    QVector<QRect> layout(int hres, int vres, int leftMargin, int topMargin,
        int spacing) const;

    QSize size(int hres, int vres, int leftMargin, int topMargin,
        int spacing) const;

    void setChannels();
    void setDisplayChannel(int display, sdvr_chan_handle_t chan);
    sdvr_chan_handle_t displayChannel(int display);

    int  selectedDisplay()  { return m_selectedDisplay; }
    void setSelectedDisplay(int display);
    sdvr_chan_handle_t selectedDisplayChannel();

    QVector<int> displaysPlayingChannel(sdvr_chan_handle_t chan) const;
    bool playingChannel(sdvr_chan_handle_t chan) const;

  	QDomElement toElement(QDomDocument &doc) const;
    static ViewSettings *fromElement(QDomElement docElem);

private:
    QString m_name;
    sdvr_video_res_decimation_e m_decimation;
    int m_rows, m_cols;
    int m_selectedDisplay;
    bool m_bEnableRawAudio;
    bool m_smo;
    uint m_smoSpacing;
    uint m_smoLeftMargin;
    uint m_smoTopMargin;
    uint m_smoBoardIndex;

    QVector<sdvr_chan_handle_t> m_displayChannel;

    ChanIdList_t m_chanIdList;
};

#endif
