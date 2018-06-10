/****************************************************************************
** Meta object code from reading C++ file 'FerrisQt.cpp'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.7)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FerrisQt.cpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Ferris__StreamToQIODeviceImpl[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      31,   30,   30,   30, 0x0a,
      69,   48,   30,   30, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Ferris__StreamToQIODeviceImpl[] = {
    "Ferris::StreamToQIODeviceImpl\0\0"
    "handleFinished()\0bytesSent,bytesTotal\0"
    "uploadProgress(qint64,qint64)\0"
};

void Ferris::StreamToQIODeviceImpl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        StreamToQIODeviceImpl *_t = static_cast<StreamToQIODeviceImpl *>(_o);
        switch (_id) {
        case 0: _t->handleFinished(); break;
        case 1: _t->uploadProgress((*reinterpret_cast< qint64(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Ferris::StreamToQIODeviceImpl::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Ferris::StreamToQIODeviceImpl::staticMetaObject = {
    { &StreamToQIODevice::staticMetaObject, qt_meta_stringdata_Ferris__StreamToQIODeviceImpl,
      qt_meta_data_Ferris__StreamToQIODeviceImpl, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Ferris::StreamToQIODeviceImpl::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Ferris::StreamToQIODeviceImpl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Ferris::StreamToQIODeviceImpl::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Ferris__StreamToQIODeviceImpl))
        return static_cast<void*>(const_cast< StreamToQIODeviceImpl*>(this));
    return StreamToQIODevice::qt_metacast(_clname);
}

int Ferris::StreamToQIODeviceImpl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = StreamToQIODevice::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
static const uint qt_meta_data_Ferris__StreamFromQIODevice_streambuf[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      39,   38,   38,   38, 0x0a,
      53,   38,   38,   38, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Ferris__StreamFromQIODevice_streambuf[] = {
    "Ferris::StreamFromQIODevice_streambuf\0"
    "\0OnReadyRead()\0OnReadChannelFinished()\0"
};

void Ferris::StreamFromQIODevice_streambuf::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        StreamFromQIODevice_streambuf *_t = static_cast<StreamFromQIODevice_streambuf *>(_o);
        switch (_id) {
        case 0: _t->OnReadyRead(); break;
        case 1: _t->OnReadChannelFinished(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Ferris::StreamFromQIODevice_streambuf::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Ferris::StreamFromQIODevice_streambuf::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Ferris__StreamFromQIODevice_streambuf,
      qt_meta_data_Ferris__StreamFromQIODevice_streambuf, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Ferris::StreamFromQIODevice_streambuf::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Ferris::StreamFromQIODevice_streambuf::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Ferris::StreamFromQIODevice_streambuf::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Ferris__StreamFromQIODevice_streambuf))
        return static_cast<void*>(const_cast< StreamFromQIODevice_streambuf*>(this));
    if (!strcmp(_clname, "ferris_basic_streambuf<char,std::char_traits<char> >"))
        return static_cast< ferris_basic_streambuf<char,std::char_traits<char> >*>(const_cast< StreamFromQIODevice_streambuf*>(this));
    return QObject::qt_metacast(_clname);
}

int Ferris::StreamFromQIODevice_streambuf::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
