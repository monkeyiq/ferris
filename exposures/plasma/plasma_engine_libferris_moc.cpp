/****************************************************************************
** Meta object code from reading C++ file 'plasma_engine_libferris.hh'
**
** Created: Fri Nov 18 07:05:12 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "plasma_engine_libferris.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'plasma_engine_libferris.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FerrisService[] = {

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

static const char qt_meta_stringdata_FerrisService[] = {
    "FerrisService\0"
};

void FerrisService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData FerrisService::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FerrisService::staticMetaObject = {
    { &Plasma::Service::staticMetaObject, qt_meta_stringdata_FerrisService,
      qt_meta_data_FerrisService, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FerrisService::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FerrisService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FerrisService::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FerrisService))
        return static_cast<void*>(const_cast< FerrisService*>(this));
    typedef Plasma::Service QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int FerrisService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Plasma::Service QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_FerrisEngine[] = {

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

static const char qt_meta_stringdata_FerrisEngine[] = {
    "FerrisEngine\0"
};

void FerrisEngine::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData FerrisEngine::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FerrisEngine::staticMetaObject = {
    { &Plasma::DataEngine::staticMetaObject, qt_meta_stringdata_FerrisEngine,
      qt_meta_data_FerrisEngine, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FerrisEngine::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FerrisEngine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FerrisEngine::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FerrisEngine))
        return static_cast<void*>(const_cast< FerrisEngine*>(this));
    typedef Plasma::DataEngine QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int FerrisEngine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Plasma::DataEngine QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
