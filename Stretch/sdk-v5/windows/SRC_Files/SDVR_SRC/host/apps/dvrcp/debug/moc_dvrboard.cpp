/****************************************************************************
** Meta object code from reading C++ file 'dvrboard.h'
**
** Created: Mon Dec 15 17:12:16 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../dvrboard.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dvrboard.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_DVRBoard[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets

       0        // eod
};

static const char qt_meta_stringdata_DVRBoard[] = {
    "DVRBoard\0"
};

const QMetaObject DVRBoard::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_DVRBoard,
      qt_meta_data_DVRBoard, 0 }
};

const QMetaObject *DVRBoard::metaObject() const
{
    return &staticMetaObject;
}

void *DVRBoard::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DVRBoard))
	return static_cast<void*>(const_cast<DVRBoard*>(this));
    return QObject::qt_metacast(_clname);
}

int DVRBoard::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
