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

#include "collectionfactory.h"
#include "collection.h"
#include "tellico_debug.h"

#include <klocale.h>

using Tellico::CollectionFactory;

CollectionFactory::CollectionFactory() {
}

void CollectionFactory::registerFunction(int type_, const QString& typeName_, CREATE_COLL_FN func_) {
  functionRegistry.insert(type_, func_);
  nameRegistry.insert(type_, typeName_);
  titleRegistry.insert(type_, QString());
}

Tellico::Data::CollPtr CollectionFactory::create(int type_, bool addDefaultFields_) const {
  Tellico::Data::CollPtr ptr;
  if(functionRegistry.contains(type_)) {
    ptr = functionRegistry.value(type_)(addDefaultFields_);
  } else {
    myWarning() << "no collection created for type = " << type_ << endl;
    ptr = new Data::Collection(addDefaultFields_);
  }
  return ptr;
}

//static
CollectionFactory& CollectionFactory::self() {
  static CollectionFactory instance;
  return instance;
}

// static
Tellico::Data::CollPtr CollectionFactory::collection(int type_, bool addDefaultFields_) {
  return self().create(type_, addDefaultFields_);
}

// static
Tellico::Data::CollPtr CollectionFactory::collection(const QString& typeName_, bool addDefaultFields_) {
  Data::CollPtr coll;
  int type = -1;
  TypeStringHash::const_iterator i = self().nameRegistry.constBegin();
  while(i != self().nameRegistry.constEnd()) {
    if(i.value() == typeName_) {
      type = i.key();
      break;
    }
  }
  if(type == -1) {
    type = Data::Collection::Base;
    myWarning() << "bad collection type name:" << typeName_ << endl;
  }
  return self().create(type, addDefaultFields_);
}

Tellico::CollectionNameHash CollectionFactory::nameHash() {
  CollectionNameHash hash;
  hash[Data::Collection::Book]        = i18n("Book Collection");
  hash[Data::Collection::Bibtex]      = i18n("Bibliography");
  hash[Data::Collection::ComicBook]   = i18n("Comic Book Collection");
  hash[Data::Collection::Video]       = i18n("Video Collection");
  hash[Data::Collection::Album]       = i18n("Music Collection");
  hash[Data::Collection::Coin]        = i18n("Coin Collection");
  hash[Data::Collection::Stamp]       = i18n("Stamp Collection");
  hash[Data::Collection::Wine]        = i18n("Wine Collection");
  hash[Data::Collection::Card]        = i18n("Card Collection");
  hash[Data::Collection::Game]        = i18n("Game Collection");
  hash[Data::Collection::File]        = i18n("File Catalog");
  hash[Data::Collection::BoardGame]   = i18n("Board Game Collection");
  hash[Data::Collection::Base]        = i18n("Custom Collection");
  return hash;
}

QString CollectionFactory::typeName(int type_) {
  if(self().nameRegistry.contains(type_)) {
    return self().nameRegistry.value(type_);
  }
  myWarning() << "collection type not implemented:" << type_;
  return QLatin1String("entry");
}

bool CollectionFactory::isDefaultField(int type_, const QString& name_) {
  Data::CollPtr coll = collection(type_, true);
  foreach(Data::FieldPtr field, coll->fields()) {
    if(field->name() == name_) {
      return true;
    }
  }
  return false;
}
