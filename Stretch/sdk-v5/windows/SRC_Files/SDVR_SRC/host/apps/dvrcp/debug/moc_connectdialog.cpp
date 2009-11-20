/****************************************************************************
** Meta object code from reading C++ file 'connectdialog.h'
**
** Created: Mon Dec 15 17:12:17 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../connectdialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'connectdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_ConnectDialog[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      21,   15,   14,   14, 0x08,
      42,   14,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ConnectDialog[] = {
    "ConnectDialog\0\0index\0onBoardSelected(int)\0"
    "onBtnChangeAVEnc()\0"
};

const QMetaObject ConnectDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_ConnectDialog,
      qt_meta_data_ConnectDialog, 0 }
};

const QMetaObject *ConnectDialog::metaObject() const
{
    return &staticMetaObject;
}

void *ConnectDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ConnectDialog))
	return static_cast<void*>(const_cast<ConnectDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int ConnectDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: onBoardSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: onBtnChangeAVEnc(); break;
        }
        _id -= 2;
    }
    return _id;
}
