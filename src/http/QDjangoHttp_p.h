/*
 * Copyright (C) 2010-2012 Jeremy Lainé
 * Contact: http://code.google.com/p/qdjango/
 *
 * This file is part of the QDjango Library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifndef QDJANGO_HTTP_P_H
#define QDJANGO_HTTP_P_H

#if defined(QDJANGO_SHARED)
#  if defined(QDJANGO_BUILD)
#    define QDJANGO_EXPORT Q_DECL_EXPORT
#  else
#    define QDJANGO_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define QDJANGO_EXPORT
#endif

#endif
