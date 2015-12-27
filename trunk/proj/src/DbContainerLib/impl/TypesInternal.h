#pragma once
#include "Types.h"
#include <memory>
#include <mutex>

namespace dbc
{
	/*
	Usage:
	class SomeClass
	{
		...
		NONCOPYABLE(SomeClass)
		...
	}
	It will work after any access specifier
	*/
	#define NONCOPYABLE_IMPL(TYPE) explicit TYPE(const TYPE&) = delete; TYPE& operator=(const TYPE&) = delete;
	#define NONCOPYABLE(TYPE) NONCOPYABLE_IMPL(TYPE)

	typedef std::vector<std::string> Strings_vt;

	typedef std::lock_guard<std::mutex> MutexLock;
	typedef std::shared_ptr<MutexLock> MutexLockGuard;
}