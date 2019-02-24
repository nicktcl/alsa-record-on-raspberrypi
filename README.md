## 用于树莓派上通过usb音频采集卡进行音频采集，采集pcm数据并直接保存pcm数据
  

## 音频参数：

声道数：		2
  
采样位数：	16bit、LE格式

采样频率：	44100Hz

## 运行环境

树莓派系统镜像：“2018-11-13-raspbian-stretch-full.img”


## 使用方法：

将以上代码保存为“alsa-record-on-raspberrypi.c”，然后在linux终端上运行以下命令。

1、需要先安装“libasound2”库
```linux
sudo apt-get install libasound2-dev
```
2、编译：
```linux
gcc -o alsa-record-on-raspberrypi -lasound alsa-record-on-raspberrypi.c
```
3、运行，按“ctrl＋c”结束运行。
```linux
./alsa-record-on-raspberrypi hw:1 test.pcm
```
	/ * PCM设备的名称，如 plughw：0,0 或 hw: 0,0 * / 
    / *第一个数字是声卡的编号，* / 
    / *第二个数字是设备的编号。* / 
4、播放pcm裸数据进行测试（需要安装ffmpeg）。
```linux
ffplay -ar 44100 -channels 2 -f s16le -i test.pcm
```

## 参考资料：

1、[A tutorial on using the ALSA Audio API](http://users.suse.com/~mana/alsa090_howto.html)
2、[ALSA Programming HOWTO v.0.0.8](http://users.suse.com/~mana/alsa090_howto.html)
3、[ALSA Library API - AlsaProject](https://www.alsa-project.org/wiki/ALSA_Library_API)