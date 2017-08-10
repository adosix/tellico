/***************************************************************************
    Copyright (C) 2001-2009 Robby Stephenson <robby@periapsis.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 ***************************************************************************/

#include "detailedentryitemdelegate.h"
#include "../models/models.h"
#include "../tellico_debug.h"

using namespace Tellico;
using Tellico::DetailedEntryItemDelegate;

DetailedEntryItemDelegate::DetailedEntryItemDelegate(QObject* parent_) : QStyledItemDelegate(parent_)
    , m_isLoading(false) {
}

void DetailedEntryItemDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const {
  if(m_isLoading) {
    return;
  }
  m_isLoading = true;
  QStyledItemDelegate::initStyleOption(option, index);

  const int state = index.data(SaveStateRole).toInt();
  if(option && (state == NewState || state == ModifiedState)) {
    option->font.setBold(true);
    option->font.setItalic(true);
  }
  m_isLoading = false;
}
