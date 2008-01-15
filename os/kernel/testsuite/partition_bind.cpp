/*
 * Copyright 2008 Google Inc.
 * Copyright 2006 Nintendo Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <new>
#include <stdlib.h>
#include <es.h>
#include <es/handle.h>
#include "vdisk.h"
#include "partition.h"

#define TEST(exp)                           \
    (void) ((exp) ||                        \
            (esPanic(__FILE__, __LINE__, "\nFailed test " #exp), 0))

IDiskManagement::Geometry VDiskGeometry;

static void SetData(u8* buf, long long size)
{
    buf[size-1] = 0;
    while (--size)
    {
        *buf++ = 'a' + size % 26;
    }
}

void PrintPartitions(IContext* context)
{
    char name[16];
    IDiskManagement::Partition params;
    esReport("boot type offset   size\n");
    Handle<IIterator> iter = context->list("");
    Handle<IBinding> binding;
    while ((binding = iter->next()))
    {
        Handle<IDiskManagement> diskManagement = binding->getObject();
        TEST(diskManagement);
        diskManagement->getLayout(&params);
        TEST(0 < binding->getName(name, sizeof(name)));
        esReport("%02x   %02x   %08llx %08llx %s\n",
                 params.bootIndicator,
                 params.partitionType,
                 params.startingOffset,
                 params.partitionLength,
                 name);

        Handle<IStream> stream = context->lookup(name);
        TEST(stream);
        long long size;
        size = stream->getSize();
        TEST(params.partitionLength == size);
        TEST(params.bootIndicator == 0x00 || params.bootIndicator == 0x80);
        TEST(params.partitionType != 0x00);
    }
    esReport("\n");
}

void CheckGeometry(Handle<IDiskManagement> dm)
{
    IDiskManagement::Geometry geometry;
    dm->getGeometry(&geometry); // throw exception when error occurs.
#ifdef VERBOSE
    esReport("Geometry\n");
    esReport("  heads           %u\n", geometry.heads);
    esReport("  cylinders       %u\n", geometry.cylinders);
    esReport("  sectorsPerTrack %u\n", geometry.sectorsPerTrack);
    esReport("  bytesPerSector  %u\n", geometry.bytesPerSector);
    esReport("  diskSize        %llu\n", geometry.diskSize);
#endif // VERBOSE
    TEST(geometry.heads == VDiskGeometry.heads);
    TEST(geometry.cylinders == VDiskGeometry.cylinders);
    TEST(geometry.sectorsPerTrack == VDiskGeometry.sectorsPerTrack);
    TEST(geometry.bytesPerSector == VDiskGeometry.bytesPerSector);
    TEST(geometry.diskSize == VDiskGeometry.diskSize);
}

void CreatePartition(IContext* context, const char* name, long long& size, u8 type)
{
    esReport("Create: %s (size %lld, type 0x%02x)\n", name ,size, type);

    Handle<IBinding> binding;
    binding = context->bind(name, 0);
    TEST(binding);

    Handle<IStream> stream = context->lookup(name);
    TEST(stream);
    Handle<IStream> object = binding->getObject();
    TEST(stream == object);

    CheckGeometry(stream);

    Handle<IDiskManagement> diskManagement = stream;
    IDiskManagement::Partition params;
    diskManagement->getLayout(&params);

    stream->setSize(size);
    size = stream->getSize();

    // set the partition type.
    diskManagement->getLayout(&params);
    params.partitionType = type;

    diskManagement->setLayout(&params);
    diskManagement->getLayout(&params);
    TEST(params.partitionType == type);
}

void Test(IContext* context)
{
    Handle<IBinding> binding;
    Handle<IStream> stream;

    // Partition0 does not exist.
    binding = context->bind("partition1", 0);
    TEST(!binding);

    long long size = 8 * 1024 * 1024LL;
    CreatePartition(context, "partition0", size, 0x04);

    size = 16 * 1024 * 1024LL;
    CreatePartition(context, "partition1", size, 0x04);

    binding = context->bind("partition1", 0);
    stream = binding->getObject();
    TEST(stream);

    long long newSize;
    newSize = stream->getSize();
    TEST(newSize == size);

    size = 8 * 1024 * 1024LL;
    CreatePartition(context, "extended", size, 0x05);
    binding = context->bind("extended", 0);
    stream = binding->getObject();
    TEST(stream);

    newSize = stream->getSize();
    TEST(newSize == size);

    size = 4 * 1024 * 1024LL;
    CreatePartition(context, "logical0", size, 0x01);

    size = 4 * 1024 * 1024LL;
    CreatePartition(context, "logical1", size, 0x01);

    binding = context->bind("logical1", 0);
    stream = binding->getObject();
    TEST(stream);

    newSize = stream->getSize();
    TEST(newSize == size);

    // Invalid name.
    binding = context->bind("partition4", 0);
    TEST(!binding);

    binding = context->bind("extended1", 0);
    TEST(!binding);

    binding = context->bind("partition1024", 0);
    TEST(!binding);

    binding = context->bind("logical1024", 0);
    TEST(!binding);

    binding = context->bind("partition*", 0);
    TEST(!binding);

    binding = context->bind("", 0);
    TEST(!binding);

    binding = context->bind("part", 0);
    TEST(!binding);

    binding = context->bind("log", 0);
    TEST(!binding);

    binding = context->bind("ext", 0);
    TEST(!binding);

    binding = context->bind("invalid", 0);
    TEST(!binding);

    // check variations are allowed.
    binding = context->bind("partition02", 0);
    TEST(binding);
    char name[16];
    binding->getName(name, sizeof(name));
    TEST(strcmp(name, "partition2") == 0);

    binding = context->bind("partition0002", 0);
    binding->getName(name, sizeof(name));
    TEST(strcmp(name, "partition2") == 0);

    binding = context->bind("partition000", 0);
    TEST(binding);
    binding->getName(name, sizeof(name));
    TEST(strcmp(name, "partition0") == 0);

    binding = context->bind("extended0", 0);
    TEST(binding);
    binding->getName(name, sizeof(name));
    TEST(strcmp(name, "extended") == 0);
}

void Init(IContext* context)
{
    // remove all partitions.
    esReport("Remove all partitions.\n");
    context->unbind("partition0");
    context->unbind("partition1");
    context->unbind("partition2");
    context->unbind("partition3");

    char name[16];
    int id = 0;
    sprintf(name, "logical%u", id);

    Handle<IStream> stream;
    Handle<IBinding> binding;
    while (stream = context->lookup(name))
    {
        ++id;
        sprintf(name, "logical%u", id);
    }

    while (id)
    {
        --id;
        sprintf(name, "logical%u", id);
        context->unbind(name);
    }

    context->unbind("extended0");

    // check
    stream = context->lookup("partition0");
    TEST(!stream);
    stream = context->lookup("partition1");
    TEST(!stream);
    stream = context->lookup("partition2");
    TEST(!stream);
    stream = context->lookup("partition3");
    TEST(!stream);
    stream = context->lookup("extended0");
    TEST(!stream);
}

int main(int argc, char* argv[])
{
    IInterface* ns = 0;
    esInit(&ns);

    Handle<IStream> disk = new VDisk(static_cast<char*>("fat16_32MB.img"));
    Handle<IDiskManagement> dm = disk;
    dm->getGeometry(&VDiskGeometry);

    Handle<IPartition> partition = new PartitionContext();
    TEST(partition);

    // mount
    TEST(partition->mount(disk) == 0);
    {
        Handle<IContext> context = partition;
        TEST(context);

        Init(context);

        Test(context);

        // show results.
        esReport("\nPartition Entry Table\n");
        PrintPartitions(context);
    }

    // unmount
    TEST(partition->unmount() == 0);
    partition = 0;

    partition = new PartitionContext();
    TEST(partition);
    TEST(partition->mount(disk) == 0);
    {
        Handle<IContext> context = partition;
        // check
        Handle<IStream> stream;
        TEST(stream = context->lookup("partition0"));
        TEST(stream = context->lookup("partition1"));
        TEST(stream = context->lookup("partition2"));
        TEST(stream = context->lookup("extended"));
        TEST(stream = context->lookup("logical0"));
        TEST(stream = context->lookup("logical1"));
        TEST(!(stream = context->lookup("logical2")));
    }
    TEST(partition->unmount() == 0);

    esReport("done.\n\n");
}
