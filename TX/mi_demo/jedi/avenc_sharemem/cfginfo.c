//#include "../version.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "cfginfo.h"
#include "Setting.h"
#include "sharemem.h"
#include "version.h"

int AppInitCfgInfoDefault(void)
{
	int i;
	char s[40];
	
    //RUN_STATUS.ucHDMIStatus = HDMI_OUT;
    share_mem->sm_run_status.ucInputStatus = NONE;
    share_mem->sm_run_status.usWidth = 0;
    share_mem->sm_run_status.usHeight = 0;
    share_mem->sm_run_status.ucFrameRate = 0;
    share_mem->sm_run_status.usAudioSampleRate = 0;
    strcpy(share_mem->sm_run_status.strHardwareVer, HD_VERSION);
    strcpy(share_mem->sm_run_status.strSoftwareVer, SW_VERSION);
    
    //ETH
    strcpy(share_mem->sm_eth_setting.strEthIp,"192.168.36.201");
    strcpy(share_mem->sm_eth_setting.strEthMask,"255.255.255.0");
    strcpy(share_mem->sm_eth_setting.strEthGateway,"192.168.36.1");
    strcpy(share_mem->sm_eth_setting.strEthMulticast,"239.255.42.1");

	//client IP and Multicast group
	//share_mem->sm_group_pack.ucIpAddress[0] = 0;
	for (i=0; i<128; i++)
	{
		share_mem->sm_group_pack.ucMultiAddress[i] = 1;
	}
	
	for (i=0; i<128; i++)
	{
		
		sprintf(s,"");
		//printf(s);
		strcpy(share_mem->sm_group_rename.rxRename[i], s);
		//printf(s);
	}
	
	for (i=0; i<24; i++)
	{
		
		sprintf(s,"");
		
		strcpy(share_mem->sm_group_rename.txRename[i], s);
		//printf(s);
		//printf("share_mem->sm_group_rename.txRename[%d] : %s \n", i, share_mem->sm_group_rename.txRename[i]);
	}
	
	//mode
	for (i=0; i<10; i++)
	{
		sprintf(s,"");
		strcpy(share_mem->sm_mode_rename.modeRename[i], s);
	}
	
	share_mem->ucCurrentMode = 1;
	share_mem->ucControlBoxFlag = 0;
	//share_mem->sm_group_pack.uuid = "0";
	//strcpy(share_mem->sm_group_pack.ucIpAddress,"000");
    //strcpy(share_mem->sm_group_pack.ucMultiAddress,"000");
    //strcpy(share_mem->sm_group_pack.uuid,"000");
	
    return 1;
}

int AppInitCfgInfoFromFile(void)
{	
	int iRetCode = 0, i;
	char strTemp[20];
	char s[40];
	int fp = -1;

    printf("open config file \n");
	fp = open(CONFIG_FILE, O_RDONLY);
	printf("fp = %d, fp = %d\n",fp,fp);
	if(fp < 0)
	{
		printf("%s open failed\n",CONFIG_FILE);
		return -1;
	}

	//iRetCode = GetConfigStringValue(fp,"ETH","CONTROL_BOX",strTemp);
	//share_mem->ucControlBoxFlag = atoi(strTemp);
#ifndef BACKUP //761 define 
    printf("get eth ip \n");
	iRetCode = GetConfigStringValue(fp,"ETH","ETH_IP",share_mem->sm_eth_setting.strEthIp);  
	if (iRetCode)
	{
		printf("get Eth IP failed, %d !\n", iRetCode);
		return -2;
	}
	iRetCode = GetConfigStringValue(fp,"ETH","ETH_MULTICAST",share_mem->sm_eth_setting.strEthMulticast);
    printf("get eth ip ok\n");
#endif
	iRetCode = GetConfigStringValue(fp,"ETH","ETH_MASK",share_mem->sm_eth_setting.strEthMask);  

	iRetCode = GetConfigStringValue(fp,"ETH","ETH_GATEWAY",share_mem->sm_eth_setting.strEthGateway);  
	//group
	for (i=0; i<128; i++)
	{
		sprintf(s,"GROUP_MULTICAST[%d]", i);
		iRetCode = GetConfigStringValue(fp,"ETH",s,strTemp);
		//printf(strTemp);
		share_mem->sm_group_pack.ucMultiAddress[i] = atoi(strTemp);
		//printf("share_mem->sm_group_pack.ucMultiAddress[%d] : %d \n", i, share_mem->sm_group_pack.ucMultiAddress[i]);
	}
	//uuid
	#if 0
	for (i=0; i<128; i++)
	{
		sprintf(s,"GROUP_UUID[%d]", i);
		iRetCode = GetConfigStringValue(fp,"ETH",s,strTemp);
		//printf(strTemp);
		//share_mem->sm_group_pack.uuid[i]->data = atoi(strTemp);
	}
	#endif

	//rx rename
	for (i=0; i<128; i++)
	{
		sprintf(s,"GROUP_RXNAME[%d]", i);
		iRetCode = GetConfigStringValue(fp,"ETH",s,strTemp);
		//printf(strTemp);
		strcpy(share_mem->sm_group_rename.rxRename[i], strTemp);
		//printf("share_mem->sm_group_rename.rxRename[%d] : %s \n", i, share_mem->sm_group_rename.rxRename[i]);
	}
	
	//tx rename
	for (i=0; i<24; i++)
	{
		sprintf(s,"GROUP_TXNAME[%d]", i);
		iRetCode = GetConfigStringValue(fp,"ETH",s,strTemp);
		//printf(strTemp);
		strcpy(share_mem->sm_group_rename.txRename[i], strTemp);
		
		//printf("share_mem->sm_group_rename.txRename[%d] : %s \n", i, share_mem->sm_group_rename.txRename[i]);
	}

	if (iRetCode == FAILURE)
	{
		printf("read error of configure file  \n");
		system(RM_CONFIG_FILE);
		sleep(1);
	}
	close(fp);
	
#if 1
	//mode rename
	printf("open config file \n");
	fp = open(CONFIG_FILE1, O_RDONLY);
	printf("fp = %d, fp = %d\n",fp,fp);
	if(fp < 0)
	{
		printf("%s open failed\n",CONFIG_FILE);
		return -1;
	}
	iRetCode = GetConfigStringValue(fp,"ETH","CONTROL_BOX",strTemp);
	share_mem->ucControlBoxFlag = atoi(strTemp);
	for (i=0; i<10; i++)
	{
		sprintf(s,"MODE[%d]", i);
		iRetCode = GetConfigStringValue(fp,"ETH",s,strTemp);
		//printf(strTemp);
		//printf(s);
		strcpy(share_mem->sm_mode_rename.modeRename[i], strTemp);
		//printf("%s",share_mem->sm_mode_rename.modeRename[i]);
	}
	//current mode
	sprintf(s,"CurrentMode");
	iRetCode = GetConfigStringValue(fp,"ETH",s,strTemp);
	//printf(strTemp);
	share_mem->ucCurrentMode = atoi(strTemp);

	printf("end");
	close(fp);
#endif
	return 0;
}

int AppWriteCfgInfotoFile(void)
{
	int iRetCode = 0 , i;
	char strTemp[200];
    FILE* fp;
    fp = fopen(CONFIG_FILE, "w");
    printf("write conf file \n");
    
    //Section Eth
    //fwrite("[ETH]\r\n",strlen("[ETH]\r\n"),1,fp);
    fprintf(fp,"[ETH]\n");
	
#ifndef BACKUP //761 define
    fprintf(fp,"ETH_IP=%s\n",share_mem->sm_eth_setting.strEthIp);
	fprintf(fp,"ETH_MULTICAST=%s\n",share_mem->sm_eth_setting.strEthMulticast);
#endif
    fprintf(fp,"ETH_MASK=%s\n",share_mem->sm_eth_setting.strEthMask);
    fprintf(fp,"ETH_GATEWAY=%s\n",share_mem->sm_eth_setting.strEthGateway);
    //group ip address 
    #if 0
    for (i=0; i<128; i++)
    {
		fprintf(fp,"GROUP_IP[%d]=", i);
		fprintf(fp,"%d\n",share_mem->sm_group_pack.ucIpAddress[i]);
		//fprintf(fp,"\n");
	}
    #endif
    
    //multicast
    for (i=0; i<128; i++)
    {
		fprintf(fp,"GROUP_MULTICAST[%d]=", i);
		fprintf(fp,"%d\n",share_mem->sm_group_pack.ucMultiAddress[i]);
		//fprintf(fp,"\n");
		//printf("%d", share_mem->sm_group_pack.ucMultiAddress[i]);
	}
    
    //uuid
	#if 0
    for (i=0; i<128; i++)
    {
		fprintf(fp,"GROUP_UUID[%d]=", i);
		//fprintf(fp,"%d\n",share_mem->sm_group_pack.uuid[i]->data);
		fprintf(fp,"%d\n",0);
		//fprintf(fp,"\n");
	}
    #endif
    //rx rename
    for (i=0; i<128; i++)
    {
		fprintf(fp, "GROUP_RXNAME[%d]=", i);
		fprintf(fp,"%s\n",share_mem->sm_group_rename.rxRename[i]);
		//printf("%s \n", share_mem->sm_group_rename.rxRename[i]);
	}
    
    //tx rename
    for (i=0; i<24; i++)
    {
		fprintf(fp, "GROUP_TXNAME[%d]=", i);
		fprintf(fp,"%s\n",share_mem->sm_group_rename.txRename[i]);
	}
	
	fprintf(fp,"[END]");
	fclose(fp);
	
	//mode rename
	fp = fopen(CONFIG_FILE1, "w");
	fprintf(fp,"[ETH]\n");
	fprintf(fp,"CONTROL_BOX=%d\n", share_mem->ucControlBoxFlag);
    for (i=0; i<10; i++)
    {
		fprintf(fp, "MODE[%d]=", i);
		fprintf(fp,"%s\n",share_mem->sm_mode_rename.modeRename[i]);
		//printf("Write config : %s",share_mem->sm_mode_rename.modeRename[i]);
	}
	//current mode
	fprintf(fp, "CurrentMode=");
	fprintf(fp,"%d\n",share_mem->ucCurrentMode);
	
	//printf("end");
    fprintf(fp,"[END]");
	fclose(fp);
	printf("write file end \n");
}

int AppWriteModeInfotoFile(void)
{
	int iRetCode = 0 , i; 
	char strTemp[200];
    FILE* fp;
	
	fp = fopen(CONFIG_FILE1, "w");
	fprintf(fp,"[ETH]\n");
	fprintf(fp,"CONTROL_BOX=%d\n", share_mem->ucControlBoxFlag);
    for (i=0; i<10; i++)
    {
		fprintf(fp, "MODE[%d]=", i);
		fprintf(fp,"%s\n",share_mem->sm_mode_rename.modeRename[i]);
		//printf("%s",share_mem->sm_mode_rename.modeRename[i]);
	}
	//current mode
	fprintf(fp, "CurrentMode=");
	fprintf(fp,"%d\n",share_mem->ucCurrentMode);
	
	//printf("end");
    fprintf(fp,"[END]");
	fclose(fp);
}