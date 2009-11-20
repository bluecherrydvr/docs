/****************************************************************************
** Meta object code from reading C++ file 'cameradialog.h'
**
** Created: Mon Dec 15 17:12:18 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../cameradialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cameradialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_CameraSetupDialog[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      22,   19,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_CameraSetupDialog[] = {
    "CameraSetupDialog\0\0on\0"
    "onGrpPrimaryVideoEncoderToggled(bool)\0"
};

const QMetaObject CameraSetupDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_CameraSetupDialog,
      qt_meta_data_CameraSetupDialog, 0 }
};

const QMetaObject *CameraSetupDialog::metaObject() const
{
    return &staticMetaObject;
}

void *CameraSetupDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CameraSetupDialog))
	return static_cast<void*>(const_cast<CameraSetupDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int CameraSetupDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: onGrpPrimaryVideoEncoderToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        }
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_CameraDialog[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      20,   14,   13,   13, 0x08,
      45,   14,   13,   13, 0x08,
      75,   14,   13,   13, 0x08,
     116,   14,   13,   13, 0x08,
     173,  167,   13,   13, 0x08,
     216,  209,   13,   13, 0x08,
     247,   13,   13,   13, 0x08,
     269,   13,   13,   13, 0x08,
     286,   13,   13,   13, 0x08,
     309,  303,   13,   13, 0x08,
     340,  303,   13,   13, 0x08,
     363,  303,   13,   13, 0x08,
     393,  303,   13,   13, 0x08,
     421,  303,   13,   13, 0x08,
     458,  451,   13,   13, 0x08,
     487,  167,   13,   13, 0x08,
     517,  303,   13,   13, 0x08,
     539,   14,   13,   13, 0x08,
     567,   14,   13,   13, 0x08,
     592,   13,   13,   13, 0x08,
     608,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_CameraDialog[] = {
    "CameraDialog\0\0index\0onBrcComboActivated(int)\0"
    "onAlarmBrcComboActivated(int)\0"
    "onComboBitRateControlMPEG4Activated(int)\0"
    "onComboEncOnAlarmBitRateControlMPEG4Activated(int)\0"
    "state\0onAdjustEncodingOnAlarmChanged(int)\0"
    "subEnc\0onBtnGrpSubEncoderClicked(int)\0"
    "onBtnPrivacyRegions()\0onBtnMDRegions()\0"
    "onBtnBDRegions()\0value\0"
    "onTranslucentValueChanged(int)\0"
    "onHueValueChanged(int)\0"
    "onBrightnessValueChanged(int)\0"
    "onContrastValueChanged(int)\0"
    "onSaturationValueChanged(int)\0nValue\0"
    "onSharpnessValueChanged(int)\0"
    "onVideoLiveUpdateChanged(int)\0"
    "onOSDItemChanged(int)\0onComboOSDLocActivated(int)\0"
    "onComboMBThresholds(int)\0onBtnResetAll()\0"
    "accept()\0"
};

const QMetaObject CameraDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_CameraDialog,
      qt_meta_data_CameraDialog, 0 }
};

const QMetaObject *CameraDialog::metaObject() const
{
    return &staticMetaObject;
}

void *CameraDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CameraDialog))
	return static_cast<void*>(const_cast<CameraDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int CameraDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: onBrcComboActivated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: onAlarmBrcComboActivated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: onComboBitRateControlMPEG4Activated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: onComboEncOnAlarmBitRateControlMPEG4Activated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: onAdjustEncodingOnAlarmChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: onBtnGrpSubEncoderClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: onBtnPrivacyRegions(); break;
        case 7: onBtnMDRegions(); break;
        case 8: onBtnBDRegions(); break;
        case 9: onTranslucentValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: onHueValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: onBrightnessValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: onContrastValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: onSaturationValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: onSharpnessValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: onVideoLiveUpdateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: onOSDItemChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: onComboOSDLocActivated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 18: onComboMBThresholds((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 19: onBtnResetAll(); break;
        case 20: accept(); break;
        }
        _id -= 21;
    }
    return _id;
}
