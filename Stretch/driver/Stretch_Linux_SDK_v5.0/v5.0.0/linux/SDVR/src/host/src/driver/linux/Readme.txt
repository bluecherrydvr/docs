Stretch S6 dvr linux driver

Directory sturcture.
drv_s6 contains all the device driver code.
lib_sct contains the user space library
test_sct contains a test application which demos how to use the sct api 
to comunicate with the board.  output folder will contains all of the 
binary files after the build.


How to compile.
to compile the whole package, linux need to have gcc, g++, libaio-devel, 
kernel-devel installed.
Type following command on the top level of driver's source tree.
make    : make all the binarys
make load   : load the driver
make unload : unload the driver
make test   : run the test application

To compile it on PPC
User needs to edit host/make.cfg. uncomment powerpc related defines. then type make.

Memory Requirement
Driver will try to use 9M memory per channel. It is recommanded that you have 1G total ram for one 16 channel
card, 2G ram for more more than one 16 channel cards
