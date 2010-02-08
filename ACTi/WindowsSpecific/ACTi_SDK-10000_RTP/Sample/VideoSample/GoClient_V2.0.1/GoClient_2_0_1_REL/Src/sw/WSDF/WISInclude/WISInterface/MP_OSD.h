
#ifndef __MP_OSD_H__
#define __MP_OSD_H__

#define	ALIGN_H_LEFT	1		// horizontal align options
#define	ALIGN_H_RIGHT	2
#define ALIGN_H_CENTER	3
#define	ALIGN_H_ABSOLUTE_POS 4
#define ALIGN_V_TOP		5		// vertical align options
#define ALIGN_V_BOTTOM	6
#define ALIGN_V_CENTER	7
#define	ALIGN_V_ABSOLUTE_POS	8

	enum OSD_OPTION { 
		SHADED_TEXT = 1,		// shade 
		ALIGN_HORIZONTAL = 2,	// horizontal align options
		ALIGN_VERTICAL = 4,		// vertical align options
		FONT_SIZE = 8,			// font size
		FONT_COLOR = 16			// in YUV mode, only the intensity of the color is used.
	};


	typedef struct _OSD_DISPLAY_OPTIONS
	{
		UINT8 tag;	// always FF to signify that the options exist in the osd data
		UINT8 h_align_options;	//  left aligned; right aligned; center aligned; absolute position
		UINT8 h_pos;	// valid only when h_align_options == 3 and the value equals to specified starting x position >> 4
		UINT8 v_align_options;	//  top aligned; bottom aligned; center aligned; absolute position
		UINT8 v_pos;	// valid only when v_align_options == 3 and the value equals to specified starting y position >> 4
		UINT8 flag;	// misc flags reserved for osd options, currently only support one option on the lowest bit, if set , shaded text is rendered
		UINT8 font_size;	// osd display font size , default 8
		UINT8 color_rgb[3];

		_OSD_DISPLAY_OPTIONS()
		{
			tag = 0xFF;
			h_align_options = ALIGN_H_RIGHT;
			v_align_options = ALIGN_V_TOP;
			h_pos = v_pos = 0;		// no use
			flag = 1;				// shade
			font_size = 8;			// size 8 font
			color_rgb[0] = color_rgb[1] = color_rgb[2] = 255; // white
		}
	} OSD_DISPLAY_OPTIONS;

#endif