/****************************************************************************
** Meta object code from reading C++ file 'tilesetmaker.h'
**
** Created: Sun Oct 28 15:53:20 2007
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "tilesetmaker.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tilesetmaker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_TilesetMaker[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      17,   14,   13,   13, 0x0a,
      42,   13,   13,   13, 0x0a,
      65,   13,   13,   13, 0x0a,
      88,   13,   13,   13, 0x0a,
     124,   13,   13,   13, 0x0a,
     159,   13,   13,   13, 0x0a,
     194,   13,   13,   13, 0x0a,
     220,   13,   13,   13, 0x0a,
     276,  271,   13,   13, 0x0a,
     304,  271,   13,   13, 0x0a,
     337,  271,   13,   13, 0x0a,
     369,  271,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_TilesetMaker[] = {
    "TilesetMaker\0\0ev\0closeEvent(QCloseEvent*)\0"
    "on_m_pbtnAdd_clicked()\0on_m_pbtnRem_clicked()\0"
    "on_m_pbtnBrowseUppertiles_clicked()\0"
    "on_m_pbtnBrowseBlendmask_clicked()\0"
    "on_m_pbtnBrowseDetailmap_clicked()\0"
    "on_m_pbtnCreate_clicked()\0"
    "on_m_plbTiles_currentItemChanged(QListWidgetItem*)\0"
    "in_b\0on_m_prbTiles_toggled(bool)\0"
    "on_m_prbUppertiles_toggled(bool)\0"
    "on_m_prbBlendmask_toggled(bool)\0"
    "on_m_prbDetailmap_toggled(bool)\0"
};

const QMetaObject TilesetMaker::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_TilesetMaker,
      qt_meta_data_TilesetMaker, 0 }
};

const QMetaObject *TilesetMaker::metaObject() const
{
    return &staticMetaObject;
}

void *TilesetMaker::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TilesetMaker))
	return static_cast<void*>(const_cast< TilesetMaker*>(this));
    return QDialog::qt_metacast(_clname);
}

int TilesetMaker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        case 1: on_m_pbtnAdd_clicked(); break;
        case 2: on_m_pbtnRem_clicked(); break;
        case 3: on_m_pbtnBrowseUppertiles_clicked(); break;
        case 4: on_m_pbtnBrowseBlendmask_clicked(); break;
        case 5: on_m_pbtnBrowseDetailmap_clicked(); break;
        case 6: on_m_pbtnCreate_clicked(); break;
        case 7: on_m_plbTiles_currentItemChanged((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 8: on_m_prbTiles_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: on_m_prbUppertiles_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: on_m_prbBlendmask_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: on_m_prbDetailmap_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        }
        _id -= 12;
    }
    return _id;
}
