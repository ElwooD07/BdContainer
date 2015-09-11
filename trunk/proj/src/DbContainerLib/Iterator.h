#pragma once

namespace dbc
{
	template<class T>
	class Iterator
	{
	public:
		Iterator()
			: m_current(0)
			, m_size(0)
		{ }

		virtual bool HasNext() const
		{
			return m_current + 1 <= m_size;
		}

		virtual T Next() = 0;

		virtual void Rewind()
		{
			m_current = 0;
		}

		virtual bool Empty() const
		{
			return m_size == 0;
		}

		virtual size_t Count() const
		{
			return m_size;
		}

	protected:
		size_t m_current;
		size_t m_size;
	};
}