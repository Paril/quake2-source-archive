//*************************************************************************************
//*************************************************************************************
// File: nptrarray.h
//*************************************************************************************
//*************************************************************************************

#ifndef __NPTRARRAY_H__
#define __NPTRARRAY_H__

//*************************************************************************************
//*************************************************************************************
// Class: CNPtrArray
//*************************************************************************************
//*************************************************************************************

class CNPtrArray
{
	public:
		CNPtrArray();

		int GetSize() const;
		int GetUpperBound() const;
		void SetSize(int nNewSize, int nGrowBy = -1);

		void FreeExtra();
		void RemoveAll();

		void* GetAt(int nIndex) const;
		void SetAt(int nIndex, void* newElement);

		void*& ElementAt(int nIndex);

		const void** GetData() const;
		void** GetData();

		// Potentially growing the array
		void SetAtGrow(int nIndex, void* newElement);

		int Add(void* newElement);

		int Append(const CNPtrArray& src);
		void Copy(const CNPtrArray& src);

		// overloaded operator helpers
		void* operator[](int nIndex) const;
		void*& operator[](int nIndex);

		// Operations that move elements around
		void InsertAt(int nIndex, void* newElement, int nCount = 1);

		void RemoveAt(int nIndex, int nCount = 1);
		void InsertAt(int nStartIndex, CNPtrArray* pNewArray);

	// Implementation
	protected:
		void** m_pData;   // the actual array of data
		int m_nSize;     // # of elements (upperBound - 1)
		int m_nMaxSize;  // max allocated
		int m_nGrowBy;   // grow amount

	public:
		~CNPtrArray();

	protected:
		// local typedefs for class templates
		typedef void* BASE_TYPE;
		typedef void* BASE_ARG_TYPE;
};
#endif

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************



