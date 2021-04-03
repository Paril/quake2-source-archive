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

// TMap.h

/*
==============================================================================

	mapListBase
 
==============================================================================
*/
template<class TKey, class TMapTo>
class mapListBase
{
protected:
	class mapPairBase
	{
	private:
		mapPairBase() {}

	public:
		TKey m_key;
		TMapTo m_value;

		mapPairBase(typename dataType<TKey>::InitType inKey, typename dataType<TMapTo>::InitType inValue)
		: m_key(inKey)
		, m_value(inValue) {}
	};

public:
};

/*
==============================================================================

	mapList
 
==============================================================================
*/
template<class TKey, class TMapTo>
class mapList : public mapListBase<TKey,TMapTo>
{
private:
	/**
	 * Data
	 */
	class mapPair : public mapPairBase
	{
	public:
		mapPair(typename dataType<TKey>::InitType inKey, typename dataType<TMapTo>::InitType inValue)
		: mapPairBase(inKey, inValue) {}
	};

	dynArray<mapPair> m_mapData;

public:
	/**
	 * Constructors
	 */
	mapList() {}

	mapList(const mapList<TKey,TMapTo> &inMap) 
	{
		Copy(inMap);
	}

	/**
	 * Operators
	 */
	inline mapList<TKey,TMapTo> &operator =(const mapList<TKey,TMapTo> &inMap)
	{
		m_mapData = inMap.m_mapData;
	}

	inline const TMapTo &operator [](const int Index) const
	{
		return m_mapData[Index].m_value;
	}
	inline TMapTo &operator [](const int Index)
	{
		return m_mapData[Index].m_value;
	}

	/**
	 * Functions
	 */
	TMapTo &Add(typename dataType<TKey>::InitType inKey, typename dataType<TMapTo>::InitType inValue)
	{
		// Make sure it wasn't already added
		for (int i=0 ; i<GetNum() ; i++)
		{
			if (m_mapData[i].m_key == inKey)
			{
				m_mapData[i].m_value = inValue;
				return m_mapData[i].m_value;
			}
		}

		// Add it
		mapPair &Pair = *new(m_mapData) mapPair(inKey, inValue);
		return Pair.m_value;
	}
	void Copy(const mapList<TKey,TMapTo> &inMap)
	{
		if (inMap.GetNum())
		{
			m_mapData = inMap.m_mapData;
		}
		else
		{
			Empty();
		}
	}
	inline TMapTo &Push(typename dataType<TKey>::InitType inKey, typename dataType<TMapTo>::InitType inValue)
	{
		return Add(inKey, inValue);
	}
	TMapTo *Find(const TKey &inKey)
	{
		for (int i=0 ; i<GetNum() ; i++)
		{
			if (m_mapData[i].m_key == inKey)
			{
				return &m_mapData[i].m_value;
			}
		}

		return NULL;
	}
	const TMapTo *Find(const TKey &inKey) const
	{
		for (int i=0 ; i<GetNum() ; i++)
		{
			if (m_mapData[i].m_key == inKey)
			{
				return &m_mapData[i].m_value;
			}
		}

		return NULL;
	}
	void Empty(const int Slack=0)
	{
		m_mapData.Empty(Slack);
	}
	void Remove(typename dataType<TKey>::InitType inKey)
	{
		for (int i=0 ; i<GetNum() ; i++)
		{
			if (m_mapData[i].m_key == inKey)
			{
				m_mapData.Remove(i);
				break;
			}
		}
	}

	inline const int GetNum() const { return m_mapData.GetNum(); }
	inline const int GetLastNum() const { return GetNum()>0 ? GetNum()-1 : 0; }

	inline TMapTo &Top() { return Last(); }
	inline const TMapTo &Top() const { return Last(); }

	inline TMapTo &Last(const int Offset=0) { return &m_mapData[NumElems-Offset-1].m_value; }
	inline const TMapTo &Last(const int Offset=0) const { return &m_mapData[NumElems-Offset-1].m_value; }

	class iterator
	{
	protected:
		dynArray<mapPair> &m_mapData;
		int m_position;

		iterator() {}
		inline iterator operator =(const iterator &) {}

	public:
		iterator(mapList &inMap, const int StartingPosition=0)
		: m_mapData(inMap.m_mapData)
		, m_position(StartingPosition) {}

		~iterator() {}

		inline void operator++() { ++m_position; } // ++Prefix
		inline void operator++(int) { m_position++; } // Postfix++;
		inline TMapTo &operator *() { return m_mapData[m_position].m_value; }
		inline TMapTo *operator->() { return &m_mapData[m_position].m_value; }
		inline operator bool() const { return m_mapData.IsValidIndex(m_position); }

		inline const TKey &Key() { return m_mapData[m_position].m_key; }
		inline TMapTo &Value() { return m_mapData[m_position].m_value; }
	};
	class reverseIterator : public iterator
	{
	public:
		reverseIterator(mapList &inMap)
		: iterator(inMap, inMap.GetLastNum()) {}

		~reverseIterator() {}

		inline void operator++() { --m_position; } // ++Prefix
		inline void operator++(int) { m_position--; } // Postfix++;
	};

	class constIterator
	{
	protected:
		const dynArray<mapPair> &m_mapData;
		int m_position;

		constIterator() {}
		inline constIterator operator =(const constIterator &) {}

	public:
		constIterator(const mapList &inMap, const int StartingPosition=0)
		: m_mapData(inMap.m_mapData)
		, m_position(StartingPosition) {}

		~constIterator() {}

		inline void operator++() { ++m_position; } // ++Prefix
		inline void operator++(int) { m_position++; } // Postfix++;
		inline const TMapTo &operator *() const { return m_mapData[m_position].m_value; }
		inline const TMapTo *operator->() const { return &m_mapData[m_position].m_value; }
		inline operator bool() const { return m_mapData.IsValidIndex(m_position); }

		inline const TKey &Key() const { return m_mapData[m_position].m_key; }
		inline const TMapTo &Value() const { return m_mapData[m_position].m_value; }
	};
	class constReverseIterator : public constIterator
	{
	public:
		constReverseIterator(const mapList &inMap)
		: constIterator(inMap, inMap.GetLastNum()) {}

		~constReverseIterator() {}

		inline void operator++() { --m_position; } // ++Prefix
		inline void operator++(int) { m_position--; } // Postfix++;
	};
};

/*
==============================================================================

	hashMapList
 
==============================================================================
*/
template<class TKey, class TMapTo>
class hashMapList : public mapListBase<TKey,TMapTo>
{
private:
	/**
	 * Data
	 */
	enum { BAD_HASH_INDEX = -1 };
	enum { HASH_TABLE_STEP = 64 };

	class dHashPair : public mapPairBase
	{
	private:
		dHashPair() {}

	public:
		int m_hashNext;

		dHashPair(typename dataType<TKey>::InitType inKey, typename dataType<TMapTo>::InitType inValue)
		: mapPairBase(inKey, inValue)
		, m_hashNext(BAD_HASH_INDEX) {}
	};

	dynArray<dHashPair> m_mapData;
	int m_hashSize;
	int *m_hashData;

	/**
	 * Functions
	 */
public:
	/**
	 * Constructors
	 */
	hashMapList()
	: m_hashSize(0)
	, m_hashData(NULL)
	{
		CheckHash(0);
	}

	hashMapList(const hashMapList<TKey,TMapTo> &inMap)
	{
		Copy(inMap);
	}

	/**
	 * Destructors
	 */
	~hashMapList()
	{
		if (m_hashData)
		{
			Mem_Free(m_hashData);
		}
	}

	/**
	 * Operators
	 */
	inline hashMapList<TKey,TMapTo> &operator =(const hashMapList<TKey,TMapTo> &inMap)
	{
		Copy(inMap);
		return *this;
	}

	inline const TMapTo &operator [](const int Index) const
	{
		return m_mapData[Index].m_value;
	}
	inline TMapTo &operator [](const int Index)
	{
		return m_mapData[Index].m_value;
	}

	/**
	 * Functions
	 */
private:
	void Rehash()
	{
		if (m_hashSize > 0)
		{
			memset(m_hashData, BAD_HASH_INDEX, sizeof(int) * m_hashSize);
			for (int i=0 ; i<m_mapData.GetNum() ; i++)
			{
				uint HashValue = (dataType<TKey>::TypeHash<uint>(m_mapData[i].m_key)&m_hashSize-1);
				m_mapData[i].m_hashNext = m_hashData[HashValue];
				m_hashData[HashValue] = i;
			}
		}
	}

	void CheckHash(const int NewNumItems, const bool bForceRehash = false)
	{
		// Make the hash table 1/4th the size of the item table
		const int NewHashSize = max( Align<int>(NewNumItems>>2, HASH_TABLE_STEP), HASH_TABLE_STEP);

		if (NewHashSize != m_hashSize)
		{
			m_hashSize = NewHashSize;
			m_hashData = Mem_ReAlloc(m_hashData, sizeof(int) * m_hashSize);

			Rehash();
		}
		else if (bForceRehash)
		{
			Rehash();
		}
	}

public:
	TMapTo &Add(typename dataType<TKey>::InitType inKey, typename dataType<TMapTo>::InitType inValue)
	{
		CheckHash(GetNum() + 1);

		const uint HashValue = (dataType<TKey>::TypeHash<uint>(inKey)&m_hashSize-1);

		// Make sure it wasn't already added
		for (int i=m_hashData[HashValue] ; i!=BAD_HASH_INDEX ; i=m_mapData[i].m_hashNext)
		{
			if (m_mapData[i].m_key == inKey)
			{
				m_mapData[i].m_value = inValue;
				return m_mapData[i].m_value;
			}
		}

		// Add it
		dHashPair &Pair = *new(m_mapData) dHashPair(inKey, inValue);
		Pair.m_hashNext = m_hashData[HashValue];
		m_hashData[HashValue] = m_mapData.GetLastNum();

		return Pair.m_value;
	}
	void Copy(const hashMapList<TKey,TMapTo> &inMap)
	{
		if (inMap.GetNum())
		{
			m_mapData = inMap.m_mapData;

			CheckHash(GetNum() + 1, true);
		}
		else
		{
			Empty();
		}
	}
	inline TMapTo &Push(typename dataType<TKey>::InitType inKey, typename dataType<TMapTo>::InitType inValue)
	{
		return Add(inKey, inValue);
	}
	TMapTo *Find(const TKey &inKey)
	{
		const uint HashValue = (dataType<TKey>::TypeHash<uint>(inKey)&m_hashSize-1);

		for (int i=m_hashData[HashValue] ; i!=BAD_HASH_INDEX ; i=m_mapData[i].m_hashNext)
		{
			if (m_mapData[i].m_key == inKey)
			{
				return &m_mapData[i].m_value;
			}
		}

		return NULL;
	}
	const TMapTo *Find(const TKey &inKey) const
	{
		const uint HashValue = (dataType<TKey>::TypeHash<uint>(inKey)&m_hashSize-1);

		for (int i=m_hashData[HashValue] ; i!=BAD_HASH_INDEX ; i=m_mapData[i].m_hashNext)
		{
			if (m_mapData[i].m_key == inKey)
			{
				return &m_mapData[i].m_value;
			}
		}

		return NULL;
	}
	void Empty(const int Slack=0)
	{
		m_mapData.Empty(Slack);
		CheckHash(Slack, true);
	}
	bool Remove(typename dataType<TKey>::InitType inKey)
	{
		CheckHash(GetNum() - 1);

		const uint HashValue = (dataType<TKey>::TypeHash<uint>(inKey)&m_hashSize-1);
		bool bResult = false;

		for (int i=m_hashData[HashValue] ; i!=BAD_HASH_INDEX ; i=m_mapData[i].m_hashNext)
		{
			if (m_mapData[i].m_key == inKey)
			{
				// Remove from array
				m_mapData.Remove(i);
				bResult = true;
				break;
			}
		}

		if (bResult)
		{
			Rehash();
		}

		return bResult;
	}

	inline const int GetNum() const { return m_mapData.GetNum(); }
	inline const int GetLastNum() const { return GetNum()>0 ? GetNum()-1 : 0; }

	inline TMapTo &Top() { return Last(); }
	inline const TMapTo &Top() const { return Last(); }

	inline TMapTo &Last(const int Offset=0) { return &m_mapData[NumElems-Offset-1].m_value; }
	inline const TMapTo &Last(const int Offset=0) const { return &m_mapData[NumElems-Offset-1].m_value; }

	class iterator
	{
	protected:
		dynArray<dHashPair> &m_mapData;
		int m_position;

		iterator() {}
		inline iterator operator =(const iterator &) {}

	public:
		iterator(hashMapList &inMap, const int StartingPosition=0)
		: m_mapData(inMap.m_mapData)
		, m_position(StartingPosition) {}

		~iterator() {}

		inline void operator++() { ++m_position; } // ++Prefix
		inline void operator++(int) { m_position++; } // Postfix++;
		inline TMapTo &operator *() { return m_mapData[m_position].m_value; }
		inline TMapTo *operator->() { return &m_mapData[m_position].m_value; }
		inline operator bool() const { return m_mapData.IsValidIndex(m_position); }

		inline const TKey &Key() { return m_mapData[m_position].m_key; }
		inline TMapTo &Value() { return m_mapData[m_position].m_value; }
	};
	class reverseIterator : public iterator
	{
	public:
		reverseIterator(hashMapList &inMap)
		: iterator(inMap, inMap.GetLastNum()) {}

		~reverseIterator() {}

		inline void operator++() { --m_position; } // ++Prefix
		inline void operator++(int) { m_position--; } // Postfix++;
	};

	class constIterator
	{
	protected:
		const dynArray<dHashPair> &m_mapData;
		int m_position;

		constIterator() {}
		inline constIterator operator =(const constIterator &) {}

	public:
		constIterator(const hashMapList &inMap, const int StartingPosition=0)
		: m_mapData(inMap.m_mapData)
		, m_position(StartingPosition) {}

		~constIterator() {}

		inline void operator++() { ++m_position; } // ++Prefix
		inline void operator++(int) { m_position++; } // Postfix++;
		inline const TMapTo &operator *() const { return m_mapData[m_position].m_value; }
		inline const TMapTo *operator->() const { return &m_mapData[m_position].m_value; }
		inline operator bool() const { return m_mapData.IsValidIndex(m_position); }

		inline const TKey &Key() const { return m_mapData[m_position].m_key; }
		inline const TMapTo &Value() const { return m_mapData[m_position].m_value; }
	};
	class constReverseIterator : public constIterator
	{
	public:
		constReverseIterator(const hashMapList &inMap)
		: constIterator(inMap, inMap.GetLastNum()) {}

		~constReverseIterator() {}

		inline void operator++() { --m_position; } // ++Prefix
		inline void operator++(int) { m_position--; } // Postfix++;
	};
};
