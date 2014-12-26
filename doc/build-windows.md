CAIx-qt: Qt5 GUI for CAIx
===============================
Windows
--------

Windows build instructions:

- Download and build all required dependencies. There's an easy to follow tutorial on [bitcointalk.org](https://bitcointalk.org/index.php?topic=149479.0). 
- We used openSSL v1.0.1j, Berkeley DB 4.8, Boost v1.57, miniupnc 1.9, protobuf 2.6.1, libpng 1.6.15 and qrencode-3.4.4 with a Qt 5.4 environment.
- If you're using newer versions of dependencies, you'll have to adjust the LIB and INCLUDE paths of those dependencies in the caix-qt.pro file
- Clone our git into a folder
- run qmake and make inside the folder
- A CAIx-qt.exe will be made