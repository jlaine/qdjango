QDjango - a Qt-based C++ web framework  
Copyright (c) 2010-2015 Jeremy Lainé

[![Build Status](https://travis-ci.org/jlaine/qdjango.png)](https://travis-ci.org/jlaine/qdjango)

About
=====

_QDjango_ is a web framework written in C++ and built on top of the Qt library.
Where possible it tries to follow django's API, hence its name.

It is released under the terms of the GNU Lesser General Public License, version 2.1 or later.

To learn more about _QDjango_, please read [the online documentation](http://doc.qdjango.org/).

Requirements
============

QDjango builds and is auto-tested both with Qt 4 and Qt 5.

Qt 4 on Debian:

    sudo apt-get install libqt4-dev

Qt 5 on Debian:

    sudo apt-get install qtbase5-dev

Qt 4 on Mac OS X:

    sudo port install qt4-mac

Building QDjango
================

    mkdir build
    cd build
    qmake ..
    make

You can pass the following arguments to qmake:

    PREFIX=<prefix>                 to change the install prefix
                                    default:
                                        unix:  /usr/local on unix
                                        other: $$[QT_INSTALL_PREFIX]
    QDJANGO_LIBRARY_TYPE=staticlib  to build a static version of QDjango

Mailing list
============

If you wish to discuss QDjango, you are welcome to join the [QDjango group](http://groups.google.com/group/qdjango).

Notes
======

MSSQL
-----

Fast forward cursors are used by default. This greatly improves performance, and has the added benefit of implicitly converting to a static cursor when it [needs to]( http://technet.microsoft.com/en-us/library/aa172573(v=sql.80).aspx). Unfortunately, this also means that these cursors can block a connection to the server. In order to deal properly with this situation, there are a few requirements:

- Connection pooling must be enabled in your [ODBC manager](http://www.unixodbc.org/doc/conn_pool.html)
- You must enable Multiple Active Result Sets in the QODBC driver using "MARS_Connection=Yes" in the connection string
- You must enable connection pooling in the QODBC driver using the "SQL_ATTR_CONNECTION_POOLING" attribute
