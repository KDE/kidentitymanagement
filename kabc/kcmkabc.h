/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qcombobox.h>
#include <qstringlist.h>
#include <qwidget.h>

#include <kcmodule.h>
#include <klineedit.h>
#include <klistview.h>
#include <kpushbutton.h>

class KAboutData;

#ifndef KCMKABC_H
#define KCMKABC_H

class ConfigPage : public QWidget
{
  Q_OBJECT

public:
  ConfigPage( QWidget *parent = 0, const char *name = 0 );

  void load();
  void save();
  void defaults();
 

public slots:
  void slotAdd();
  void slotRemove();
  void slotEdit();
  void slotConvert();
  void slotStandard();
  void slotSelectionChanged();

signals:
  void changed(bool);

private:
  KConfig *config;

  KListView* mListView;
  QPushButton* mAddButton;
  QPushButton* mRemoveButton;
  QPushButton* mEditButton;
  QPushButton* mConvertButton;
  QPushButton* mStandardButton;

  QListViewItem* mLastItem;
};

class KCMkabc : public KCModule
{
  Q_OBJECT

public:
  KCMkabc( QWidget *parent = 0, const char *name = 0 );

  void load();
  void save();
  void defaults();
  const KAboutData* aboutData() const;


private:
  ConfigPage *mConfigPage;
};

class ConfigViewItem : public QObject, public QCheckListItem
{
  Q_OBJECT

public:
  ConfigViewItem( QListView *parent, QString name, QString type,
      QString identifier = QString::null );

  void setStandard( bool value );
  bool standard();

  QString key;
  QString type;
  bool readOnly;

signals:
  void changed( bool );

protected:
  void stateChange( bool value );

private:
  bool isStandard;
};


#endif
