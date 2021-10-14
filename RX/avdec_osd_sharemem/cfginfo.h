#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H

int AppInitCfgInfoDefault(void);
int AppInitCfgInfoFromFile(int *fp);
int AppWriteCfgInfotoFile(void);

#endif