#include "g_local.h"

entity_iterator::entity_iterator(const size_t &offset) :
	_ptr(&globals.entities.pool[offset])
{
}

entity_iterator &entity_iterator::operator++()
{
	_ptr++;
	return *this;
}

entity_iterator entity_iterator::operator+(const size_t &offset)
{
	entity_iterator retval = *this;
	retval._ptr += offset;
	return retval;
}