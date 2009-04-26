/***************************************************************************
    copyright            : (C) 2003-2009 by Robby Stephenson
    email                : robby@periapsis.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of version 2 of the GNU General Public License as  *
 *   published by the Free Software Foundation;                            *
 *                                                                         *
 ***************************************************************************/

#ifndef WINECOLLECTION_H
#define WINECOLLECTION_H

#include "../collection.h"

namespace Tellico {
  namespace Data {

/**
 * A wine collection.
 *
 * @author Robby Stephenson
 */
class WineCollection : public Collection {
Q_OBJECT

public:
  /**
   * The constructor
   *
   * @param title The title of the collection
   */
  WineCollection(bool addDefaultFields, const QString& title = QString());

  virtual Type type() const { return Wine; }

  static FieldList defaultFields();
};

  } // end namespace
} // end namespace
#endif
