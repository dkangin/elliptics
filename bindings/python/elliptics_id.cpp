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

#include "elliptics_id.h"

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>

namespace ioremap { namespace elliptics { namespace python {

static void convert_from_list(const bp::list &l, unsigned char *dst, int dlen)
{
	memset(dst, 0, dlen);
	int i = 0;
	for (bp::stl_input_iterator<unsigned char> it(l), end; (it != end) && (i < dlen); ++it) {
		dst[i] = *it;
		++i;
	}
}

static bp::list convert_to_list(const unsigned char *src, unsigned int size)
{
	bp::list result;
	for (unsigned int i = 0; i < size; ++i)
		result.append(src[i]);
	return result;
}

elliptics_id::elliptics_id(								) : key() {}
elliptics_id::elliptics_id(const std::string &remote	) : key(remote) {}
elliptics_id::elliptics_id(const dnet_id &id			) : key(id) {}
elliptics_id::elliptics_id(const dnet_raw_id &id		) : key(id) {}
elliptics_id::elliptics_id(const key &other				) : key(other) {}
elliptics_id::elliptics_id(const elliptics_id &other	) : key(other) {}
elliptics_id::elliptics_id(const bp::list &id,
                           const uint32_t &group_id		) : key() {
	set_id(id);
	set_group_id(group_id);
}

elliptics_id::elliptics_id(const uint8_t *raw_id) {
	dnet_raw_id id;
	memcpy(id.id, raw_id, sizeof(id.id));
	key::set_id(id);
}

bp::list elliptics_id::get_id() const {
	return convert_to_list(id().id, sizeof(id().id));
}

void elliptics_id::set_id(const bp::list &id) {
	dnet_id _id;
	convert_from_list(id, _id.id, sizeof(_id.id));
	key::set_id(_id);
}

uint32_t elliptics_id::group_id() const {
	return id().group_id;
}

void elliptics_id::set_group_id(const uint32_t &group_id) {
	key::set_group_id(group_id);
}

int elliptics_id::cmp(const elliptics_id &other) const {
	return dnet_id_cmp_str(id().id, other.id().id);
}

elliptics_id elliptics_id::convert(const bp::api::object &id) {
	bp::extract<elliptics_id> get_id(id);
	if (get_id.check())
		return get_id();

	bp::extract<std::string> get_string(id);
	if (get_string.check())
		return elliptics_id(get_string());

	PyErr_SetString(PyExc_ValueError, "Couldn't convert id to elliptics id");
	bp::throw_error_already_set();

	return elliptics_id();
}

// Implements __str__ method.
// Always returns printable hex representation of all id bytes
std::string elliptics_id::to_str() const {
	char buffer[2*DNET_ID_SIZE + 1] = {0};
	return std::string(dnet_dump_id_len_raw(id().id, DNET_ID_SIZE, buffer));
}

// Implements __repr__ method.
// Returns group, hex id prefix, and original key string
// (depending on key's previous history, any of those could be zero or empty).
std::string elliptics_id::to_repr() const {
	std::string result("<id: ");
	result += dnet_dump_id_len(&id(), DNET_DUMP_NUM);
	result += ", '";
	result += remote();
	result += "'>";
	return result;
}

struct id_pickle : bp::pickle_suite
{
	static bp::tuple getinitargs(const elliptics_id& id) {
		return getstate(id);
	}

	static bp::tuple getstate(const elliptics_id& id) {
		return bp::make_tuple(id.get_id(), id.group_id());
	}

	static void setstate(elliptics_id& id, bp::tuple state) {
		if (len(state) != 2) {
			PyErr_SetObject(PyExc_ValueError,
				("expected 2-item tuple in call to __setstate__; got %s"
					% state).ptr()
				);
			bp::throw_error_already_set();
		}

		id.set_id(bp::extract<bp::list>(state[0]));
		id.set_group_id(bp::extract<uint32_t>(state[1]));
	}
};

void init_elliptics_id() {
	bp::class_<elliptics_id>(
	    "Id", "elliptics.Id is used as a key for all operations with Elliptics")
		.def(bp::init<bp::list, uint32_t>(bp::args("key", "group_id"),
		     "__init__(key, group_id)"
		     "    Initializes elliptics.Id\n"
		     "    -- key - list of 64 integers from [0, 255] which represents 512 bit key\n"
		     "    -- group_id - Elliptics group identificator from which key is considered\n\n"
		     "    id = elliptics.Id(key=[0] * 64, group_id = 1)"))
		.def(bp::init<std::string>(bp::args("key"),
		     "__init__(key)\n"
		     "    Initializes elliptics.Id\n"
		     "    -- key - string key\n\n"
		     "    id = elliptics.Id(key='some key')"))
		.add_property("id", &elliptics_id::get_id, &elliptics_id::set_id,
		     "Internal representation of the key\n\n"
		     "internal_id = id.id\n"
		     "id.id = [0] * 64")
		.add_property("group_id", &elliptics_id::group_id, &elliptics_id::set_group_id,
		     "Group identificator from which the key is considered\n\n"
		     "group_id = id.group_id\n"
		     "id.group_id = 1")
		.def("__cmp__", &elliptics_id::cmp,
		     "x.__cmp__(y) <==> cmp(x,y)")
		.def("__str__", &elliptics_id::to_str,
		     "x.__str__() <==> str(x)")
		.def_pickle(id_pickle())
		.def("__repr__", &elliptics_id::to_repr,
		     "x.__repr__() <==> repr(x)")
	;
}

} } } // namespace ioremap::elliptics::python
