prog_audio_all_test_case使用说明:
-t: 程序的运行时间（秒数），不指定则会一直运行 Y 10
-I: 使能AI Y
-o: AI录音的输出路径 Y /mnt
-d: AI的设备ID(Amic[0] Dmic[1] I2S RX[2] Linein[3]) Y 0
-m： AI的声道模式(Mono[0] Stereo[1] Queue[2]) Y 1
-c: AI通道数 Y 1
-v: AI音量参数(Amic 0~21, Dmic 0~4, Linein 0~7) Y 0
-s: AI采样率8000/16000/32000/48000 Y 48000
-h: 使能AI Hpf N
-g: 使能AI Agc N
-e: 使能AI Eq N
-n: 使能AI NR N
-r: AI 重采样采样率8000/16000/32000/48000 N
-a: AI 编码类型g711a/g711u/g726_16/g726_24/g726_32/g726_40 Y g711a
-A: 使能AED N
-b: 使能AEC N 
-B： 使能Hw loop back AEC N 
-S: 使能SSL N 
-F: 使能BF N 
-M： mic的间距（使能SSL或BF时，必须设置，步长为1cm） N 
-w: 指定AEC运行的采样率(大部分情况下不需要设置) N 
-W：开启录制原始的PCM Y 
-C: 指定bf的角度 N 


-O: 使能AO
-i: AO播放的输入文件路径
-D: AO设备ID(Lineout[0] I2S TX[1])
-V: AO音量参数
-H: 使能AO Hpf
-G: 使能AO Agc
-E: 使能AO Eq
-N: 使能AO NR
-R: AO 重采样采样率8000/16000/32000/48000

./prog_audio -t 10 -I -o /mnt -d 0 -m 1 -c 1 -v 0 -s 48000 -a g711a -W
./prog_audio_all_test_case -t 10 -I -o /customer -d 0 -m 0 -c 1 -v 0 -s 48000 -a g726_16 -W


AI OutPut Path:/mnt
Device:Amic
ChnNum:1
Mode:Stereo
Aed:Disable
Vqe working sample rate:not specified
Aec:Disable
Hpf:Disable
Nr:Disable
Agc:Disable
Eq:Disable
Ssl:Disable
Bf:Disable
Resample:Disable
