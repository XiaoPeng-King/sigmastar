#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>
#include <sys/types.h>

#include "config.h"
  
int GetConfigStringValue(int fpConfig,char *pInSectionName,char *pInKeyName,char *pOutKeyValue)  
{  
	char szBuffer[150];  
	char *pStr1,*pStr2;  
	char *pStr;  
	int iRetCode = 0;  
	int cnt = 0;  
	int seek = 0; 
	unsigned long int count = 0;
	//int timeout = 0;
	usleep(500); //cup 

	iRetCode = lseek(fpConfig, 0, SEEK_SET);
	if (iRetCode < 0) {
		//printf("'lseek' failed, request offset = 0");
		return -1;
	}
	/*test*/  
#if 0 
	printf("pInSectionName: %s !\n",pInSectionName); 
	printf("pInKeyName: %s !\n",pInKeyName); 
#endif

	while(1)  
	{
		//printf("GetConifgStringValue \n");
		count++;
		//printf("\n %d", count);
		if (count > 5000)
		{
			printf("\n\n*** config file error \n\n");
			system(RM_COONFIG);
			//reboot1();
		}
		cnt =0;
		pStr = szBuffer;

		do{
			iRetCode = read(fpConfig, pStr, 1);
			if (iRetCode < 0) {
				break;
			}
			cnt++;
		} while((*pStr ++) != '\n');
		seek = seek + cnt;
		iRetCode = lseek(fpConfig, seek, SEEK_SET);
		if (iRetCode < 0) {
			//printf("'lseek' failed, request offset = 0");
			return -1;
		}

		pStr1 = szBuffer ;    
		while( (' '==*pStr1) || ('\t'==*pStr1) )  
			pStr1++;  
		if( '['==*pStr1 )  
		{  
			pStr1++;  
			while( (' '==*pStr1) || ('\t'==*pStr1) )  
				pStr1++;  
			pStr2 = pStr1;  
			while( (']'!=*pStr1) && ('\0'!=*pStr1) )  
				pStr1++;  
			if( '\0'==*pStr1)     
				continue;  
			while( ' '==*(pStr1-1) )  
				pStr1--;
			*pStr1 = '\0';  

			iRetCode = CompareString(pStr2,pInSectionName);   
			if( !iRetCode )/*检查节名*/  
			{  
				iRetCode = GetKeyValue(fpConfig,pInKeyName,pOutKeyValue);  
				return iRetCode; 
			}     
		}                     
	}  

	return SECTIONNAME_NOTEXIST;  
}  
 
/*区分大小写*/  
int CompareString(char *pInStr1,char *pInStr2)  
{  
	//int timeOut;

	//printf("Compare \n");
    if( strlen(pInStr1)!=strlen(pInStr2) )  
    {  
        return STRING_LENNOTEQUAL;  
    }     
          
    /*while( toupper(*pInStr1)==toupper(*pInStr2) )*//*#include <ctype.h>*/  
    while( *pInStr1==*pInStr2 )  
    {  
        if( '\0'==*pInStr1 )  
            break;    
        pInStr1++;  
        pInStr2++;    
    }  
      
    if( '\0'==*pInStr1 )  
        return STRING_EQUAL;  
          
    return STRING_NOTEQUAL;   
      
}  

int GetKeyValue(int fpConfig,char *pInKeyName,char *pOutKeyValue)  
{  
	char szBuffer[150];  
	char *pStr1,*pStr2,*pStr3;  
	char *pStr;  
	unsigned int uiLen;  
	int iRetCode = 0;  
	int cnt = 0;  
	int seek = 0;
	int timeOut = 0;
	//printf("Key \n");

	memset(szBuffer,0,sizeof(szBuffer));      
	while(1)  
	{   
		timeOut++;
		if (timeOut > 500)
		{
			printf("\n GetKeyValue time out \n");
			return FAILURE;
		}
		//printf("*");
		cnt =0;
		pStr = szBuffer ;    

		do{
			iRetCode = read(fpConfig, pStr, 1);
			if (iRetCode < 0) {
				break;
			}
			cnt++;
		} while((*pStr ++) != '\n');
		seek = seek + cnt;
		iRetCode = lseek(fpConfig, seek, SEEK_SET);
		if (iRetCode < 0) {
			//printf("'lseek' failed, request offset = 0");
			return -1;
		} 
		pStr1 = szBuffer;     
		while( (' '==*pStr1) || ('\t'==*pStr1) )  
			pStr1++;  
		if( '#'==*pStr1 )     
			continue;  
		if( ('/'==*pStr1)&&('/'==*(pStr1+1)) )    
			continue;     
		if( ('\0'==*pStr1)||(0x0d==*pStr1)||(0x0a==*pStr1) )      
			continue;     
		if( '['==*pStr1 )  
		{  
			pStr2 = pStr1;  
			while( (']'!=*pStr1)&&('\0'!=*pStr1) )  
				pStr1++;  
			if( ']'==*pStr1 )  
			{
				break;
			}
			pStr1 = pStr2;    
		}     
		pStr2 = pStr1;  
		while( ('='!=*pStr1)&&('\0'!=*pStr1) )  
			pStr1++;  
		if( '\0'==*pStr1 )    
			continue;  
		pStr3 = pStr1+1;  
		if( pStr2==pStr1 )  
			continue;     
		*pStr1 = '\0';  
		pStr1--;  
		while( (' '==*pStr1)||('\t'==*pStr1) )  
		{  
			*pStr1 = '\0';  
			pStr1--;  
		}  

		iRetCode = CompareString(pStr2,pInKeyName);  
		if( !iRetCode )/*检查键名*/  
		{  
			pStr1 = pStr3;  
			while( (' '==*pStr1)||('\t'==*pStr1) )  
				pStr1++;  
			pStr3 = pStr1;  
			while( ('\0'!=*pStr1)&&(0x0d!=*pStr1)&&(0x0a!=*pStr1) )  
			{  
				if( ('/'==*pStr1)&&('/'==*(pStr1+1)) )  
					break;  
				pStr1++;      
			}     
			*pStr1 = '\0';  
			uiLen = strlen(pStr3);  
			memcpy(pOutKeyValue,pStr3,uiLen);  
			*(pOutKeyValue+uiLen) = '\0';  
			return SUCCESS;  
		}  
	}  

	return KEYNAME_NOTEXIST;  
}

