#ifndef ELLIPTICS_PYTHON_ELLIPTICS_TIME_HPP
#define ELLIPTICS_PYTHON_ELLIPTICS_TIME_HPP

#include <elliptics/packet.h>

#include <string>

namespace ioremap { namespace elliptics { namespace python {

struct elliptics_time {
	elliptics_time(uint64_t tsec = -1, uint64_t tnsec = -1);

	elliptics_time(const dnet_time &timestamp);

	int cmp_raw(const dnet_time &other) const;

	int cmp(const elliptics_time &other) const;

	void set_tsec(uint64_t tsec);
	uint64_t get_tsec();

	void set_tnsec(uint64_t tnsec);
	uint64_t get_tnsec();

	std::string to_str() const;

	std::string to_repr() const;

	static elliptics_time now();

	dnet_time m_time;
};

void init_elliptcs_time();

} } } // namespace ioremap::elliptics::python

#endif // ELLIPTICS_PYTHON_ELLIPTICS_TIME_HPP
