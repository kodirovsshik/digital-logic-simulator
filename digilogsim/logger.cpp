
#include "logger.hpp"



void logger_t::add(std::ostream& os)
{
	this->m_files.push_back(std::addressof(os));
}

void logger_t::flush() const noexcept
{
	for (auto& p : this->m_files)
		p->flush();
}
