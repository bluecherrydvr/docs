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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QPainter>
#include <QMouseEvent>
#include <sdvr_ui_sdk.h>

#include "camera.h"
#include "log.h"
#include "sdvr.h"
#include "regiondialog.h"

static DrawingArea *g_drawingArea;

static QColor color[] = { Qt::red, Qt::blue, Qt::cyan, Qt::yellow };

#ifdef Q_OS_LINUX
static char stip_data0[] =  { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 };
static char stip_data1[] =  { 0xAA, 0x44, 0xAA, 0x11, 0xAA, 0x44, 0xAA, 0x11 };
static char stip_data2[] =  { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
static char stip_data3[] =  { 0x22, 0x00, 0x88, 0x00, 0x22, 0x00, 0x88, 0x00 };
#endif


DrawingArea::DrawingArea(CRegion *reg, QWidget *parent) :
	QWidget(parent),
	m_showLayer(reg->layers(), true)
{
	m_region = reg;
	setFixedSize(reg->frameWidth(), reg->frameHeight());
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_PaintOnScreen, true);

	m_mode = DRAW;
	m_drawLayer = 0;
	m_modified = false;
	m_scribbling = false;
	for (int i = 0; i < reg->layers(); ++i)
	{
		QImage *image = new QImage(reg->cols(), reg->rows(), QImage::Format_RGB32);
		image->fill(Qt::black);
		m_layers.append(image);
	}

#ifdef Q_OS_LINUX
	m_x11Info = x11Info();
	Display *dpy = m_x11Info.display();
	stip[0] = XCreateBitmapFromData(dpy, winId(), stip_data0, 8, 8);
    stip[1] = XCreateBitmapFromData(dpy, winId(), stip_data1, 8, 8);
    stip[2] = XCreateBitmapFromData(dpy, winId(), stip_data2, 8, 8);
    stip[3] = XCreateBitmapFromData(dpy, winId(), stip_data3, 8, 8);
#endif

	load();
}

DrawingArea::~DrawingArea()
{
	qDeleteAll(m_layers);
	m_layers.clear();

    g_drawingArea = NULL;

#ifdef Q_OS_LINUX
	for (int i = 0; i < 4; ++i)
		XFreePixmap(m_x11Info.display(), stip[i]);
#endif
};

void DrawingArea::paintEvent(QPaintEvent *)
{
    return;
}

void DrawingArea::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
	{
		m_lastPoint = event->pos();
		m_scribbling = true;
	}
}

void DrawingArea::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && m_scribbling)
    {
        drawLineTo(event->pos());
    }
}

void DrawingArea::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_scribbling)
	{
		drawLineTo(event->pos());
		m_scribbling = false;
	}
}

#ifdef WIN32
static void preview_cb(HDC hdc)
{
	if (g_drawingArea != NULL)
		g_drawingArea->drawRegion(hdc);
}

void DrawingArea::drawRegion(HDC hdc)
{
    HBITMAP hbitmap;       // Bitmap handle
    BITMAPINFO bmi;        // Bitmap header
    BLENDFUNCTION bf;      // Structure for alpha blending
	void *pvBits;          // Pointer to DIB section
	int x1, y1, x2, y2;

    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = CRegion::BLOCK_SIZE;
	bmi.bmiHeader.biHeight = CRegion::BLOCK_SIZE;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;         // Four 8-bit components
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage =  CRegion::BLOCK_SIZE * CRegion::BLOCK_SIZE * 4;


	HDC hdc2 = CreateCompatibleDC(hdc);

	// Create DIB section and select the bitmap into the DC
    hbitmap = CreateDIBSection(hdc2, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);
    SelectObject(hdc2, hbitmap);

    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 100;   // A number between 0 and 255
    bf.AlphaFormat = 0;	            // Ignore source alpha channel

	x1 = y1 = 0;
	x2 = width()  / CRegion::BLOCK_SIZE;
	y2 = height() / CRegion::BLOCK_SIZE;

	int nLayers = m_layers.size();
    for (int n = 0; n < nLayers; ++n)
    {
    	if (m_showLayer[n] || n == m_drawLayer)
    	{
			UINT32 c = (((color[n].red() << 8) + color[n].green()) << 8) + color[n].blue();

			for (int i = 0; i < CRegion::BLOCK_SIZE * CRegion::BLOCK_SIZE; ++i)
				((UINT32 *)pvBits)[i] = c;

			for (int x = x1; x < x2; ++x)
			{
				for (int y = y1; y < y2; ++y)
				{
					if (m_layers[n]->pixel(x, y) == 0xFFFFFFFF)
					{
						int left = x * CRegion::BLOCK_SIZE;
						int top = y * CRegion::BLOCK_SIZE;
						AlphaBlend(hdc, left, top, CRegion::BLOCK_SIZE, CRegion::BLOCK_SIZE,
							hdc2, 0, 0, CRegion::BLOCK_SIZE, CRegion::BLOCK_SIZE, bf);
					}
				}
			}
		}
    }

	DeleteObject(hbitmap);
	DeleteDC(hdc2);
}

#else

static void preview_cb(Display *dpy, Drawable win)
{
	if (g_drawingArea != NULL)
		g_drawingArea->drawRegion(dpy, win);
}

void DrawingArea::drawRegion(Display *dpy, Drawable win)
{
	GC gc = XCreateGC(dpy, win, 0, NULL);
    XSetFillStyle(dpy, gc, FillStippled);

    int x1, y1, x2, y2;
	x1 = y1 = 0;
	x2 = width()  / CRegion::BLOCK_SIZE;
	y2 = height() / CRegion::BLOCK_SIZE;

	int nLayers = m_layers.size();
    for (int n = 0; n < nLayers; ++n)
    {
    	if (m_showLayer[n] || n == m_drawLayer)
    	{
			ulong c = (((color[n].red() << 8) + color[n].green()) << 8) + color[n].blue();
    		XSetForeground(dpy, gc, c);
    	    XSetStipple(dpy, gc, stip[n]);

			for (int x = x1; x < x2; ++x)
			{
				for (int y = y1; y < y2; ++y)
				{
					if (m_layers[n]->pixel(x, y) == 0xFFFFFFFF)
					{
						int left = x * CRegion::BLOCK_SIZE;
						int top = y * CRegion::BLOCK_SIZE;
						XFillRectangle(dpy, win, gc, left, top,
								CRegion::BLOCK_SIZE, CRegion::BLOCK_SIZE);
					}
				}
			}
		}
    }

    XFreeGC(dpy, gc);
}

#endif

void DrawingArea::drawLineTo(const QPoint &endPoint)
{
    QPainter painter(m_layers[m_drawLayer]);
    painter.setPen((m_mode == DRAW) ? Qt::white : Qt::black);

	QPoint scaledLastPoint(
		m_lastPoint.x() / CRegion::BLOCK_SIZE,
		m_lastPoint.y() / CRegion::BLOCK_SIZE);

	QPoint scaledEndPoint(
		endPoint.x() / CRegion::BLOCK_SIZE,
		endPoint.y() / CRegion::BLOCK_SIZE);

    painter.drawLine(scaledLastPoint, scaledEndPoint);
    m_modified = true;

    QRect clipRect = QRect(m_lastPoint, endPoint).normalized();
	clipRect.adjust(
		-clipRect.left() % CRegion::BLOCK_SIZE,
		-clipRect.top() % CRegion::BLOCK_SIZE,
		CRegion::BLOCK_SIZE - clipRect.right() % CRegion::BLOCK_SIZE,
		CRegion::BLOCK_SIZE - clipRect.bottom() % CRegion::BLOCK_SIZE);

    m_lastPoint = endPoint;
}

void DrawingArea::setDrawingLayer(int layer)
{
	Q_ASSERT(0 <= layer && layer < m_layers.size());
	m_drawLayer = layer;
}

void DrawingArea::showLayer(int layer)
{
	Q_ASSERT(0 <= layer && layer < m_layers.size());
	m_showLayer[layer] = true;
}

void DrawingArea::hideLayer(int layer)
{
	Q_ASSERT(0 <= layer && layer < m_layers.size());
	m_showLayer[layer] = false;
}

void DrawingArea::fillAll()
{
	m_layers[m_drawLayer]->fill(0xFFFFFFFF);
}

void DrawingArea::clear()
{
	m_layers[m_drawLayer]->fill(Qt::black);
}

void DrawingArea::load()
{
	for (int x = 0; x < m_region->cols(); ++x)
		for (int y = 0; y < m_region->rows(); ++y)
			for (int n = 0; n < m_region->layers(); ++n)
				if (m_region->block(y, x, n))
					m_layers[n]->setPixel(x, y, 0xFFFFFFFF);
}

void DrawingArea::save()
{
	for (int x = 0; x < m_region->cols(); ++x)
		for (int y = 0; y < m_region->rows(); ++y)
			for (int n = 0; n < m_region->layers(); ++n)
				m_region->setBlock(y, x, n,
					m_layers[n]->pixel(x, y) == 0xFFFFFFFF);
}


RegionDialog::RegionDialog(Camera *cam, CRegion *reg, QWidget *parent)
    : QDialog(parent)
{
	m_camera = cam;
	m_previewId = -1;

	g_drawingArea = m_drawingArea = new DrawingArea(reg);

    QPushButton *drawButton = new QPushButton("&Draw");
    drawButton->setCheckable(true);
    drawButton->setChecked(true);

    QPushButton *eraseButton = new QPushButton("&Erase");
    eraseButton->setCheckable(true);
    eraseButton->setChecked(false);

    QButtonGroup *buttonGroup1 = new QButtonGroup();
    buttonGroup1->addButton(drawButton, DrawingArea::DRAW);
    buttonGroup1->addButton(eraseButton, DrawingArea::ERASE);

    QPushButton *eraseAllButton = new QPushButton("Erase All");
	QPushButton *fillAllButton = new QPushButton("Fill All");

	QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->setOrientation(Qt::Vertical);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel
			| QDialogButtonBox::NoButton | QDialogButtonBox::Ok);

	QHBoxLayout *hboxLayout = new QHBoxLayout;
    hboxLayout->addWidget(m_drawingArea);

    QVBoxLayout *vboxLayout = new QVBoxLayout;
    vboxLayout->addWidget(drawButton);
    vboxLayout->addWidget(eraseButton);
    vboxLayout->addWidget(eraseAllButton);
    vboxLayout->addWidget(fillAllButton);

	if (reg->layers() > 1)
    {
		QPixmap pixmap(8, 8);
        QButtonGroup *buttonGroup2 = new QButtonGroup(this);
    	QGridLayout *grid = new QGridLayout();
		for (int n = 0; n < reg->layers(); ++n)
		{
			QCheckBox *cb = new QCheckBox();
			cb->setChecked(true);
			cb->setFixedSize(16, 16);
			cb->setProperty("layer", n);
			connect(cb, SIGNAL(stateChanged(int)),
					this, SLOT(showStateChanged(int)));
			m_showCheckBoxes.append(cb);

			pixmap.fill(color[n]);
			QPushButton *pb = new QPushButton(QIcon(pixmap), "");
			pb->setFixedSize(16, 16);
			pb->setCheckable(true);
			if (n == 0)
				pb->setChecked(true);

			buttonGroup2->addButton(pb, n);

			grid->addWidget(cb, n, 0, Qt::AlignCenter);
			grid->addWidget(pb, n, 1, Qt::AlignCenter);
		}

		QGroupBox *groupBox = new QGroupBox("Show | Draw");
		groupBox->setLayout(grid);
		vboxLayout->addSpacing(10);
		vboxLayout->addWidget(groupBox);

		connect(buttonGroup2, SIGNAL(buttonClicked(int)),
				m_drawingArea, SLOT(setDrawingLayer(int)));
    }

    vboxLayout->addWidget(buttonBox, 1, Qt::AlignBottom);

    hboxLayout->addLayout(vboxLayout);
    setLayout(hboxLayout);

    connect(buttonGroup1, SIGNAL(buttonClicked(int)),
    		m_drawingArea, SLOT(setDrawingMode(int)));
    connect(eraseAllButton, SIGNAL(clicked()), m_drawingArea, SLOT(clear()));
	connect(fillAllButton, SIGNAL(clicked()), m_drawingArea, SLOT(fillAll()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void RegionDialog::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);
	playVideo();
}

void RegionDialog::closeEvent(QCloseEvent *event)
{
	stopVideo();
	QDialog::closeEvent(event);
}

void RegionDialog::hideEvent(QHideEvent *event)
{
	stopVideo();
	QDialog::hideEvent(event);
}

void RegionDialog::showStateChanged(int state)
{
	QCheckBox *cb = static_cast<QCheckBox *>(sender());
	int n = cb->property("layer").toInt();
	if (state == Qt::Checked)
		m_drawingArea->showLayer(n);
	else
		m_drawingArea->hideLayer(n);
}

bool RegionDialog::playVideo()
{
	if (!m_camera->isChannelOpen())
		return false;

	if (m_camera->setRawVideo(SDVR_VIDEO_RES_DECIMATION_EQUAL, true) != SDVR_ERR_NONE)
		return false;

	sdvr_ui_region_t reg;
    reg.top_left_x = reg.top_left_y = 0;
    reg.width = m_drawingArea->width();
    reg.height = m_drawingArea->height();

	sdvr_err_ui_e err = sdvr_ui_start_video_preview(m_camera->handle(),
		m_drawingArea->winId(), &reg, &m_previewId);

	if (err != SDVR_UI_ERR_NONE)
		return false;

	sdvr_ui_set_preview_callback(m_camera->handle(), m_previewId, preview_cb);
	return true;
}

void RegionDialog::stopVideo()
{
	if (m_previewId != -1)
	{
		m_camera->setRawVideo(SDVR_VIDEO_RES_DECIMATION_EQUAL, false);
		sdvr_ui_set_preview_callback(m_camera->handle(), m_previewId, NULL);
		sdvr_ui_stop_video_preview(m_camera->handle(), m_previewId);
		m_previewId = -1;
	}
}

void RegionDialog::accept()
{
	m_drawingArea->save();
	QDialog::accept();
}
