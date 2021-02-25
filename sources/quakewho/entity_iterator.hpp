#pragma once

class entity_iterator
{
	edict_t *_ptr = nullptr;

public:
	entity_iterator(const size_t &offset);
	entity_iterator& operator++();
	entity_iterator operator+(const size_t &offset);

	entity_iterator operator++(int)
	{
		entity_iterator retval = *this;
		++(*this);
		return retval;
	}

	bool operator==(const entity_iterator &other) const { return _ptr == other._ptr; }
	bool operator!=(const entity_iterator &other) const { return !(*this == other); }
	edict_t &operator*()
	{
		return *_ptr;
	}

	// iterator traits
	using difference_type = ptrdiff_t;
	using value_type = edict_t*;
	using pointer = const edict_t**;
	using reference = const edict_t*&;
	using iterator_category = std::forward_iterator_tag;
};

struct entity_range
{
private:
	size_t _start, _end;

public:
	entity_range(size_t start, size_t end) :
		_start(start),
		_end(end)
	{
	}

	entity_iterator begin() { return entity_iterator(_start); }
	entity_iterator end() { return entity_iterator(_end); }
};