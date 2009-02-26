/*************************************************************************
    copyright            : (C) 2003-2008 by Robby Stephenson
    email                : robby@periapsis.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of version 2 of the GNU General Public License as  *
 *   published by the Free Software Foundation;                            *
 *                                                                         *
 ***************************************************************************/

#include <config.h>

#include "bibtexmlexporter.h"
#include "bibtexhandler.h"
#include "../document.h"
#include "../collections/bibtexcollection.h"
#include "../filehandler.h"
#include "tellico_xml.h"
#include "../stringset.h"
#include "../tellico_debug.h"

#include <klocale.h>

#include <QDomDocument>
#include <QRegExp>
#include <QTextCodec>

using Tellico::Export::BibtexmlExporter;

QString BibtexmlExporter::formatString() const {
  return i18n("Bibtexml");
}

QString BibtexmlExporter::fileFilter() const {
  return i18n("*.xml|Bibtexml Files (*.xml)") + QLatin1Char('\n') + i18n("*|All Files");
}

bool BibtexmlExporter::exec() {
  Data::CollPtr c = collection();
  if(!c || c->type() != Data::Collection::Bibtex) {
    return false;
  }
  const Data::BibtexCollection* coll = static_cast<const Data::BibtexCollection*>(c.data());

// there are some special fields
// the entry-type specifies the entry type - book, inproceedings, whatever
  QString typeField;
// the key specifies the cite-key
  QString keyField;

  const QString bibtex = QLatin1String("bibtex");
// keep a list of all the 'ordinary' fields to iterate through later
  Data::FieldList fields;
  Data::FieldList vec = coll->fields();
  foreach(Data::FieldPtr it, vec) {
    QString bibtexField = it->property(bibtex);
    if(bibtexField == QLatin1String("entry-type")) {
      typeField = it->name();
    } else if(bibtexField == QLatin1String("key")) {
      keyField = it->name();
    } else if(!bibtexField.isEmpty()) {
      fields.append(it);
    }
  }

  QDomImplementation impl;
  QDomDocumentType doctype = impl.createDocumentType(QLatin1String("file"),
                                                     QString(),
                                                     XML::dtdBibtexml);
  //default namespace
  const QString& ns = XML::nsBibtexml;

  QDomDocument dom = impl.createDocument(ns, QLatin1String("file"), doctype);

  // root element
  QDomElement root = dom.documentElement();

  QString encodeStr = QLatin1String("version=\"1.0\" encoding=\"");
  if(options() & Export::ExportUTF8) {
    encodeStr += QLatin1String("UTF-8");
  } else {
    encodeStr += QLatin1String(QTextCodec::codecForLocale()->name());
  }
  encodeStr += QLatin1Char('"');

  // createDocument creates a root node, insert the processing instruction before it
  dom.insertBefore(dom.createProcessingInstruction(QLatin1String("xml"), encodeStr), root);
  QString comment = QLatin1String("Generated by Tellico ") + QLatin1String(VERSION);
  dom.insertBefore(dom.createComment(comment), root);

  bool format = options() & Export::ExportFormatted;

  StringSet usedKeys;
  QString type, key, newKey, value, elemName, parElemName;
  QDomElement btElem, entryElem, parentElem, fieldElem;
  Data::EntryList entries = this->entries();
  foreach(Data::EntryPtr entryIt, entries) {
    key = entryIt->field(keyField);
    if(key.isEmpty()) {
      key = BibtexHandler::bibtexKey(entryIt);
    }
    QString newKey = key;
    char c = 'a';
    while(usedKeys.has(newKey)) {
      // duplicate found!
      newKey = key + QLatin1Char(c);
      ++c;
    }
    key = newKey;
    usedKeys.add(key);

    btElem = dom.createElement(QLatin1String("entry"));
    btElem.setAttribute(QLatin1String("id"), key);
    root.appendChild(btElem);

    type = entryIt->field(typeField);
    if(type.isEmpty()) {
      kWarning() << "the entry for '" << entryIt->title()
                 << "' has no entry-type, skipping it!" << endl;
      continue;
    }

    entryElem = dom.createElement(type);
    btElem.appendChild(entryElem);

    // now iterate over attributes
    foreach(Data::FieldPtr field, fields) {
      value = entryIt->field(field->name(), format);
      if(value.isEmpty()) {
        continue;
      }

/* Bibtexml has special container elements for titles, authors, editors, and keywords
   I'm going to ignore the titlelist element for right now. All authors are contained in
   an authorlist element, editors in an editorlist element, and keywords are in a
   keywords element, and themselves as a keyword. Also, Bibtexml can format names
   similar to docbook, with first, middle, last, etc elements. I'm going to ignore that
   for now, too.*/
      elemName = field->property(bibtex);
      // split text for author, editor, and keywords
      if(elemName == QLatin1String("author") ||
         elemName == QLatin1String("editor") ||
         elemName == QLatin1String("keywords")) {
        if(elemName == QLatin1String("author")) {
          parElemName = QLatin1String("authorlist");
        } else if(elemName == QLatin1String("editor")) {
          parElemName = QLatin1String("editorlist");
        } else { // keywords
          parElemName = QLatin1String("keywords");
          elemName = QLatin1String("keyword");
        }

        parentElem = dom.createElement(parElemName);
        const QStringList values = entryIt->fields(field->name(), false);
        foreach(const QString& value, values) {
          fieldElem = dom.createElement(elemName);
          fieldElem.appendChild(dom.createTextNode(value));
          parentElem.appendChild(fieldElem);
        }
        if(parentElem.hasChildNodes()) {
          entryElem.appendChild(parentElem);
        }
      } else {
        fieldElem = dom.createElement(elemName);
        fieldElem.appendChild(dom.createTextNode(value));
        entryElem.appendChild(fieldElem);
      }
    }
  }

  return FileHandler::writeTextURL(url(), dom.toString(),
                                   options() & ExportUTF8, options() & Export::ExportForce);
}

#include "bibtexmlexporter.moc"
