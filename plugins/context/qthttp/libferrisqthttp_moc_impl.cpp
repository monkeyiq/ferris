/****************************************************************************
** Meta object code from reading C++ file 'libferrisqthttp.cpp'
**
** Created: Sat Jan 28 22:22:02 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'libferrisqthttp.cpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Ferris__qtHTTPContext[] = {

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
      23,   22,   22,   22, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Ferris__qtHTTPContext[] = {
    "Ferris::qtHTTPContext\0\0"
    "On_QNetworkReply_MetaDataChanged()\0"
};

void Ferris::qtHTTPContext::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        qtHTTPContext *_t = static_cast<qtHTTPContext *>(_o);
        switch (_id) {
        case 0: _t->On_QNetworkReply_MetaDataChanged(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Ferris::qtHTTPContext::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Ferris::qtHTTPContext::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Ferris__qtHTTPContext,
      qt_meta_data_Ferris__qtHTTPContext, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Ferris::qtHTTPContext::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Ferris::qtHTTPContext::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Ferris::qtHTTPContext::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Ferris__qtHTTPContext))
        return static_cast<void*>(const_cast< qtHTTPContext*>(this));
    if (!strcmp(_clname, "StateLessEAHolder<qtHTTPContext,FakeInternalContext>"))
        return static_cast< StateLessEAHolder<qtHTTPContext,FakeInternalContext>*>(const_cast< qtHTTPContext*>(this));
    return QObject::qt_metacast(_clname);
}

int Ferris::qtHTTPContext::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_Ferris__qtHTTPServerContext[] = {

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

static const char qt_meta_stringdata_Ferris__qtHTTPServerContext[] = {
    "Ferris::qtHTTPServerContext\0"
};

void Ferris::qtHTTPServerContext::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Ferris::qtHTTPServerContext::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Ferris::qtHTTPServerContext::staticMetaObject = {
    { &qtHTTPContext::staticMetaObject, qt_meta_stringdata_Ferris__qtHTTPServerContext,
      qt_meta_data_Ferris__qtHTTPServerContext, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Ferris::qtHTTPServerContext::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Ferris::qtHTTPServerContext::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Ferris::qtHTTPServerContext::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Ferris__qtHTTPServerContext))
        return static_cast<void*>(const_cast< qtHTTPServerContext*>(this));
    return qtHTTPContext::qt_metacast(_clname);
}

int Ferris::qtHTTPServerContext::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = qtHTTPContext::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
