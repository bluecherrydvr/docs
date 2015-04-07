#include <stdio.h>
#include <stdlib.h>

#include "isil_chan_chip_map.h"
#include "isil_ap_mediasdk.h"



int main(int argc ,char *argv[])
{
	int video_size,ret;
	CHIP_CHN_PHY_ID phy_id;
	unsigned int logid = 19;

	init_glb_sys_chans_map();

	debug_glb_sys_chans_map();

	video_size = get_video_size_by_logic_id(logid);

	fprintf( stderr,"video_size:[%d] .\n",video_size);

	ret = get_chip_chn_phy_id_by_logic_map(&phy_id,logid);
	if(ret < 0) {
		fprintf( stderr,"get phy by log:[%d] err .\n",logid);
		return -1;
	}
	
	fprintf( stderr,"chip_id:[%d] ,chan_id:[%d] .\n",phy_id.chipid,phy_id.chanid);
	return 0;

}
