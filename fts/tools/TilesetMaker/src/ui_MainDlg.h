/********************************************************************************
** Form generated from reading ui file 'MainDlg.ui'
**
** Created: Sun Oct 28 13:19:41 2007
**      by: Qt User Interface Compiler version 4.3.1
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_MAINDLG_H
#define UI_MAINDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

class Ui_TilesetMaker
{
public:
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout1;
    QHBoxLayout *hboxLayout1;
    QVBoxLayout *vboxLayout2;
    QRadioButton *m_prbTiles;
    QPushButton *m_pbtnAdd;
    QPushButton *m_pbtnRem;
    QSpacerItem *spacerItem;
    QListWidget *m_plbTiles;
    QGridLayout *gridLayout;
    QPushButton *m_pbtnBrowseUppertiles;
    QLineEdit *m_pedUppertiles;
    QSpinBox *m_psbUppertilesCount;
    QRadioButton *m_prbUppertiles;
    QGridLayout *gridLayout1;
    QPushButton *m_pbtnBrowseBlendmask;
    QLineEdit *m_pedBlendmask;
    QRadioButton *m_prbBlendmask;
    QGridLayout *gridLayout2;
    QPushButton *m_pbtnBrowseDetailmap;
    QLineEdit *m_pedDetailmap;
    QRadioButton *m_prbDetailmap;
    QLabel *m_pimgCurrentPreview;
    QHBoxLayout *hboxLayout2;
    QPushButton *m_pbtnCreate;
    QSpacerItem *spacerItem1;
    QPushButton *m_pbtnQuit;

    void setupUi(QDialog *TilesetMaker)
    {
    if (TilesetMaker->objectName().isEmpty())
        TilesetMaker->setObjectName(QString::fromUtf8("TilesetMaker"));
    TilesetMaker->resize(631, 454);
    TilesetMaker->setWindowIcon(QIcon(QString::fromUtf8(":/newicon16.png")));
    vboxLayout = new QVBoxLayout(TilesetMaker);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    vboxLayout2 = new QVBoxLayout();
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    m_prbTiles = new QRadioButton(TilesetMaker);
    m_prbTiles->setObjectName(QString::fromUtf8("m_prbTiles"));
    m_prbTiles->setChecked(true);

    vboxLayout2->addWidget(m_prbTiles);

    m_pbtnAdd = new QPushButton(TilesetMaker);
    m_pbtnAdd->setObjectName(QString::fromUtf8("m_pbtnAdd"));

    vboxLayout2->addWidget(m_pbtnAdd);

    m_pbtnRem = new QPushButton(TilesetMaker);
    m_pbtnRem->setObjectName(QString::fromUtf8("m_pbtnRem"));

    vboxLayout2->addWidget(m_pbtnRem);

    spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout2->addItem(spacerItem);


    hboxLayout1->addLayout(vboxLayout2);

    m_plbTiles = new QListWidget(TilesetMaker);
    m_plbTiles->setObjectName(QString::fromUtf8("m_plbTiles"));
    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(m_plbTiles->sizePolicy().hasHeightForWidth());
    m_plbTiles->setSizePolicy(sizePolicy);

    hboxLayout1->addWidget(m_plbTiles);


    vboxLayout1->addLayout(hboxLayout1);

    gridLayout = new QGridLayout();
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    m_pbtnBrowseUppertiles = new QPushButton(TilesetMaker);
    m_pbtnBrowseUppertiles->setObjectName(QString::fromUtf8("m_pbtnBrowseUppertiles"));
    QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(m_pbtnBrowseUppertiles->sizePolicy().hasHeightForWidth());
    m_pbtnBrowseUppertiles->setSizePolicy(sizePolicy1);

    gridLayout->addWidget(m_pbtnBrowseUppertiles, 1, 1, 1, 1);

    m_pedUppertiles = new QLineEdit(TilesetMaker);
    m_pedUppertiles->setObjectName(QString::fromUtf8("m_pedUppertiles"));
    QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Fixed);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(m_pedUppertiles->sizePolicy().hasHeightForWidth());
    m_pedUppertiles->setSizePolicy(sizePolicy2);

    gridLayout->addWidget(m_pedUppertiles, 1, 0, 1, 1);

    m_psbUppertilesCount = new QSpinBox(TilesetMaker);
    m_psbUppertilesCount->setObjectName(QString::fromUtf8("m_psbUppertilesCount"));

    gridLayout->addWidget(m_psbUppertilesCount, 0, 1, 1, 1);

    m_prbUppertiles = new QRadioButton(TilesetMaker);
    m_prbUppertiles->setObjectName(QString::fromUtf8("m_prbUppertiles"));

    gridLayout->addWidget(m_prbUppertiles, 0, 0, 1, 1);


    vboxLayout1->addLayout(gridLayout);

    gridLayout1 = new QGridLayout();
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    m_pbtnBrowseBlendmask = new QPushButton(TilesetMaker);
    m_pbtnBrowseBlendmask->setObjectName(QString::fromUtf8("m_pbtnBrowseBlendmask"));
    sizePolicy1.setHeightForWidth(m_pbtnBrowseBlendmask->sizePolicy().hasHeightForWidth());
    m_pbtnBrowseBlendmask->setSizePolicy(sizePolicy1);

    gridLayout1->addWidget(m_pbtnBrowseBlendmask, 1, 1, 1, 1);

    m_pedBlendmask = new QLineEdit(TilesetMaker);
    m_pedBlendmask->setObjectName(QString::fromUtf8("m_pedBlendmask"));
    sizePolicy2.setHeightForWidth(m_pedBlendmask->sizePolicy().hasHeightForWidth());
    m_pedBlendmask->setSizePolicy(sizePolicy2);

    gridLayout1->addWidget(m_pedBlendmask, 1, 0, 1, 1);

    m_prbBlendmask = new QRadioButton(TilesetMaker);
    m_prbBlendmask->setObjectName(QString::fromUtf8("m_prbBlendmask"));

    gridLayout1->addWidget(m_prbBlendmask, 0, 0, 1, 1);


    vboxLayout1->addLayout(gridLayout1);

    gridLayout2 = new QGridLayout();
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    m_pbtnBrowseDetailmap = new QPushButton(TilesetMaker);
    m_pbtnBrowseDetailmap->setObjectName(QString::fromUtf8("m_pbtnBrowseDetailmap"));
    sizePolicy1.setHeightForWidth(m_pbtnBrowseDetailmap->sizePolicy().hasHeightForWidth());
    m_pbtnBrowseDetailmap->setSizePolicy(sizePolicy1);

    gridLayout2->addWidget(m_pbtnBrowseDetailmap, 1, 1, 1, 1);

    m_pedDetailmap = new QLineEdit(TilesetMaker);
    m_pedDetailmap->setObjectName(QString::fromUtf8("m_pedDetailmap"));
    sizePolicy2.setHeightForWidth(m_pedDetailmap->sizePolicy().hasHeightForWidth());
    m_pedDetailmap->setSizePolicy(sizePolicy2);

    gridLayout2->addWidget(m_pedDetailmap, 1, 0, 1, 1);

    m_prbDetailmap = new QRadioButton(TilesetMaker);
    m_prbDetailmap->setObjectName(QString::fromUtf8("m_prbDetailmap"));

    gridLayout2->addWidget(m_prbDetailmap, 0, 0, 1, 1);


    vboxLayout1->addLayout(gridLayout2);


    hboxLayout->addLayout(vboxLayout1);

    m_pimgCurrentPreview = new QLabel(TilesetMaker);
    m_pimgCurrentPreview->setObjectName(QString::fromUtf8("m_pimgCurrentPreview"));
    QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(m_pimgCurrentPreview->sizePolicy().hasHeightForWidth());
    m_pimgCurrentPreview->setSizePolicy(sizePolicy3);
    m_pimgCurrentPreview->setMinimumSize(QSize(256, 256));
    m_pimgCurrentPreview->setPixmap(QPixmap(QString::fromUtf8(":/newicon48.png")));
    m_pimgCurrentPreview->setScaledContents(false);
    m_pimgCurrentPreview->setAlignment(Qt::AlignCenter);

    hboxLayout->addWidget(m_pimgCurrentPreview);


    vboxLayout->addLayout(hboxLayout);

    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    m_pbtnCreate = new QPushButton(TilesetMaker);
    m_pbtnCreate->setObjectName(QString::fromUtf8("m_pbtnCreate"));

    hboxLayout2->addWidget(m_pbtnCreate);

    spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout2->addItem(spacerItem1);

    m_pbtnQuit = new QPushButton(TilesetMaker);
    m_pbtnQuit->setObjectName(QString::fromUtf8("m_pbtnQuit"));

    hboxLayout2->addWidget(m_pbtnQuit);


    vboxLayout->addLayout(hboxLayout2);

    QWidget::setTabOrder(m_pbtnAdd, m_pbtnRem);
    QWidget::setTabOrder(m_pbtnRem, m_plbTiles);
    QWidget::setTabOrder(m_plbTiles, m_pedUppertiles);
    QWidget::setTabOrder(m_pedUppertiles, m_pbtnBrowseUppertiles);
    QWidget::setTabOrder(m_pbtnBrowseUppertiles, m_pedBlendmask);
    QWidget::setTabOrder(m_pedBlendmask, m_pbtnBrowseBlendmask);
    QWidget::setTabOrder(m_pbtnBrowseBlendmask, m_pedDetailmap);
    QWidget::setTabOrder(m_pedDetailmap, m_pbtnBrowseDetailmap);
    QWidget::setTabOrder(m_pbtnBrowseDetailmap, m_pbtnCreate);
    QWidget::setTabOrder(m_pbtnCreate, m_pbtnQuit);

    retranslateUi(TilesetMaker);
    QObject::connect(m_pbtnQuit, SIGNAL(clicked()), TilesetMaker, SLOT(close()));

    QMetaObject::connectSlotsByName(TilesetMaker);
    } // setupUi

    void retranslateUi(QDialog *TilesetMaker)
    {
    TilesetMaker->setWindowTitle(QApplication::translate("TilesetMaker", "TilesetMaker", 0, QApplication::UnicodeUTF8));
    m_prbTiles->setToolTip(QApplication::translate("TilesetMaker", "If you select this, you will see the image on the right, that's all.", 0, QApplication::UnicodeUTF8));
    m_prbTiles->setStatusTip(QApplication::translate("TilesetMaker", "If you select this, you will see the image on the right, that's all.", 0, QApplication::UnicodeUTF8));
    m_prbTiles->setWhatsThis(QApplication::translate("TilesetMaker", "If you select this, you will see the image on the right, that's all.", 0, QApplication::UnicodeUTF8));
    m_prbTiles->setText(QApplication::translate("TilesetMaker", "&Tiles:", 0, QApplication::UnicodeUTF8));
    m_pbtnAdd->setToolTip(QApplication::translate("TilesetMaker", "This adds a tile to the list of (Lower)tiles.", 0, QApplication::UnicodeUTF8));
    m_pbtnAdd->setStatusTip(QApplication::translate("TilesetMaker", "This adds a tile to the list of (Lower)tiles.", 0, QApplication::UnicodeUTF8));
    m_pbtnAdd->setWhatsThis(QApplication::translate("TilesetMaker", "This adds a tile to the list of (Lower)tiles.", 0, QApplication::UnicodeUTF8));
    m_pbtnAdd->setText(QApplication::translate("TilesetMaker", "&Add Tile", 0, QApplication::UnicodeUTF8));
    m_pbtnRem->setToolTip(QApplication::translate("TilesetMaker", "This removes the currently selected tile from the list of (Lower)tiles.", 0, QApplication::UnicodeUTF8));
    m_pbtnRem->setStatusTip(QApplication::translate("TilesetMaker", "This removes the currently selected tile from the list of (Lower)tiles.", 0, QApplication::UnicodeUTF8));
    m_pbtnRem->setWhatsThis(QApplication::translate("TilesetMaker", "This removes the currently selected tile from the list of (Lower)tiles.", 0, QApplication::UnicodeUTF8));
    m_pbtnRem->setText(QApplication::translate("TilesetMaker", "Remove Tile", 0, QApplication::UnicodeUTF8));
    m_plbTiles->setStatusTip(QApplication::translate("TilesetMaker", "This is the list of Lowertiles.", 0, QApplication::UnicodeUTF8));
    m_plbTiles->setWhatsThis(QApplication::translate("TilesetMaker", "This is the list of Lowertiles.", 0, QApplication::UnicodeUTF8));
    m_pbtnBrowseUppertiles->setToolTip(QApplication::translate("TilesetMaker", "This opens a Dialog that lets you choose a file.", 0, QApplication::UnicodeUTF8));
    m_pbtnBrowseUppertiles->setStatusTip(QApplication::translate("TilesetMaker", "This opens a Dialog that lets you choose a file.", 0, QApplication::UnicodeUTF8));
    m_pbtnBrowseUppertiles->setWhatsThis(QApplication::translate("TilesetMaker", "This opens a Dialog that lets you choose a file.", 0, QApplication::UnicodeUTF8));
    m_pbtnBrowseUppertiles->setText(QApplication::translate("TilesetMaker", "...", 0, QApplication::UnicodeUTF8));
    m_pedUppertiles->setToolTip(QApplication::translate("TilesetMaker", "This is the path to the image file containing all your uppertiles.", 0, QApplication::UnicodeUTF8));
    m_pedUppertiles->setStatusTip(QApplication::translate("TilesetMaker", "This is the path to the image file containing all your uppertiles.", 0, QApplication::UnicodeUTF8));
    m_pedUppertiles->setWhatsThis(QApplication::translate("TilesetMaker", "This is the path to the image file containing all your uppertiles.", 0, QApplication::UnicodeUTF8));
    m_psbUppertilesCount->setToolTip(QApplication::translate("TilesetMaker", "This is how much uppertiles there are in your uppertiles map.", 0, QApplication::UnicodeUTF8));
    m_psbUppertilesCount->setStatusTip(QApplication::translate("TilesetMaker", "This is how much uppertiles there are in your uppertiles map.", 0, QApplication::UnicodeUTF8));
    m_psbUppertilesCount->setWhatsThis(QApplication::translate("TilesetMaker", "This is how much uppertiles there are in your uppertiles map.", 0, QApplication::UnicodeUTF8));
    m_prbUppertiles->setToolTip(QApplication::translate("TilesetMaker", "If you select this, you will see the image on the right, that's all.", 0, QApplication::UnicodeUTF8));
    m_prbUppertiles->setStatusTip(QApplication::translate("TilesetMaker", "If you select this, you will see the image on the right, that's all.", 0, QApplication::UnicodeUTF8));
    m_prbUppertiles->setWhatsThis(QApplication::translate("TilesetMaker", "If you select this, you will see the image on the right, that's all.", 0, QApplication::UnicodeUTF8));
    m_prbUppertiles->setText(QApplication::translate("TilesetMaker", "&Upper Tiles:", 0, QApplication::UnicodeUTF8));
    m_pbtnBrowseBlendmask->setToolTip(QApplication::translate("TilesetMaker", "This opens a Dialog that lets you choose a file.", 0, QApplication::UnicodeUTF8));
    m_pbtnBrowseBlendmask->setStatusTip(QApplication::translate("TilesetMaker", "This opens a Dialog that lets you choose a file.", 0, QApplication::UnicodeUTF8));
    m_pbtnBrowseBlendmask->setWhatsThis(QApplication::translate("TilesetMaker", "This opens a Dialog that lets you choose a file.", 0, QApplication::UnicodeUTF8));
    m_pbtnBrowseBlendmask->setText(QApplication::translate("TilesetMaker", "...", 0, QApplication::UnicodeUTF8));
    m_pedBlendmask->setToolTip(QApplication::translate("TilesetMaker", "This is the path to your blendmask image.", 0, QApplication::UnicodeUTF8));
    m_pedBlendmask->setStatusTip(QApplication::translate("TilesetMaker", "This is the path to your blendmask image.", 0, QApplication::UnicodeUTF8));
    m_pedBlendmask->setWhatsThis(QApplication::translate("TilesetMaker", "This is the path to your blendmask image.", 0, QApplication::UnicodeUTF8));
    m_prbBlendmask->setToolTip(QApplication::translate("TilesetMaker", "If you select this, you will see the image on the right, that's all.", 0, QApplication::UnicodeUTF8));
    m_prbBlendmask->setStatusTip(QApplication::translate("TilesetMaker", "If you select this, you will see the image on the right, that's all.", 0, QApplication::UnicodeUTF8));
    m_prbBlendmask->setWhatsThis(QApplication::translate("TilesetMaker", "If you select this, you will see the image on the right, that's all.", 0, QApplication::UnicodeUTF8));
    m_prbBlendmask->setText(QApplication::translate("TilesetMaker", "&Blendmask:", 0, QApplication::UnicodeUTF8));
    m_pbtnBrowseDetailmap->setToolTip(QApplication::translate("TilesetMaker", "This opens a Dialog that lets you choose a file.", 0, QApplication::UnicodeUTF8));
    m_pbtnBrowseDetailmap->setStatusTip(QApplication::translate("TilesetMaker", "This opens a Dialog that lets you choose a file.", 0, QApplication::UnicodeUTF8));
    m_pbtnBrowseDetailmap->setWhatsThis(QApplication::translate("TilesetMaker", "This opens a Dialog that lets you choose a file.", 0, QApplication::UnicodeUTF8));
    m_pbtnBrowseDetailmap->setText(QApplication::translate("TilesetMaker", "...", 0, QApplication::UnicodeUTF8));
    m_pedDetailmap->setToolTip(QApplication::translate("TilesetMaker", "This is the path to your detailmap image.", 0, QApplication::UnicodeUTF8));
    m_pedDetailmap->setStatusTip(QApplication::translate("TilesetMaker", "This is the path to your detailmap image.", 0, QApplication::UnicodeUTF8));
    m_pedDetailmap->setWhatsThis(QApplication::translate("TilesetMaker", "This is the path to your detailmap image.", 0, QApplication::UnicodeUTF8));
    m_prbDetailmap->setToolTip(QApplication::translate("TilesetMaker", "If you select this, you will see the image on the right, that's all.", 0, QApplication::UnicodeUTF8));
    m_prbDetailmap->setStatusTip(QApplication::translate("TilesetMaker", "If you select this, you will see the image on the right, that's all.", 0, QApplication::UnicodeUTF8));
    m_prbDetailmap->setWhatsThis(QApplication::translate("TilesetMaker", "If you select this, you will see the image on the right, that's all.", 0, QApplication::UnicodeUTF8));
    m_prbDetailmap->setText(QApplication::translate("TilesetMaker", "&Detailmap:", 0, QApplication::UnicodeUTF8));
    m_pimgCurrentPreview->setText(QString());
    m_pbtnCreate->setToolTip(QApplication::translate("TilesetMaker", "This creates the tileset, but it first asks you where to save it.", 0, QApplication::UnicodeUTF8));
    m_pbtnCreate->setStatusTip(QApplication::translate("TilesetMaker", "This creates the tileset, but it first asks you where to save it.", 0, QApplication::UnicodeUTF8));
    m_pbtnCreate->setWhatsThis(QApplication::translate("TilesetMaker", "This creates the tileset, but it first asks you where to save it.", 0, QApplication::UnicodeUTF8));
    m_pbtnCreate->setText(QApplication::translate("TilesetMaker", "&Create the Tileset !", 0, QApplication::UnicodeUTF8));
    m_pbtnQuit->setStatusTip(QApplication::translate("TilesetMaker", "This closes the TilesetMaker without saving your work.", 0, QApplication::UnicodeUTF8));
    m_pbtnQuit->setWhatsThis(QApplication::translate("TilesetMaker", "This closes the TilesetMaker without saving your work.", 0, QApplication::UnicodeUTF8));
    m_pbtnQuit->setText(QApplication::translate("TilesetMaker", "&Quit", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(TilesetMaker);
    } // retranslateUi

};

namespace Ui {
    class TilesetMaker: public Ui_TilesetMaker {};
} // namespace Ui

#endif // UI_MAINDLG_H
