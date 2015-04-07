#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "isil_enc_sdk.h"
#include "isil_data_stream.h"


/*encode channel  numbers */

static const int chan_nums = 4;


static int GetAudioCallBack( void *av_packet , void *pContext)
{

    

    ISIL_AV_PACKET *pck = (ISIL_AV_PACKET *)av_packet;
    if( !pck ) {
        return -1;
    }
    
    isil_av_packet_clone(pck);
   //fprintf(stderr,"get  data .\n");

	/*DEBUG stream, please look at isil_data_stream.h */
    isil_av_packet_debug(pck);

    isil_av_packet_release(pck);

    return 0;
    
}


/* if you want to exit ,Press CTRL + C*/

int main(int argc ,char *argv[])
{
    int ret ,i ;

    

    ret = ISIL_MediaSDK_Init();
    if(ret < 0 ) {
        exit (-1);
    }

    ret = ISIL_MediaSDK_Run();

    if(ret < 0 ) {
        exit (-1);
    }


	for( i = 0 ; i < chan_nums ; i++) {

		/*register audio get frame */
		ret = ISIL_MediaSDK_RegAudioCallback(0,i,GetAudioCallBack,NULL);
        if(ret < 0 ) {
            fprintf( stderr,"ISIL_MediaSDK_RegAudioCallback err .\n");
            exit (-1);
        }

		/*start transfer stream*/

		ret = ISIL_MediaSDK_StartAudioChannel( 0, i);
        if(ret < 0 ) {
            fprintf( stderr,"ISIL_MediaSDK_StartAudioChannel err .\n");
            exit (-1);
        }
	}


	//ISIL_MediaSDK_Exit();

    ISIL_MediaSDK_Cleanup();
	
	return 0;
}



