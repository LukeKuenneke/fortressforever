////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2005-12-27 08:19:43 -0500 (Tue, 27 Dec 2005) $
// $LastChangedRevision: 1094 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __USERFLAGS_H__
#define __USERFLAGS_H__

#include "Omni-Bot_BasicTypes.h"

class UserFlags32
{
public:
	bool CheckFlag(obint32 _flag) const
	{
		return ((m_Flags & ((obint32)1<<_flag)) == _flag);
	}
	void SetFlag(obint32 _flag)
	{
		m_Flags |= ((obint32)1<<_flag);
	}
	void ClearFlag(obint32 _flag)
	{
		m_Flags &= ~((obint32)1<<_flag);
	}
	void ClearAll()
	{
		m_Flags = 0;
	}
	UserFlags32() : m_Flags(0) {}
private:
	obint32	m_Flags;
	//obint32	m_Persistant;
};

class UserFlags64
{
public:
	bool CheckFlag(obint32 _flag) const
	{
		obint64 flg = (obint64)1<<_flag;
		return ((m_Flags & flg) == flg);
	}
	void SetFlag(obint32 _flag)
	{
		m_Flags |= ((obint64)1<<_flag);
	}
	void ClearFlag(obint32 _flag)
	{
		m_Flags &= ~((obint64)1<<_flag);
	}
	void ClearAll()
	{
		m_Flags = 0;
	}
	UserFlags64() : m_Flags(0) {}
private:
	obint64	m_Flags;
	//obint64	m_Persistant;
};


#endif
