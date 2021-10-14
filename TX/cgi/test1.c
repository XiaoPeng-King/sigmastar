#include "string.h"
#include "stdlib.h"
#include "dirent.h"
#include <wchar.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
#if 1		
    if(remove("/tmp/rtsp-mdev-vpu0-vpp_pv")==0)
       printf("remove succeed\r\n\r\n");
    else
       printf("remove fail\r\n\r\n");
    system("/bin/killall rtsp-mdev-vpu0-vpp_pv"); 
#endif
	system("/sbin/reboot");
	
	return 0;
}
