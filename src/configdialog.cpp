/***************************************************************************
                              configdialog.cpp
                             -------------------
    begin                : Wed Dec 5 2001
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

#include "configdialog.h"
#include "bcattribute.h"

#include <klineedit.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kconfig.h>

#include <qsize.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <qpixmap.h>
#include <qgrid.h>
#include <qwhatsthis.h>
#include <qregexp.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qpushbutton.h>
#include <qvbox.h>
#include <qhbox.h>

static const int CONFIG_MIN_WIDTH = 600;
static const int CONFIG_MIN_HEIGHT = 420;

ConfigDialog::ConfigDialog(QWidget* parent_, const char* name_/*=0*/)
    : KDialogBase(IconList, i18n("Configure Bookcase"), Ok|Apply|Cancel|Default,
                  Ok, parent_, name_, true, false) {
  setupGeneralPage();
  setupPrintingPage();

  updateGeometry();
  QSize s = sizeHint();
  resize(QMAX(s.width(), CONFIG_MIN_WIDTH),
         QMAX(s.height(), CONFIG_MIN_HEIGHT));
}

void ConfigDialog::slotOk() {
  slotApply();
  accept();
}

void ConfigDialog::slotApply() {
  emit signalConfigChanged();
}

void ConfigDialog::slotDefault() {
  // only change the defaults on the active page
  switch(activePageIndex()) {
    case 0:
      m_cbOpenLastFile->setChecked(true);
      m_cbShowTipDay->setChecked(true);
      m_cbCapitalize->setChecked(true);
      m_cbFormat->setChecked(true);
      m_cbShowCount->setChecked(true);
      m_leArticles->setText(BCAttribute::defaultArticleList().join(QString::fromLatin1(", ")));
      m_leSuffixes->setText(BCAttribute::defaultSuffixList().join(QString::fromLatin1(", ")));
      m_lePrefixes->setText(BCAttribute::defaultSurnamePrefixList().join(QString::fromLatin1(", ")));
      break;

    case 1:
      m_cbPrintHeaders->setChecked(true);
      m_cbPrintFormatted->setChecked(true);
      m_cbPrintGrouped->setChecked(false);
      break;
  }
}

void ConfigDialog::setupGeneralPage() {
  QPixmap pix = KGlobal::iconLoader()->loadIcon(QString::fromLatin1("bookcase"), KIcon::User,
                                                KIcon::SizeMedium);
  QFrame* frame = addPage(i18n("General"), i18n("General Options"), pix);
  QVBoxLayout* l = new QVBoxLayout(frame, KDialog::marginHint(), KDialog::spacingHint());

  m_cbOpenLastFile = new QCheckBox(i18n("Reopen file at startup"), frame);
  QWhatsThis::add(m_cbOpenLastFile, i18n("If checked, the file that was last open "
                                         "will be re-opened at program start-up."));
  l->addWidget(m_cbOpenLastFile);
  m_cbDict.insert(QString::fromLatin1("openLastFile"), m_cbOpenLastFile);

  m_cbShowTipDay = new QCheckBox(i18n("Show \"Tip of the Day\" at startup"), frame);
  QWhatsThis::add(m_cbShowTipDay, i18n("If checked, the \"Tip of the Day\" will be "
                                       "shown at program start-up."));
  l->addWidget(m_cbShowTipDay);
  m_cbDict.insert(QString::fromLatin1("showTipDay"), m_cbShowTipDay);

  m_cbShowCount = new QCheckBox(i18n("Show number of items in group"), frame);
  QWhatsThis::add(m_cbShowCount, i18n("If checked, the number of items in the group "
                                      "will be appended to the group name."));
  l->addWidget(m_cbShowCount);
  m_cbDict.insert(QString::fromLatin1("showCount"), m_cbShowCount);

  QVGroupBox* formatGroup = new QVGroupBox(i18n("Formatting Options"), frame);
  l->addWidget(formatGroup);

  QString restart = QString::fromLatin1(" ") + i18n("(Requires restart)");
  m_cbCapitalize = new QCheckBox(i18n("Auto capitalize titles and names")+restart, formatGroup);
  QWhatsThis::add(m_cbCapitalize, i18n("If checked, titles and names will "
                                       "be automatically capitalized."));
  m_cbDict.insert(QString::fromLatin1("capitalize"), m_cbCapitalize);

  m_cbFormat = new QCheckBox(i18n("Auto format titles and names")+restart, formatGroup);
  QWhatsThis::add(m_cbFormat, i18n("If checked, titles and names will "
                                   "be automatically formatted."));
  m_cbDict.insert(QString::fromLatin1("format"), m_cbFormat);
  connect(m_cbFormat, SIGNAL(toggled(bool)), this, SLOT(slotToggleFormatted(bool)));

  QGrid* g1 = new QGrid(2, formatGroup);
  g1->setSpacing(5);
  
  QLabel* l1 = new QLabel(i18n("Articles:"), g1);
  m_leArticles = new KLineEdit(g1);
  QStringList articles = BCAttribute::articleList();
  if(!articles.isEmpty()) {
    m_leArticles->setText(articles.join(QString::fromLatin1(", ")));
  }
  QWhatsThis::add(l1, i18n("A comma-separated list of words which should be "
                           "considered as articles if they are the first word "
                           "in a title."));
  QWhatsThis::add(m_leArticles, i18n("A comma-separated list of words which should be "
                                     "considered as articles if they are the first word "
                                     "in a title."));

  QStringList suffixes = BCAttribute::suffixList();
  QLabel* l2 = new QLabel(i18n("Personal suffixes:"), g1);
  m_leSuffixes = new KLineEdit(g1);
  if(!suffixes.isEmpty()) {
    m_leSuffixes->setText(suffixes.join(QString::fromLatin1(", ")));
  }
  QWhatsThis::add(l2, i18n("A comma-separated list of suffixes which might "
                           "be used in personal names."));
  QWhatsThis::add(m_leSuffixes, i18n("A comma-separated list of suffixes which might "
                                     "be used in personal names."));

  QStringList prefixes = BCAttribute::surnamePrefixList();
  QLabel* l3 = new QLabel(i18n("Surname prefixes:"), g1);
  m_lePrefixes = new KLineEdit(g1);
  if(!prefixes.isEmpty()) {
    m_lePrefixes->setText(prefixes.join(QString::fromLatin1(", ")));
  }
  QWhatsThis::add(l3, i18n("A comma-separated list of prefixes which might "
                           "be used in surnames."));
  QWhatsThis::add(m_lePrefixes, i18n("A comma-separated list of prefixes which might "
                                     "be used in surnames."));

  // stretch to fill lower area
  l->addStretch(1);
}

void ConfigDialog::setupPrintingPage() {
  QPixmap pix = KGlobal::iconLoader()->loadIcon(QString::fromLatin1("print_printer"),
                                                KIcon::Toolbar,
                                                KIcon::SizeMedium);
  QFrame* frame = addPage(i18n("Printing"), i18n("Printing Options"), pix);
  QVBoxLayout* l = new QVBoxLayout(frame, KDialog::marginHint(), KDialog::spacingHint());
  
  QVGroupBox* formatOptions = new QVGroupBox(i18n("Formatting Options"), frame);
  l->addWidget(formatOptions);

  m_cbPrintFormatted = new QCheckBox(i18n("Format titles and names"), formatOptions);
  QWhatsThis::add(m_cbPrintFormatted, i18n("If checked, titles and names will "
                                           "be automatically formatted."));
  m_cbDict.insert(QString::fromLatin1("printFormatted"), m_cbPrintFormatted);

  m_cbPrintHeaders = new QCheckBox(i18n("Print field headers"), formatOptions);
  QWhatsThis::add(m_cbPrintHeaders, i18n("If checked, the field names will be "
                                         "printed as table headers."));
  m_cbDict.insert(QString::fromLatin1("printHeaders"), m_cbPrintHeaders);

  QHGroupBox* groupOptions = new QHGroupBox(i18n("Grouping Options"), frame);
  l->addWidget(groupOptions);

  m_cbPrintGrouped = new QCheckBox(i18n("Group the entries"), groupOptions);
  QWhatsThis::add(m_cbPrintGrouped, i18n("If checked, the entries will be grouped by "
                                         "the selected field."));
  m_cbDict.insert(QString::fromLatin1("printGrouped"), m_cbPrintGrouped);

  // stretch to fill lower area
  l->addStretch(1);
}

void ConfigDialog::readConfiguration(KConfig* config_) {
  config_->setGroup("TipOfDay");
  bool showTipDay = config_->readBoolEntry("RunOnStart", true);
  m_cbShowTipDay->setChecked(showTipDay);

  config_->setGroup("General Options");
  
  bool openLastFile = config_->readBoolEntry("Reopen Last File", true);
  m_cbOpenLastFile->setChecked(openLastFile);

  bool autoCapitals = config_->readBoolEntry("Auto Capitalization", true);
  m_cbCapitalize->setChecked(autoCapitals);

  bool autoFormat = config_->readBoolEntry("Auto Format", true);
  m_cbFormat->setChecked(autoFormat);
  slotToggleFormatted(autoFormat);

  bool showCount = config_->readBoolEntry("Show Group Count", true);
  m_cbShowCount->setChecked(showCount);

  // PRINTING
  config_->setGroup(QString::fromLatin1("Printing"));

  bool printHeaders = config_->readBoolEntry("Print Field Headers", true);
  m_cbPrintHeaders->setChecked(printHeaders);

  bool printFormatted = config_->readBoolEntry("Print Formatted", true);
  m_cbPrintFormatted->setChecked(printFormatted);

  bool printGrouped = config_->readBoolEntry("Print Grouped", false);
  m_cbPrintGrouped->setChecked(printGrouped);
}

void ConfigDialog::saveConfiguration(KConfig* config_) {
  config_->setGroup("TipOfDay");
  config_->writeEntry("RunOnStart", m_cbShowTipDay->isChecked());

  config_->setGroup("General Options");
  config_->writeEntry("Reopen Last File", m_cbOpenLastFile->isChecked());

  bool autoCapitals = m_cbCapitalize->isChecked();
  config_->writeEntry("Auto Capitalization", autoCapitals);
  // TODO: somehow, this should take immediate effect, but that's complicated
  BCAttribute::setAutoCapitalize(autoCapitals);
  
  bool autoFormat = m_cbFormat->isChecked();
  config_->writeEntry("Auto Format", autoFormat);
  // TODO: somehow, this should take immediate effect, but that's complicated
  BCAttribute::setAutoFormat(autoFormat);

  config_->writeEntry("Show Group Count", m_cbShowCount->isChecked());

  // there might be spaces before or after the commas in the lineedit box
  QString articlesStr = m_leArticles->text().replace(QRegExp(QString::fromLatin1("\\s*,\\s*")),
                                                     QString::fromLatin1(","));
  QStringList articles = QStringList::split(QString::fromLatin1(","), articlesStr, false);
// ok for articles to be empty
  config_->writeEntry("Articles", articles, ',');
  BCAttribute::setArticleList(articles);

  // there might be spaces before or after the commas in the lineedit box
  QString suffixesStr = m_leSuffixes->text().replace(QRegExp(QString::fromLatin1("\\s*,\\s*")),
                                                     QString::fromLatin1(","));
  QStringList suffixes = QStringList::split(QString::fromLatin1(","), suffixesStr, false);
// ok to be empty
  config_->writeEntry("Name Suffixes", suffixes, ',');
  BCAttribute::setSuffixList(suffixes);

  QString prefixesStr = m_lePrefixes->text().replace(QRegExp(QString::fromLatin1("\\s*,\\s*")),
                                                     QString::fromLatin1(","));
  QStringList prefixes = QStringList::split(QString::fromLatin1(","), prefixesStr, false);
  config_->writeEntry("Surname Prefixes", prefixes, ',');
  BCAttribute::setSurnamePrefixList(prefixes);

  config_->setGroup(QString::fromLatin1("Printing"));
  config_->writeEntry("Print Field Headers", m_cbPrintHeaders->isChecked());
  config_->writeEntry("Print Formatted", m_cbPrintFormatted->isChecked());
  config_->writeEntry("Print Grouped", m_cbPrintGrouped->isChecked());

  config_->sync();
}

bool ConfigDialog::configValue(const QString& key_) {
  QCheckBox* cb = m_cbDict.find(key_);
  return cb && cb->isChecked();
}

void ConfigDialog::slotToggleFormatted(bool checked_) {
  m_leArticles->setEnabled(checked_);
  m_leSuffixes->setEnabled(checked_);
  m_lePrefixes->setEnabled(checked_);
}
