/***************************************************************************
                                bcunititem.h
                             -------------------
    begin                : Sat Oct 20 2001
    copyright            : (C) 2001, 2002, 2003 by Robby Stephenson
    email                : robby@periapsis.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of version 2 of the GNU General Public License as  *
 *   published by the Free Software Foundation;                            *
 *                                                                         *
 ***************************************************************************/

#ifndef BCUNITITEM_H
#define BCUNITITEM_H

class BCUnit;

#include <klistview.h>

// for QColorGroup
#include <qpalette.h>
//#include <qguardedptr.h>

/**
 * The BCUnitItem is a subclass of KListViewItem containing a pointer to a BCUnit.
 *
 * The unit pointer allows easy access to listview items which refer to a certain unit.
 *
 * @see BCUnit
 *
 * @author Robby Stephenson
 * @version $Id: bcunititem.h 168 2003-10-06 01:20:51Z robby $
 */
class BCUnitItem : public KListViewItem {
public:
  /**
   * This constructor is for items which are direct children of a KListView object.
   *
   * @param parent A pointer to the parent
   * @param unit A pointer to the unit to which the item refers
   */
  BCUnitItem(KListView* parent, BCUnit* unit)
      : KListViewItem(parent), m_unit(unit), m_customSort(parent->isA("BCDetailedListView")) {}
  /**
   * This constructor is for items which have other KListViewItems as parents. It
   * initializes the text in the first column, as well.
   *
   * @param parent A pointer to the parent
   * @param text The text in the first column
   * @param unit A pointer to the unit to which the item refers
   */
  BCUnitItem(KListViewItem* parent, const QString& text, BCUnit* unit)
      : KListViewItem(parent, text), m_unit(unit), m_customSort(false) {}

  /**
   * Compares one column to another, calling @ref BCDetailedListView::isNumber() and
   * using that to determine whether to do numerical or alphabetical comparison.
   *
   * @param item Pointer to comparison item
   * @param col Column to compare
   * @return Comparison result, -1,0, or 1
   */
  int compareColumn(QListViewItem* item, int col) const;
  /**
   * Compares one column to another. If the parent is a @ref BCDetailedListView,
   * it calls @ref compareColumn, otherwise just does default comparison.
   *
   * @param item Pointer to comparison item
   * @param col Column to compare
   * @param ascending Whether ascending or descing comparison, ignored
   * @return Comparison result, -1,0, or 1
   */
  virtual int compare(QListViewItem* item, int col, bool ascending) const;
  /**
   * Returns the key for the list item. The key is just the text, unless there is none,
   * in which case a tab character is returned if there is a non-null pixmap.
   *
   * @param col Column to compare
   * @return The key string
   */
  virtual QString key(int col, bool) const;
  /**
   * Returns a const pointer to the unit to which the item refers
   *
   * @return The unit pointer
   */
  BCUnit* const unit() const { return m_unit; }

private:
  // if I make this a QGuardedPtr, the app crashes, why?
  BCUnit* m_unit;
  // if the parent is a BCDetailedListView
  // this way, I don't have to call listView()->isA("BCDetailedListView") every time
  // when I want to do funky comparisons
  bool m_customSort;
};

/**
 * The ParentItem is a subclass of KListViewItem which includes an id reference number.
 *
 * The id allows for matching on collections or whatever. The id is not a unique identifier
 * of the item itself.
 *
 *
 * @author Robby Stephenson
 * @version $Id: bcunititem.h 168 2003-10-06 01:20:51Z robby $
 */
class ParentItem : public KListViewItem {
public:
  /**
   * This constructor is for items which are direct children of a KListView object.
   *
   * @param parent A pointer to the parent
   * @param text The text in the first column
   * @param id The id number
   */
  ParentItem(KListView* parent, const QString& text, int id)
      : KListViewItem(parent, text), m_id(id) {}
  /**
   * This constructor is for items which are children of another ParentItem and do not
   * have an id reference number. It is primarily used for grouping of the BCUnitItems.
   * The id is set to -1.
   *
   * @see BCUnitItem
   *
   * @param parent A pointer to the parent
   * @param text The text in the first column
   */
  ParentItem(ParentItem* parent, const QString& text)
      : KListViewItem(parent, text), m_id(-1) {}

  /**
   * Sets the count for the number of items.
   *
   * @param c The count
   */
  void setCount(int c) { m_count = c; }
  /**
   * Returns the id reference number of the ParentItem.
   *
   * @return The id number
   */
  int id() const { return m_id; }
  /**
   * Returns the key for sorting the listitems. The text used for an empty
   * value should be sorted first, so the returned key is "_". Since the text may
   * have the number of units or something added to the name, only check if the
   * text begins with the empty name. Maybe there should be something better.
   *
   * @param col The column number
   * @return The key
   */
  virtual QString key(int col, bool) const;

  /**
   * Paints the cell, adding the number count.
   */
  virtual void paintCell(QPainter* p, const QColorGroup& cg,
                         int column, int width, int align);
  virtual int width(const QFontMetrics& fm, const QListView* lv, int c) const;

private:
  int m_id;
  int m_count;

// since I do an expensive RegExp match for the surname prefixes, I want to
// cache the text and the resulting key
  mutable QString m_text;
  mutable QString m_key;
};

#endif
