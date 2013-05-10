/* Copyright (c) <2009> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#include "dContainersStdAfx.h"
#include "dString.h"


#define D_USE_POOL_BUKECT_ALLOCATOR
#define D_STRING_MEM_GRANULARITY		16
#define D_STRING_MEM_MAX_BUCKET_SIZE	256
#define D_STRING_MEM_BUCKETS			(D_STRING_MEM_MAX_BUCKET_SIZE / D_STRING_MEM_GRANULARITY)
#define D_DSTRING_ENTRIES_IN_FREELIST	32




class dString::dStringAllocator
{
	public:
	#ifdef D_USE_POOL_BUKECT_ALLOCATOR
		class dMemBucket
		{
			public:

			class dDataChunk
			{
				public: 
				int m_size;
				int m_count;
				dDataChunk* m_next;
				
			};

			dMemBucket()
				:m_freeListDataChunk(NULL)
			{
			}
			~dMemBucket()
			{
			}

			void Prefetch (int chunckSize)
			{
				for (int i = 0; i < D_DSTRING_ENTRIES_IN_FREELIST; i ++) {
					dDataChunk* const data = (dDataChunk*) new char[chunckSize + sizeof (int)];
					data->m_count = i + 1; 
					data->m_size = chunckSize;
					data->m_next = m_freeListDataChunk; 
					m_freeListDataChunk = data;
				}
			}

			void Flush ()
			{
				for (int i = 0; m_freeListDataChunk && (i < D_DSTRING_ENTRIES_IN_FREELIST); i ++) {
					dDataChunk* const ptr = m_freeListDataChunk;
					m_freeListDataChunk = m_freeListDataChunk->m_next;
					delete[] (char*) ptr;
				}
			}

			char* Alloc(int size)
			{
				if (!m_freeListDataChunk) {
					Prefetch (size);
				}
				dDataChunk* const data = m_freeListDataChunk;
				_ASSERTE (size == data->m_size);
				m_freeListDataChunk = m_freeListDataChunk->m_next;
				return ((char*)data) + sizeof (int);
			}

			void Free(char * const ptr)
			{
				char* const realPtr = ptr - sizeof (int);
				dMemBucket::dDataChunk* const dataChunck = (dMemBucket::dDataChunk*) (realPtr);

				dataChunck->m_count = m_freeListDataChunk ? m_freeListDataChunk->m_count + 1 : 1;
				dataChunck->m_next = m_freeListDataChunk;
				m_freeListDataChunk = dataChunck;
				if (dataChunck->m_count >= 2 * D_DSTRING_ENTRIES_IN_FREELIST) {
					Flush();
				}
			}

			dDataChunk* m_freeListDataChunk;
		};

	
		dStringAllocator()
		{

			for (int i = 0; i < sizeof (m_buckects) / sizeof (m_buckects[0]); i ++) {
				m_buckects[i].Prefetch ((i + 1)* D_STRING_MEM_GRANULARITY);
			}
		}
		~dStringAllocator()
		{
			for (int i = 0; i < sizeof (m_buckects) / sizeof (m_buckects[0]); i ++) {
				m_buckects[i].Flush();
			}
		}

		char* Alloc(int size)
		{
			_ASSERTE (size >= 1);
			if (size <= D_STRING_MEM_MAX_BUCKET_SIZE) {
				int buckectEntry = (size - 1) / D_STRING_MEM_GRANULARITY;
				int buckectSize = (buckectEntry + 1) * D_STRING_MEM_GRANULARITY;
				return m_buckects[buckectEntry].Alloc(buckectSize);
			}
			dMemBucket::dDataChunk* const ptr = (dMemBucket::dDataChunk*) new char[size + sizeof (int)];
			ptr->m_size = size;
			return ((char*)ptr) + sizeof (int);
		}

		void Free(char* const ptr)
		{
			char* const realPtr = ptr-sizeof (int);
			dMemBucket::dDataChunk* const dataChunck = (dMemBucket::dDataChunk*) (realPtr);
			if (dataChunck->m_size <= D_STRING_MEM_MAX_BUCKET_SIZE) {
				int buckectEntry = dataChunck->m_size / D_STRING_MEM_GRANULARITY - 1;
				m_buckects[buckectEntry].Free(ptr);
			} else {
				delete[] realPtr;
			}
		}

		dMemBucket m_buckects [D_STRING_MEM_BUCKETS];

	#else 
		char* Alloc(int size)
		{
			return new char[size];
		}

		void Free(char* const ptr)
		{
			delete[] ptr;
		}
	#endif
};

//dString::dStringAllocator dString::m_allocator;

dString::dString ()
	:m_string(NULL)
	,m_size(0)
	,m_capacity(0)
{
}

dString::dString (const dString& src)
	:m_string(NULL)
	,m_size(0)
	,m_capacity(0)
{
	if (src.m_string) {
		m_size = src.m_size;
		m_capacity = m_size + 1;

		m_string = AllocMem (src.m_size + 1);
		CopyData (m_string, src.m_string, src.m_size + 1);
		m_string[m_size] = 0;
	}
}

dString::dString (const char* const data)
	:m_string(NULL)
	,m_size(0)
	,m_capacity(0)
{
	if (data) {
		m_size = CalculateSize (data);
		m_capacity = m_size + 1; 

		m_string = AllocMem (m_size + 1);
		CopyData (m_string, data, m_size + 1);
		m_string[m_size] = 0;
	}
}

dString::dString (const char* const data, int maxSize)
	:m_string(NULL)
	,m_size(0)
	,m_capacity(0)
{
	if (data) {
		m_size = dMin (CalculateSize (data), maxSize);
		m_capacity = m_size + 1; 
		m_string = AllocMem (m_size + 1);
		CopyData (m_string, data, m_size + 1);
		m_string[m_size] = 0;
	}
}

dString::dString (const dString& src, const char* const concatenate, int concatenateSize)
	:m_string(NULL)
	,m_size(0)
	,m_capacity(0)
{
	m_string = AllocMem (src.m_size + concatenateSize + 1);
	memcpy (m_string, src.m_string, src.m_size);
	memcpy (&m_string[src.m_size], concatenate, concatenateSize);
	m_size = src.m_size + concatenateSize;
	m_string[m_size] = 0;
	m_capacity = m_size + 1;
}



dString::dString (char chr)
	:m_string(NULL)
	,m_size(0)
	,m_capacity(0)
{
	m_string = AllocMem (2);
	m_string[0] = chr;
	m_string[1] = 0;
	m_size = 1;
	m_capacity = m_size + 1;
}

dString::dString (int val)
	:m_string(NULL)
	,m_size(0)
	,m_capacity(0)
{
	char tmp[256];

	int count = 0;
	unsigned mag = abs (val);
	do {
		unsigned digit = mag % 10;
		mag /= 10;  
		tmp[count] = '0' + char(digit);
		count ++;
	} while (mag > 0);

	int offset = (val >= 0) ? 0: 1;
	m_string = AllocMem (count + offset + 1);
	if (offset) {
		m_string[0] = '-';
	}
	for (int i = 0; i < count; i ++) {
		m_string[i + offset] = tmp[count - i - 1];
	}

	m_string[count + offset] = 0;
	m_size = count + offset;
	m_capacity = m_size + 1;
}

dString::dString (long long val)
	:m_string(NULL)
	,m_size(0)
	,m_capacity(0)
{
	char tmp[256];

	int count = 0;
	unsigned long long mag = (val > 0ll) ? val : -val;
	do {
		unsigned long long digit = mag % 10ll;
		mag /= 10ll;  
		tmp[count] = '0' + char(digit);
		count ++;
	} while (mag > 0);

	int offset = (val >= 0ll) ? 0: 1;
	m_string = AllocMem (count + offset + 1);
	if (offset) {
		m_string[0] = '-';
	}
	for (int i = 0; i < count; i ++) {
		m_string[i + offset] = tmp[count - i - 1];
	}

	m_string[count + offset] = 0;
	m_size = count + offset;
	m_capacity = m_size + 1;
}


dString::~dString ()
{
	Empty();
}




void dString::Empty()
{
	if (m_capacity && m_string) {
		FreeMem (m_string);
	}
	m_size = 0;
	m_capacity = 0;
	m_string = NULL;
}

void dString::LoadFile (FILE* const file)
{
	Empty();
	fseek (file, 0, SEEK_END);
	int size = ftell (file);
	fseek (file, 0, SEEK_SET);
	Expand (size);
	fread (m_string, 1, size, file);
	m_string[size] = 0;
	m_size = size;
	m_capacity = m_size + 1;
}


dString dString::SubString(int start, int size) const
{
	_ASSERTE (m_string);
	return dString (&m_string[start], size);
}

void dString::operator+= (const char* const src)
{
	char* const oldData = m_string;
	int size = CalculateSize (src);
	m_string = AllocMem (m_size + size + 1);
	memcpy (m_string, oldData, m_size);
	memcpy (&m_string[m_size], src, size);
	m_size = m_size + size;
	m_string[m_size] = 0;
	m_capacity = m_size + 1;
	FreeMem(oldData);
}




const char* dString::GetStr () const
{
	return m_string;
}

int dString::Size() const
{
	return m_size;
}

int dString::Capacity() const
{
	return m_capacity;
}

void dString::CopyData (char* const dst, const char* const src, int size) const
{
	_ASSERTE (dst);
	_ASSERTE (src);
	memcpy (dst, src, size);
}


int dString::Compare (const char* const str0, const char* const str1) const
{
	_ASSERTE (str0);
	_ASSERTE (str1);
	return strcmp (str0, str1);
}


bool dString::operator== (const dString& src) const
{
	return Compare (m_string, src.m_string) == 0;
}

bool dString::operator!= (const dString& src) const
{
	return Compare (m_string, src.m_string) != 0;
}


bool dString::operator< (const dString& src) const
{
	return Compare (m_string, src.m_string) < 0;
}

bool dString::operator> (const dString& src) const
{
	return Compare (m_string, src.m_string) > 0;
}

bool dString::operator<= (const dString& src) const
{
	return Compare (m_string, src.m_string) <= 0;
}

bool dString::operator>= (const dString& src) const
{
	return Compare (m_string, src.m_string) >= 0;
}




int dString::ToInteger() const
{
	int value = 0;
	if (m_size) {
		int base = (m_string[0] == '-') ? 1 : 0;
		for (int i = base; i < m_size; i ++) {
			char ch = m_string[i]; 		
			if ((ch >= '0') && (ch <= '9')) {
				value = value * 10 + ch - '0';
			} else {
				break;
			}
		}
		value *= base ? -1 : 1;
	}
	return value;
}

long long dString::ToInteger64() const
{
	long long value = 0;
	if (m_size) {
		int base = (m_string[0] == '-') ? 1 : 0;
		for (int i = base; i < m_size; i ++) {
			char ch = m_string[i]; 		
			if ((ch >= '0') && (ch <= '9')) {
				value = value * 10ll + ch - '0';
			} else {
				break;
			}
		}
		value *= base ? -1 : 1;
	}
	return value;
}


dString& dString::operator= (const dString& src)
{
	if (m_capacity && m_string) {
		FreeMem (m_string);
	}
	m_string = NULL;
	m_capacity = 0;
	m_size = src.m_size;
	if (src.m_string) {
		m_capacity = src.m_size + 1;
		m_string = AllocMem (src.m_size + 1);
		CopyData (m_string, src.m_string, src.m_size + 1);
	}
	return *this;
}


int dString::CalculateSize (const char* const data) const
{
	int size = 0;
	if (data) {
		for (int i = 0; data[i]; i ++) {
			size ++;
		}
	}
	return size;
}

void dString::ToLower()
{
	if (m_string) {
		_strlwr (m_string);
	}
}

int dString::Find (char ch, int from) const
{
	for (int i = from; i < m_size; i ++) {
		if (m_string[i] == ch) {
			return i;
		}
	}
	return -1;
}

//int dString::Find (const dString& subStream, int from) const
int dString::Find (const char* const subString, int subStringLength, int from, int lenght) const
{
	_ASSERTE (from >= 0);
	//_ASSERTE (subStream.m_size >= 0);
	_ASSERTE (subStringLength >= 1);

	int location = -1;
	if (m_size) {
		const int str2Size = dMin (subStringLength, lenght);
		if (str2Size == 1) {
			char ch = subString[0];
			const char* const ptr1 = m_string;
			for (int i = 0; i < m_size; i ++) {
				if (ch == ptr1[i]) {
					return i;
				}
			}
		} else if ((str2Size <= 4) || (m_size < 256)) {
			const int size = m_size - str2Size;
			for (int j = from; j <= size; j ++) {
				const char* const ptr1 = &m_string[j];
				int i = 0;
				while (subString[i] && (ptr1[i] == subString[i])) {
					i ++;
				}
				if (!subString[i]) {
					return j;
				}
			}
		} else {
			// for large strings smart search
			short frequency[256];
			memset (frequency, -1, sizeof (frequency));
			for (int i = 0; i < str2Size; i ++) {
				frequency[subString[i]] = short(i);
			}

			int j = from;
			const int size = m_size - str2Size;
			while (j <= size) {
				const char* const ptr1 = &m_string[j];
				int i = str2Size - 1;
				while ((i >= 0) && (ptr1[i] == subString[i])) {
					i --;
				}
				if (i < 0) {
					return j;
					
				}
				j += dMax(i - frequency[ptr1[i]], 1);
			}
		}
	}
	return location;
}


void dString::Replace (int start, int size, const char* const str, int strSize)
{
	char* const oldData = m_string;
	m_string = AllocMem (m_size - size + strSize + 1);
	memcpy (m_string, oldData, start);
	memcpy (&m_string[start], str, strSize);
	memcpy (&m_string[start + strSize], &oldData[start + size], m_size - (start + size));
	m_size = m_size - size + strSize;
	m_capacity = m_size - size + strSize + 1;
	m_string[m_size] = 0;
	FreeMem(oldData);
}


void dString::Expand (int size)
{
	char* const oldData = m_string;
	m_string = AllocMem (m_size + size + 1);
	
	if (m_capacity) {
		memcpy (m_string, oldData, m_size);
		FreeMem(oldData);
	}
	m_string[m_size] = 0;
	m_capacity = m_size + size + 1;
}


dString::dStringAllocator& dString::GetAllocator() const
{
	static dStringAllocator allocator;
	return allocator;
}


char* dString::AllocMem(int size)
{
	return GetAllocator().Alloc(size);
}

void dString::FreeMem (char* const ptr)
{
	if (ptr) {
		GetAllocator().Free(ptr);
	}
}
