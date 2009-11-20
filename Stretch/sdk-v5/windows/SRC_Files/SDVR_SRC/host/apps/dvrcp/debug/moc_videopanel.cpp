/****************************************************************************
** Meta object code from reading C++ file 'videopanel.h'
**
** Created: Mon Dec 15 17:12:11 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../videopanel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'videopanel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_VideoPanel[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_VideoPanel[] = {
    "VideoPanel\0\0screenSelected()\0"
};

const QMetaObject VideoPanel::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_VideoPanel,
      qt_meta_data_VideoPanel, 0 }
};

const QMetaObject *VideoPanel::metaObject() const
{
    return &staticMetaObject;
}

void *VideoPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_VideoPanel))
	return static_cast<void*>(const_cast<VideoPanel*>(this));
    return QWidget::qt_metacast(_clname);
}

int VideoPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: screenSelected(); break;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void VideoPanel::screenSelected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
