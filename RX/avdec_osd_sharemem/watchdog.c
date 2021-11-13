#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

void *watchdog_main(void *arg)
{
	int fd_watchdog = -1;
	char food = 'a';
	fd_watchdog = open("/dev/watchdog", O_WRONLY);

	while (1)
	{
		sleep(1);
		write(fd_watchdog, &food, 1);
		//printf("feed dog \n");
	}
	return 0;
}



