
//把二维码转换为BMP图片

#include <stdio.h>
#include <qrencode.h>  //二维码库的头文件
#include <string.h>
#include <stdlib.h>

#pragma pack(1) //字节对齐

typedef struct BITMAPFILEHEADER /* size: 14 */
{
   unsigned char bfType[2];        	// 文件的类型，也就是字符'BM'。
   unsigned int  bfSize;        	// 位图文件的大小，用字节为单位
   unsigned short bfReserved1;		// 保留，必须设置为0
   unsigned short bfReserved2;		// 保留，必须设置为0
   unsigned int  bfOffBits;			// 位图数据距离文件开头偏移量，用字节为单位
} BMPFILEHEADER;

  
typedef struct BITMAPINFOHEADER  /* size: 40 */
{
   unsigned int  biSize;             // BITMAPINFOHEADER结构所需要的字数
   unsigned int  Width;              // 图像宽度，单位为像素
   unsigned int  Height;             // 图像高度，单位为像素，负数，则说明图像是正向的
   unsigned short biPlanes;          // 为目标设备说明位面数，其值将总是被设为1
   unsigned short biBitCount;        // 一个像素占用的bit位，值位1、4、8、16、24、32
   unsigned int  biCompression;      // 压缩类型
   unsigned int  biSizeImage;        // 位图数据的大小，以字节为单位
   unsigned int  biXPelsPerMeter;    // 水平分辨率，单位 像素/米
   unsigned int  biYPelsPerMeter;    // 垂直分辨率，单位 像素/米
   unsigned int  biClrUsed;          // 当该字段设置为 0 时，颜色表中的颜色数量基于 biBitCount 字段（1 表示 2 种颜色，4 表示 16 种，8 表示 256 种，24 表示无颜色表）。
   unsigned int  biClrImportant;     // 指定颜色表的前 x 颜色对 DIB 很重要。如果其余颜色不可用，图像仍然以可接受的方式保留其含义。 
} BMPINFOHEADER;

#pragma pack() //取消字节对齐


int main(int argc, char const *argv[])
{
	int width  = 0;
	int height = 0;
	
	//1.调用libqrencode库的函数接口，把字符串编码为二维码图像  如果打算发送http请求时携带一些信息，则可以在URL中使用?的方式进行查询式的发送 ?后面需要跟着键值对  key=value  多个键值对之间使用&
	
	QRcode *qrdata = QRcode_encodeString("http://baidu.com", 0 , QR_ECLEVEL_M, QR_MODE_8,1);
	//QRcode *qrdata = QRcode_encodeString("http://8.138.80.120:80?phonenumber=xxxxxxxxxxx", 0 , QR_ECLEVEL_M, QR_MODE_8,1);

	printf("version = %d\n",qrdata->version);	//版本说明
	printf("width = %d\n",qrdata->width);		//宽度说明   二维码是宽==高
	
	width  = qrdata->width;
	height = qrdata->width; 

	//2.创建bmp图片，并设置bmp图片的头部信息(54字节)
	FILE * bmp_fp = fopen("qrcode.bmp","wb");

	BMPFILEHEADER bmp_fileheader; //存储文件头  14字节
	BMPINFOHEADER bmp_infoheader; //存储信息头  40字节

	int line_size = width * 24 / 8 + ( ( 4 - (width*3 % 4) ) % 4 ) ; //BMP图片每行必须是4的倍数。以字节为单位  == 图片宽度 * 像素色深 / 8 + 需要补齐的字节数
	printf("line size = %d\n",line_size);

	//构造文件头
	bmp_fileheader.bfType[0] = 0x42;
	bmp_fileheader.bfType[1] = 0x4D;
	bmp_fileheader.bfSize    = line_size * height + 54;	 //图像大小，以字节为单位
	bmp_fileheader.bfReserved1 = 0;
	bmp_fileheader.bfReserved2 = 0;
	bmp_fileheader.bfOffBits   = 54;

	//构造信息头
	bmp_infoheader.biSize   = 40;
	bmp_infoheader.Width = width;
	bmp_infoheader.Height= height;
	bmp_infoheader.biPlanes = 1;
	bmp_infoheader.biBitCount = 24;
	bmp_infoheader.biCompression = 0;
	bmp_infoheader.biSizeImage = line_size * height;
	bmp_infoheader.biXPelsPerMeter =0;
	bmp_infoheader.biYPelsPerMeter =0;
	bmp_infoheader.biClrUsed       =0;
	bmp_infoheader.biClrImportant  =0;

	//3.把构造的BMP的头部信息先写入BMP文件中
	fwrite(&bmp_fileheader,1,sizeof(bmp_fileheader),bmp_fp);
	fwrite(&bmp_infoheader,1,sizeof(bmp_infoheader),bmp_fp);


	//4.准备一块缓冲区，把二维码对应的黑白点的像素写入到缓存区
	unsigned char * buf = calloc(1,line_size* height);

	memset(buf,0xFF,line_size* height);  //相当于缓冲区中每个字节都是0xFF，由于该缓冲区是为了存储二维码颜色分量的 所以R=0xFF G=0xFF  B=0xFF  所以背景是白色


	for (int y = 0; y <height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{	
			//判断二维码中的当前字节是否是黑色
			if(qrdata->data[y*width+x] & 0x01)
			{
				//BMP一个像素点是3字节  B G R 
				buf[(height-y-1)*line_size + 3*x  ] = 0;
				buf[(height-y-1)*line_size + 3*x+1] = 0;
				buf[(height-y-1)*line_size + 3*x+2] = 0;
			}
		}
	}

	//5.把已经存储二维码颜色分量的缓冲区写入到BMP图片
	fwrite(buf,1,line_size* height,bmp_fp);

	//6.关闭BMP
	fclose(bmp_fp);
	return 0;
}
