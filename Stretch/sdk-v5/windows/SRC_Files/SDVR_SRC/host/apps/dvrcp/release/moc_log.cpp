/****************************************************************************
** Meta object code from reading C++ file 'log.h'
**
** Created: Wed Dec 24 15:05:19 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../log.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'log.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_Log[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
       5,    4,    4,    4, 0x05,

 // slots: signature, parameters, type, tag, flags
      18,    4,    4,    4, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_Log[] = {
    "Log\0\0alarmReset()\0resetAlarm()\0"
};

const QMetaObject Log::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Log,
      qt_meta_data_Log, 0 }
};

const QMetaObject *Log::metaObject() const
{
    return &staticMetaObject;
}

void *Log::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Log))
	return static_cast<void*>(const_cast<Log*>(this));
    return QWidget::qt_metacast(_clname);
}

int Log::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: alarmReset(); break;
        case 1: resetAlarm(); break;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void Log::alarmReset()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
