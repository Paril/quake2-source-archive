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

// TArray.h

#define dAlloca alloca

/*
==============================================================================

	dynArray
 
==============================================================================
*/

// Try to maintain a minimum alignment of 16B
#define ARRAY_GRANULARITY ((sizeof(TType)==1) ? 16 : 8)

template<typename TType, const int TGranularity = ARRAY_GRANULARITY>
class dynArray
{
protected:
	/**
	 * Data
	 */
	int m_maxElems;
	int m_numElems;
	TType *m_data;

public:
	/**
	 * Constructors
	 */
	dynArray()
	: m_data(NULL)
	, m_numElems(0)
	, m_maxElems(0)
	{
	}

protected:
	dynArray(const int Num)
	: m_data(NULL)
	, m_maxElems(0)
	, m_numElems(Num)
	{
		ReAllocate(m_numElems);
	}

	dynArray(const size_t Num)
	: m_data(NULL)
	, m_maxElems(0)
	, m_numElems(Num)
	{
		ReAllocate(m_numElems);
	}

public:
	dynArray(const dynArray<TType,TGranularity> &Source)
	: m_data(NULL)
	, m_numElems(0)
	, m_maxElems(0)
	{
		Copy(Source);
	}

	/**
	 * Destructors
	 */
	~dynArray()
	{
		assert(m_numElems >= 0);
		assert(m_maxElems >= m_numElems);

		Empty();
	}

	/**
	 * Operators
	 */
	inline const bool operator !=(const dynArray<TType,TGranularity> &Array)
	{
		if (m_numElems == Array.m_numElems)
		{
			for (int i=0 ; i<m_numElems ; i++)
			{
				if (!((*this)[i] == Array[i]))
					return true;
			}

			return false;
		}

		return true;
	}

	inline dynArray<TType> &operator +=(const dynArray<TType,TGranularity> &Source)
	{
		Append(Source);
		return *this;
	}

	inline dynArray<TType> &operator =(const dynArray<TType,TGranularity> &Source)
	{
		Copy(Source);
		return *this;
	}

	inline const bool operator ==(const dynArray<TType,TGranularity> &Array) const
	{
		if (m_numElems == Array.m_numElems)
		{
			for (int i=0 ; i<m_numElems ; i++)
			{
				if (!((*this)[i] == Array[i]))
					return false;
			}

			return true;
		}

		return false;
	}

	inline const TType &operator [](const int Index) const
	{
		assert(Index >= 0);
		assert(Index < m_numElems || (Index == 0 && m_numElems == 0));
		assert(m_maxElems >= m_numElems);

		return m_data[Index];
	}
	inline TType &operator [](const int Index)
	{
		assert(Index >= 0);
		assert(Index < m_numElems || (Index == 0 && m_numElems == 0));
		assert(m_maxElems >= m_numElems);

		return m_data[Index];
	}

	/**
	 * Item functions
	 */
	int AddItem(const TType &Item)
	{
		new(*this) TType(Item);
		return m_numElems-1;
	}

	int AddUniqueItem(const TType &Item)
	{
		for (int i=0 ; i<m_numElems ; i++)
		{
			if ((*this)[i] == Item)
				return i;
		}

		return AddItem(Item);
	}

	inline const bool ContainsItem(const TType &Item) const
	{
		return (FindItemIndex(Item) != -1);
	}

	const bool FindItem(const TType &Item, int &OutIndex) const
	{
		for (OutIndex=0 ; OutIndex<m_numElems ; OutIndex++)
		{
			if ((*this)[OutIndex] == Item)
				return true;
		}

		OutIndex = -1;
		return false;
	}

	const int FindItemIndex(const TType &Item) const
	{
		for (int Result=0 ; Result<m_numElems ; Result++)
		{
			if ((*this)[Result] == Item)
				return Result;
		}

		return -1;
	}

	int InsertItem(const TType &Item, const int Index)
	{
		new(*this, Index) TType(Item);
		return Index;
	}

	void RemoveItem(const TType &Item)
	{
		for (int i=0 ; i<m_numElems ; i++)
		{
			if ((*this)[i] == Item)
				Remove(i--);
		}
	}

	inline void SwapItems(const int Index1, const int Index2)
	{
		Swap(Index1, Index2);
	}

	/**
	 * Array functions
	 */
	int Add(const int Num, const bool bZeroFill = false)
	{
		const int Result = m_numElems;

		m_numElems += Num;
		if (m_numElems > m_maxElems)
		{
			ReAllocate(m_numElems);
		}

		if (bZeroFill)
		{
			memset((byte*)m_data + Result*GetElemSize(), 0, Num*GetElemSize());
		}

		return Result;
	}

	void Append(const dynArray<TType,TGranularity> &Array)
	{
		if (this != &Array && Array.m_numElems > 0)
		{
			Reserve(m_numElems + Array.m_numElems);

			// Construct if needed
			if (dataType<TType>::NeedsCtor)
			{
				for (int i=0 ; i<Array.m_numElems ; i++)
				{
					::new(*this) TType(Array[i]);
				}
			}
			else
			{
				// Otherwise just copy
				memcpy((byte*)m_data+GetUsedSize(), Array.GetData(), Array.GetUsedSize());
				m_numElems += Array.m_numElems;
			}
		}
	}

	void Copy(const dynArray<TType,TGranularity> &Source)
	{
		if (this != &Source)
		{
			if (Source.m_numElems > 0)
			{
				// This will empty our array, and make sure enough space is allocated for the replacement
				Empty(Source.m_numElems);

				// Construct if necessary
				if (dataType<TType>::NeedsCtor)
				{
					for (int i=0 ; i<Source.m_numElems ; i++)
					{
						new(*this) TType(Source[i]);
					}
				}
				else
				{
					// Otherwise just copy
					memcpy(m_data, Source.GetData(), Source.GetUsedSize());
					m_numElems = Source.m_numElems;
				}
			}
			else
			{
				Empty();
			}
		}
	}

	void Empty(const int Slack=0)
	{
		if (dataType<TType>::NeedsCtor)
		{
			for (int i=0 ; i<m_numElems ; i++)
				(&(*this)[i])->~TType();
		}

		m_numElems = 0;
		if (m_maxElems != Slack)
		{
			ReAllocate(Slack);
		}
	}

	void Insert(const int Index, const int Num=1, const bool bZeroFill = false)
	{
		assert(Num >= 1);
		assert(m_numElems >= 0);
		assert(m_maxElems >= m_numElems);
		assert(Index >= 0);
		assert(Index <= m_numElems);

		const int OldNumElems = m_numElems;

		m_numElems += Num;
		if (m_numElems > m_maxElems)
		{
			ReAllocate(m_numElems);
		}

		memmove((byte*)m_data + (Index+Num)*GetElemSize(), (byte*)m_data + Index*GetElemSize(), (OldNumElems-Index)*GetElemSize());

		if (bZeroFill)
		{
			memset((byte*)m_data + Index*GetElemSize(), 0, Num*GetElemSize());
		}
	}

	inline const bool IsValidIndex(const int Index) const
	{
		return (Index >= 0 && Index < m_numElems);
	}

	inline TType &Top() { return Last(); }
	inline const TType &Top() const { return Last(); }

	inline TType &Last(const int Offset=0) { return ((TType*)m_data)[m_numElems-Offset-1]; }
	inline const TType &Last(const int Offset=0) const { return ((TType*)m_data)[m_numElems-Offset-1]; }

	TType Pop()
	{
		TType Result = ((TType*)m_data)[m_numElems-1];
		Remove(m_numElems-1);
		return Result;
	}

	inline int Push(const TType &Item)
	{
		return AddItem(Item);
	}

	void Remove(const int Start, const int Count=1)
	{
		assert(Count >= 1 || (Count == 0 && m_numElems == 0));
		assert(Start <= m_numElems && (Start+Count) <= m_numElems);

		if (Count > 0)
		{
			if (dataType<TType>::NeedsCtor)
			{
				for (int i=Start ; i<Start+Count ; i++)
					(&(*this)[i])->~TType();
			}

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

	void Reserve(const int Num)
	{
		if (Num > m_maxElems)
		{
			ReAllocate(Num);
		}
	}

	void Shrink()
	{
		if (m_maxElems != m_numElems)
		{
			ReAllocate(m_numElems, false);
		}
	}

	void Swap(const int Index1, const int Index2)
	{
		assert(Index1 >= 0 && Index1 < m_numElems);
		assert(Index2 >= 0 && Index2 < m_numElems);

		if (Index1 != Index2)
		{
			const size_t Size = GetElemSize();
			void *Temp = dAlloca(Size);
			memcpy(Temp, (byte*)m_data+Index1*GetElemSize(), Size);
			memcpy((byte*)m_data+Index1*GetElemSize(), (byte*)m_data+Index2*GetElemSize(), Size);
			memcpy((byte*)m_data+Index2*GetElemSize(), Temp, Size);
		}
	}

	inline const size_t GetAllocSize() const { return m_maxElems * sizeof(TType); }
	inline const size_t GetElemSize() const { return sizeof(TType); }
	inline const size_t GetUsedSize() const { return m_numElems * sizeof(TType); }

	inline void *GetData() { return m_data; }
	inline const void *GetData() const{ return m_data; }

	inline TType *GetTypedData() { return m_data; }
	inline const TType *GetTypedData() const { return m_data; }

	inline const int GetNum() const { return m_numElems; }
	inline const int GetLastNum() const { return GetNum()>0 ? GetNum()-1 : 0; }

	class iterator
	{
	protected:
		dynArray<TType,TGranularity> &m_array;
		int m_position;

		iterator() {}
		inline iterator operator =(const iterator &) {}

	public:
		iterator(dynArray<TType,TGranularity> &inArray, const int StartingPosition=0)
		: m_array(inArray)
		, m_position(StartingPosition) {}

		~iterator() {}

		inline void operator++() { ++m_position; } // ++Prefix
		inline void operator++(int) { m_position++; } // Postfix++;
		inline TType &operator *() { return m_array[m_position]; }
		inline TType &operator->() { return m_array[m_position]; }
		inline operator bool() const { return m_array.IsValidIndex(m_position); }

		inline TType &Value() { return m_array[m_position]; }
	};
	class reverseIterator : public iterator
	{
	public:
		reverseIterator(dynArray<TType,TGranularity> &inArray)
		: iterator(inArray, inArray.GetLastNum()) {}

		~reverseIterator() {}

		inline void operator++() { --m_position; } // ++Prefix
		inline void operator++(int) { m_position--; } // Postfix++;
	};

	class constIterator
	{
	protected:
		dynArray<TType,TGranularity> &m_array;
		int m_position;

		constIterator() {}
		inline constIterator operator =(const constIterator &) {}

	public:
		constIterator(const dynArray<TType,TGranularity> &inArray, const int StartingPosition=0)
		: m_array(inArray)
		, m_position(StartingPosition) {}

		~constIterator() {}

		inline void operator++() { ++m_position; } // ++Prefix
		inline void operator++(int) { m_position++; } // Postfix++;
		inline const TType &operator *() const { return m_array[m_position]; }
		inline const TType &operator->() const { return m_array[m_position]; }
		inline operator bool() const { return m_array.IsValidIndex(m_position); }

		inline const TType &Value() const { return m_array[m_position]; }
	};
	class constReverseIterator : public constIterator
	{
	public:
		constReverseIterator(const dynArray<TType,TGranularity> &inArray)
		: constIterator(inArray, inArray.GetLastNum()) {}

		~constReverseIterator() {}

		inline void operator++() { --m_position; } // ++Prefix
		inline void operator++(int) { m_position--; } // Postfix++;
	};

protected:
	void ReAllocate(const int NewMaxElems, const bool bAlign=true)
	{
		int NewValue = NewMaxElems;
		if (bAlign && NewMaxElems > 0)
		{
			// FIXME: Test these other algos once there's a LOT more usage of this class
			//
			//const int NewValue = Align<int>(NewMaxElems+TGranularity, TGranularity);
			//
			//const int NewValue = Align<int>(NewMaxElems+(TGranularity-1), TGranularity);
			//
			//const int NewValue = Align<int>(NewMaxElems+((TGranularity*2)-1), TGranularity);
			//
			//const int NewValue = Align<int>(NewMaxElems+(TGranularity*2), TGranularity);
			NewValue = Align<int>(NewMaxElems, TGranularity);
		}

		if (m_maxElems == NewValue && m_data)
			return;

		if (m_data || NewValue)
		{
			extern void *_Mem_ReAlloc(void *ptr, size_t newSize, const char *fileName, const int fileLine);
			m_data = (char*)_Mem_ReAlloc(m_data, NewValue * sizeof(TType), __FILE__, __LINE__);
			m_maxElems = NewValue;
		}
	}
};

// dynArray operator new/delete
template<class TType, const int TGranularity>
static inline void *operator new(size_t Size, dynArray<TType,TGranularity> &Array)
{
	assert(Size == sizeof(TType));
	return &Array[Array.Add(1)];
}

template<class TType, const int TGranularity>
static inline void *operator new(size_t Size, dynArray<TType,TGranularity> &Array, const int Index)
{
	assert(Size == sizeof(TType));

	Array.Insert(Index, 1);
	return &Array[Index];
}

template<class TType, const int TGranularity>
static inline void operator delete(void *MemPointer, dynArray<TType,TGranularity> &Array, const int Index)
{
	assert(Size == sizeof(TType));
	Array.Remove(Index);
}

template<class TType>
static inline void *operator new(size_t Size, dynArray<TType> &List)
{
	assert(Size == sizeof(TType));
	return &List[List.Add(1)];
}
