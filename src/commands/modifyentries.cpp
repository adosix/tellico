/***************************************************************************
    copyright            : (C) 2005 by Robby Stephenson
    email                : robby@periapsis.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of version 2 of the GNU General Public License as  *
 *   published by the Free Software Foundation;                            *
 *                                                                         *
 ***************************************************************************/

#include "modifyentries.h"
#include "../collection.h"
#include "../controller.h"
#include "../tellico_debug.h"

#include <klocale.h>

using Tellico::Command::ModifyEntries;

ModifyEntries::ModifyEntries(Data::Collection* coll_, Data::EntryVec oldEntries_, Data::EntryVec newEntries_)
    : KCommand()
    , m_coll(coll_)
    , m_oldEntries(oldEntries_)
    , m_entries(newEntries_)
{
#ifndef NDEBUG
  if(m_oldEntries.count() != m_entries.count()) {
    kdDebug() << "ModifyEntriesCommand() - unequal number of entries" << endl;
  }
#endif
}

void ModifyEntries::execute() {
  if(!m_coll || m_entries.isEmpty()) {
    return;
  }
  // loans expose a field named "loaned", and the user might modify that without
  // checking in the loan, so verify that. Heavy-handed, yes...
  const QString loaned = QString::fromLatin1("loaned");
  bool hasLoanField = m_coll->hasField(loaned);
  for(Data::EntryVecIt entry = m_entries.begin(); entry != m_entries.end(); ++entry) {
    if(hasLoanField && entry->field(loaned).isEmpty()) {
      Data::EntryVec notLoaned;
      notLoaned.append(entry);
      Controller::self()->slotCheckIn(notLoaned);
    }
    m_coll->updateDicts(entry);
    Controller::self()->modifiedEntry(entry);
  }
}

void ModifyEntries::unexecute() {
  if(!m_coll || m_entries.isEmpty()) {
    return;
  }
  // since things like the detailedlistview and the entryiconview hold pointers to the entries
  // can't just call Controller::modifiedEntry() on the old pointers
  for(size_t i = 0; i < m_entries.count(); ++i) {
    // need to swap entry values, not just pointers
    Data::Entry e(*m_entries.at(i));
    *m_entries.at(i) = *m_oldEntries.at(i);
    *m_oldEntries.at(i) = e;
    m_coll->updateDicts(m_entries.at(i));
    Controller::self()->modifiedEntry(m_entries.at(i));
  }
}

QString ModifyEntries::name() const {
  return m_entries.count() > 1 ? i18n("Save Entries")
                               : i18n("Save (Entry Title)", "Save %1").arg(m_entries.begin()->title());
}
