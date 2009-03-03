/***************************************************************************
    copyright            : (C) 2005-2008 by Robby Stephenson
    email                : robby@periapsis.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of version 2 of the GNU General Public License as  *
 *   published by the Free Software Foundation;                            *
 *                                                                         *
 ***************************************************************************/

#include "filelistingimporter.h"
#include "../collections/filecatalog.h"
#include "../entry.h"
#include "../field.h"
#include "../imagefactory.h"
#include "../tellico_utils.h"
#include "../tellico_kernel.h"
#include "../progressmanager.h"
#include "../core/netaccess.h"
#include "../tellico_debug.h"

#include <kapplication.h>
#include <kmountpoint.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kde_file.h>

#include <QCheckBox>
#include <QGroupBox>
#include <QFile>
#include <QFileInfo>
#include <QVBoxLayout>

namespace {
  static const int FILE_PREVIEW_SIZE = 128;
  // volume name starts at 16*2048+40 bytes into the header
  static const size_t VOLUME_NAME_POS = 32808;
  static const size_t VOLUME_NAME_SIZE = 32;
}

using Tellico::Import::FileListingImporter;

FileListingImporter::FileListingImporter(const KUrl& url_) : Importer(url_), m_coll(0), m_widget(0),
    m_job(0), m_cancelled(false) {
}

bool FileListingImporter::canImport(int type) const {
  return type == Data::Collection::File;
}

Tellico::Data::CollPtr FileListingImporter::collection() {
  if(m_coll) {
    return m_coll;
  }

  ProgressItem& item = ProgressManager::self()->newProgressItem(this, i18n("Scanning files..."), true);
  item.setTotalSteps(100);
  connect(&item, SIGNAL(signalCancelled(ProgressItem*)), SLOT(slotCancel()));
  ProgressItem::Done done(this);

  // going to assume only one volume will ever be imported
  QString volume = volumeName();

  m_job = m_recursive->isChecked()
          ? KIO::listRecursive(url(), KIO::DefaultFlags, false)
          : KIO::listDir(url(), KIO::DefaultFlags, false);
  connect(m_job, SIGNAL(entries(KIO::Job*, const KIO::UDSEntryList&)),
          SLOT(slotEntries(KIO::Job*, const KIO::UDSEntryList&)));

  if(!KIO::NetAccess::synchronousRun(m_job, Kernel::self()->widget()) || m_cancelled) {
    return Data::CollPtr();
  }

  const bool usePreview = m_filePreview->isChecked();

  const QString title    = QLatin1String("title");
  const QString url      = QLatin1String("url");
  const QString desc     = QLatin1String("description");
  const QString vol      = QLatin1String("volume");
  const QString folder   = QLatin1String("folder");
  const QString type     = QLatin1String("mimetype");
  const QString size     = QLatin1String("size");
  const QString perm     = QLatin1String("permissions");
  const QString owner    = QLatin1String("owner");
  const QString group    = QLatin1String("group");
  const QString created  = QLatin1String("created");
  const QString modified = QLatin1String("modified");
  const QString metainfo = QLatin1String("metainfo");
  const QString icon     = QLatin1String("icon");

  m_coll = new Data::FileCatalog(true);
  QString tmp;
  const uint stepSize = qMax(1, m_files.count()/100);
  const bool showProgress = options() & ImportProgress;

  item.setTotalSteps(m_files.count());
  uint j = 0;
  foreach(const KFileItem& item, m_files) {
    if(m_cancelled) {
      break;
    }

    Data::EntryPtr entry(new Data::Entry(m_coll));

    const KUrl u = item.url();
    entry->setField(title,  u.fileName());
    entry->setField(url,    u.url());
    entry->setField(desc,   item.mimeComment());
    entry->setField(vol,    volume);
    tmp = KUrl::relativePath(this->url().path(), u.directory());
    // remove "./" from the string
    entry->setField(folder, tmp.right(tmp.length()-2));
    entry->setField(type,   item.mimetype());
    entry->setField(size,   KIO::convertSize(item.size()));
    entry->setField(perm,   item.permissionsString());
    entry->setField(owner,  item.user());
    entry->setField(group,  item.group());

    KDateTime dt = item.time(KFileItem::CreationTime);
    if(!dt.isNull()) {
      entry->setField(created, dt.toString(KDateTime::ISODate));
    }
    dt = item.time(KFileItem::ModificationTime);
    if(!dt.isNull()) {
      entry->setField(modified, dt.toString(KDateTime::ISODate));
    }
    const KFileMetaInfo& meta = item.metaInfo();
    if(meta.isValid()) {
      const QStringList keys = meta.supportedKeys();
      QStringList strings;
      foreach(const QString& key, keys) {
        KFileMetaInfoItem item = meta.item(key);
        if(item.isValid()) {
          QString s = item.value().toString();
          if(!s.isEmpty()) {
            strings << item.name() + QLatin1String("::") + s;
          }
        }
      }
      entry->setField(metainfo, strings.join(QLatin1String("; ")));
    }

    if(!m_cancelled && usePreview) {
      m_pixmap = Tellico::NetAccess::filePreview(item, FILE_PREVIEW_SIZE);
      if(m_pixmap.isNull()) {
        m_pixmap = item.pixmap(0);
      }
    } else {
      m_pixmap = item.pixmap(0);
    }

    if(!m_pixmap.isNull()) {
      // is png best option?
      QString id = ImageFactory::addImage(m_pixmap, QLatin1String("PNG"));
      if(!id.isEmpty()) {
        entry->setField(icon, id);
      }
    }

    m_coll->addEntries(entry);

    if(showProgress && j%stepSize == 0) {
      ProgressManager::self()->setProgress(this, j);
      kapp->processEvents();
    }
    ++j;
  }

  if(m_cancelled) {
    m_coll = Data::CollPtr();
    return m_coll;
  }

  return m_coll;
}

QWidget* FileListingImporter::widget(QWidget* parent_) {
  if(m_widget) {
    return m_widget;
  }

  m_widget = new QWidget(parent_);
  QVBoxLayout* l = new QVBoxLayout(m_widget);

  QGroupBox* gbox = new QGroupBox(i18n("File Listing Options"), m_widget);
  QVBoxLayout* vlay = new QVBoxLayout(gbox);

  m_recursive = new QCheckBox(i18n("Recursive folder search"), gbox);
  m_recursive->setWhatsThis(i18n("If checked, folders are recursively searched for all files."));
  // by default, make it checked
  m_recursive->setChecked(true);

  m_filePreview = new QCheckBox(i18n("Generate file previews"), gbox);
  m_filePreview->setWhatsThis(i18n("If checked, previews of the file contents are generated, which can slow down "
                                      "the folder listing."));
  // by default, make it no previews
  m_filePreview->setChecked(false);

  vlay->addWidget(m_recursive);
  vlay->addWidget(m_filePreview);

  l->addWidget(gbox);
  l->addStretch(1);
  return m_widget;
}

void FileListingImporter::slotEntries(KIO::Job* job_, const KIO::UDSEntryList& list_) {
  if(m_cancelled) {
    job_->kill();
    m_job = 0;
    return;
  }

  for(KIO::UDSEntryList::ConstIterator it = list_.begin(); it != list_.end(); ++it) {
    KFileItem item(*it, url(), false, true);
    if(item.isFile()) {
      m_files.append(item);
    }
  }
}

QString FileListingImporter::volumeName() const {
  // this functions turns /media/cdrom into /dev/hdc, then reads 32 bytes after the 16 x 2048 header
  QString volume;
  const KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::NeedRealDeviceName);
  foreach(const KSharedPtr<KMountPoint>& mountPoint, mountPoints) {
    // path() could be /media/cdrom
    // which could be the mount point of the device
    // I know it works for iso9660 (cdrom) and udf (dvd)
    if(url().path() == mountPoint->mountPoint()
       && (mountPoint->mountType() == QLatin1String("iso9660")
           || mountPoint->mountType() == QLatin1String("udf"))) {
      volume = mountPoint->mountPoint();
      if(!mountPoint->realDeviceName().isEmpty()) {
        QString devName = mountPoint->realDeviceName();
        if(devName.endsWith(QLatin1Char('/'))) {
          devName.truncate(devName.length()-1);
        }
        // QFile can't do a sequential seek, and I don't want to do a 32808x loop on getch()
        FILE* dev = 0;
        if((dev = fopen(devName.toLatin1(), "rb")) != 0) {
          // returns 0 on success
          if(KDE_fseek(dev, VOLUME_NAME_POS, SEEK_SET) == 0) {
            char buf[VOLUME_NAME_SIZE];
            size_t ret = fread(buf, 1, VOLUME_NAME_SIZE, dev);
            if(ret == VOLUME_NAME_SIZE) {
              volume = QString::fromLatin1(buf, VOLUME_NAME_SIZE).trimmed();
            }
          } else {
            myDebug() << "can't seek " << devName;
          }
          fclose(dev);
        } else {
          myDebug() << "can't read " << devName;
        }
      }
      break;
    }
  }
  return volume;
}

void FileListingImporter::slotCancel() {
  m_cancelled = true;
  if(m_job) {
    m_job->kill();
  }
}

#include "filelistingimporter.moc"
