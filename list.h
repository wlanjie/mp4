//
// Created by wlanjie on 2018/2/7.
//

#ifndef MP4_LIST_H
#define MP4_LIST_H

#include "types.h"
#include "results.h"

namespace mp4 {
// TODO name
template <typename T>
class List
{
public:
    // types
    class Item {
    public:
        // types
        class Operator {
        public:
            // methods
            virtual ~Operator() {}
            virtual Result action(T* data) const = 0;
        };

        class Finder {
        public:
            // methods
            virtual ~Finder() {}
            virtual Result test(T* data) const = 0;
        };

        // methods
        Item(T* data) : m_Data(data), m_Next(0), m_Prev(0) {}
        ~Item() {}
        Item* getNext() { return m_Next; }
        Item* getPrev() { return m_Prev; }
        T*    getData() { return m_Data; }

    private:
        // members
        T*    m_Data;
        Item* m_Next;
        Item* m_Prev;

        // friends
        friend class List;
    };

    // methods
    List<T>(): m_ItemCount(0), m_Head(0), m_Tail(0) {}
    virtual     ~List<T>();
    Result   clear();
    Result   add(T *data);
    Result   add(Item *item);
    Result   remove(Item *item);
    Result   remove(T *data);
    Result   insert(Item *where, T *data);
    Result   get(Ordinal idx, T *&data) const;
    Result   popHead(T *&data);
    Result   apply(const typename Item::Operator &op) const;
    Result   applyUntilFailure(const typename Item::Operator &op) const;
    Result   applyUntilSuccess(const typename Item::Operator &op) const ;
    Result   reverseApply(const typename Item::Operator &op) const;
    Result   find(const typename Item::Finder &finder, T *&data) const;
    Result   reverseFind(const typename Item::Finder &finder, T *&data) const;
    Result   deleteReferences();
    Cardinal itemCount() const { return m_ItemCount; }
    Item* firstItem() const { return m_Head; }
    Item* lastItem()  const { return m_Tail; }

protected:
    // members
    Cardinal m_ItemCount;
    Item*    m_Head;
    Item*    m_Tail;

private:
    // these cannot be used
    List<T>(const List<T>&);
    List<T>& operator=(const List<T>&);
};

template <typename T>
List<T>::~List()
{
    clear();
}

template <typename T>
inline Result List<T>::clear() {
    Item* item = m_Head;

    while (item) {
        Item* next = item->m_Next;
        delete item;
        item = next;
    }
    m_ItemCount = 0;
    m_Head = m_Tail = NULL;

    return SUCCESS;
}

template <typename T>
inline Result List<T>::add(T *data) {
    return add(new Item(data));
}

template <typename T>
Result List<T>::add(Item *item) {
    // add element at the tail
    if (m_Tail) {
        item->m_Prev = m_Tail;
        item->m_Next = NULL;
        m_Tail->m_Next = item;
        m_Tail = item;
    } else {
        m_Head = item;
        m_Tail = item;
        item->m_Next = NULL;
        item->m_Prev = NULL;
    }

    // one more item in the list now
    m_ItemCount++;

    return SUCCESS;
}

template <typename T>
Result List<T>::remove(Item *item) {
    if (item->m_Prev) {
        // item is not the head
        if (item->m_Next) {
            // item is not the tail
            item->m_Next->m_Prev = item->m_Prev;
            item->m_Prev->m_Next = item->m_Next;
        } else {
            // item is the tail
            m_Tail = item->m_Prev;
            m_Tail->m_Next = NULL;
        }
    } else {
        // item is the head
        m_Head = item->m_Next;
        if (m_Head) {
            // item is not the tail
            m_Head->m_Prev = NULL;
        } else {
            // item is also the tail
            m_Tail = NULL;
        }
    }

    // delete the item
    delete item;

    // one less item in the list now
    m_ItemCount--;

    return SUCCESS;
}

template <typename T>
Result List<T>::remove(T *data) {
    Item* item = m_Head;

    while (item) {
        if (item->m_Data == data) {
            // delete item
            return remove(item);
        }
        item = item->m_Next;
    }

    return ERROR_NO_SUCH_ITEM;
}

template <typename T>
Result List<T>::insert(Item *where, T *data) {
    Item* item = new Item(data);

    if (where == NULL) {
        // insert as the head
        if (m_Head) {
            // replace the current head
            item->m_Prev = NULL;
            item->m_Next = m_Head;
            m_Head->m_Prev = item;
            m_Head = item;
        } else {
            // this item becomes the head and tail
            m_Head = item;
            m_Tail = item;
            item->m_Next = NULL;
            item->m_Prev = NULL;
        }
    } else {
        // insert after the 'where' item
        if (where == m_Tail) {
            // add the item at the end
            return add(item);
        } else {
            // update the links
            item->m_Prev = where;
            item->m_Next = where->m_Next;
            where->m_Next->m_Prev = item;
            where->m_Next = item;
        }
    }

    // one more item in the list now
    ++m_ItemCount;

    return SUCCESS;
}

template <typename T>
Result List<T>::get(Ordinal idx, T *&data) const {
    Item* item = m_Head;

    if (idx < m_ItemCount) {
        while (idx--) item = item->m_Next;
        data = item->m_Data;
        return SUCCESS;
    } else {
        data = NULL;
        return ERROR_NO_SUCH_ITEM;
    }
}

template <typename T>
Result List<T>::popHead(T *&data) {
    // check that we have at least one item
    if (m_Head == NULL) {
        return ERROR_LIST_EMPTY;
    }

    // remove the item and return it
    data = m_Head->m_Data;
    Item* head = m_Head;
    m_Head = m_Head->m_Next;
    if (m_Head) {
        m_Head->m_Prev = NULL;
    } else {
        m_Tail = NULL;
    }

    // delete item
    delete head;

    // one less item in the list now
    m_ItemCount--;

    return SUCCESS;
}

template <typename T>
inline Result List<T>::apply(const typename Item::Operator &op) const {
    Item* item = m_Head;

    while (item) {
        op.action(item->m_Data);
        item = item->m_Next;
    }

    return SUCCESS;
}

template <typename T>
inline Result List<T>::applyUntilFailure(const typename Item::Operator &op) const {
    Item* item = m_Head;

    while (item) {
        Result result;
        result = op.Action(item->m_Data);
        if (result != SUCCESS) return result;
        item = item->m_Next;
    }

    return SUCCESS;
}

template <typename T>
inline Result List<T>::applyUntilSuccess(const typename Item::Operator &op) const {
    Item* item = m_Head;

    while (item) {
        Result result;
        result = op.Action(item->m_Data);
        if (result == SUCCESS) return SUCCESS;
        item = item->m_Next;
    }

    return FAILURE;
}

template <typename T>
inline Result List<T>::reverseApply(const typename Item::Operator &op) const
{
    Item* item = m_Tail;

    while (item) {
        if (op.Action(item->m_Data) != SUCCESS) {
            return ERROR_LIST_OPERATION_ABORTED;
        }
        item = item->m_Prev;
    }

    return SUCCESS;
}

template <typename T>
inline Result List<T>::find(const typename Item::Finder &finder, T *&data) const {
    Item* item = m_Head;

    while (item) {
        if (finder.test(item->m_Data) == SUCCESS) {
            data = item->m_Data;
            return SUCCESS;
        }
        item = item->m_Next;
    }

    data = NULL;
    return ERROR_NO_SUCH_ITEM;
}

template <typename T>
inline Result List<T>::reverseFind(const typename Item::Finder &finder, T *&data) const {
    Item* item = m_Tail;

    while (item) {
        if (finder.test(item->m_Data) == SUCCESS) {
            data = item->m_Data;
            return SUCCESS;
        }
        item = item->m_Prev;
    }

    data = NULL;
    return ERROR_NO_SUCH_ITEM;
}

template <typename T>
inline Result List<T>::deleteReferences() {
    Item* item = m_Head;

    while (item) {
        Item* next = item->m_Next;
        delete item->m_Data;
        delete item;
        item = next;
    }

    // no more items
    m_Head = m_Tail = NULL;
    m_ItemCount = 0;

    return SUCCESS;
}

}
#endif //MP4_LIST_H
