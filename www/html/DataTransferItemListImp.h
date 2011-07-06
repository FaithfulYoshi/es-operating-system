// Generated by esidl (r1752).
// This file is expected to be modified for the Web IDL interface
// implementation.  Permission to use, copy, modify and distribute
// this file in any software license is hereby granted.

#ifndef ORG_W3C_DOM_BOOTSTRAP_DATATRANSFERITEMLISTIMP_H_INCLUDED
#define ORG_W3C_DOM_BOOTSTRAP_DATATRANSFERITEMLISTIMP_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <org/w3c/dom/html/DataTransferItemList.h>

#include <org/w3c/dom/file/File.h>
#include <org/w3c/dom/html/DataTransferItem.h>

namespace org
{
namespace w3c
{
namespace dom
{
namespace bootstrap
{
class DataTransferItemListImp : public ObjectMixin<DataTransferItemListImp>
{
public:
    // DataTransferItemList
    unsigned int getLength() __attribute__((weak));
    html::DataTransferItem getElement(unsigned int index) __attribute__((weak));
    void deleteElement(unsigned int index) __attribute__((weak));
    void clear() __attribute__((weak));
    html::DataTransferItem add(std::u16string data, std::u16string type) __attribute__((weak));
    html::DataTransferItem add(file::File data) __attribute__((weak));
    // Object
    virtual Any message_(uint32_t selector, const char* id, int argc, Any* argv)
    {
        return html::DataTransferItemList::dispatch(this, selector, id, argc, argv);
    }
    static const char* const getMetaData()
    {
        return html::DataTransferItemList::getMetaData();
    }
};

}
}
}
}

#endif  // ORG_W3C_DOM_BOOTSTRAP_DATATRANSFERITEMLISTIMP_H_INCLUDED
