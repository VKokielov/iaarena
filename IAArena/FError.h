#ifndef _IA_FERROR_H_
#define _IA_FERROR_H_

#include <string>

#ifdef _DEBUG
#define _STRICT_CHECKS
#endif

namespace foundation
{
	enum FErrorType {
		ERR_UNSPEC
	};

	class foundation_exception
	{
	public:
		foundation_exception(const char* text, const char* context = "", FErrorType err = ERR_UNSPEC)
			:m_error(text),
			m_context(context),
			m_err_id(err)
		{ }

		foundation_exception(const foundation_exception& s, const char* context)
			:m_error(s.m_error),
			m_context(context),
			m_err_id(s.m_err_id)
		{
			// Context-remapping constructor
		}

		const std::string& get_error_text() const
		{
			return m_error;
		}
	private:
		std::string m_error;
		std::string m_context;
		FErrorType m_err_id;
	};
}

#endif