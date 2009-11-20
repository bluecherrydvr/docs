/****************************************************************************
** Meta object code from reading C++ file 'decoderdialog.h'
**
** Created: Wed Dec 24 15:05:21 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../decoderdialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'decoderdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_DecoderDialog[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x08,
      24,   14,   14,   14, 0x08,
      39,   33,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DecoderDialog[] = {
    "DecoderDialog\0\0accept()\0browse()\0index\0"
    "onComboBoardIdSelected(int)\0"
};

const QMetaObject DecoderDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DecoderDialog,
      qt_meta_data_DecoderDialog, 0 }
};

const QMetaObject *DecoderDialog::metaObject() const
{
    return &staticMetaObject;
}

void *DecoderDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DecoderDialog))
	return static_cast<void*>(const_cast<DecoderDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int DecoderDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: accept(); break;
        case 1: browse(); break;
        case 2: onComboBoardIdSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        }
        _id -= 3;
    }
    return _id;
}
