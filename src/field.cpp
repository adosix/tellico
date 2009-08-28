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

#include "field.h"
#include "fieldformat.h"
#include "tellico_utils.h"
#include "tellico_debug.h"

#include <klocale.h>
#include <kglobal.h>

using Tellico::Data::Field;

// this constructor is for anything but Choice type
Field::Field(const QString& name_, const QString& title_, Type type_/*=Line*/)
    : QSharedData(), m_name(name_), m_title(title_),  m_category(i18n("General")), m_desc(title_),
      m_type(type_), m_flags(0), m_formatFlag(FormatNone) {

  Q_ASSERT(m_type != Choice);
  // a paragraph's category is always its title, along with tables
  if(isSingleCategory()) {
    m_category = m_title;
  }
  switch(m_type) {
    case Table:
    case Table2:
      m_flags = AllowMultiple;
      if(m_type == Table2) {
        m_type = Table;
        setProperty(QLatin1String("columns"), QLatin1String("2"));
      } else {
        setProperty(QLatin1String("columns"), QLatin1String("1"));
      }
      break;
    case Date:  // hidden from user
      m_formatFlag = FormatDate;
      break;
    case Rating:
      setProperty(QLatin1String("minimum"), QLatin1String("1"));
      setProperty(QLatin1String("maximum"), QLatin1String("5"));
      break;
    case ReadOnly:
      m_flags = NoEdit;
      m_type = Line;
      break;
    case Dependent:
      m_flags = Derived;
      m_type = Line;
      break;
    default: // ssshhhhhhhh
      break;
  }
}

// if this constructor is called, the type is necessarily Choice
Field::Field(const QString& name_, const QString& title_, const QStringList& allowed_)
    : QSharedData(), m_name(name_), m_title(title_), m_category(i18n("General")), m_desc(title_),
      m_type(Field::Choice), m_allowed(allowed_), m_flags(0), m_formatFlag(FormatNone) {
}

Field::Field(const Field& field_)
    : QSharedData(field_), m_name(field_.name()), m_title(field_.title()), m_category(field_.category()),
      m_desc(field_.description()), m_type(field_.type()), m_allowed(field_.allowed()),
      m_flags(field_.flags()), m_formatFlag(field_.formatFlag()),
      m_properties(field_.propertyList()) {
}

Field& Field::operator=(const Field& field_) {
  if(this == &field_) return *this;

//  static_cast<QSharedData&>(*this) = static_cast<const QSharedData&>(field_);
  m_name = field_.name();
  m_title = field_.title();
  m_category = field_.category();
  m_desc = field_.description();
  m_type = field_.type();
  m_allowed = field_.allowed();
  m_flags = field_.flags();
  m_formatFlag = field_.formatFlag();
  m_properties = field_.propertyList();
  return *this;
}

Field::~Field() {
}

void Field::setTitle(const QString& title_) {
  m_title = title_;
  if(isSingleCategory()) {
    m_category = title_;
  }
}

void Field::setType(Field::Type type_) {
  m_type = type_;
  if(m_type != Field::Choice) {
    m_allowed = QStringList();
  }
  switch(m_type) {
    case Table:
    case Table2:
      m_flags |= AllowMultiple;
      if(m_type == Table2) {
        m_type = Table;
        setProperty(QLatin1String("columns"), QLatin1String("2"));
      }
      if(property(QLatin1String("columns")).isEmpty()) {
        setProperty(QLatin1String("columns"), QLatin1String("1"));
      }
      break;
    case Date:
      m_formatFlag = FormatDate;
      break;
    case ReadOnly:
      m_flags |= NoEdit;
      m_type = Line;
      break;
    case Dependent:
      m_flags |= Derived;
      m_type = Line;
      break;
    default: // ssshhhhhhhh
      break;
  }
  if(isSingleCategory()) {
    m_category = m_title;
  }
}

void Field::setCategory(const QString& category_) {
  if(!isSingleCategory()) {
    m_category = category_;
  }
}

void Field::setFlags(int flags_) {
  // tables always have multiple allowed
  if(m_type == Table) {
    m_flags = AllowMultiple | flags_;
  } else {
    m_flags = flags_;
  }
}

bool Field::hasFlag(FieldFlag flag_) const {
  return m_flags & flag_;
}

void Field::setFormatFlag(FormatFlag flag_) {
  // Choice and Data fields are not allowed a format
  if(m_type != Choice && m_type != Date) {
    m_formatFlag = flag_;
  }
}

QString Field::defaultValue() const {
  return property(QLatin1String("default"));
}

void Field::setDefaultValue(const QString& value_) {
  if(value_.isEmpty() || m_type != Choice || m_allowed.contains(value_)) {
    setProperty(QLatin1String("default"), value_);
  }
}

bool Field::isSingleCategory() const {
  return (m_type == Para || m_type == Table || m_type == Image);
}

QString Field::format(const QString& value_, FormatFlag flag_) {
  if(value_.isEmpty()) {
    return value_;
  }

  QString text;
  switch(flag_) {
    case FormatTitle:
      text = FieldFormat::title(value_);
      break;
    case FormatName:
      text = FieldFormat::name(value_);
      break;
    case FormatDate:
      text = FieldFormat::date(value_);
      break;
    case FormatPlain:
      text = FieldFormat::capitalize(value_, true /*check config */);
      break;
    case FormatNone:
      text = value_;
      break;
  }
  return text;
}

// if these are changed, then CollectionFieldsDialog should be checked since it
// checks for equality against some of these strings
Field::FieldMap Field::typeMap() {
  FieldMap map;
  map[Field::Line]      = i18n("Simple Text");
  map[Field::Para]      = i18n("Paragraph");
  map[Field::Choice]    = i18n("Choice");
  map[Field::Bool]      = i18n("Checkbox");
  map[Field::Number]    = i18n("Number");
  map[Field::URL]       = i18n("URL");
  map[Field::Table]     = i18n("Table");
  map[Field::Image]     = i18n("Image");
  map[Field::Date]      = i18n("Date");
  map[Field::Rating]    = i18n("Rating");
  return map;
}

// just for formatting's sake
QStringList Field::typeTitles() {
  const FieldMap& map = typeMap();
  QStringList list;
  list.append(map[Field::Line]);
  list.append(map[Field::Para]);
  list.append(map[Field::Choice]);
  list.append(map[Field::Bool]);
  list.append(map[Field::Number]);
  list.append(map[Field::URL]);
  list.append(map[Field::Date]);
  list.append(map[Field::Table]);
  list.append(map[Field::Image]);
  list.append(map[Field::Rating]);
  return list;
}

QStringList Field::split(const QString& string_, bool allowEmpty_) {
  return string_.isEmpty() ? QStringList()
                           : string_.split(FieldFormat::delimiter(), allowEmpty_ ? QString::KeepEmptyParts
                                                                                 : QString::SkipEmptyParts);
}

void Field::addAllowed(const QString& value_) {
  if(m_type != Choice) {
    return;
  }
  if(!m_allowed.contains(value_)) {
    m_allowed += value_;
  }
}

void Field::setProperty(const QString& key_, const QString& value_) {
  if(value_.isEmpty()) {
    m_properties.remove(key_);
  } else {
    m_properties.insert(key_, value_);
  }
}

void Field::setPropertyList(const Tellico::StringMap& props_) {
  m_properties = props_;
}

void Field::convertOldRating(Tellico::Data::FieldPtr field_) {
  if(field_->type() != Data::Field::Choice) {
    return; // nothing to do
  }

  if(field_->name() != QLatin1String("rating")
     && field_->property(QLatin1String("rating")) != QLatin1String("true")) {
    return; // nothing to do
  }

  int min = 10;
  int max = 1;
  bool ok;
  const QStringList& allow = field_->allowed();
  for(QStringList::ConstIterator it = allow.begin(); it != allow.end(); ++it) {
    int n = Tellico::toUInt(*it, &ok);
    if(!ok) {
      return; // no need to convert
    }
    min = qMin(min, n);
    max = qMax(max, n);
  }
  min = qMax(max, 1);
  max = qMin(max, 10);
  if(min >= max) {
    min = 1;
    max = 5;
  }
  field_->setProperty(QLatin1String("minimum"), QString::number(min));
  field_->setProperty(QLatin1String("maximum"), QString::number(max));
  //remove any old property
  field_->setProperty(QLatin1String("rating"), QString());
  field_->setType(Rating);
}
