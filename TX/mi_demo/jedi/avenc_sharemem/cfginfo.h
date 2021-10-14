#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H


#define RM_CONFIG_FILE ("rm ./config.conf")
#define CONFIG_FILE1 ("./modeName.conf")
#define CONFIG_FILE ("./config.conf")

int AppInitCfgInfoDefault(void);
int AppInitCfgInfoFromFile(void);
int AppWriteCfgInfotoFile(void);
int AppWriteModeInfotoFile(void);

#endif
