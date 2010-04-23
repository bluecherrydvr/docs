About
=====
This document describes some different ways to access, store and play
the MPEG-4 stream, delivered by Axis cameras, on a Linux client. The document 
describes both standard Linux applications and the Axis open source application
(rtsp_client) that is provided with this package. 

rtsp_client (with provided C++ source code) is an application that retrieves an 
elementary stream (video or audio) from an MPEG capable Axis camera or video 
server. The retrieved stream can then be used to create a .mp4 that can be 
stored for later viewing.

Note!  The provided source code is intended to be used as example and 
inspiration  only. Axis Communications AB does not provide any form of support
for the code. 
Axis Communications AB provides no guarantees that any of the examples will 
work for any particular application.
Axis Communications AB cannot and will not be held liable for any damage 
inflicted to any device as a result of the examples or instructions mentioned.

mplayer
=======
The mplayer application for Linux (http://www.mplayerhq.hu/) can retrieve
and play MPEG streams from Axis cameras. To be able to retrieve the streams,
mplayer needs the live.com library (http://live555.com/mplayer).

To retrieve and display video over RTP+RTSP (data goes over UDP) do e.g.

mplayer rtsp://<ip_address>/mpeg4/media.amp
or mplayer -fps 25 rtsp://<ip_address>/mpeg4/media.amp

To access it over RTP/RTSP (data goes over TCP) do:

mplayer -rtsp-stream-over-tcp rtsp://<ip_address>/mpeg4/media.amp

To access a specific camera do e.g.:

mplayer rtsp://<ip_address>/mpeg4/2/media.amp

rtsp_client
===========
To build the application do (you need to have g++ installed):
cd rtsp_client
make

Typical usages are:
"./rtsp_client -s <ip_address>"            Gets the video stream over RTP and 
                                           print statistics
"./rtsp_client -r -c 2 <ip_address>"       Gets the video stream from camera 2 
                                           over RTP/RTSP.
"./rtsp_client -h -o <file> <ip_address>"  Gets the video stream over 
                                           RTP/RTSP/HTTP and store it in file.
"./rtsp_client -a -o <file> <ip_address>"  Gets the audio stream over RTP and
                                           store it in file.

The provided source code can be used as base for or inspiration for
your own client.

An alternative to rtsp_client is to use Live Network's openRTSP client 
(http://live555.com/openRTSP/).

mpeg4ip
=======
MPEG4IP (http://mpeg4ip.sourceforge.net) is a large package for Linux with lots 
of useful stuff for MPEG4. One of the useful applications is mp4creator that
creates a .mp4 file that can be viewed with e.g. mplayer or quicktime.

One very useful sequence is to store an MPEG video stream with rtsp_client and 
encode it with mp4creator:
./rtsp_client -o out.mp4v
Press CTRL-C to stop the recording
and then create a .mp4 file with
mp4creator -r 25 -c out.mp4v out.mp4
This clip can then be viewed by
mplayer out.mp4

If you want the audio stream you can do the same thing:
./rtsp_client -a -o out.mp4v
Press CTRL-C to stop the recording
and then create a .mp4 file with
mp4creator -c out.mp4v out.mp4
This clip can then be viewed by
mplayer out.mp4

You can also retrieve both audio and video and combine it into one .mp4 file
by running two instances of rtsp_client and run mp4 creator twice:

Shell 1                                 Shell 2
=======                                 =======
./rtsp_client -o video.mp4v             ./rtsp_client -a -o audio.mp4v
Press CTRL-C to stop the recording      Press CTRL-C to stop the recording
and then create a .mp4 file with        
mp4creator -r 25 -c video.mp4v out.mp4
mp4creator -c audio.mp4v out.mp4
This clip can then be viewed by
mplayer out.mp4

To make this really useful rtsp_client needs to modified so that it fetches
both audio and video simultaneously and stores the streams in two separate
files.


