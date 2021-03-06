/***************************************************************************
    Copyright (C) 2005-2009 Robby Stephenson <robby@periapsis.org>
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

#include "datefieldwidget.h"
#include "datewidget.h"
#include "../field.h"

using Tellico::GUI::DateFieldWidget;

DateFieldWidget::DateFieldWidget(Tellico::Data::FieldPtr field_, QWidget* parent_)
    : FieldWidget(field_, parent_) {

  m_widget = new DateWidget(this);
  connect(m_widget, &DateWidget::signalModified, this, &DateFieldWidget::checkModified);

  registerWidget();
}

QString DateFieldWidget::text() const {
  return m_widget->text();
}

void DateFieldWidget::setTextImpl(const QString& text_) {
  m_widget->setDate(text_);
}

void DateFieldWidget::clearImpl() {
  m_widget->clear();
  editMultiple(false);
}

QWidget* DateFieldWidget::widget() {
  return m_widget;
}
