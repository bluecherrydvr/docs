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

#include "statviewdialog.h"
#include "recview.h"

StatViewDialog::StatViewDialog(RecView *statView, QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    m_statView = statView;
    uint info = m_statView->selectedInfo();

    if (info & RecView::REC_STATUS)
    	recStatCheckBox->setCheckState(Qt::Checked);

    if (info & RecView::ENCODED_FPS)
    	encFpsCheckBox->setCheckState(Qt::Checked);

    if (info & RecView::RAW_VIDEO_FPS)
    	rawFpsCheckBox->setCheckState(Qt::Checked);

    if (info & RecView::BITRATE)
    	bitRateCheckBox->setCheckState(Qt::Checked);
}

void StatViewDialog::accept()
{
	uint info = 0;

	if (recStatCheckBox->isChecked())
		info |= RecView::REC_STATUS;

	if (encFpsCheckBox->isChecked())
		info |= RecView::ENCODED_FPS;

	if (rawFpsCheckBox->isChecked())
		info |= RecView::RAW_VIDEO_FPS;

	if (bitRateCheckBox->isChecked())
		info |= RecView::BITRATE;

	m_statView->selectInfo(info);
	QDialog::accept();
}
