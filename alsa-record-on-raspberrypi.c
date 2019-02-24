/* 
  用于树莓派上通过usb音频采集卡进行音频采集，采集pcm数据并直接保存pcm数据
  
  参考资料：1、[A tutorial on using the ALSA Audio API](http://users.suse.com/~mana/alsa090_howto.html)
			2、[ALSA Programming HOWTO v.0.0.8](http://users.suse.com/~mana/alsa090_howto.html)
			3、[ALSA Library API - AlsaProject](https://www.alsa-project.org/wiki/ALSA_Library_API)
  
  声道数：		2
  采样位数：	16bit、LE格式
  采样频率：	44100Hz
  
  需要先安装“libasound2”库
		sudo apt-get install libasound2-dev
	
  使用方法：
		编译：				gcc -o alsa-record-on-raspberrypi -lasound alsa-record-on-raspberrypi.c
		运行：				./alsa-record-on-raspberrypi hw:1 test.pcm
		播放pcm裸数据：		ffplay -ar 44100 -channels 2 -f s16le -i test.pcm
		
	/ * PCM设备的名称，如 plughw：0,0 或 hw: 0,0 * / 
    / *第一个数字是声卡的编号，* / 
    / *第二个数字是设备的编号。* / 
*/

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
	      
void main (int argc, char *argv[])
{
	int i;
	int err;
	char *buffer;
	int buffer_frames = 128;
	unsigned int rate = 44100;			// 采样频率：	44100Hz
	snd_pcm_t *capture_handle;			// 一个指向PCM设备的句柄
	snd_pcm_hw_params_t *hw_params;		// 此结构包含有关硬件的信息，可用于指定PCM流的配置
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;		// 采样位数：16bit、LE格式

	FILE *sig_FOut;		// 一个指向文件的指针，文件输出流

	// 打开音频采集卡硬件，并判断硬件是否打开成功，若打开失败则打印出错误提示
	if ((err = snd_pcm_open (&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0)) < 0) 
	{
		fprintf (stderr, "cannot open audio device %s (%s)\n",  argv[1], snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "audio interface opened\n");

	/* 	
		将流与文件之间的关系建立起来，文件名为 out_pcm.raw，并判断是否设置成功
		w是以文本方式打开文件，
		wb是二进制方式打开文件，
		wb+是读写打开或建立一个二进制文件，允许读和写。
	*/
	if ((sig_FOut = fopen(argv[2], "wb")) == NULL)
	{
		printf("cannot open audio file for recording\n");
		exit (1);
	} 
	fprintf(stdout, "audio file for recording opened\n");

	// 分配一个硬件变量对象，并判断是否分配成功
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) 
	{
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n", snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "hw_params allocated\n");
	
	// 按照默认设置对硬件对象进行设置，并判断是否设置成功
	if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) 
	{
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n", snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "hw_params initialized\n");

	/*
		设置数据为交叉模式，并判断是否设置成功
		interleaved/non interleaved:交叉/非交叉模式。
		表示在多声道数据传输的过程中是采样交叉的模式还是非交叉的模式。
		对多声道数据，如果采样交叉模式，使用一块buffer即可，其中各声道的数据交叉传输；
		如果使用非交叉模式，需要为各声道分别分配一个buffer，各声道数据分别传输。
	*/
	if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) 
	{
		fprintf (stderr, "cannot set access type (%s)\n", snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "hw_params access setted\n");

	// 设置数据编码格式为PCM、有符号、16bit、LE格式，并判断是否设置成功
	if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) 
	{
		fprintf (stderr, "cannot set sample format (%s)\n",  snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "hw_params format setted\n");

	// 设置采样频率，并判断是否设置成功
	if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) 
	{
		fprintf (stderr, "cannot set sample rate (%s)\n", snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "hw_params rate setted\n");

	//  设置为双声道，并判断是否设置成功
	if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0) 
	{
		fprintf (stderr, "cannot set channel count (%s)\n", snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "hw_params channels setted\n");

	// 将配置写入驱动程序中，并判断是否配置成功
	if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) 
	{
		fprintf (stderr, "cannot set parameters (%s)\n", snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "hw_params setted\n");

	// 使采集卡处于空闲状态
	snd_pcm_hw_params_free (hw_params);
	fprintf(stdout, "hw_params freed\n");

	// 准备音频接口，并判断是否准备好
	if ((err = snd_pcm_prepare (capture_handle)) < 0) 
	{
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror (err));
		exit (1);
	}
	fprintf(stdout, "audio interface prepared\n");

	// 配置一个数据缓冲区用来缓冲数据
	buffer = malloc(128 * snd_pcm_format_width(format) / 8 * 2);
	fprintf(stdout, "buffer allocated\n");

	// 开始采集音频pcm数据
	while (1) 
	{
		// 读取
		if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) 
		{
			  fprintf (stderr, "read from audio interface failed (%s)\n", err, snd_strerror (err));
			  exit (1);
		}
		//fprintf(stdout, "read %d done\n", i);
		fwrite(buffer, (buffer_frames*2), sizeof(short), sig_FOut);
		fprintf(stdout, "recording...\r");
	}

	// 释放数据缓冲区
	free(buffer);
	fprintf(stdout, "buffer freed\n");

	// 关闭音频采集卡硬件
	snd_pcm_close (capture_handle);
	fprintf(stdout, "audio interface closed\n");

	// 关闭文件流
	fclose(sig_FOut);
	exit (0);
}
