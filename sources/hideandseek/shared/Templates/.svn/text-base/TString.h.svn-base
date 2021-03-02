/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

// TString.h

/*
==============================================================================

	STRING RELATED FUNCTIONS

==============================================================================
*/

void	Q_snprintfz(char *dest, size_t size, const char *fmt, ...);
void	Q_strcatz(char *dst, const char *src, int dstSize);
size_t	Q_strncpyz(char *dest, const char *src, size_t size);

char	*Q_strlwr(char *s);
char	*Q_strupr(char *s);

#ifdef id386
int __cdecl Q_tolower (int c);
#else // id386
#define Q_tolower(chr) (tolower((chr)))
#endif // id386
#define Q_toupper(chr) (toupper((chr)))

// ===========================================================================

int		Q_WildcardMatch (const char *filter, const char *string, int ignoreCase);
char	*Q_VarArgs (char *format, ...);

/*
==============================================================================

	dynString
 
==============================================================================
*/
// Also try to maintain 16B allocations, even on non-unicode!
#define STRING_GRANULARITY (16)

class dynString : public dynArray<char,STRING_GRANULARITY>
{
public:
	/**
	 * Constructors
	 */
	dynString() { }

	dynString(const dynString &String)
	: dynArray<char,STRING_GRANULARITY>(String.m_numElems)
	{
		if (m_numElems)
		{
			memcpy(m_data, String.m_data, GetUsedSize());
		}
	}

	dynString(const char *String)
	: dynArray<char,STRING_GRANULARITY>(*String ? strlen(String)+1 : 0)
	{
		if (m_numElems)
		{
			Q_strncpyz(m_data, String, GetNum());
		}
	}

	dynString(const int Length, const char *String)
	: dynArray<char,STRING_GRANULARITY>(Length ? Length+1 : 0)
	{
		if (m_numElems)
		{
			Q_strncpyz(&(*this)[0], String, GetNum());
		}
	}

	/**
	 * Destructors
	 */
	~dynString()
	{
		dynArray<char,STRING_GRANULARITY>::Empty();
	}

	/**
	 * Operators
	 */
	inline bool operator!=(const dynString &String) { return (Q_stricmp(**this, *String) != 0); }
	inline const bool operator!=(const dynString &String) const { return (Q_stricmp(**this, *String) != 0); }
	inline bool operator!=(const char *String) { return (Q_stricmp(**this, String) != 0); }
	inline const bool operator!=(const char *String) const { return (Q_stricmp(**this, String) != 0); }
	inline bool operator!=(const char Character) { return (!m_data || m_data[0] != Character || m_data[1] != '\0'); }
	inline const bool operator!=(const char Character) const { return (!m_data || m_data[0] != Character || m_data[1] != '\0'); }

	inline dynString operator +(const char *String) const
	{
		assert(m_numElems >= 0);
		assert(m_maxElems >= m_numElems);

		return dynString(*this) += String;
	}
	inline dynString operator +(const dynString &String) const
	{
		assert(m_numElems >= 0);
		assert(m_maxElems >= m_numElems);

		return operator +(*String);
	}

	inline dynString &operator +=(const char Character)
	{
		assert(m_numElems >= 0);
		assert(m_maxElems >= m_numElems);

		if (Character != '\0')
		{
			if (m_numElems != 0)
			{
				const int Index = m_numElems-1;
				Add(1);

				assert(m_data[Index] == '\0');

				m_data[Index] = Character;
				m_data[Index+1] = '\0';
			}
			else
			{
				Add(2);

				m_data[0] = Character;
				m_data[1] = '\0';
			}
		}

		return *this;
	}
	inline dynString &operator +=(const char *String)
	{
		assert(String);
		assert(m_numElems >= 0);
		assert(m_maxElems >= m_numElems);

		if (*String != '\0')
		{
			if (m_numElems != 0)
			{
				const int Index = m_numElems-1;
				Add(strlen(String));
				Q_strncpyz(&(*this)[Index], String, GetNum()-Index);
			}
			else
			{
				Add(strlen(String)+1);
				Q_strncpyz(&(*this)[0], String, GetNum());
			}
		}
		return *this;
	}
	inline dynString &operator +=(const dynString &String)
	{
		return operator +=(*String);
	}

	inline dynString &operator =(const char Character)
	{
		assert(m_numElems >= 0);
		assert(m_maxElems >= m_numElems);

		if (*this != Character)
		{
			m_numElems = 2;
			ReAllocate(m_numElems);

			assert(m_numElems == 2);

			m_data[0] = Character;
			m_data[1] = '\0';
		}

		return *this;
	}
	inline dynString &operator =(const char *String)
	{
		assert(m_numElems >= 0);
		assert(m_maxElems >= m_numElems);

		if (*this != String)
		{
			m_numElems = *String ? strlen(String)+1 : 0;
			ReAllocate(m_numElems);

			if (m_numElems)
			{
				Q_strncpyz(m_data, String, GetNum());
			}
		}

		return *this;
	}
	inline dynString &operator =(const dynString &String)
	{
		assert(m_numElems >= 0);
		assert(m_maxElems >= m_numElems);

		if (this != &String)
		{
			m_numElems = String.Length()+1;
			ReAllocate(m_numElems);

			if (m_numElems)
			{
				Q_strncpyz(m_data, *String, GetNum());
			}
		}

		return *this;
	}

	inline bool operator ==(const dynString &String) { return (Q_stricmp(**this, *String) == 0); }
	inline const bool operator ==(const dynString &String) const { return (Q_stricmp(**this, *String) == 0); }
	inline bool operator ==(const char *String) { return (Q_stricmp(**this, String) == 0); }
	inline const bool operator ==(const char *String) const { return (Q_stricmp(**this, String) == 0); }
	inline bool operator==(const char Character) { return (m_data && m_data[0] == Character && m_data[1] == '\0'); }
	inline const bool operator==(const char Character) const { return (m_data && m_data[0] == Character && m_data[1] == '\0'); }

	inline const char &operator [](const int Index) const
	{
		assert(Index >= 0);
		assert(Index < m_numElems || (Index == 0 && m_numElems == 0));
		assert(m_maxElems >= m_numElems);

		return ((char*)m_data)[Index];
	}
	inline char &operator [](const int Index)
	{
		assert(Index >= 0);
		assert(Index < m_numElems || (Index == 0 && m_numElems == 0));
		assert(m_maxElems >= m_numElems);

		return ((char*)m_data)[Index];
	}

	inline const char *operator *() const { return Length() ? &(*this)[0] : ""; }

	/**
	 * Functions
	 */
	void Append(const char *String, const int Length)
	{
		assert(String && *String);
		assert(Length > 0);

		Reserve(m_numElems + Length);
		Q_strncpyz((char*)m_data+m_numElems, String, Length);
		m_numElems += Length;
	}

	// Remove from end
	void Backspace(int Num=1)
	{
		if (m_numElems > 0)
		{
			Num = Max<int>(m_numElems-Num-1, 0);
			if (Num > 0)
			{
				m_data[Num] = '\0';
				m_numElems = Num+1;
			}
			else
			{
				m_numElems = 0;
			}

			ReAllocate(m_numElems);
		}
	}

	inline char *Contains(char *String)
	{
		return (Length() > 0) ? strstr(GetTypedData(), String) : NULL;
	}
	inline char *Contains(char Character)
	{
		return (Length() > 0) ? strchr(GetTypedData(), Character) : NULL;
	}

	// Remove from beginning
	void Delete(const int Num=1)
	{
		if (m_numElems > 0)
		{
			const size_t Space = Q_strncpyz(m_data, m_data+Num, m_numElems);
			m_numElems = Max<int>((m_numElems-Space)+1, 0);
			m_data[m_numElems] = '\0';

			ReAllocate(m_numElems);
		}
	}

	inline void Empty(const int Slack=0)
	{
		dynArray<char,STRING_GRANULARITY>::Empty(Slack > 0 ? Slack+1 : Slack);
		if (m_data)
		{
			m_data[0] = '\0';
		}
	}

	const bool EndsWith(const char *String) const
	{
		const int Len = strlen(String);
		if (Len != 0)
		{
			return (Q_strnicmp(GetTypedData()+(Length()-Len), String, Len) == 0);
		}

		return false;
	}
	inline const bool EndsWith(const char Char) const
	{
		return (*GetTypedData()+(Length()-1) == Char);
	}

	inline const char *FindFirst(const char Char) const
	{
		return strchr(m_data, Char);
	}
	inline const char *FindLast(const char Char) const
	{
		return strrchr(m_data, Char);
	}

	inline bool GetBool() const
	{
		if (GetFloat() == 0.0f || !Q_stricmp(m_data, "true"))
			return false;

		return true;
	}
	inline float GetFloat() const
	{
		if (!Length())
			return 0.0f;
		return atof(GetTypedData());
	}
	inline int GetInt() const
	{
		if (!Length())
			return 0;
		return atoi(GetTypedData());
	}

	dynString GetReversed() const
	{
		dynString Result;

		Result.Reserve(GetNum());
		for (int i=Length()-1 ; i>-1 ; i--)
		{
			Result += SubStr(i, 1);
		}

		return Result;
	}

	dynString GetLower() const
	{
		dynString Result(**this);

		Q_strlwr(Result.GetTypedData());

		return Result;
	}
	dynString GetUpper() const
	{
		dynString Result(**this);

		Q_strupr(Result.GetTypedData());

		return Result;
	}

	dynString Left(const int Count) const
	{
		return dynString(Clamp<int>(Count,0,Length()), GetTypedData());
	}
	dynString LeftChop(const int Count) const
	{
		return dynString(Clamp<int>(Length()-Count,0,Length()), GetTypedData());
	}

	inline const size_t Length() const { return GetNum() ? GetNum()-1 : 0; }
	inline const size_t LengthSize() const { return Length() ? Length()*sizeof(char) : 0; }

	dynString Right(const int Count) const
	{
		return dynString(GetTypedData()+(Length()-Clamp<int>(Count,0,Length())));
	}

	void Remove(const int Start, const int Count=1)
	{
		assert(Count >= 1 || (Count == 0 && m_numElems == 0));
		assert(Start <= m_numElems && (Start+Count+1) <= m_numElems);

		if (Count > 0)
		{
			memmove((byte*)m_data + Start*GetElemSize(), (byte*)m_data + (Start+Count)*GetElemSize(), (m_numElems-Start-Count) * GetElemSize());
			m_numElems -= Count;

			if (m_numElems > 0)
			{
				// Resize if possible
				if (m_numElems < m_maxElems)
				{
					ReAllocate(m_numElems);
				}
			}
			else
			{
				ReAllocate(0, false);
			}

			assert(m_numElems >= 0);
		}
	}

	template<int BufferSize>
	int SPrintf(const char *Format, ...)
	{
		char *Buffer = (char*)dAlloca(BufferSize*sizeof(char));
		va_list ArgPtr;

		va_start(ArgPtr, Format);
		int Result = dVsnprintf(Buffer, BufferSize, Format, ArgPtr);
		va_end(ArgPtr);

		Buffer[BufferSize-1] = '\0';

		if (Result > 0)
		{
			m_numElems = strlen(Buffer);
			if (m_numElems > 0)
				m_numElems++;
			ReAllocate(m_numElems);
			Q_strncpyz(m_data, Buffer, GetNum());
		}
		else
		{
			Empty();
			assert(0);
		}

		return Result;
	}

	dynString SubStr(int Start, int Count = INT_MAX) const
	{
		int End = Start+Count;

		const size_t Len = Length();
		Start = Clamp<int>(Start, 0, Len);
		End = Clamp<int>(End, Start, Len);

		return dynString(End-Start, GetTypedData()+Start);
	}

	const bool StartsWith(const char *String) const
	{
		const int Len = strlen(String);
		if (Len != 0)
		{
			return (Q_strnicmp(**this, String, Len) == 0);
		}

		return false;
	}
	const bool StartsWith(const char Char) const
	{
		return (*GetTypedData() == Char);
	}

	void ToLower()
	{
		Q_strlwr(GetTypedData());
	}
	void ToUpper()
	{
		Q_strupr(GetTypedData());
	}

	void TrimLeft(dynString ExtraRemove = "")
	{
		if (Length() > 0)
		{
			const char *Ptr = m_data;

			int NumRemove = 0;
			while (*Ptr<=' ' || ExtraRemove.Contains(*Ptr))
			{
				NumRemove++;
				Ptr++;
			}

			if (NumRemove > 0)
			{
				Remove(0, NumRemove);
			}
		}
	}
	void TrimRight(dynString ExtraRemove = "")
	{
		if (Length() > 0)
		{
			const char *Ptr = (char*)m_data+Length();
			assert(*Ptr == '\0');
			Ptr--;

			int NumRemove = 0;
			while (*Ptr<=' ' || ExtraRemove.Contains(*Ptr))
			{
				NumRemove++;
				Ptr--;
			}

			if (NumRemove > 0)
			{
				Remove(Length()-NumRemove, NumRemove);
			}
		}
	}
	inline void Trim(dynString ExtraRemove = "")
	{
		TrimLeft(ExtraRemove);
		TrimRight(ExtraRemove);
	}
};

template<> class dataType<dynString> : public dataType_Constructed<dynString>
{
public:
	typedef const char *InitType;

	template<typename TRetType>
	static inline TRetType TypeHash(const dynString &Var)
	{
		uint Result = 0;
		const char *String = *Var;
		for ( ; *String ; )
		{
			char Ch = *(String++);
			Result = Result * 33 + tolower(Ch);
		}

		return (Result + (Result>>5));
	}
};
