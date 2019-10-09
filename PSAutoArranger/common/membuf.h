#pragma once

#include <iostream>

struct MemoryBuffer : std::streambuf
{
	char* begin;
	char* end;

	MemoryBuffer(uint8_t* begin, uint8_t* end) :
		begin((char*)begin),
		end((char*)end)
	{
		this->setg((char*)begin, (char*)begin, (char*)end);
		this->setp((char*)begin, (char*)begin, (char*)end);
	}

	virtual pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in) override
	{
		pos_type ret;

		if ((which & std::ios_base::in) > 0)
		{
			if (dir == std::ios_base::cur)
			{
				gbump((int32_t)off);
			}
			else if (dir == std::ios_base::end)
			{
				setg(begin, end + off, end);
			}
			else if (dir == std::ios_base::beg)
			{
				setg(begin, begin + off, end);
			}

			ret = gptr() - eback();
		}

		if ((which & std::ios_base::out) > 0)
		{
			if (dir == std::ios_base::cur)
			{
				pbump((int32_t)off);
			}
			else if (dir == std::ios_base::end)
			{
				setp(begin, end + off, end);
			}
			else if (dir == std::ios_base::beg)
			{
				setp(begin, begin + off, end);
			}

			ret = pptr() - pbase();
		}

		return ret;
	}

	virtual pos_type seekpos(std::streampos pos, std::ios_base::openmode mode) override
	{
		return seekoff(pos - pos_type(off_type(0)), std::ios_base::beg, mode);
	}

	/*size_t size() const { return m_size; }

protected:

	std::streamsize xsputn(const char_type* __s, std::streamsize __n) override
	{
		this->m_size += __n; return __n;
	}

private:

	size_t m_size = 0;*/

};
