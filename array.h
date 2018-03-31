//
// Created by wlanjie on 2018/2/7.
//

#ifndef MP4_ARRAY_H
#define MP4_ARRAY_H

#include <new>
#include "types.h"
#include "results.h"

namespace mp4 {

const int ARRAY_INITIAL_COUNT = 64;

template <typename T>
class Array
{
public:
    // methods
    Array(): m_AllocatedCount(0), m_ItemCount(0), m_Items(0) {}
    Array(const T* items, Size count);
    Array<T>(const Array<T>& copy);
    Array<T>& operator=(const Array<T>& copy);
    virtual ~Array();
    Cardinal ItemCount() const { return m_ItemCount; }
    Result   Append(const T& item);
    Result   RemoveLast();
    T& operator[](unsigned long idx) { return m_Items[idx]; }
    const T& operator[](unsigned long idx) const { return m_Items[idx]; }
    Result Clear();
    Result EnsureCapacity(Cardinal count);
    Result SetItemCount(Cardinal item_count);

protected:
    // members
    Cardinal m_AllocatedCount;
    Cardinal m_ItemCount;
    T*           m_Items;
};

/*----------------------------------------------------------------------
|   Array<T>::Array<T>
+---------------------------------------------------------------------*/
template <typename T>
Array<T>::Array(const T* items, Size count) :
        m_AllocatedCount(count),
        m_ItemCount(count),
        m_Items((T*)::operator new(count*sizeof(T)))
{
    for (unsigned int i=0; i<count; i++) {
        new ((void*)&m_Items[i]) T(items[i]);
    }
}

/*----------------------------------------------------------------------
|   Array<T>::Array<T>
+---------------------------------------------------------------------*/
template <typename T>
inline
Array<T>::Array(const Array<T>& copy) :
        m_AllocatedCount(0),
        m_ItemCount(0),
        m_Items(0)
{
    EnsureCapacity(copy.ItemCount());
    for (unsigned int i=0; i<copy.m_ItemCount; i++) {
        new ((void*)&m_Items[i]) T(copy.m_Items[i]);
    }
    m_ItemCount = copy.m_ItemCount;
}

/*----------------------------------------------------------------------
|   Array<T>::~Array<T>
+---------------------------------------------------------------------*/
template <typename T>
Array<T>::~Array()
{
    Clear();
    ::operator delete((void*)m_Items);
}

/*----------------------------------------------------------------------
|   Array<T>::operator=
+---------------------------------------------------------------------*/
template <typename T>
Array<T>&
Array<T>::operator=(const Array<T>& copy)
{
    // do nothing if we're assigning to ourselves
    if (this == &copy) return *this;

    // destroy all elements
    Clear();

    // copy all elements from the other object
    EnsureCapacity(copy.ItemCount());
    m_ItemCount = copy.m_ItemCount;
    for (unsigned int i=0; i<copy.m_ItemCount; i++) {
        new ((void*)&m_Items[i]) T(copy.m_Items[i]);
    }

    return *this;
}

/*----------------------------------------------------------------------
|   NPT_Array<T>::Clear
+---------------------------------------------------------------------*/
template <typename T>
Result
Array<T>::Clear()
{
    // destroy all items
    for (Ordinal i=0; i<m_ItemCount; i++) {
        m_Items[i].~T();
    }

    m_ItemCount = 0;

    return SUCCESS;
}

/*----------------------------------------------------------------------
|   Array<T>::EnsureCapacity
+---------------------------------------------------------------------*/
template <typename T>
Result
Array<T>::EnsureCapacity(Cardinal count)
{
    // check if we already have enough
    if (count <= m_AllocatedCount) return SUCCESS;

    // (re)allocate the items
    T* new_items = (T*) ::operator new (count*sizeof(T));
    if (new_items == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }
    if (m_ItemCount && m_Items) {
        for (unsigned int i=0; i<m_ItemCount; i++) {
            new ((void*)&new_items[i]) T(m_Items[i]);
            m_Items[i].~T();
        }
        ::operator delete((void*)m_Items);
    }
    m_Items = new_items;
    m_AllocatedCount = count;

    return SUCCESS;
}

/*----------------------------------------------------------------------
|   Array<T>::SetItemCount
+---------------------------------------------------------------------*/
template <typename T>
Result
Array<T>::SetItemCount(Cardinal item_count)
{
    // shortcut
    if (item_count == m_ItemCount) return SUCCESS;

    // check for a reduction in the number of items
    if (item_count < m_ItemCount) {
        // destruct the items that are no longer needed
        for (unsigned int i=item_count; i<m_ItemCount; i++) {
            m_Items[i].~T();
        }
        m_ItemCount = item_count;
        return SUCCESS;
    }

    // grow the list
    Result result = EnsureCapacity(item_count);
    if (FAILED(result)) return result;

    // construct the new items
    for (unsigned int i=m_ItemCount; i<item_count; i++) {
        new ((void*)&m_Items[i]) T();
    }
    m_ItemCount = item_count;
    return SUCCESS;
}

/*----------------------------------------------------------------------
|   Array<T>::RemoveLast
+---------------------------------------------------------------------*/
template <typename T>
Result
Array<T>::RemoveLast()
{
    if (m_ItemCount) {
        m_Items[--m_ItemCount].~T();
        return SUCCESS;
    } else {
        return ERROR_OUT_OF_RANGE;
    }
}

/*----------------------------------------------------------------------
|   Array<T>::Append
+---------------------------------------------------------------------*/
template <typename T>
Result
Array<T>::Append(const T& item)
{
    // ensure that we have enough space
    if (m_AllocatedCount < m_ItemCount+1) {
        // try double the size, with a minimum
        Cardinal new_count = m_AllocatedCount?2*m_AllocatedCount:ARRAY_INITIAL_COUNT;

        // if that's still not enough, just ask for what we need
        if (new_count < m_ItemCount+1) new_count = m_ItemCount+1;

        // reserve the space
        Result result = EnsureCapacity(new_count);
        if (result != SUCCESS) return result;
    }

    // store the item
    new ((void*)&m_Items[m_ItemCount++]) T(item);

    return SUCCESS;
}

}
#endif //MP4_ARRAY_H
