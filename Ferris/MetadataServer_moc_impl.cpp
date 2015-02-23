/****************************************************************************
** Meta object code from reading C++ file 'MetadataServer.cpp'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MetadataServer.cpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Ferris__MetadataBrokerHandler[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      58,   31,   30,   30, 0x0a,
     130,  108,   30,   30, 0x0a,
     195,  179,   30,   30, 0x0a,
     236,   31,   30,   30, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Ferris__MetadataBrokerHandler[] = {
    "Ferris::MetadataBrokerHandler\0\0"
    "reqid,earl,eno,ename,edesc\0"
    "onAsyncGetFailed(int,QString,int,QString,QString)\0"
    "reqid,earl,name,value\0"
    "onAsyncGetResult(int,QString,QString,QByteArray)\0"
    "reqid,earl,name\0"
    "onAsyncPutCommitted(int,QString,QString)\0"
    "onAsyncPutFailed(int,QString,int,QString,QString)\0"
};

void Ferris::MetadataBrokerHandler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MetadataBrokerHandler *_t = static_cast<MetadataBrokerHandler *>(_o);
        switch (_id) {
        case 0: _t->onAsyncGetFailed((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< const QString(*)>(_a[5]))); break;
        case 1: _t->onAsyncGetResult((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QByteArray(*)>(_a[4]))); break;
        case 2: _t->onAsyncPutCommitted((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 3: _t->onAsyncPutFailed((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< const QString(*)>(_a[5]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Ferris::MetadataBrokerHandler::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Ferris::MetadataBrokerHandler::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Ferris__MetadataBrokerHandler,
      qt_meta_data_Ferris__MetadataBrokerHandler, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Ferris::MetadataBrokerHandler::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Ferris::MetadataBrokerHandler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Ferris::MetadataBrokerHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Ferris__MetadataBrokerHandler))
        return static_cast<void*>(const_cast< MetadataBrokerHandler*>(this));
    return QObject::qt_metacast(_clname);
}

int Ferris::MetadataBrokerHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
