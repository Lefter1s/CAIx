CAIx-qt: Qt5 GUI for CAIx
===============================
Unix
-------

First, make sure that the required packages for Qt5 development of your
distribution are installed, for Debian and Ubuntu these are:

    apt-get install qt5-default qt5-qmake qtbase5-dev-tools qttools5-dev-tools \
        build-essential libboost-dev libboost-system-dev \
        libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev \
        libssl-dev libdb++-dev

Secondly, download our latest github sources and navigate to the created folder in your terminal using cd.  
Then execute the following:

    qmake
    make

Alternatively, install Qt Creator and open the `caix-qt.pro` file.  
An executable named `CAIx-qt` will be built.

Note: if u get a leveldb error run the following commands:

	cd ./src/leveldb
	sudo chmod +x build_detect_platforms
	sudo make libleveldb.a liblevelmeme.a
	cd ../..

And run the qmake and make commands again.