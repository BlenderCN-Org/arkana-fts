/***************************************************************************
 *   Copyright (C) 2007 by Pompei2                                         *
 *   pompei2@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QMessageBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QInputDialog>
#include <QCloseEvent>
#include <QProcess>
#include <QTextStream>

#include "tilesetmaker.h"

#if WINDOOF
#  define FTS_PACKER_BIN "fts_packer.exe"
#  define FTS_PACKER_CMD FTS_PACKER_BIN
#else
#  define FTS_PACKER_BIN "fts_packer"
#  define FTS_PACKER_CMD "./"FTS_PACKER_BIN
#endif

#define D_CPIO  'c'
#define D_TAR   't'
#define D_NONE  'n'
#define D_GZIP  'g'
#define D_BZIP2 'b'

TilesetMaker::TilesetMaker(QWidget* parent, Qt::WFlags fl)
        : QDialog(parent, fl), Ui::TilesetMaker()
{
    setupUi(this);
    m_LastDir = QDir::home();

    m_TempDir = QDir::tempPath( ) + "TilesetMaker";
    m_TempDir.mkdir( m_TempDir.path( ) );

    // copy the packer exe to a temp dir.
    QFile::copy( FTS_PACKER_BIN, m_TempDir.filePath( FTS_PACKER_BIN ) );

    m_pCompressionProcess = new QProcess( this );
    m_pCompressionProcess->setWorkingDirectory( m_TempDir.path( ) );

    m_cArchive = D_TAR;
    m_cCompression = D_BZIP2;
}

TilesetMaker::~TilesetMaker()
{
}

bool TilesetMaker::isTileInList(QString in_sNewName)
{
    for( int i = 0 ; i < m_plbTiles->count() ; i++ ) {
        if(m_plbTiles->item(i)->text() == in_sNewName)
            return true;
    }

    return false;
}

void TilesetMaker::closeEvent(QCloseEvent *ev)
{
    // delete all temporary files.
    // FIXME: this deleted my whole system because TempDir was empty!!!
//    QString sCmd = "rm -Rf " + m_TempDir.path( ) + "/*";
//    system( sCmd.toAscii( ).constData( ) );

    // Exit.
    ev->accept( );
}

void TilesetMaker::on_m_pbtnAdd_clicked(void)
{
    QListWidgetItem *pItem = NULL;
    QStringList sFiles;
    QFileInfo file;
    sFiles = QFileDialog::getOpenFileNames(this,
                                           tr("Choose one or more Tiles"),
                                           m_LastDir.absolutePath(),
                                           tr("PNG Images (*.png)"));
    if( sFiles.isEmpty( ) )
        return ;

    bool bDo = true, bAsk = true;
    for(int i = 0 ; i < sFiles.size() ; ++i) {
        if(m_plbTiles->count() >= 5 && bAsk) {
            QString sTemp = tr("Warning ! If you add the file ");
            sTemp += sFiles.at(i);
            sTemp += tr(" to the list, there will be more then 5 tiles and"
                        " thus it will be possible to create corrupted maps with this tileset !"
                        " Do you really want to add the file to the tiles list ?");
            switch(QMessageBox::warning(this, tr("Warning: too much tiles"), sTemp,
                                        QMessageBox::Yes | QMessageBox::YesToAll |
                                        QMessageBox::No | QMessageBox::NoToAll, QMessageBox::No)) {
            case QMessageBox::Yes:
                bDo = true;
                bAsk = true;
                break;
            case QMessageBox::YesToAll:
                bDo = true;
                bAsk = false;
                break;
            case QMessageBox::No:
                bDo = false;
                bAsk = true;
                break;
            case QMessageBox::NoToAll:
                bDo = false;
                bAsk = false;
                break;
            default:
                bDo = false;
                bAsk = true;
                break;
            }
        }

        if(bDo) {
            pItem = new QListWidgetItem(m_plbTiles);
            file.setFile(sFiles.at(i));
            QString sItemName = file.baseName();

            // Check if the name is longer then 1 char.
            if(file.baseName().length() > 1 ) {
                QString sNewName;
                bool bOk = false;
                do {
                    sNewName = QInputDialog::getText(this, tr("New tile name"),
                                                     tr("The tile you selected (")
                                                     + sFiles.at(i) + tr(") has a name that is longer"
                                                     " then one character,\nor already exists in the"
                                                     " list of tiles,\nplease enter a new name"
                                                     " that is only one character long or press CANCEL"
                                                     " to ignore this tile:"), QLineEdit::Normal, "", &bOk);
                } while((sNewName.length() != 1 || this->isTileInList(sNewName)) && bOk);

                sItemName = sNewName;
            }

            m_LastDir.setPath(file.path());
            pItem->setText(sItemName);
            pItem->setStatusTip(sFiles.at(i));
            pItem->setIcon(QIcon(sFiles.at(i)));
        }
    }

    m_plbTiles->setCurrentItem(pItem);

    // Update the image.
    this->on_m_prbTiles_toggled(true);
}

void TilesetMaker::on_m_pbtnRem_clicked( void )
{
    QListWidgetItem *pItem = m_plbTiles->takeItem(m_plbTiles->currentRow());

    if(pItem == NULL)
        return ;

    // Update the image.
    this->on_m_prbTiles_toggled(true);
}

void TilesetMaker::on_m_pbtnBrowseUppertiles_clicked( void )
{
    QString sFile = QFileDialog::getOpenFileName(this,
                                                 tr("Choose the Uppertile file"),
                                                 m_LastDir.absolutePath(),
                                                 tr("Images (*.png *.jpg *.jpeg)"));
    if( sFile.isEmpty( ) )
        return ;

    QFileInfo file(sFile);
    m_LastDir.setPath(file.path());

    m_pedUppertiles->setText(sFile);
    m_pedUppertiles->setCursorPosition(m_pedUppertiles->text().length());

    // Update the image.
    if(m_prbUppertiles->isChecked())
        on_m_prbUppertiles_toggled(true);
    else
        m_prbUppertiles->setChecked(true);
}

void TilesetMaker::on_m_pbtnBrowseBlendmask_clicked( void )
{
    QString sFile = QFileDialog::getOpenFileName(this,
                                                 tr("Choose the Blendmask file"),
                                                 m_LastDir.absolutePath(),
                                                 tr("PNG Images (*.png)"));
    if( sFile.isEmpty( ) )
        return ;

    QFileInfo file(sFile);
    m_LastDir.setPath(file.path());

    m_pedBlendmask->setText(sFile);
    m_pedBlendmask->setCursorPosition(m_pedBlendmask->text().length());

    // Update the image.
    if(m_prbBlendmask->isChecked())
        on_m_prbBlendmask_toggled(true);
    else
        m_prbBlendmask->setChecked(true);
}

void TilesetMaker::on_m_pbtnBrowseDetailmap_clicked( void )
{
    QString sFile = QFileDialog::getOpenFileName(this,
                                                 tr("Choose the Detailmap file"),
                                                 m_LastDir.absolutePath(),
                                                 tr("Images (*.png *.jpg *.jpeg)"));
    if( sFile.isEmpty( ) )
        return ;

    QFileInfo file(sFile);
    m_LastDir.setPath(file.path());

    m_pedDetailmap->setText(sFile);
    m_pedDetailmap->setCursorPosition(m_pedDetailmap->text().length());

    // Update the image.
    if(m_prbDetailmap->isChecked())
        on_m_prbDetailmap_toggled(true);
    else
        m_prbDetailmap->setChecked(true);
}

void TilesetMaker::on_m_pbtnCreate_clicked( void )
{
    QString sCommand;
    QStringList sArgs;

    QFileInfo fi;
    QString sOutFile;
    bool bInvalid = true;

    // First check if all fields have been filled in with legal files.
    if(m_plbTiles->count() < 1 ||
       !QFile::exists(m_pedBlendmask->text()) ||
       !QFile::exists(m_pedDetailmap->text()) ||
       !QFile::exists(m_pedUppertiles->text())) {
        QMessageBox::critical(this, tr("Fill in everything !"),
                              tr("You have not filled in everything with valid files,"
                                 " please select all files before creating the tileset !"));
        return ;
    }

    for( int i = 0 ; i < m_plbTiles->count() ; i++ ) {
        if(!QFile::exists(m_plbTiles->item(i)->statusTip())) {
            QMessageBox::critical(this, tr("Fill in everything !"),
                                  tr("The following file does not exist:\n") + m_plbTiles->item(i)->statusTip());
            return;
        }
    }

    do {
        sOutFile = QFileDialog::getSaveFileName( this,
                                                 "Choose a filename to save under",
                                                 m_LastDir.filePath("name.tileset"),
                                                 "Tilesets (*.tileset)" );
        if( sOutFile.isEmpty( ) )
            return ;

        fi.setFile(sOutFile);

        // Check for validity.
        if(fi.fileName().length() > 13) {
            bInvalid = true;
            QMessageBox::critical(this, tr("Wrong filename !"),
                                  tr("The name of the file can't be longer then 5 characters"
                                     " (13 with the extension) !\n"),
                                  QMessageBox::Ok, QMessageBox::Ok);
        } else if(!sOutFile.endsWith(".tileset", Qt::CaseInsensitive)) {
            bInvalid = true;
            QMessageBox::critical(this, tr("Wrong filename !"),
                                  tr("The name of the file must end with \".tileset\" !\n"),
                                  QMessageBox::Ok, QMessageBox::Ok);
        } else {
            bInvalid = false;
        }
    } while(bInvalid);

    // Choose the packing format.
    QStringList sList;
    bool bOk = true;
    sList << tr("cpio");
    sList << tr("tar (recommended)");
    QString s = QInputDialog::getItem( this, tr("Choose the packing format"),
                                       tr("Please choose the format used to pack the files:"),
                                       sList, 1, false, &bOk );
    if(bOk) {
        if(s == tr("tar (recommended)"))
            m_cArchive = D_TAR;
        else
            m_cArchive = D_CPIO;
    }

    // Choose the compression format.
    sList.clear();
    bOk = true;
    sList << tr("none");
    sList << tr("gzip (recommended)");
    sList << tr("bzip 2");
    s = QInputDialog::getItem( this, tr("Choose the compression format"),
                               tr("Please choose the format used to compress the files:"),
                               sList, 1, false, &bOk );
    if(bOk) {
        if(s == tr("none"))
            m_cCompression = D_NONE;
        else if(s == tr("bzip 2"))
            m_cCompression = D_BZIP2;
        else
            m_cCompression = D_GZIP;
    }

    sArgs << "-c" << sOutFile << "-a" << QString(m_cArchive) + QString(m_cCompression);

    // Create the temporary directory with the tileset name.
//     QString sTilesetBasename = fi.fileName().left(fi.fileName().length()-8);
//     QDir dataDir(m_TempDir.filePath(sTilesetBasename));
//     m_TempDir.mkdir( dataDir.path() );

    QString sName = QInputDialog::getText(this, tr("Tileset name"),
                                                tr("Please enter the name of the tileset:"));

    QString sFileContent = "# This file is part of the\n"
                           "#  _____________   ____\n"
                           "#  \\_   _____/  |_/ __\n"
                           "#   |   ___) \\  __\\____\n"
                           "#   |  |     |  |   __ \\\n"
                           "#   \\__|     |__|  ____/\n"
                           "#\n"
                           "# Project wich is distributed under the terms of the GPL license v2.0\n"
                           "# Pompei2\n"
                           "#\n"
                           "\n"
                           "# This file describes a tileset.\n"
                           "# This one is for the old style test-tileset.\n"
                           "#\n"
                           "\n"
                           "# This  is the long name of the tileset as it will been shown"
                           " to the user (e.g. in the editor).\n"
                           "LongName = \"" + sName + "\"\n\n";

    sFileContent += "# A space-separated list of all defined tiles. One tile has to be one char\n"
                    "# and this char will be used in the map to refer to this tile. The tile's texture\n"
                    "# is named [tile_char].png\n"
                    "#\n"
                    "Tiles = \"";
    for( int i = 0 ; i < m_plbTiles->count() ; i++ ) {
        QString sTileName = m_plbTiles->item(i)->text();

        sFileContent += sTileName;
        if(i != m_plbTiles->count() - 1)
            sFileContent += " ";

        // Create file names. Also prepend the currently selected path.
        QString sTmpFile = m_TempDir.filePath( sTileName + ".png" );

        // Copy the file (overwrite if it exists).
        QFile::remove( sTmpFile );
        QFile::copy( m_plbTiles->item(i)->statusTip(), sTmpFile );

        sArgs << sTileName + ".png";
    }
    sFileContent += "\"\n\n";

    sFileContent += "# A space-separated list of all defined blend-textures. One blend-texture has to be one char\n"
                    "# and this char will be used in the map to refer to this blend-texture. The texture is saved\n"
                    "# in a file named [blendtex_char].png\n"
                    "#\n"
                    "# A blend texture is a texture where only the alpha channel is used to blend"
                    " two tiles together.\n"
                    "#\n"
                    "Blends = \"a\"\n\n";

    // Create file names. Also prepend the currently selected path.
    m_TempDir.mkdir("blends");
    m_TempDir.cd("blends");
    QString sTmpFile = m_TempDir.filePath( "a.png" );
    m_TempDir.cdUp();

    // Copy the file (overwrite if it exists).
    QFile::remove( sTmpFile );
    QFile::copy( m_pedBlendmask->text(), sTmpFile );
    sArgs << "blends/a.png";

    fi.setFile(m_pedUppertiles->text());
    sFileContent += "# This is the name of the one big picture that contains all uppercase tiles.\n"
                    "# The names of the uppercase tiles in this picture are single chars beginning\n"
                    "# from the 0x01 an ending with 0xFF. They are counted from the left to the right\n"
                    "# and from top to bottom, this the third tile in the first row is named 0x03\n"
                    "# and the second tile in the second row is named 0x0C if there are 10 tiles per row.\n"
                    "#\n"
                    "UppercaseMap = \"" + fi.fileName() + "\"\n\n";

    // Create file names. Also prepend the currently selected path.
    sTmpFile = m_TempDir.filePath( fi.fileName() );

    // Copy the file (overwrite if it exists).
    QFile::remove( sTmpFile );
    QFile::copy( m_pedUppertiles->text(), sTmpFile );
    sArgs << fi.fileName();

    sFileContent += "# This is the number of uppercase tiles there are in the map.\n"
                    "# This value avoids to read \"empty\" tiles from the map.\n"
                    "#\n"
                    "UppercaseCount = " + QString::number(m_psbUppertilesCount->value()) + "\n\n";

    fi.setFile(m_pedDetailmap->text());
    sFileContent += "# This is the detail map, should be somewhat bigger then the tiles,"
                    " recommended size is 256 or 512.\n"
                    "# It will be blended repeadetly over the map to \"put dirt\" on it"
                    " so it should be very sharp.\n"
                    "#\n"
                    "Detail = \"" + fi.fileName() + "\"\n\n";

    // Create file names. Also prepend the currently selected path.
    sTmpFile = m_TempDir.filePath( fi.fileName() );

    // Copy the file (overwrite if it exists).
    QFile::remove( sTmpFile );
    QFile::copy( m_pedDetailmap->text(), sTmpFile );
    sArgs << fi.fileName();

    // Now write the conf file.
    sTmpFile = m_TempDir.filePath( "info.conf" );
    QFile::remove( sTmpFile );
    QFile f(sTmpFile);
    if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&f);
    out << sFileContent;
    f.flush();
    f.close();

    sArgs << "info.conf";

    m_pCompressionProcess->start( FTS_PACKER_CMD, sArgs );

    QMessageBox::warning( this, tr("DA BlubOrZrz"),
                          tr("I'm doing this, boy.\nCurrently you can't see the progress,"
                             " sorry, this is still on my TODO list ... wait 2-3 sec.\n"
                             "If you think there was an error, you can still look into the file:")
                          + m_TempDir.filePath("error.log"),
                          QMessageBox::Ok, QMessageBox::Ok );
}

void TilesetMaker::on_m_plbTiles_currentItemChanged(QListWidgetItem *)
{
    // Update the image.
    this->on_m_prbTiles_toggled(true);
}

void TilesetMaker::on_m_prbTiles_toggled(bool in_b)
{
    if(in_b) {
        QListWidgetItem *pItem = m_plbTiles->currentItem();
        // Who knows ...
        if(pItem == NULL) {
            return;
        }

        // Update the image.
        m_Image.load(pItem->statusTip());
        m_pimgCurrentPreview->setPixmap(m_Image);
    }
}

void TilesetMaker::on_m_prbUppertiles_toggled(bool in_b)
{
    if(in_b) {
        // Update the image.
        m_Image.load(m_pedUppertiles->text());
        m_pimgCurrentPreview->setPixmap(m_Image);
    }
}

void TilesetMaker::on_m_prbBlendmask_toggled(bool in_b)
{
    if(in_b) {
        // Update the image.
        m_Image.load(m_pedBlendmask->text());
        m_pimgCurrentPreview->setPixmap(m_Image);
    }
}

void TilesetMaker::on_m_prbDetailmap_toggled(bool in_b)
{
    if(in_b) {
        // Update the image.
        m_Image.load(m_pedDetailmap->text());
        m_pimgCurrentPreview->setPixmap(m_Image);
    }
}

/*$SPECIALIZATION$*/


