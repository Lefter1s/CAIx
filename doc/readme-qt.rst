CAIx-qt: Qt5 GUI for CAIx
===============================

Build instructions
===================

Debian
-------

First, make sure that the required packages for Qt5 development of your
distribution are installed, for Debian and Ubuntu these are:

::

    apt-get install qt5-default qt5-qmake qtbase5-dev-tools qttools5-dev-tools \
        build-essential libboost-dev libboost-system-dev \
        libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev \
        libssl-dev libdb++-dev

Secondly, download our latest github sources and navigate to the created folder in your terminal using cd.

then execute the following:

::

    qmake
    make

Alternatively, install Qt Creator and open the `caix-qt.pro` file.

An executable named `CAIx-qt` will be built.

* Note: if u get a leveldb error run the following commands:

::
	cd ./src/leveldb
	sudo chmod +x build_detect_platforms
	sudo make libleveldb.a liblevelmeme.a
	cd ../..
	
And run the qmake and make commands again.
	


Windows
--------

Windows build instructions:

- Download and build all required dependencies. There's an easy to follow tutorial on `bitcointalk.org`_. We used openSSL v1.0.1i, Berkeley DB 4.8, Boost v1.55, miniupnc 1.9, protobuf 2.5.0, libpng 1.6.12 and qrencode-3.4.4 with a Qt 5.3.1 environment.

- If you're using newer versions of dependencies, you'll have to adjust the LIB and INCLUDE paths of those dependencies in the caix-qt.pro file

- Clone our git into a folder

- run qmake and make inside the folder

- A CAIx-qt.exe will be made

.. _`bitcointalk.org`: https://bitcointalk.org/index.php?topic=149479.0

Mac OS X
--------

- Install XCode and the Command Line Developer tools

- Download and install `MacPorts`_.

- Execute the following commands in a terminal to get the dependencies:

::

	sudo port selfupdate
	sudo port install boost db48@+no_java openssl miniupnpc autoconf pkgconfig automake libtool 
	sudo port install qt5-mac qrencode protobuf-cpp

- Clone our latest github sources into a folder and navigate to that folder in your terminal by using cd.

- run qmake and make

.. _`MacPorts`: http://www.macports.org/install.php

* Note: if u get a leveldb error run the following commands:

::
	cd ./src/leveldb
	sudo chmod +x build_detect_platforms
	sudo make libleveldb.a liblevelmeme.a
	cd ../..
	
And run the qmake and make commands again.


Build configuration options
============================

UPNnP port forwarding
---------------------

To use UPnP for port forwarding behind a NAT router (recommended, as more connections overall allow for a faster and more stable blackcoin experience), pass the following argument to qmake:

::

    qmake "USE_UPNP=1"

(in **Qt Creator**, you can find the setting for additional qmake arguments under "Projects" -> "Build Settings" -> "Build Steps", then click "Details" next to **qmake**)

This requires miniupnpc for UPnP port mapping.  It can be downloaded from
http://miniupnp.tuxfamily.org/files/.  UPnP support is not compiled in by default.

Set USE_UPNP to a different value to control this:

+------------+--------------------------------------------------------------------------+
| USE_UPNP=- | no UPnP support, miniupnpc not required;                                 |
+------------+--------------------------------------------------------------------------+
| USE_UPNP=0 | (the default) built with UPnP, support turned off by default at runtime; |
+------------+--------------------------------------------------------------------------+
| USE_UPNP=1 | build with UPnP support turned on by default at runtime.                 |
+------------+--------------------------------------------------------------------------+

Notification support for recent (k)ubuntu versions
---------------------------------------------------

To see desktop notifications on (k)ubuntu versions starting from 10.04, enable usage of the
FreeDesktop notification interface through DBUS using the following qmake option:

::

    qmake "USE_DBUS=1"

Generation of QR codes
-----------------------

libqrencode may be used to generate QRCode images for payment requests. 
It can be downloaded from http://fukuchi.org/works/qrencode/index.html.en, or installed via your package manager. Pass the USE_QRCODE 
flag to qmake to control this:

+--------------+---------------------------------------------------------------------------------------+
| USE_QRCODE=0 | No QRCode support - libarcode not required                                            |
+--------------+---------------------------------------------------------------------------------------+
| USE_QRCODE=1 | (Always enabled with our latest wallet) QRCode support enabled                        |
+--------------+---------------------------------------------------------------------------------------+


Berkely DB version warning
==========================

A warning for people using the *static binary* version of BlackCoin on a Linux/UNIX-ish system (tl;dr: **Berkely DB databases are not forward compatible**).

The static binary version of BlackCoin is linked against libdb 5.0 (see also `this Debian issue`_).

Now the nasty thing is that databases from 5.X are not compatible with 4.X.

If the globally installed development package of Berkely DB installed on your system is 5.X, any source you
build yourself will be linked against that. The first time you run with a 5.X version the database will be upgraded,
and 4.X cannot open the new format. This means that you cannot go back to the old statically linked version without
significant hassle!

.. _`this Debian issue`: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=621425

Ubuntu 11.10 warning
====================

Ubuntu 11.10 has a package called 'qt-at-spi' installed by default.  At the time of writing, having that package
installed causes blackcoin-qt to crash intermittently.  The issue has been reported as `launchpad bug 857790`_, but
isn't yet fixed.

Until the bug is fixed, you can remove the qt-at-spi package to work around the problem, though this will presumably
disable screen reader functionality for Qt apps:

::

    sudo apt-get remove qt-at-spi

.. _`launchpad bug 857790`: https://bugs.launchpad.net/ubuntu/+source/qt-at-spi/+bug/857790
