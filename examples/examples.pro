TEMPLATE = subdirs
SUBDIRS = http-server

lessThan(QT_VERSION, 5.6) {
  script-console
}
