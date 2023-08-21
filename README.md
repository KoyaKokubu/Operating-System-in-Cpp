# Operating-System-in-Cpp
This is based on "MikanOS" which is an educational Operating System created in Japan.
You can buy the book from [here](https://www.amazon.com/%E3%82%BC%E3%83%AD%E3%81%8B%E3%82%89%E3%81%AEOS%E8%87%AA%E4%BD%9C%E5%85%A5%E9%96%80/dp/4839975868).

# Files
- MikanLoaderPkg
    - The MikanOS loader as a UEFI Application
- kernel
    - Kernel of MikanOS

# What I learned and implemented
- UEFI BIOS, UEFI application and EDK II
- Boot Loader
- PCI
- USB driver
- Class driver for USB Mouse and Keyboard
- Interruption
- Memory Management
- Graphic User Interface (ex. window system, layer, etc.)
- Timer and APCI
- Pre-emptive Multi-tasking
- Terminal and commands (ex. echo, clear, lspci, cd, ls, free, more, |, cat, etc.)
- File System (FAT32)
- Paging
- System Call
- Applications via Cpp (ex. 2D game, text editor, etc.)
- Read/Write files from application
- Application memory management
    - Demand Paging
    - Memory Mapped File
    - Copy-on-write
- Character code (UTF-8)
