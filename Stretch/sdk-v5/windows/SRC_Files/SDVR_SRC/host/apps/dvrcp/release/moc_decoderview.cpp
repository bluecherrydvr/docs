/****************************************************************************
** Meta object code from reading C++ file 'decoderview.h'
**
** Created: Wed Dec 24 15:05:20 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../decoderview.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'decoderview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_DecoderView[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      27,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DecoderView[] = {
    "DecoderView\0\0itemClicked()\0update()\0"
};

const QMetaObject DecoderView::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_DecoderView,
      qt_meta_data_DecoderView, 0 }
};

const QMetaObject *DecoderView::metaObject() const
{
    return &staticMetaObject;
}

void *DecoderView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DecoderView))
	return static_cast<void*>(const_cast<DecoderView*>(this));
    return QWidget::qt_metacast(_clname);
}

int DecoderView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: itemClicked(); break;
        case 1: update(); break;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void DecoderView::itemClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}