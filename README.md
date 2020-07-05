# DiskSector
Protect Your Important Sectors!

------------------------------------------
# What is it?
This is a driver with the `IRP_of Hook`\Device\\\Harddisk0\\DR0`MJ_WRITE` to filter sector writes.

# What is its default behavior?
It filters by default writes to 0 sectors on the *first disk* and no R3 program can write to 0 sectors on the *first disk*

------------------------------------------
# 这是什么?
这是一个驱动,以Hook`\\Device\\Harddisk0\\DR0`的`IRP_MJ_WRITE`以过滤对扇区的写入.

# 它的默认行为是什么?
它默认过滤*第一块磁盘*上0扇区的写入,任何R3程序无法写入*第一块磁盘*的0扇区
