/***************************************************************************
 *   Copyright (C) 2007 by Pompei2   *
 *   pompei2@gmail.com   *
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

#ifndef TILESETMAKER_H
#define TILESETMAKER_H

#include <QString>
#include <QDialog>
#include <QPixmap>
#include <QDir>
#include "ui_MainDlg.h"

class QProcess;

class TilesetMaker : public QDialog, private Ui::TilesetMaker
{
  Q_OBJECT

public:
  TilesetMaker(QWidget* parent = 0, Qt::WFlags fl = 0 );
  ~TilesetMaker();
  /*$PUBLIC_FUNCTIONS$*/
    bool isTileInList(QString in_sNewName);

public slots:
  /*$PUBLIC_SLOTS$*/
    void closeEvent(QCloseEvent *ev);

    void on_m_pbtnAdd_clicked( void );
    void on_m_pbtnRem_clicked( void );

    void on_m_pbtnBrowseUppertiles_clicked( void );
    void on_m_pbtnBrowseBlendmask_clicked( void );
    void on_m_pbtnBrowseDetailmap_clicked( void );

    void on_m_pbtnCreate_clicked( void );

    void on_m_plbTiles_currentItemChanged(QListWidgetItem *);

    void on_m_prbTiles_toggled(bool in_b);
    void on_m_prbUppertiles_toggled(bool in_b);
    void on_m_prbBlendmask_toggled(bool in_b);
    void on_m_prbDetailmap_toggled(bool in_b);

protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/

protected:
    QPixmap m_Image;
    QDir m_LastDir;
    QDir m_TempDir;
    QProcess *m_pCompressionProcess;

    char m_cArchive;
    char m_cCompression;
};

#endif

