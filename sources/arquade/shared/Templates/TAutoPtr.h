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

// TAutoPtr.h

/*
==============================================================================

	autoPtr
 
==============================================================================
*/
template<typename TType>
class autoPtr
{
private:
	/**
	 * Data
	 */
	TType *Pointer;

	autoPtr() {}

	template<typename TStructType>
	struct autoPtr_Ref
	{
	private:
		autoPtr_Ref() {}

	public:
		TStructType *Pointer;

		explicit autoPtr_Ref(TStructType *NewPointer)
		: Pointer(NewPointer) {}

		~autoPtr_Ref() {}
	};

public:
	typedef TType ElementType;

	/**
	 * Constructors
	 */
	explicit autoPtr(TType* NewPointer = NULL)
	: Pointer(NewPointer) {}

	// Steal posession
	autoPtr(autoPtr &Other)
	: Pointer(Other.Release()) {}

	// Allow conversions from related types
	template<typename TConstructType>
	autoPtr(autoPtr<TConstructType> &Other)
	: Pointer(Other.Release()) {}

	autoPtr(autoPtr_Ref<TType> Other)
	: Pointer(Other.Pointer) {}

	/**
	 * Destructors
	 */
	~autoPtr()
	{
		delete Pointer;
	}

	/**
	 * Operators
	 */
	// Steal posession
	autoPtr &operator =(autoPtr &Other)
	{
		Reset(Other.Release());
		return *this;
	}

	// Allow conversions from related types
	template<typename TOperatorType>
	autoPtr &operator =(autoPtr<TOperatorType> &Other)
	{
		Reset(Other.Release());
		return *this;
	}

	autoPtr &operator =(autoPtr_Ref<TType> Other)
	{
		if (Other.Pointer != Pointer)
		{
			delete Pointer;
			Pointer = Other.Pointer;
		}
		return *this;
	}

	template<typename TOperatorType>
	operator autoPtr_Ref<TOperatorType>() { return autoPtr_Ref<TOperatorType>(Release()); }

	template<typename TOperatorType>
	operator autoPtr<TOperatorType>() { return autoPtr<TOperatorType>(Release()); }

	// Referencing
	TType &operator *() { return Pointer; }
	TType *operator ->() { return Pointer; }

	/**
	 * Functions
	 */
	inline TType *Get() const { return Pointer; }
	inline TType *Release()
	{
		TType *Temp = Pointer;
		Pointer = NULL;
		return Temp;
	}
	inline void Reset(TType *NewPointer = NULL)
	{
		if (Pointer != NewPointer)
		{
			delete Pointer;
			Pointer = NewPointer;
		}
	}
};
