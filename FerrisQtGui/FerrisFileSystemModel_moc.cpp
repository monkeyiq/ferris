/****************************************************************************
** Meta object code from reading C++ file 'FerrisFileSystemModel.hh'
**
** Created: Fri Nov 18 07:00:45 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "FerrisFileSystemModel.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FerrisFileSystemModel.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Ferris__FerrisFileSystemModel[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_Ferris__FerrisFileSystemModel[] = {
    "Ferris::FerrisFileSystemModel\0"
};

void Ferris::FerrisFileSystemModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Ferris::FerrisFileSystemModel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Ferris::FerrisFileSystemModel::staticMetaObject = {
    { &QAbstractTableModel::staticMetaObject, qt_meta_stringdata_Ferris__FerrisFileSystemModel,
      qt_meta_data_Ferris__FerrisFileSystemModel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Ferris::FerrisFileSystemModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Ferris::FerrisFileSystemModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Ferris::FerrisFileSystemModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Ferris__FerrisFileSystemModel))
        return static_cast<void*>(const_cast< FerrisFileSystemModel*>(this));
    return QAbstractTableModel::qt_metacast(_clname);
}

int Ferris::FerrisFileSystemModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
