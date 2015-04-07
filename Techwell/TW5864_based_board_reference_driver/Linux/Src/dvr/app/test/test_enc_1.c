#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "isil_sdk_main.h"
#include "isil_enc_sdk.h"
#include "isil_data_stream.h"

#define TEST_ENC_TOTAL_NUM (16)

int GetH264CallBack( void *av_packet , void *pContext)
{

    

    ISIL_AV_PACKET *pck = (ISIL_AV_PACKET *)av_packet;
    if( !pck ) {
        return -1;
    }
    
    isil_av_packet_clone(pck);
   //fprintf(stderr,"get  data .\n");
//    isil_av_packet_debug(pck);
    isil_av_packet_release(pck);

    return 0;
    
}

#if 0

void thread_callback1( void* arg)
{
    
    int ret;
    int i = *((int *)arg);

    while(i--) {
        DEBUG_FUNCTION();
        ret = isil_msg_set_param((void *)(&i) , msg_callback1,0);
        if( ret < 0 ) {
            fprintf(stderr,"msg return valu"
                           "e err .\n");
            break;
        }
        DEBUG_FUNCTION();

    }

    return;
}

#endif


int main(int argc , char *argv[])
{

    int ret ;
    int i;
    

    ret = ISIL_MediaSDK_Init();
    if(ret < 0 ) {
        return -1;
    }



#if 1
    ret = ISIL_MediaSDK_Run();

    if(ret < 0 ) {
        return -1;
    }
#endif

    for(i = 0 ; i <  TEST_ENC_TOTAL_NUM ; i ++) {
        
        ret = ISIL_MediaSDK_RegH264MainCallback(0,i,GetH264CallBack,NULL);
        if(ret < 0 ) {
            fprintf( stderr,"ISIL_MediaSDK_RegH264MainCallback err .\n");
            return -1;
        }

        

        ret = ISIL_MediaSDK_RegH264SubCallback(0,i,GetH264CallBack,NULL);
        if(ret < 0 ) {
            fprintf( stderr,"ISIL_MediaSDK_RegH264SubCallback err .\n");
            return -1;
        }

        
        ret = ISIL_MediaSDK_RegMJpegCallback(0,i,GetH264CallBack,NULL);
        if(ret < 0 ) {
            fprintf( stderr,"ISIL_MediaSDK_RegMjpegCallback err .\n");
            return -1;
        }

        

        ret = ISIL_MediaSDK_RegAudioCallback(0,i,GetH264CallBack,NULL);
        if(ret < 0 ) {
            fprintf( stderr,"ISIL_MediaSDK_RegAudioCallback err .\n");
            return -1;
        }

    
        
    }


    for(i = 0 ; i <  TEST_ENC_TOTAL_NUM ; i ++) {
        
        ret = ISIL_MediaSDK_StartH264MainChannel(0,i);
        if(ret < 0 ) {
            fprintf( stderr,"ISIL_MediaSDK_StartH264MainChannel err .\n");
            return -1;
        }

        ret = ISIL_MediaSDK_StartH264SubChannel(0,i);
        if(ret < 0 ) {
            fprintf( stderr,"ISIL_MediaSDK_StartH264SubChannel err .\n");
            return -1;
        }

        ret = ISIL_MediaSDK_StartCapMjpeg(0,i);
        if(ret < 0 ) {
            fprintf( stderr,"ISIL_MediaSDK_StartCapMjpeg err .\n");
            return -1;
        }

        ret = ISIL_MediaSDK_StartAudioChannel(0,i);
        if(ret < 0 ) {
            fprintf( stderr,"ISIL_MediaSDK_StartAudioChannel err .\n");
            return -1;
        }


        
    }


    sleep(5);
    
    ISIL_MediaSDK_Exit();
    ISIL_MediaSDK_Cleanup();


    return 0;
}

