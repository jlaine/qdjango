TEMPLATE = subdirs

SUBDIRS = db http

lessThan(QT_VERSION, 5.6) {
  SUBDIRS += script
}

CONFIG += ordered
