#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "isil_chan_chip_map.h"

static SYS_CHIPS_CHAN_PRI_MAP_T glb_sys_chans_map;

static CHIP_CHANS_DEF_T glb_chip_chans_def_arr[]=
{
	{ TEST_CHIP0_SD , TEST_CHIP0_HD },
	{ TEST_CHIP1_SD , TEST_CHIP1_HD },
	{ TEST_CHIP2_SD , TEST_CHIP2_HD },
	{ TEST_CHIP3_SD , TEST_CHIP3_HD }
};





static int check_glb_chip_chans_def_arr( void )
{
	int i ;
	CHIP_CHANS_DEF_T *def_t;

	if( TEST_CHIP_CNT <= 0) {
		fprintf( stderr ,"TEST_CHIP_CNT is %d.\n", TEST_CHIP_CNT);
		return -1;
	}

	if(TEST_CHIP_CNT >= (sizeof(glb_chip_chans_def_arr)/sizeof(glb_chip_chans_def_arr[0]))) {
		fprintf( stderr ,"TEST_CHIP_CNT too big,is %d.\n", TEST_CHIP_CNT);
		return -1;
	}

	for( i = 0 ; i < TEST_CHIP_CNT ; i++) {

		def_t = &glb_chip_chans_def_arr[i];
		if( (def_t->sd_chan_cnts + def_t->hd_chan_cnts) > MAX_ENC_NUM_PER_CHIP ) {
			fprintf( stderr ,"chip[%d] :sd_chan_cnts+hd_chan_cnts  %d.\n",i, def_t->sd_chan_cnts + def_t->hd_chan_cnts);
			return -1;
		}

	}

	return 0;
}



static int init_chip_chan_pri_map( CHIP_CHAN_PRI_MAP_T * map_t,
							 unsigned int chip_id ,
							 unsigned int logic_chan_start_value)
{
	int i, logic_chan_end_value = -1;
	int valid_sd , valid_hd;
	CHIP_CHANS_DEF_T *def_t;

	

	if( !map_t ) {
		fprintf( stderr ,"init_chip_chan_pri_map err,map_t is null.\n");
		assert(0);
	}

	if( chip_id >= TEST_CHIP_CNT) {
		fprintf( stderr ,"chip_id >= %d[TEST_CHIP_CNT].\n",TEST_CHIP_CNT);
		assert(0);
	}
	
	
	
	memset(map_t , 0x00 , sizeof(CHIP_CHAN_PRI_MAP_T));
	map_t->chip_id = chip_id;
	def_t = &glb_chip_chans_def_arr[chip_id];

	map_t->valid_chan_nums = def_t->sd_chan_cnts + def_t->hd_chan_cnts;

	if( !map_t->valid_chan_nums ) {
		fprintf( stderr ,"valid_chan_nums = 0.\n");
		assert(0);
	}

	if( map_t->valid_chan_nums > MAX_ENC_NUM_PER_CHIP) {
		fprintf( stderr ,"valid_chan_nums >= %d[MAX_ENC_NUM_PER_CHIP].\n",MAX_ENC_NUM_PER_CHIP);
		assert(0);
	}
	
	

	valid_sd = def_t->sd_chan_cnts;
	valid_hd = def_t->hd_chan_cnts;


	
	for( i = 0 ; i < MAX_ENC_NUM_PER_CHIP ; i++ ) {

		if(valid_sd) {
			logic_chan_end_value = logic_chan_start_value+i;
			map_t->logic_chanid_arr[i] = logic_chan_end_value;
			map_t->log_chan_v_size_arr[i] = V_CHAN_SIZE_IS_SD;
			valid_sd--;
		}
		else{
			if(valid_hd) {
				logic_chan_end_value = logic_chan_start_value+i;
				map_t->logic_chanid_arr[i] = logic_chan_end_value;
				map_t->log_chan_v_size_arr[i] = V_CHAN_SIZE_IS_HD;
				valid_hd--;
			}
			
		}
	}

	return (logic_chan_end_value+1);
}


SYS_CHIPS_CHAN_PRI_MAP_T* get_glb_sys_chips_chan_pri_map( void )
{
	return &glb_sys_chans_map;
}

void init_glb_sys_chans_map( void )
{
	int i , ret;
	unsigned int log_chan_start = 0;
	CHIP_CHAN_PRI_MAP_T * map_t;
	ret = check_glb_chip_chans_def_arr();
	if( ret < 0) {
		fprintf( stderr ,"check_glb_chip_chans_def_arr err.\n");
		assert(0);
	}

	glb_sys_chans_map.chip_nums = TEST_CHIP_CNT;
	
	for( i = 0 ; i < TEST_CHIP_CNT ; i++) {
		map_t = &glb_sys_chans_map.chip_chan_pri_map_arr[i];
		
		ret = init_chip_chan_pri_map(map_t,i,log_chan_start);
		if( ret < 0 ) {
			fprintf( stderr ,"init_chip_chan_pri_map err.\n");
			assert(0);
		}
		log_chan_start = ret;
	} 

}


int get_video_size_by_logic_id(unsigned int log_id)
{
	int i,arr_value,ret = -1;
	CHIP_CHAN_PRI_MAP_T * map_t;

	

	for( i = 0 ; i < glb_sys_chans_map.chip_nums ; i++) {

		map_t = &glb_sys_chans_map.chip_chan_pri_map_arr[i];
		if(log_id >= (map_t->logic_chanid_arr[0] + map_t->valid_chan_nums)) {
			continue;
		}
		else{
			arr_value = log_id - map_t->logic_chanid_arr[0];
			return map_t->log_chan_v_size_arr[arr_value];
		}

	}

	return ret;


}


int get_chip_chn_phy_id_by_logic_map(CHIP_CHN_PHY_ID * phy_id,unsigned int log_id)
{
	int i ,arr_value ;
	
	CHIP_CHAN_PRI_MAP_T * map_t;

	if( !phy_id ) {
		return -1;
	}

	for( i = 0 ; i < glb_sys_chans_map.chip_nums ; i++){
		map_t = &glb_sys_chans_map.chip_chan_pri_map_arr[i];
		if(log_id >= (map_t->logic_chanid_arr[0] + map_t->valid_chan_nums)) {
			continue;
		}
		arr_value = log_id - map_t->logic_chanid_arr[0];
		phy_id->chipid = map_t->chip_id;
		phy_id->chanid = arr_value;
		return 0;
	}

	return -1;
	
}


int get_valid_chan_num_by_chip( unsigned int chip_id)
{
	int ret;

	CHIP_CHAN_PRI_MAP_T * map_t;

	if(chip_id >= glb_sys_chans_map.chip_nums) {
		return -1;
	}

	map_t = &glb_sys_chans_map.chip_chan_pri_map_arr[chip_id];
	return map_t->valid_chan_nums;

}



static void debug_chip_chan_pri_map( CHIP_CHAN_PRI_MAP_T * map_t )
{
	int i;

	if( !map_t ) {
		return;
	}

	fprintf( stderr,"chip_id:[%d] ,valid_chan_nums:[%d].\n",
			 map_t->chip_id,
			 map_t->valid_chan_nums);

	for( i = 0 ; i < map_t->valid_chan_nums ;i++) {
		printf(" log num:[%d] ,v_pri:[%d]  ",
				 map_t->logic_chanid_arr[i],
				 map_t->log_chan_v_size_arr[i]);
		if(i%6 == 0) {
			fprintf(stderr,"\n");
		}
	}

	fprintf(stderr,"\n");

	return;

}


void debug_glb_sys_chans_map( void )
{
	int i ;
	CHIP_CHAN_PRI_MAP_T* map_t;

	fprintf( stderr,"chips num:[%d] .\n",glb_sys_chans_map.chip_nums);

	for(i = 0 ; i < (int)glb_sys_chans_map.chip_nums ; ++i ) {

		map_t = &glb_sys_chans_map.chip_chan_pri_map_arr[i];
		debug_chip_chan_pri_map(map_t);
	}

	return;
}



