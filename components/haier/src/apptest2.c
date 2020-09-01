#include "haier_config.h"
#include "apptest2.h"
#include "lib_test2.h"

//累加和校验算法
unsigned char DataAccumulateSumCRC2(void *DataBuff, unsigned int DataLen)
{
	/*unsigned int crc=0;
	unsigned int i;
	unsigned char *Ptr = (unsigned char*)DataBuff;

	//if((Ptr == NULL) || (DataLen==0))
		//return crc;
    for(i=0; i < DataLen; i++)
    {
    	crc += *Ptr++;
    }
	return (unsigned char)(crc&0xff);  // 取低位字节*/
	
	//unsigned int crc=0;
	//unsigned int i;
	unsigned char *Ptr = (unsigned char*)DataBuff;
	
	return Lib_test2(Ptr, DataLen);
	
	//return DataAccumulateSumCRC1(Ptr, DataLen);
}

int gTestData = 1;
int gTestBss = 0;

static void zk_ThreadEntry(void *param)
{
    //OSI_LOGI(0, "application thread enter, param 0x%x", param);
	unsigned int len = 10;
	unsigned char buff[10];

    for (int n = 0; n < 50; n++)
    {
        OSI_LOGI(0, "[zkd] hello world %d result=%d", n, DataAccumulateSumCRC2(buff, len));
		
        osiThreadSleep(2000);
    }

    osiThreadExit();
}

int zk_appimg_enter()
{
    //OSI_LOGI(0, "application image enter, param 0x%x", param);
    OSI_LOGI(0, "[zk] data=%d (expect 1), bss=%d", gTestData, gTestBss);

    osiThreadCreate("zkthread", zk_ThreadEntry, NULL, OSI_PRIORITY_NORMAL, 1024, 0);
    return 0;
}