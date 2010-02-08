#include "stdafx.h"
#include "version.h"
#include "Utility.h"

int fnTrans( const unsigned char *buf, const int buflen, unsigned char *target, const int maxsize )
{
	if( target == NULL )
	{
		return 0;
	}

	unsigned char *pbuffer ;
	int nbufferlen = 0;

	nbufferlen = maxsize;
	pbuffer = target;

	//memset( pbuffer, 0, nbufferlen);

	int ret = 0;
	int jpeghdr_len = sizeof(jpeghdr);
	int jpeghdr_rst_len = sizeof(jpeghdr_rst);
	int rtpjhdr_qtab_len = sizeof(jpeghdr_qtable);
	int rtpjhdr_len = jpeghdr_len+jpeghdr_rst_len+rtpjhdr_qtab_len;


	jpeghdr * rtpjhdr = (jpeghdr *)&buf[0];
	jpeghdr_rst * rtpjhdr_rst = (jpeghdr_rst *)&buf[ jpeghdr_len];
	jpeghdr_qtable * rtpjhdr_qtab = (jpeghdr_qtable *)&buf[ jpeghdr_len + jpeghdr_rst_len];

	unsigned char *p = (unsigned char *)target;
	unsigned char *first = (unsigned char *)target;


	{ // SOI
		*p++ = 0xFF;
		*p++ = 0xD8;
		*p++ = 0xFF;
		*p++ = 0xE0;
		*p++ = 0x00;
		*p++ = 0x10;

		*p++ = 'J';
		*p++ = 'F';
		*p++ = 'I';
		*p++ = 'F';
		*p++ = 0x00;

		*p++ = 0x01;	// version (2 bytes)
		*p++ = 0x01;

		*p++ = 0x00;	// units (1 byte) Units for the X and Y densities.
						// units = 0: no units, X and Y specify the pixel aspect ratio
						// units = 1: X and Y are dots per inch
						// units = 2: X and Y are dots per cm

		*p++ = 0x00;	// Xdensity (2 bytes) Horizontal pixel density
		*p++ = 0x01;	
		*p++ = 0x00;	// Ydensity (2 bytes) Vertical pixel density
		*p++ = 0x01;	
		*p++ = 0x00;	// Xthumbnail (1 byte) Thumbnail horizontal pixel count
		*p++ = 0x00;	// Ythumbnail (1 byte) Thumbnail vertical pixel count
	}

	int quanttab_len = htons( rtpjhdr_qtab->length);
	{	// QUANT
		const unsigned char * tablep = &buf[ rtpjhdr_len];
		if( quanttab_len != 64*2 )
		{
			return 0;
		}
		*p++ = 0xFF;
		*p++ = 0xDB;
		*p++ = 0x00;	// len (2 bytes)
		*p++ = 0x43;
		*p++ = 0x00;	// quant tab 0 (1 byte)

		memcpy( p, tablep, 64 );
		p += 64;
		
		*p++ = 0xFF;
		*p++ = 0xDB;
		*p++ = 0x00;	// len (2 bytes)
		*p++ = 0x43;
		*p++ = 0x01;	// quant tab 1 (1 byte)

		memcpy( p, tablep+64, 64 );
		p += 64;
	}

	{	// SOF
		*p++ = 0xFF;
		*p++ = 0xC0;		/* SOF */
		*p++ = 0;			/* length msb */
		*p++ = 17;			/* length lsb */

		if( rtpjhdr_qtab->precision == 0 )
		{
			*p++ = 8;
		}
		else
		{
			*p++ = rtpjhdr_qtab->precision*8+8;
		}

		int h = rtpjhdr->height << 3;
		int w = rtpjhdr->width << 3;

		*p++ = h >> 8;	/* height msb */
		*p++ = h;		/* height lsb */
		*p++ = w >> 8;	/* width msb */
		*p++ = w;		/* wudth lsb */

		*p++ = 3;	/* number of components */
		*p++ = 1;	/* comp 1 */

		if( rtpjhdr_rst->dri )
			rtpjhdr->type &= 0x40;

		if (rtpjhdr->type == 0)
			*p++ = 0x21;		/* hsamp = 2, vsamp = 1 */
		else
			*p++ = 0x22;		/* hsamp = 2, vsamp = 2 */

		*p++ = 0x00;	/* quant table 0 */
		*p++ = 0x02;	/* comp 1 */
		*p++ = 0x11;	/* hsamp = 1, vsamp = 1 */
		*p++ = 0x01;	/* quant table 1 */
		*p++ = 0x03;	/* comp 2 */
		*p++ = 0x11;	/* hsamp = 1, vsamp = 1 */
		*p++ = 0x01;	/* quant table 1 */
	}

	{
		*p++ = 0xFF;
		*p++ = 0xDD;
		*p++ = 0x00;
		*p++ = 0x04;

		
		*p++ = rtpjhdr_rst->dri & 0xff;	/* dri lsb */
		*p++ = rtpjhdr_rst->dri >> 8;	/* dri msb */

	}

	{	// SOS
		*p++ = 0xFF;
		*p++ = 0xDA;
		*p++ = 0x00;
		*p++ = 0x0C;
		*p++ = 0x03;
		*p++ = 0x01;
		*p++ = 0x00;
		*p++ = 0x02;
		*p++ = 0x11;
		*p++ = 0x03;
		*p++ = 0x11;
		*p++ = 0x00;
		*p++ = 0x3F;
		*p++ = 0x00;
	}

	int rtpjhdr_full_len = (rtpjhdr_len + quanttab_len);
	int njpegdata_len = buflen-rtpjhdr_full_len;

	memcpy( p, buf+rtpjhdr_full_len, njpegdata_len);

	ret = (int)(p-first) + njpegdata_len;

	return ret;
}
