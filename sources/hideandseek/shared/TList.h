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

// TList.h
// my interpretation of .NET's lists

template <typename T>
class IList
{
public:
	virtual ~IList()
	{
	}

	virtual void Add (const T &value) = 0;
	virtual void AddRange (const T *values, const uint32 numValues) = 0;
	virtual uint32 Count() const = 0;
	virtual uint32 Capacity() const = 0;
	virtual void RemoveAt (const uint32 index) = 0;
	virtual void Remove (const T &value) = 0;
	virtual void Clear() = 0;
	virtual void InsertAt(const T &value, const uint32 index) = 0;
	virtual T &ValueAt (const uint32 index) const = 0;
	virtual int IndexOf (const T &value) const = 0;
};

template <typename T>
class TLinkedList : IList<T>
{
public:
	class Node
	{
	public:
		T	Value;

		Node(const T &value)
		{
			Value = value;
			Next = Prev = null;
		}

		Node	*Next,
				*Prev;
	};

	uint32		_count; // makes counting faster if I do it manually
	Node		*_head,
				*_tail;

	TLinkedList ()
	{
		_head = _tail = null;
		_count = 0;
	}

	Node *Head() const
	{
		return _head;
	}

	Node *Tail() const
	{
		return _tail;
	}

	void Add (const T &value)
	{
		AddToBack(value);
	}

	Node *AddToBack (const T &value)
	{
		Node *node = new Node(value);

		if (_head == null)
		{
			_head = _tail = node;
			_count++;
			return node;
		}

		_tail->Next = node;
		node->Prev = _tail;
		_tail = node;
		_count++;

		return node;
	}

	Node *AddToFront (const T &value)
	{
		if (_head == null)
			return AddToBack(value);

		Node *node = new Node(value);

		node->Next = _head;
		_head->Prev = node;
		_head = node;
		_count++;

		return node;
	}

	// NOTE: note will be invalidated after this call.
	void RemoveNode (Node *node)
	{
		if (node->Prev != null)
			node->Prev->Next = node->Next;
		else
			_head = node->Next;

		if (node->Next != null)
			node->Next->Prev = node->Prev;
		else
			_tail = node->Prev;

		_count--;
		delete node;
	}
	
	void RemoveFromFront ()
	{
		RemoveNode(_head);
	}

	void RemoveFromBack()
	{
		RemoveNode(_tail);
	}

	T PopFront ()
	{
		T value = _head->Value;
		RemoveFromFront();
		return value;
	}

	T PopBack ()
	{
		T value = _tail->Value;
		RemoveFromBack();
		return value;
	}

	void InsertAt(const T &value, const uint32 index)
	{
	}

	void InsertAt(const T &value, Node *node)
	{
	}

	void AddRange (const T *values, const uint32 numValues)
	{
	}

	uint32 Count() const
	{
		return _count;
	}

	uint32 Capacity() const
	{
		return _count;
	}

	void RemoveAt (const uint32 index)
	{
	}

	void Remove (const T &value)
	{
	}

	void Clear()
	{
		while (_head != null)
			RemoveNode(_head);
	}
	
	T &ValueAt (const uint32 index) const
	{
		uint32 i = 0;

		for (Node *node = _head; node != null; node = node->Next)
		{
			if (i == index)
				return node->Value;

			++i;
		}

		throw std::exception();
	}
	
	Node *NodeAt (const uint32 index) const
	{
		uint32 i = 0;

		for (Node *node = _head; node != null; node = node->Next)
		{
			if (i == index)
				return node;

			++i;
		}

		return null;
	}
	
	int IndexOf (const T &value) const
	{
		uint32 i = 0;

		for (Node *node = _head; node != null; node = node->Next)
		{
			if (node->Value == value)
				return i;

			++i;
		}

		return -1;
	}
};

template <typename T>
class TList : IList<T>
{
protected:
	uint32 _reserved, _count;
	T *_array;
	bool _resetReserved;

	void ConstructFromValues (const T *values, uint32 numValues)
	{
		_reserved = numValues + 4;
		_count = numValues;
		_array = new T[_reserved];

		for (uint32 i = 0; i < numValues; ++i)
			_array[i] = values[i];
	}
	
	void ConstructFromReserved (uint32 reserved)
	{
		_reserved = 4;
		_count = 0;
		_array = new T[reserved];
	}

	void Resize (uint32 newSize)
	{
		var oldArray = _array;
		uint32 oldCount = _count;

		_reserved = newSize + 4;

		_array = new T[_reserved];
		for (uint32 i = 0; i < oldCount; ++i)
			_array[i] = oldArray[i];

		delete[] oldArray;
	}

	void Destroy ()
	{
		delete[] _array;
	}

public:
	TList (const T *values, const uint32 numValues)		
	{
		ConstructFromValues(values, numValues);
		_resetReserved = true;
	}

	TList (const uint32 reserved)
	{
		ConstructFromReserved(reserved);
		_resetReserved = true;
	}

	TList (const TList &copy)
	{
		_reserved = copy.Capacity();
		_count = copy.Count();

		_array = new T[_reserved];
		for (uint32 i = 0; i < _reserved; ++i)
			_array[i] = copy._array[i];
		_resetReserved = true;
	}

	TList ()
	{
		ConstructFromReserved(4);
		_resetReserved = true;
	}

	virtual ~TList()
	{
		Destroy();
	}

	TList &operator = (const TList &r)
	{
		Destroy();

		_reserved = r.Capacity();
		_count = r.Count();

		_array = new T[_reserved];
		for (uint32 i = 0; i < _count; ++i)
			_array[i] = r._array[i];

		return *this;
	}

	void Add (const T &value)
	{
		if (_count >= _reserved)
			Resize(_count + 1);

		_array[_count++] = value;
	}

	void AddRange (const T *values, const uint32 numValues)
	{
		// FIXME: quicker way with memcpy maybe?
		for (uint32 i = 0; i < numValues; ++i)
			Add(values[i]);
	}

	void AddRange (const TList &copy)
	{
		AddRange(copy._array, copy._count);
	}

	void Sort(int (*comparer) (const T &l, const T &r), int left, int right)
	{
		int l_hold, r_hold;
		
		l_hold = left;
		r_hold = right;
		T pivot = _array[left];
		
		while(left < right)
		{
			while( (comparer(_array[right], pivot) >= 0) && (left < right) )
				right--;
			
			if( left != right )
			{
				_array[left] = _array[right];
				left++;
			}
			
			while( (comparer(_array[left], pivot) <= 0) && (left < right) )
				left++;
			
			if( left != right )
			{
				_array[right] = _array[left];
				right--;
			}
		}

		_array[left] = pivot;
		int pivTemp = left;
		left = l_hold;
		right = r_hold;

		if( left < pivTemp )
			Sort(comparer, left, pivTemp-1 );

		if (right > pivTemp)
			Sort(comparer, pivTemp+1, right );
	}

	void Sort (int (*comparer) (const T &l, const T &r))
	{
		Sort(comparer, 0, _count - 1);
	}

	uint32 Count() const
	{
		return _count;
	}

	uint32 Capacity() const
	{
		return _reserved;
	}

	const T *Array() const
	{
		return _array;
	}

	void RemoveAt (const uint32 index, const bool autoResize)
	{
		uint32 oldCount = _count;
		uint32 oldReserved = _reserved;

		_count--;
		_reserved--;

		T *newArray = new T[_reserved];

		if (index == 0)
		{
			for (uint32 i = 0; i < oldCount - 1; ++i)
				newArray[i] = _array[i + 1];
		}
		else if (index == oldCount - 1)
		{
			for (uint32 i = 0; i < oldCount - 1; ++i)
				newArray[i] = _array[i];
		}
		else
		{
			// split array up
			T *oldLeft, *oldRight;
			uint32 oldLeftCount = 0, oldRightCount = 0;

			oldLeft = _array;
			oldLeftCount = index;

			oldRight = _array + index + 1;
			oldRightCount = oldCount - index - 1;

			for (uint32 i = 0; i < oldLeftCount; ++i)
			{
				newArray[i] = oldLeft[i];
				newArray[i + index] = oldRight[i];
			}
		}

		delete[] _array;
		_array = newArray;
	}

	void RemoveAt (const uint32 index)
	{
		RemoveAt(index, (_resetReserved) ? true : false);
	}

	void Remove (const T &value, const bool autoResize)
	{
		RemoveAt(IndexOf(value), autoResize);
	}

	void Remove (const T &value)
	{
		RemoveAt(IndexOf(value));
	}

	void InsertAt (const T &value, const uint32 index)
	{
	}

	void Clear ()
	{
		if (_resetReserved)
		{
			delete[] _array;
			ConstructFromReserved(4);
		}
		else
			_count = 0;
	}

	inline T &ValueAt (const uint32 index) const
	{
		if (index < 0 || index >= _count)
			throw new ExceptionIndexOutOfRange(String("index"));

		return _array[index];
	}

	int IndexOf (const T &value) const
	{
		for (uint32 i = 0; i < _count; ++i)
		{
			if (_array[i] == value)
				return i;
		}

		return -1;
	}

	TList &operator+= (const T &value)
	{
		Add(value);
		return *this;
	}

	TList &operator-= (const T &value)
	{
		Remove(value);
		return *this;
	}

	inline T &operator[] (const uint32 index) const
	{
		return ValueAt(index);
	}
};
