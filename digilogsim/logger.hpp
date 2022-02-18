
#ifndef _DIGILOG_LOGGER_HPP_
#define _DIGILOG_LOGGER_HPP_


#include <ostream>
#include <vector>


class logger_t
{
private:
	std::vector<std::ostream*> m_files;

public:
	void add(std::ostream&);

	void flush() const noexcept;

	template<class T>
	friend logger_t& operator<<(logger_t&, T&&);
};




template<class T>
inline logger_t& operator<<(logger_t& self, T&& x)
{
	for (auto p : self.m_files)
		*p << x;
	return self;
}

#endif //!_DIGILOG_LOGGER_HPP_
