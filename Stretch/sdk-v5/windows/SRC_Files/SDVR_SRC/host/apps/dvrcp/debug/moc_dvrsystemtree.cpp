/****************************************************************************
** Meta object code from reading C++ file 'dvrsystemtree.h'
**
** Created: Mon Dec 15 17:12:15 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../dvrsystemtree.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dvrsystemtree.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_DVRSystemTree[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets

       0        // eod
};

static const char qt_meta_stringdata_DVRSystemTree[] = {
    "DVRSystemTree\0"
};

const QMetaObject DVRSystemTree::staticMetaObject = {
    { &QTreeWidget::staticMetaObject, qt_meta_stringdata_DVRSystemTree,
      qt_meta_data_DVRSystemTree, 0 }
};

const QMetaObject *DVRSystemTree::metaObject() const
{
    return &staticMetaObject;
}

void *DVRSystemTree::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DVRSystemTree))
	return static_cast<void*>(const_cast<DVRSystemTree*>(this));
    return QTreeWidget::qt_metacast(_clname);
}

int DVRSystemTree::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTreeWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
