/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2004 Live Networks, Inc.  All rights reserved.
// RTP sink for H.263+ video (RFC 2429)
// Implementation

#include "H263plusVideoRTPSink.hh"

H263plusVideoRTPSink
::H263plusVideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs,
		       unsigned char rtpPayloadFormat)
  : VideoRTPSink(env, RTPgs, rtpPayloadFormat, 90000, "H263-1998"),
    fAreInFragmentedFrame(False) {
}

H263plusVideoRTPSink::~H263plusVideoRTPSink() {
}

H263plusVideoRTPSink*
H263plusVideoRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs,
				unsigned char rtpPayloadFormat) {
  return new H263plusVideoRTPSink(env, RTPgs, rtpPayloadFormat);
}

Boolean H263plusVideoRTPSink
::frameCanAppearAfterPacketStart(unsigned char const* /*frameStart*/,
				 unsigned /*numBytesInFrame*/) const {
  // A packet can contain only one frame
  return False;
}

void H263plusVideoRTPSink
::doSpecialFrameHandling(unsigned fragmentationOffset,
			 unsigned char* frameStart,
			 unsigned numBytesInFrame,
			 struct timeval frameTimestamp,
			 unsigned numRemainingBytes) {
  if (fragmentationOffset == 0) {
    // This packet contains the first (or only) fragment of the frame.
    // Set the 'P' bit in the special header:
    unsigned short specialHeader = 0x0400;

    // Also, reuse the first two bytes of the payload for this special
    // header.  (They should both have been zero.)
    if (numBytesInFrame < 2) {
      envir() << "H263plusVideoRTPSink::doSpecialFrameHandling(): bad frame size "
	      << numBytesInFrame << "\n";
      return;
    }
    if (frameStart[0] != 0 || frameStart[1] != 0) {
      envir() << "H263plusVideoRTPSink::doSpecialFrameHandling(): unexpected non-zero first two bytes: "
	      << (void*)(frameStart[0]) << "," << (void*)(frameStart[1]) << "\n";
    }
    frameStart[0] = specialHeader>>8;
    frameStart[1] = (unsigned char)specialHeader;
  } else {
    unsigned short specialHeader = 0;
    setSpecialHeaderBytes((unsigned char*)&specialHeader, 2);
  }

  if (numRemainingBytes == 0) {
    // This packet contains the last (or only) fragment of the frame.
    // Set the RTP 'M' ('marker') bit:
    setMarkerBit();
  }

  fAreInFragmentedFrame = (numRemainingBytes > 0); // for next time

  // Also set the RTP timestamp:
  setTimestamp(frameTimestamp);
}


unsigned H263plusVideoRTPSink::specialHeaderSize() const {
  // There's a 2-byte special video header.  However, if we're the first
  // (or only) fragment of a frame, then we reuse the first 2 bytes of
  // the payload instead.
  return fAreInFragmentedFrame ? 2 : 0;
}