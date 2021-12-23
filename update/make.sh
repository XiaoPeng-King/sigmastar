gcc -o CECoFF_PC CECoFF.c 
gcc -o CECoN_PC CECoN.c 
gcc -o SysCmd_PC SysCmd.c 

gcc -o UpGradeServer_PC UpGradeServer.c 

arm-linux-gnueabihf-gcc -o UpGradeClient UpGradeClient.c -Wall -O2 -fPIC -mcpu=cortex-a9 -mfpu=neon-fp16 -mfloat-abi=hard -mthumb-interwork -marm -lm -lpthread

cp UpGradeServer_PC /mnt/nfs/RX
cp UpGradeServer_PC /mnt/nfs/TX
cp UpGradeClient /mnt/nfs/RX
cp UpGradeClient /mnt/nfs/TX






