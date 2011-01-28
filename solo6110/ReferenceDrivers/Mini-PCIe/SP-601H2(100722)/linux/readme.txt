Append vga=0x317 to your kernel boot parameter and linux will load vesafb when reboot.

1)  build and install module(default OS is FC7, kernel version is 2.6.21, the highest supported kernel version is 2.6.21 current)
    building on FC7 steps is:
    # cd ./module
    # make clean
    # make 
    # insmod solo6x10.ko video_type=0  (video_type=0 for NTSC, 1 for PAL)

    for different Linux OS, you should specify the kernel source directory when build, the steps should be:
    # cd ./module
    # make KERNELDIR=/your/kernel/source/directory clean
    # make KERNELDIR=/your/kernel/source/directory 
    # insmod solo6x10.ko video_type=0  (video_type=0 for NTSC, 1 for PAL)

2)  build and install lib
    # cd ./lib
    # make clean
    # make
    # make install

3)  build and run demo
    # cd ./demo
    # make clean
    # make
    # ./scodedemo

You can set encode parameters in the right side of the window, such as SCALE, QP, INTVL and GOP.
INTVL is same as Skipped Frame Number, FPS can be calculated as following: 
---------------------------------------------------------------
      |  Picture Size  |                   FPS	               |
---------------------------------------------------------------      
NTSC  |  D1            |                    |   30 /(INTVL +1) |	
---------------------------------------------------------------
      |  Half D1, CIF  |   IF (INTVL == 0 ) |   60             | 
      --------------------------------------------------------- 
      |                |   ELSE             |   30 /INTVL      |
---------------------------------------------------------------
PAL   |  D1            |                    |   25 /(INTVL +1) |
---------------------------------------------------------------
      |  Half D1, CIF  |   IF (INTVL == 0 ) |   50             |
      ---------------------------------------------------------
      |                |   ELSE             |   25 /INTVL      |
---------------------------------------------------------------                        
Each channel's encode parameter can set separately.
Enable 'Record' label will start recoding, the compressed file will be stored as ./demo/testxxxxxxxxxxxxxx.mp4 
To decode and play it, you can just run ./scodedemo testxxxxxxxxxxxxxx.mp4 

WaterMark:
Input polynomial, initial key and strength(strength range is 0~15) and press 'Enter' , then start record, the compressed file will have watermark.
