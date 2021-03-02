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

//
// string.c
//

#include "shared.h"

/*
============================================================================

	LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/

/*
===============
Q_snprintfz
===============
*/
void Q_snprintfz (char *dest, size_t size, const char *fmt, ...)
{
	if (size) {
		va_list		argptr;

		va_start (argptr, fmt);
		vsnprintf (dest, size, fmt, argptr);
		va_end (argptr);

		dest[size-1] = '\0';
	}
}


/*
===============
Q_strcatz
===============
*/
void Q_strcatz (char *dst, const char *src, int dstSize)
{
	int		len;

	len = strlen (dst);
	if (len >= dstSize) {
		Com_Printf (PRNT_ERROR, "Q_strcatz: already overflowed");
		return;
	}

	Q_strncpyz (dst + len, src, dstSize - len);
}


/*
===============
Q_strncpyz
===============
*/
size_t Q_strncpyz(char *dest, const char *src, size_t size)
{
	if (size)
	{
		while (--size && (*dest++ = *src++)) ;
		*dest = '\0';
	}

	return size;
}


/*
===============
Q_strlwr
===============
*/
char *Q_strlwr(char *s)
{
	char *p;

	if (s)
	{
		for (p=s ; *s ; s++)
			*s = Q_tolower(*s);
		return p;
	}

	return NULL;
}


/*
===============
Q_strupr
===============
*/
char *Q_strupr(char *s)
{
	char *p;

	if (s)
	{
		for (p=s ; *s ; s++)
			*s = Q_toupper(*s);
		return p;
	}

	return NULL;
}


/*
===============
Q_tolower

by R1CH
===============
*/
#ifdef id386
__declspec(naked) int __cdecl Q_tolower (int c)
{
	__asm {
			mov eax, [esp+4]		;get character
			cmp	eax, 5Ah
			ja  short finish1

			cmp	eax, 41h
			jb  short finish1

			or  eax, 00100000b		;to lower (-32)
		finish1:
			ret	
	}
}
#endif // id386

// =========================================================================

/*
============
Q_WildcardMatch

from Q2ICE
============
*/
int Q_WildcardMatch (const char *filter, const char *string, int ignoreCase)
{
	switch (*filter) {
	case '\0':	return !*string;
	case '*':	return Q_WildcardMatch (filter + 1, string, ignoreCase) || *string && Q_WildcardMatch (filter, string + 1, ignoreCase);
	case '?':	return *string && Q_WildcardMatch (filter + 1, string + 1, ignoreCase);
	default:	return (*filter == *string || (ignoreCase && toupper (*filter) == toupper (*string))) && Q_WildcardMatch (filter + 1, string + 1, ignoreCase);
	}
}


/*
============
Q_VarArgs

Does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
============
*/
char *Q_VarArgs (char *format, ...)
{
	va_list		argptr;
	static char	string[2][1024];
	static int	strIndex;

	strIndex ^= 1;

	va_start (argptr, format);
	vsnprintf (string[strIndex], sizeof(string[strIndex]), format, argptr);
	va_end (argptr);

	return string[strIndex];
}


// string class
void String::Initialize (const char *str, int realLen)
{
	if (str == null)
	{
		_array = null;
		_count = 0;
		return;
	}

	int len = (realLen == -1) ? strlen(str) : realLen;

	_array = new char[len + 1];
	memcpy(_array, str, len);
	_array[len] = '\0';
	_count = len + 1;
}

void String::Destroy ()
{
	if (_count == 0)
		return;

	delete[] _array;
	_array = null;
	_count = 0;
}

String::String()
{
	Initialize(null);
}

String::String (nullptr_t)
{
	Initialize(null);
}

String::String(const char *str)
{
	Initialize(str);
}

String::String (const String &str)
{
	Initialize(str._array);
}

String::~String()
{
	Destroy();
}

void String::Clear ()
{
	Destroy();
	Initialize(null);
}

String String::Clone () const
{
	return String(*this);
}

String &String::Concatenate (const char *str, int length)
{
	if (_count == 0 || _array == null)
	{
		Initialize(str, length);
		return *this;
	}

	int len = (length == -1) ? strlen(str) : length;

	int newCount = Count() + len + 1;

	char *oldArray = _array;
	_array = new char[newCount];
	memcpy (_array, oldArray, _count - 1);
	memcpy(_array + _count - 1, str, len);
	_array[newCount - 1] = '\0';
	delete[] oldArray;
	_count = newCount;

	return *this;
}

String &String::Concatenate (const String &str)
{
	return Concatenate(str.CString(), str.Count());
}

String &String::Remove (const int startIndex, const int length)
{
	int realLength = length;

	if (realLength == -1)
		realLength = Count() - startIndex;

	if (startIndex >= (int)Count() || startIndex < 0)
		throw ExceptionIndexOutOfRange(String("startIndex"));
	if ((startIndex + realLength) > (int)Count() || realLength < 0)
		throw ExceptionIndexOutOfRange(String("length"));

	int newCount = Count() - realLength;

	char *oldArray = _array;
	_array = new char[newCount + 1];
	char *left = oldArray;
	int leftCount = startIndex;

	char *right = oldArray + startIndex + realLength;
	int rightCount = (_count - 1) - startIndex - realLength;

	memcpy(_array, left, leftCount);
	memcpy(_array + leftCount, right, rightCount);
	_array[newCount] = 0;

	_count = newCount + 1;

	delete[] oldArray;

	return *this;
}

String String::Substring (const int startIndex, const int length) const
{
	int newStringLength = length;

	if (length == -1)
		newStringLength = Count() - startIndex;

	if (startIndex >= (int)Count() || startIndex < 0)
		throw ExceptionIndexOutOfRange(String("startIndex"));
	if ((startIndex + newStringLength) > (int)Count() || newStringLength < 0)
		throw ExceptionIndexOutOfRange(String("length"));

	String newStr;
	newStr._array = new char[newStringLength + 1];
	memcpy(newStr._array, _array + startIndex, newStringLength);
	newStr._array[newStringLength] = 0;
	newStr._count = newStringLength + 1;

	return newStr;
}

int String::Compare (const char *str, int maxLength) const
{
	if (maxLength == -1)
		maxLength = (((int)strlen(str) > Count()) ? Count() : (int)strlen(str)) + 1;

	return strncmp(_array, str, maxLength);
}

int String::CompareCaseInsensitive (const char *str, int maxLength) const
{
	if (maxLength == -1)
		maxLength = (((int)strlen(str) > Count()) ? Count() : (int)strlen(str)) + 1;

	return _strnicmp(_array, str, maxLength);
}

int String::Compare (const String &str, int maxLength) const
{
	if (maxLength == -1)
		maxLength = ((str.Count() > Count()) ? Count() : str.Count()) + 1;

	return strncmp(_array, str._array, maxLength);
}

int String::CompareCaseInsensitive (const String &str, int maxLength) const
{
	if (maxLength == -1)
		maxLength = ((str.Count() > Count()) ? Count() : str.Count()) + 1;

	return _strnicmp(_array, str._array, maxLength);
}

// FIXME: insensitive versions
int String::IndexOf (char ch, const int offset) const
{
	if (offset > (int)Count() || offset < 0)
		throw ExceptionIndexOutOfRange(String("offset"));

	for (uint32 i = offset; i < Count(); ++i)
	{
		if (_array[i] == ch)
			return i;
	}

	return -1;
}

int String::IndexOf(const String &str, const int offset) const
{
	if (offset > (int)Count() || offset < 0)
		throw ExceptionIndexOutOfRange(String("offset"));

	for (uint32 i = offset; i < Count(); ++i)
	{
		if ((i + str.Count()) > Count())
			break;

		if (strncmp(_array + i, str.CString(), str.Count()) == 0)
			return i;
	}

	return -1;
}

int String::IndexOf(const char *str, const int offset) const
{
	if (offset > (int)Count() || offset < 0)
		throw ExceptionIndexOutOfRange(String("offset"));

	uint32 len = strlen(str);
	for (uint32 i = offset; i < Count(); ++i)
	{
		if ((i + len) > Count())
			break;

		if (strncmp(_array + i, str, len) == 0)
			return i;
	}

	return -1;
}

int String::LastIndexOf (const char ch, const int offset) const
{
	if (offset > (int)Count() || offset < 0)
		throw ExceptionIndexOutOfRange(String("offset"));

	for (uint32 i = Count() - 1 - offset; i >= 0; --i)
	{
		if (_array[i] == ch)
			return i;
	}

	return -1;
}

int String::LastIndexOf(const String &str, const int offset) const
{
	if (offset > (int)Count() || offset < 0)
		throw ExceptionIndexOutOfRange(String("offset"));

	for (int i = Count() - 1 - offset; i >= 0; --i)
	{
		if ((i + str.Count()) > Count())
			continue;

		if (strncmp(_array + i, str.CString(), str.Count()) == 0)
			return i;
	}

	return -1;
}

int String::LastIndexOf(const char *str, const int offset) const
{
	if (offset > (int)Count() || offset < 0)
		throw ExceptionIndexOutOfRange(String("offset"));

	uint32 len = strlen(str);
	for (uint32 i = Count() - 1 - offset; i >= 0; --i)
	{
		if ((i + len) > Count())
			continue;

		if (strncmp(_array + i, str, len) == 0)
			return i;
	}

	return -1;
}

// FIXME: insensitive versions
bool String::Contains (const String &str) const
{
	return IndexOf(str) != -1;
}

bool String::Contains (const char *str) const
{
	return IndexOf(str) != -1;
}

// FIXME: insensitive versions
bool String::EndsWith (const String &str) const
{
	return (strncmp(str._array, (_array + Count()) - str.Count(), str.Count()) == 0);
}

bool String::EndsWith (const char *str) const
{
	return (strncmp(str, (_array + Count()) - strlen(str), strlen(str)) == 0);
}

// FIXME: insensitive versions
bool String::StartsWith (const String &str) const
{
	return (strncmp(str._array, _array, str.Count()) == 0);
}

bool String::StartsWith(const char *str) const
{
	return (strncmp(str, _array, strlen(str)) == 0);
}

String &String::Insert (const char *str, const int position)
{
	if (position >= (int)Count() || position < 0)
		throw ExceptionIndexOutOfRange(String("position"));

	int len = strlen(str);
	int newLength = _count + len;

	char *oldArray = _array;
	_array = new char[newLength];

	char *left = oldArray;
	int leftChars = position;

	char *right = oldArray + position;
	int rightChars = (_count - 1) - position;

	memcpy(_array, left, leftChars);
	memcpy(_array + position, str, len);
	memcpy(_array + len + position, right, rightChars);
	_count += len;
	_array[_count - 1] = 0;

	delete[] oldArray;
	return *this;
}

String &String::Insert (const String &str, const int position)
{
	if (position >= (int)Count() || position < 0)
		throw ExceptionIndexOutOfRange(String("position"));

	int newLength = _count + str.Count();

	char *oldArray = _array;
	_array = new char[newLength];

	char *left = oldArray;
	int leftChars = position;

	char *right = oldArray + position;
	int rightChars = (_count - 1) - position;

	memcpy(_array, left, leftChars);
	memcpy(_array + position, str._array, str.Count());
	memcpy(_array + str.Count() + position, right, rightChars);
	_count += str.Count();
	_array[_count - 1] = 0;

	delete[] oldArray;
	return *this;
}

String &String::Insert (char ch, const int position)
{
	char tempString[2] = { ch, 0 };
	return Insert(tempString, position);
}

/*static*/ String String::Format (const char *format, ...)
{
	static char tempBuffer[8192];
	va_list		argptr;

	va_start (argptr, format);
	vsnprintf (tempBuffer, sizeof(tempBuffer), format, argptr);
	va_end (argptr);

	tempBuffer[sizeof(tempBuffer)-1] = '\0';

	return String(tempBuffer);
}

bool String::IsNullOrEmpty () const
{
	return (_array == null || _count == 0);
}

/*static*/ bool String::IsNullOrEmpty(const String &str)
{
	return str.IsNullOrEmpty();
}

/*static*/ bool String::IsNullOrEmpty (const String *str)
{
	if (str == null)
		return true;

	return str->IsNullOrEmpty();
}

bool String::operator== (const String &right) const
{
	return Compare(right) == 0;
}

bool String::operator== (const char *right) const
{
	return Compare(right) == 0;
}

bool String::operator!= (const String &right) const
{
	return !(*this == right);
}

bool String::operator!= (const char *right) const
{
	return !(*this == right);
}

String &String::operator= (const String &r)
{
	Destroy();

	_count = r._count;

	_array = new char[_count];
	memcpy(_array, r._array, _count);

	return *this;
}

String &String::operator= (const char *r)
{
	Destroy();

	_count = strlen(r) + 1;

	_array = new char[_count];
	memcpy(_array, r, _count - 1);
	_array[_count - 1] = 0;

	return *this;
}

String &String::operator+= (const String &r)
{
	return Concatenate(r);
}

String &String::operator+= (const char *r)
{
	return Concatenate(r);
}

String String::operator+ (const String &r)
{
	return String(*this).Concatenate(r);
}

String String::operator+ (const char *r)
{
	return String(*this).Concatenate(r);
}

String &String::operator+= (const char &ch)
{
	return Concatenate(&ch, 1);
}

String String::operator+ (const char &ch)
{
	return String(*this).Concatenate(&ch, 1);
}

char &String::operator[] (const int &index) const
{
	return _array[index];
}

/*static*/ String String::Join (int count, ...)
{
	va_list ap;
	String newStr;

	va_start(ap, count); //Requires the last fixed parameter (to get the address)

	for (int j = 0; j < count; j++)
		newStr.Concatenate(va_arg(ap, String)); //Requires the type to cast to. Increments ap to the next argument.

	va_end(ap);

	return newStr;
}

/*static*/ String String::Join (const String *list, int count)
{
	String newStr;

	for (int j = 0; ; j++)
	{
		if (count == -1)
		{
			if (&list[j] == null)
				break;
		}
		else
		{
			if (j >= count)
				break;
		}

		newStr.Concatenate(list[j]); //Requires the type to cast to. Increments ap to the next argument.
	}

	return newStr;
}

/*static*/ String String::Join (const TList<String> &list)
{
	return Join(list.Array(), (int)list.Count());
}

String &String::Replace (const char from, const char to)
{
	for (uint32 i = 0; i < Count(); ++i)
	{
		if (_array[i] == from)
			_array[i] = to;
	}

	return *this;
}

String &String::Replace (const String &from, const String &to)
{
	int index = -(int)to.Count();
	while (true)
	{
		int v = IndexOf(from, index + to.Count());

		if (v == -1)
			break;

		Remove(v, from.Count());
		Insert(to, v);
	}

	return *this;
}

String &String::Replace (const char *from, const char *to)
{
	int toCount = strlen(to);
	int fromCount = strlen(from);

	int index = -toCount;
	while (true)
	{
		int v = IndexOf(from, index + toCount);

		if (v == -1)
			break;

		Remove(v, fromCount);
		Insert(to, v);
	}

	return *this;
}

bool String::IsNullOrWhiteSpace () const
{
	if (IsNullOrEmpty())
		return true;

	for (uint32 i = 0; i < Count(); ++i)
		if (!std::isspace(_array[i]))
			return false;

	return true;
}

TList<String> String::Split (const char *characters, int count, bool removeEmpty) const
{
	TList<String> split;

	int lastGot = 0;
	uint32 i = 0;
	while (true)
	{
		if ((i >= Count()) || IsAny(_array[i], characters, count))
		{
			if (lastGot != (i - lastGot) &&
				(i - lastGot) != 0)
			{
				split.Add(Substring(lastGot, i - lastGot));
				lastGot = i + 1;
			}
		}

		if (i >= Count())
			break;

		++i;
	}

	return split;
}

TList<String> String::Split (const TList<char> &characters, bool removeEmpty) const
{
	return Split(characters.Array(), (int)characters.Count(), removeEmpty);
}

const char defaultSplitChars[] = {' ', '\t', '\r', '\n'};
TList<String> String::Split (bool removeEmpty) const
{
	return Split(defaultSplitChars, 3, removeEmpty);
}

TList<char> String::ToCharArray () const
{
	return TList<char> (_array, Count());
}

String &String::ToLower ()
{
	for (uint32 i = 0; i < Count(); ++i)
		_array[i] = std::tolower(_array[i]);

	return *this;
}

String &String::ToUpper ()
{
	for (uint32 i = 0; i < Count(); ++i)
		_array[i] = std::toupper(_array[i]);

	return *this;
}

String &String::Trim (int forceStart, int forceEnd)
{
	if (IsNullOrWhiteSpace())
	{
		Clear();
		return *this;
	}

	int whiteStart = forceStart;
	int whiteEnd = forceEnd;

	if (whiteStart == -1)
		whiteStart = 0;
	if (whiteEnd == -1)
		whiteEnd = Count() - 1;

	if (forceStart == -1 && std::isspace(_array[whiteStart]))
	{
		for (uint32 i = 0; i < Count(); ++i)
		{
			if (!std::isspace(_array[i]))
			{
				whiteStart = i;
				break;
			}
		}
	}

	if (forceEnd == -1 && std::isspace(_array[whiteEnd]))
	{
		for (int i = Count() - 1; i >= 0; --i)
		{
			if (!std::isspace(_array[i]))
			{
				whiteEnd = i;
				break;
			}
		}
	}

	char *oldArray = _array;
	int newCount = whiteEnd - whiteStart + 1;
	_array = new char[newCount + 1];
	memcpy (_array, oldArray + whiteStart, newCount);
	_array[newCount] = 0;
	_count = newCount + 1;
	delete[] oldArray;

	return *this;
}

String &String::TrimStart ()
{
	return Trim(-1, Count() - 1);
}

String &String::TrimEnd ()
{
	return Trim(0, -1);
}

uint32 String::Count() const
{
	return (_count == 0) ? 0 : (_count - 1);
}

char String::At (int index) const
{
	if (index >= (int)Count() || index < 0)
		throw ExceptionIndexOutOfRange(String("index"));

	return _array[index];
}

const char *String::CString() const
{
	return _array;
}

static const String EmptyString ("");
/*static*/ const String &String::Empty()
{
	return EmptyString;
}