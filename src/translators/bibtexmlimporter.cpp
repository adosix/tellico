/***************************************************************************
                             bibtexmlimporter.h
                             -------------------
    begin                : Sat Aug 2 2003
    copyright            : (C) 2003 by Robby Stephenson
    email                : robby@periapsis.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of version 2 of the GNU General Public License as  *
 *   published by the Free Software Foundation;                            *
 *                                                                         *
 ***************************************************************************/

#include "bibtexmlimporter.h"
#include "bibtexhandler.h"
#include "../collections/bibtexcollection.h"
#include "../collections/bibtexattribute.h"

#include <klocale.h>
#include <kdebug.h>

BibtexmlImporter::BibtexmlImporter(const KURL& url_) : TextImporter(url_), m_coll(0) {
  readDomDocument(text());
}

BCCollection* BibtexmlImporter::collection() {
  if(!m_coll) {
    loadDomDocument();
  }
  return m_coll;
}

void BibtexmlImporter::readDomDocument(const QString& text_) {
  if(text_.isEmpty()) {
    return;
  }

  // Is it XML ?
  if(text_.left(5) != QString::fromLatin1("<?xml")) {
    setStatusMessage(i18n("Bookcase is unable to load the file - %1.").arg(url().fileName()));
    return;
  }

  QString errorMsg;
  int errorLine, errorColumn;
  if(!m_doc.setContent(text_, true, &errorMsg, &errorLine, &errorColumn)) {
    QString str = i18n("Bookcase is unable to load the file - %1.").arg(url().fileName());
    str += i18n("There is an XML parsing error in line %1, column %2.").arg(errorLine).arg(errorColumn);
    str += QString::fromLatin1("\n");
    str += i18n("The error message from Qt is:");
    str += QString::fromLatin1("\n\t") + errorMsg;
    setStatusMessage(str);
    return;
  }
}
void BibtexmlImporter::loadDomDocument() {
  QDomElement root = m_doc.documentElement();
  if(root.tagName() != QString::fromLatin1("file")) {
    setStatusMessage(i18n("Bookcase is unable to load the file - %1.").arg(url().fileName()));
    return;
  }

  QString ns = BibtexHandler::s_bibtexmlNamespace;
  m_coll = new BibtexCollection(true);

  QDomNodeList entryelems = root.elementsByTagNameNS(ns, QString::fromLatin1("entry"));
//  kdDebug() << "BibtexmlImporter::loadDomDocument - found " << entryelems.count() << " entries" << endl;

  unsigned count = entryelems.count();
  for(unsigned j = 0; j < entryelems.count(); ++j) {
    readEntry(entryelems.item(j));

    if(j%s_stepSize == 0) {
      emit signalFractionDone(static_cast<float>(j)/static_cast<float>(count));
    }
  } // end unit loop
}

void BibtexmlImporter::readEntry(const QDomNode& entryNode_) {
  QDomNode node = (QDomNode&)entryNode_;

  BCUnit* unit = new BCUnit(m_coll);

/* The Bibtexml format looks like
  <entry id="...">
   <book>
    <authorlist>
     <author>...</author>
    </authorlist>
    <publisher>...</publisher> */

  QString type = node.firstChild().toElement().tagName();
  unit->setAttribute(QString::fromLatin1("entry-type"), type);
  QString id = node.toElement().attribute(QString::fromLatin1("id"));
  unit->setAttribute(QString::fromLatin1("bibtex-key"), id);

  QString name, value;
  // field values are first child of first child of entry node
  for(QDomNode n = node.firstChild().firstChild(); !n.isNull(); n = n.nextSibling()) {
    // n could be something like authorlist, with multiple authors, or just
    // a plain element with a single text child...
    // second case first
    if(n.firstChild().isText()) {
      name = n.toElement().tagName();
      value = n.toElement().text();
    } else {
      // is either titlelist, authorlist, editorlist, or keywords
      QString parName = n.toElement().tagName();
      if(parName == QString::fromLatin1("titlelist")) {
        for(QDomNode n2 = node.firstChild(); !n2.isNull(); n2 = n2.nextSibling()) {
          name = n2.toElement().tagName();
          value = n2.toElement().text();
          if(!name.isEmpty() && !value.isEmpty()) {
            BibtexHandler::setAttributeValue(unit, name, value.simplifyWhiteSpace());
          }
        }
        name.truncate(0);
        value.truncate(0);
      } else {
        name = n.firstChild().toElement().tagName();
        if(name == QString::fromLatin1("keyword")) {
          name = QString::fromLatin1("keywords");
        }
        value.truncate(0);
        for(QDomNode n2 = n.firstChild(); !n2.isNull(); n2 = n2.nextSibling()) {
          // n2 could have first, middle, lastname elements...
          if(name == QString::fromLatin1("person")) {
            QStringList names;
            names << QString::fromLatin1("initials") << QString::fromLatin1("first")
                  << QString::fromLatin1("middle") << QString::fromLatin1("prelast")
                  << QString::fromLatin1("last") << QString::fromLatin1("lineage");
            for(QStringList::ConstIterator it = names.begin(); it != names.end(); ++it) {
              QDomNodeList list = n2.toElement().elementsByTagName(*it);
              if(list.count() > 1) {
                value += list.item(0).toElement().text();
              }
              if(*it != names.last()) {
                value += QString::fromLatin1(" ");
              }
            }
          }
          for(QDomNode n3 = n2.firstChild(); !n3.isNull(); n3 = n3.nextSibling()) {
            if(n3.isElement()) {
              value += n3.toElement().text();
            } else if(n3.isText()) {
              value += n3.toText().data();
            }
            if(n3 != n2.lastChild()) {
              value += QString::fromLatin1(" ");
            }
          }
          if(n2 != n.lastChild()) {
            value += QString::fromLatin1("; ");
          }
        }
      }
    }
    if(!name.isEmpty() && !value.isEmpty()) {
      BibtexHandler::setAttributeValue(unit, name, value.simplifyWhiteSpace());
    }
  }

  m_coll->addUnit(unit);
}

