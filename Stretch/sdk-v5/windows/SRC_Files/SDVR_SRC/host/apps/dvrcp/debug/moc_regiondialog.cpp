/****************************************************************************
** Meta object code from reading C++ file 'regiondialog.h'
**
** Created: Mon Dec 15 17:12:13 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../regiondialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'regiondialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_DrawingArea[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      18,   13,   12,   12, 0x0a,
      44,   38,   12,   12, 0x0a,
      65,   38,   12,   12, 0x0a,
      80,   38,   12,   12, 0x0a,
      95,   12,   12,   12, 0x0a,
     105,   12,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_DrawingArea[] = {
    "DrawingArea\0\0mode\0setDrawingMode(int)\0"
    "layer\0setDrawingLayer(int)\0showLayer(int)\0"
    "hideLayer(int)\0fillAll()\0clear()\0"
};

const QMetaObject DrawingArea::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_DrawingArea,
      qt_meta_data_DrawingArea, 0 }
};

const QMetaObject *DrawingArea::metaObject() const
{
    return &staticMetaObject;
}

void *DrawingArea::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DrawingArea))
	return static_cast<void*>(const_cast<DrawingArea*>(this));
    return QWidget::qt_metacast(_clname);
}

int DrawingArea::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setDrawingMode((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: setDrawingLayer((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: showLayer((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: hideLayer((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: fillAll(); break;
        case 5: clear(); break;
        }
        _id -= 6;
    }
    return _id;
}
static const uint qt_meta_data_RegionDialog[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      20,   14,   13,   13, 0x08,
      42,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_RegionDialog[] = {
    "RegionDialog\0\0state\0showStateChanged(int)\0"
    "accept()\0"
};

const QMetaObject RegionDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_RegionDialog,
      qt_meta_data_RegionDialog, 0 }
};

const QMetaObject *RegionDialog::metaObject() const
{
    return &staticMetaObject;
}

void *RegionDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RegionDialog))
	return static_cast<void*>(const_cast<RegionDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int RegionDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: showStateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: accept(); break;
        }
        _id -= 2;
    }
    return _id;
}
