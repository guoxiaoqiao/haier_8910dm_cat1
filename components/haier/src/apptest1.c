#include "apptest1.h"
#include "apptest2.h"
#include "lib_test4.h"

//累加和校验算法
unsigned char DataAccumulateSumCRC1(void *DataBuff, unsigned int DataLen)
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
	
	/*unsigned int crc=0;
	unsigned int i;
	unsigned char *Ptr = (unsigned char*)DataBuff;*/
	
	//return DataAccumulateSumCRC(DataBuff, DataLen);
	OSI_LOGI(0, "[zkd] hello world end result=%d", Lib_test4(DataBuff, DataLen));
	return 0;
}

