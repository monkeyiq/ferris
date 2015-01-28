/****************************************************************************
** Meta object code from reading C++ file 'libferrisplasma.cpp'
**
** Created: Fri Nov 18 07:04:33 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'libferrisplasma.cpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Ferris__PlasmaSourceContext[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      47,   41,   29,   28, 0x0a,
      90,   74,   28,   28, 0x0a,
     143,  136,   28,   28, 0x0a,
     183,   28,   28,   28, 0x2a,
     232,   28,  212,   28, 0x0a,
     257,  255,  244,   28, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Ferris__PlasmaSourceContext[] = {
    "Ferris::PlasmaSourceContext\0\0std::string\0"
    "v,sct\0toEA(QVariant,XSDBasic_t&)\0"
    "sourceName,data\0"
    "dataUpdated(QString,Plasma::DataEngine::Data)\0"
    "eaname\0ensureAttributesAreCreated(std::string)\0"
    "ensureAttributesAreCreated()\0"
    "Plasma::DataEngine*\0getEngine()\0"
    "fh_istream\0m\0priv_getIStream(ferris_ios::openmode)\0"
};

void Ferris::PlasmaSourceContext::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PlasmaSourceContext *_t = static_cast<PlasmaSourceContext *>(_o);
        switch (_id) {
        case 0: { std::string _r = _t->toEA((*reinterpret_cast< QVariant(*)>(_a[1])),(*reinterpret_cast< XSDBasic_t(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< std::string*>(_a[0]) = _r; }  break;
        case 1: _t->dataUpdated((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const Plasma::DataEngine::Data(*)>(_a[2]))); break;
        case 2: _t->ensureAttributesAreCreated((*reinterpret_cast< const std::string(*)>(_a[1]))); break;
        case 3: _t->ensureAttributesAreCreated(); break;
        case 4: { Plasma::DataEngine* _r = _t->getEngine();
            if (_a[0]) *reinterpret_cast< Plasma::DataEngine**>(_a[0]) = _r; }  break;
        case 5: { fh_istream _r = _t->priv_getIStream((*reinterpret_cast< ferris_ios::openmode(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< fh_istream*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Ferris::PlasmaSourceContext::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Ferris::PlasmaSourceContext::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Ferris__PlasmaSourceContext,
      qt_meta_data_Ferris__PlasmaSourceContext, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Ferris::PlasmaSourceContext::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Ferris::PlasmaSourceContext::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Ferris::PlasmaSourceContext::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Ferris__PlasmaSourceContext))
        return static_cast<void*>(const_cast< PlasmaSourceContext*>(this));
    if (!strcmp(_clname, "StateLessEAHolder<PlasmaSourceContext,leafContext>"))
        return static_cast< StateLessEAHolder<PlasmaSourceContext,leafContext>*>(const_cast< PlasmaSourceContext*>(this));
    return QObject::qt_metacast(_clname);
}

int Ferris::PlasmaSourceContext::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
