/****************************************************************************
** Meta object code from reading C++ file 'systemdialog.h'
**
** Created: Wed Dec 24 15:05:14 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../systemdialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'systemdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_StartupDialog[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x08,
      39,   14,   14,   14, 0x08,
      60,   14,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_StartupDialog[] = {
    "StartupDialog\0\0browseStartupFirmware()\0"
    "browseSystemFolder()\0accept()\0"
};

const QMetaObject StartupDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_StartupDialog,
      qt_meta_data_StartupDialog, 0 }
};

const QMetaObject *StartupDialog::metaObject() const
{
    return &staticMetaObject;
}

void *StartupDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_StartupDialog))
	return static_cast<void*>(const_cast<StartupDialog*>(this));
    if (!strcmp(_clname, "Ui::StartupDialog"))
	return static_cast<Ui::StartupDialog*>(const_cast<StartupDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int StartupDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: browseStartupFirmware(); break;
        case 1: browseSystemFolder(); break;
        case 2: accept(); break;
        }
        _id -= 3;
    }
    return _id;
}
static const uint qt_meta_data_SystemDialog[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SystemDialog[] = {
    "SystemDialog\0\0browseFontFile()\0"
};

const QMetaObject SystemDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_SystemDialog,
      qt_meta_data_SystemDialog, 0 }
};

const QMetaObject *SystemDialog::metaObject() const
{
    return &staticMetaObject;
}

void *SystemDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SystemDialog))
	return static_cast<void*>(const_cast<SystemDialog*>(this));
    if (!strcmp(_clname, "Ui::SystemDialog"))
	return static_cast<Ui::SystemDialog*>(const_cast<SystemDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int SystemDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: browseFontFile(); break;
        }
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_SystemNewDialog[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x08,
      26,   16,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SystemNewDialog[] = {
    "SystemNewDialog\0\0browse()\0accept()\0"
};

const QMetaObject SystemNewDialog::staticMetaObject = {
    { &SystemDialog::staticMetaObject, qt_meta_stringdata_SystemNewDialog,
      qt_meta_data_SystemNewDialog, 0 }
};

const QMetaObject *SystemNewDialog::metaObject() const
{
    return &staticMetaObject;
}

void *SystemNewDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SystemNewDialog))
	return static_cast<void*>(const_cast<SystemNewDialog*>(this));
    return SystemDialog::qt_metacast(_clname);
}

int SystemNewDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SystemDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: browse(); break;
        case 1: accept(); break;
        }
        _id -= 2;
    }
    return _id;
}
static const uint qt_meta_data_SystemEditDialog[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SystemEditDialog[] = {
    "SystemEditDialog\0\0accept()\0"
};

const QMetaObject SystemEditDialog::staticMetaObject = {
    { &SystemDialog::staticMetaObject, qt_meta_stringdata_SystemEditDialog,
      qt_meta_data_SystemEditDialog, 0 }
};

const QMetaObject *SystemEditDialog::metaObject() const
{
    return &staticMetaObject;
}

void *SystemEditDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SystemEditDialog))
	return static_cast<void*>(const_cast<SystemEditDialog*>(this));
    return SystemDialog::qt_metacast(_clname);
}

int SystemEditDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SystemDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: accept(); break;
        }
        _id -= 1;
    }
    return _id;
}
