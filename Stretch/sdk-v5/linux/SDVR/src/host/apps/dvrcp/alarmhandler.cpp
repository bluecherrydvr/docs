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

#include "alarmhandler.h"
#include "dvrsystem.h"
#include "camera.h"
#include "log.h"


AlarmHandler::AlarmHandler(DVRSystem *system)
{
    m_system = system;
}

void AlarmHandler::registerMessage(sdvr_chan_handle_t handle,
        sdvr_video_alarm_e alarm_type, sx_uint32 data)
{
    AlarmMessage msg;
    msg.handle = handle;
    msg.alarm_type = alarm_type;
    msg.data = data;

    m_messageListMutex.lock();
    m_messageList.append(msg);
    m_messageListMutex.unlock();
}

void AlarmHandler::processMessages()
{
    QList<AlarmMessage> messages;

    m_messageListMutex.lock();
    if (!m_messageList.empty())
    {
        messages = m_messageList;
        m_messageList.clear();
    }
    m_messageListMutex.unlock();

    QList<AlarmMessage>::iterator i;
    for (i = messages.begin(); i != messages.end(); ++i)
    {
        Camera *cam = m_system->findCameraByHandle((*i).handle);
        if (cam != NULL)
        {
            switch ((*i).alarm_type)
            {
            case SDVR_VIDEO_ALARM_MOTION:
                if (!cam->isMDTriggered())
                {
                    sx_uint8 motion_val_1;
                    sx_uint8 motion_val_2;
                    sx_uint8 motion_val_3;
                    sx_uint8 motion_val_4;

                    sdvr_get_alarm_motion_value((*i).data,1, &motion_val_1);
                    sdvr_get_alarm_motion_value((*i).data,2, &motion_val_2);
                    sdvr_get_alarm_motion_value((*i).data,3, &motion_val_3);
                    sdvr_get_alarm_motion_value((*i).data,4, &motion_val_4);

                    cam->setMDTriggered(true);
                    Log::alarm(QString("camera '%1': Motion alarm detected. Motion values (%2 , %3, %4, %5)")
                                       .arg(cam->name()).arg(motion_val_1)
                                       .arg(motion_val_2).arg(motion_val_3).arg(motion_val_4));
                }
                break;

            case SDVR_VIDEO_ALARM_BLIND:
                if (!cam->isBDTriggered())
                {
                    cam->setBDTriggered(true);
                    Log::alarm(QString("camera '%1': Blind alarm detected. Blind value (%2)")
                                       .arg(cam->name()).arg((*i).data));
                }
                break;

            case SDVR_VIDEO_ALARM_NIGHT:
                if (!cam->isNDTriggered())
                {
                    cam->setNDTriggered(true);
                    Log::alarm(QString("camera '%1': Night alarm detected. Night value (%2)")
                                       .arg(cam->name()).arg((*i).data));
                }
                break;

            case SDVR_VIDEO_ALARM_LOSS:
                if (!cam->isVLTriggered())
                {
                    cam->setVLTriggered(true);
                    Log::alarm(QString("camera '%1': Video loss detected")
                                       .arg(cam->name()));
                }
                break;
            case SDVR_VIDEO_ALARM_DETECTED:
 //               if (!cam->isVDTriggered())
                {
                    cam->setVDTriggered(true);
                    Log::info(QString("ALARM: camera '%1' is connected.")
                                       .arg(cam->name()));
                }
                break;

            case SDVR_VIDEO_ALARM_NONE:
	        break;

            } //switch ((*i).alarm_type)
        } //if (cam != NULL)
    } // for (i = messages.begin(); i != messages.end(); ++i)
}

void AlarmHandler::reset()
{
    QList<Camera *>::iterator i = m_system->cameraList.begin();
    while (i != m_system->cameraList.end())
    {
        (*i)->setMDTriggered(false);
        (*i)->setBDTriggered(false);
        (*i)->setNDTriggered(false);
        (*i)->setVLTriggered(false);
        (*i)->setVDTriggered(false);
        ++i;
    }
}
