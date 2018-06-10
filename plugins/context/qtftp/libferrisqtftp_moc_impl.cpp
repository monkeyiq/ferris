/****************************************************************************
** Meta object code from reading C++ file 'libferrisqtftp.cpp'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.7)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'libferrisqtftp.cpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Ferris__qtFtpContext[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   22,   21,   21, 0x0a,
      48,   45,   21,   21, 0x0a,
      76,   67,   21,   21, 0x0a,
     101,   67,   21,   21, 0x0a,
     125,   67,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Ferris__qtFtpContext[] = {
    "Ferris::qtFtpContext\0\0i\0OnListInfo(QUrlInfo)\0"
    "id\0OnListStarted(int)\0id,error\0"
    "OnListFinished(int,bool)\0"
    "OnGetFinished(int,bool)\0"
    "OnGeneralFinished(int,bool)\0"
};

void Ferris::qtFtpContext::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        qtFtpContext *_t = static_cast<qtFtpContext *>(_o);
        switch (_id) {
        case 0: _t->OnListInfo((*reinterpret_cast< const QUrlInfo(*)>(_a[1]))); break;
        case 1: _t->OnListStarted((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->OnListFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 3: _t->OnGetFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->OnGeneralFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Ferris::qtFtpContext::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Ferris::qtFtpContext::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Ferris__qtFtpContext,
      qt_meta_data_Ferris__qtFtpContext, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Ferris::qtFtpContext::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Ferris::qtFtpContext::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Ferris::qtFtpContext::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Ferris__qtFtpContext))
        return static_cast<void*>(const_cast< qtFtpContext*>(this));
    if (!strcmp(_clname, "StateLessEAHolder<qtFtpContext,FakeInternalContext>"))
        return static_cast< StateLessEAHolder<qtFtpContext,FakeInternalContext>*>(const_cast< qtFtpContext*>(this));
    return QObject::qt_metacast(_clname);
}

int Ferris::qtFtpContext::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
static const uint qt_meta_data_Ferris__qtFtpServerContext[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      37,   28,   27,   27, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Ferris__qtFtpServerContext[] = {
    "Ferris::qtFtpServerContext\0\0id,error\0"
    "OnConnectFinished(int,bool)\0"
};

void Ferris::qtFtpServerContext::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        qtFtpServerContext *_t = static_cast<qtFtpServerContext *>(_o);
        switch (_id) {
        case 0: _t->OnConnectFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Ferris::qtFtpServerContext::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Ferris::qtFtpServerContext::staticMetaObject = {
    { &qtFtpContext::staticMetaObject, qt_meta_stringdata_Ferris__qtFtpServerContext,
      qt_meta_data_Ferris__qtFtpServerContext, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Ferris::qtFtpServerContext::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Ferris::qtFtpServerContext::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Ferris::qtFtpServerContext::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Ferris__qtFtpServerContext))
        return static_cast<void*>(const_cast< qtFtpServerContext*>(this));
    return qtFtpContext::qt_metacast(_clname);
}

int Ferris::qtFtpServerContext::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = qtFtpContext::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
