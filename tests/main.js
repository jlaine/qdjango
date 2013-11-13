/*
 * Copyright (C) 2010-2013 Jeremy Lainé
 * Contact: https://github.com/jlaine/qdjango
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

function fail(msg)
{
    print(msg);
    quit();
}

load("qdjango.test");
syncdb();

/* create a user */
u = new User();
u.username = "foouser";
u.password = "foopass";
u.save();

/* create a message */
m = new Message();
m.text = "some message";
m.user_id = u.pk;
m.save();

/* find message */
qs = Message.objects.filter({"pk": 1});
if (qs.size() != 1) fail("Wrong number of messages");
m2 = qs.at(0)
if (m2.text != "some message") fail("Wrong message text");

/* quit */
quit();
