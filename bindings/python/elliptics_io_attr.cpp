/*
* 2013+ Copyright (c) Kirill Smorodinnikov <shaitkir@gmail.com>
* All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*/

#include "elliptics_io_attr.h"

#include <boost/python.hpp>

namespace bp = boost::python;

namespace ioremap { namespace elliptics { namespace python {

elliptics_io_attr::elliptics_io_attr()
{
	start		= 0;
	num			= 0;
	user_flags	= 0;
	flags		= 0;
	offset		= 0;
	size		= 0;
}

elliptics_io_attr::elliptics_io_attr(const dnet_io_attr &io)
: parent(io.parent)
, id(io.id)
, time(io.timestamp)
{
	memcpy(static_cast<dnet_io_attr*>(this), &io, sizeof(io));

}

void init_elliptics_io_attr() {
	bp::class_<elliptics_io_attr>(
		    "IoAttr", "IO attributes of operation")
		.def_readwrite("parent", &elliptics_io_attr::parent,
		    "io_attr.parent = elliptics.Id('some key')")
		.def_readwrite("id", &elliptics_io_attr::id,
		    "elliptics.Id of object on which an operation is executed\n\n"
		    "io_attr.id = elliptics.Id('some key')")
		.def_readwrite("timestamp", &elliptics_io_attr::time,
		    "Timestamp of object creation\n\n"
		    "io_attr.timestamp = elliptics.Time.now()")
		.def_readwrite("start", &dnet_io_attr::start,
		    "Used in range request as start and number for LIMIT(start, num)\n"
		    "start also used in cache writes:\n"
		    "it is treated as object lifetime in seconds,\n"
		    "if zero, object is never removed.\n"
		    "When object's lifetime is over\n"
		    "it is removed from cache, but not from disk.\n\n"
		    "io_attr.start = 200")
		.def_readwrite("num", &dnet_io_attr::num,
		    "Used in range request as start and number for LIMIT(start, num)\n\n"
		    "io_attr.num = 5000")
		.def_readwrite("user_flags", &dnet_io_attr::user_flags,
		    "Custom flags which are defined by user\n\n"
		    "io_attr = 123456789")
		.def_readwrite("flags", &dnet_io_attr::flags,
		    "Bit set of elliptics.io_flags\n\n"
		    "io_attr.flags = elliptics.io_flags.cache | elliptics.io_flags.append")
		.def_readwrite("offset", &dnet_io_attr::offset,
		    "Offset operation is performed\n\n"
		    "io_attr.offset = 10")
		.def_readwrite("size", &dnet_io_attr::size,
		    "Size of operation object\n\n"
		    "io_flags.size = len('object data')")
	;
}

} } } // namespace ioremap::elliptics::python
