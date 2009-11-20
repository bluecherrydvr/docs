/****************************************************************************
** Meta object code from reading C++ file 'alarmhandler.h'
**
** Created: Wed Dec 24 15:05:24 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../alarmhandler.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'alarmhandler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_AlarmHandler[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_AlarmHandler[] = {
    "AlarmHandler\0\0reset()\0"
};

const QMetaObject AlarmHandler::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_AlarmHandler,
      qt_meta_data_AlarmHandler, 0 }
};

const QMetaObject *AlarmHandler::metaObject() const
{
    return &staticMetaObject;
}

void *AlarmHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AlarmHandler))
	return static_cast<void*>(const_cast<AlarmHandler*>(this));
    return QObject::qt_metacast(_clname);
}

int AlarmHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: reset(); break;
        }
        _id -= 1;
    }
    return _id;
}
