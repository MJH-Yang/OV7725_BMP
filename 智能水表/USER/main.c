#include "delay.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "piclib.h"
#include "ov7725.h"
#include "key.h"
#include "exti.h"
#include "led.h"
#include "imageprocess.h"

extern u8 ov_sta;	//��exit.c���涨��
extern u16 USART2_RX_STA;//��usart.c�ж���
extern u8 USART2_RX_BUF[USART_REC_LEN];//��usart.c�ж���
extern u8 data[11];//��crc16.c�ж���
extern char res[22];//��crc16.c�ж���

//����OV7725��������װ��ʽԭ��,OV7725_WINDOW_WIDTH�൱��LCD�ĸ߶ȣ�OV7725_WINDOW_HEIGHT�൱��LCD�Ŀ��
//ע�⣺�˺궨��ֻ��OV7725��Ч
#define  OV7725_WINDOW_WIDTH		320 // <=320
#define  OV7725_WINDOW_HEIGHT		240 // <=240

FRESULT res_sd;//�ļ��������
FIL fnew; //�ļ�
UINT fnum; //�ļ��ɹ���д����

//����LCD��ʾ
void camera_refresh(void)
{
	u32 i,j;
 	u16 color;
	BITMAPINFO bmp;
	
	//����S1����ͼƬ
	if(ov_sta && KEY_Scan(1) == S1)
	{
		//���ļ����������ھʹ���
		res_sd = f_open(&fnew, "0:test1.bmp", FA_OPEN_ALWAYS | FA_WRITE);
		
		//�ļ��򿪳ɹ�
		if(res_sd == FR_OK)
		{
			//��д�ļ���Ϣͷ��Ϣ  
			bmp.bmfHeader.bfType = 0x4D42;				//bmp����  
			bmp.bmfHeader.bfOffBits=sizeof(bmp.bmfHeader) + sizeof(bmp.bmiHeader) + sizeof(bmp.RGB_MASK);						//λͼ��Ϣ�ṹ����ռ���ֽ���
			bmp.bmfHeader.bfSize= bmp.bmfHeader.bfOffBits + 320*240*2;	//�ļ���С����Ϣ�ṹ��+�������ݣ�
			bmp.bmfHeader.bfReserved1 = 0x0000;		//����������Ϊ0
			bmp.bmfHeader.bfReserved2 = 0x0000;  			
			
			//��дλͼ��Ϣͷ��Ϣ  
			bmp.bmiHeader.biSize=sizeof(bmp.bmiHeader);  				    //λͼ��Ϣͷ�Ĵ�С
			bmp.bmiHeader.biWidth=320;  														//λͼ�Ŀ��
			bmp.bmiHeader.biHeight=240;  			    //ͼ��ĸ߶�
			bmp.bmiHeader.biPlanes=1;  				    //Ŀ�����ļ��𣬱�����1
			bmp.bmiHeader.biBitCount=16;          //ÿ����λ��
			bmp.bmiHeader.biCompression=3;  	    //ÿ�����صı�����ָ�������루RGB565���룩������  (�ǳ���Ҫ)
			bmp.bmiHeader.biSizeImage=320*240*2;  //ʵ��λͼ��ռ�õ��ֽ�����������λͼ�������ݣ�
			bmp.bmiHeader.biXPelsPerMeter=0;			//ˮƽ�ֱ���
			bmp.bmiHeader.biYPelsPerMeter=0; 			//��ֱ�ֱ���
			bmp.bmiHeader.biClrImportant=0;   	  //˵��ͼ����ʾ����ҪӰ�����ɫ������Ŀ��0�������е���ɫһ����Ҫ
			bmp.bmiHeader.biClrUsed=0;  			    //λͼʵ��ʹ�õĲ�ɫ���е���ɫ��������0��ʾʹ�����еĵ�ɫ����
			
			//RGB565��ʽ����
			bmp.RGB_MASK[0] = 0X00F800;
			bmp.RGB_MASK[1] = 0X0007E0;
			bmp.RGB_MASK[2] = 0X00001F;
			
			//д�ļ�ͷ���ļ�  
			res_sd= f_write(&fnew, &bmp, sizeof(bmp), &fnum);
		
			//��ָ�븴λ
			OV7725_RRST=0;				//��ʼ��λ��ָ��
			OV7725_RCK_L;
			OV7725_RCK_H;
			OV7725_RCK_L;
			OV7725_RRST=1;				//��λ��ָ����� 
			OV7725_RCK_H; 
			
			/*ͼ������ԭ�����ڶ�ȡʱ�ĸ��źͶ�ȡʱ©����������*/
			for(i=0;i<240;i++)
			{
				for(j=0;j<320;j++)
				{
					OV7725_RCK_L;
					color=GPIOC->IDR&0XFF;	//������
					OV7725_RCK_H; 
					color<<=8;  
					OV7725_RCK_L;
					color|=GPIOC->IDR&0XFF;	//������
					OV7725_RCK_H; 
					
					//дλͼ��Ϣͷ���ڴ濨
					f_write(&fnew, &color, sizeof(color), &fnum);
				}
			}
		}
				
		//�ر��ļ�
		f_close(&fnew);
	}
	
	//û�а������£�ˢ��LCD
	if(ov_sta)
	{
		LCD_Scan_Dir(U2D_L2R);		//���ϵ���,������ 
		LCD_WriteRAM_Prepare();   //��ʼд��GRAM	
		
		//��ָ�븴λ
		OV7725_RRST=0;				//��ʼ��λ��ָ�� 
		OV7725_RCK_L;
		OV7725_RCK_H;
		OV7725_RCK_L;
		OV7725_RRST=1;				//��λ��ָ����� 
		OV7725_RCK_H; 
		
		/*ͼ������ԭ�����ڶ�ȡʱ�ĸ��źͶ�ȡʱ©����������*/
		for(i=0;i<240;i++)
		{
			for(j=0;j<320;j++)
			{
				OV7725_RCK_L;
				color=GPIOC->IDR&0XFF;	//������
				OV7725_RCK_H; 
				color<<=8;  
				OV7725_RCK_L;
				color|=GPIOC->IDR&0XFF;	//������
				OV7725_RCK_H; 
				LCD->LCD_RAM=color; 
			}
		}
		ov_sta=0;					//��ʼ��һ�βɼ�
		LCD_Scan_Dir(DFT_SCAN_DIR);	//�ָ�Ĭ��ɨ�跽�� 
	}
}

//ͨ������2����ATָ���NB-IoTģ��
void SendData()
{
	data[0] = 0x01;
	data[1] = 0x46;
	data[2] = 0x00;
	data[3] = 0x00;
	data[4] = 0x00;
	data[5] = 0x01;
	data[6] = 0x02;
	data[7] = 0x00;//����
	data[8] = 0x12;
				
	getCrc16(data, res);
				
	Usart_SendString(USART2, "AT+NMGS=11,");
	Usart_SendString(USART2, res);
	Usart_SendString(USART2, "\r\n");
}

int main(void)
{
	u8 lightmode=0,saturation=2,brightness=2,contrast=2,effect=0;
	u8 i = 0, j = 0;
	int res[5];//ʶ����
	char r[6];
	u8 flag = 0;
	
	delay_init();	    	//��ʱ������ʼ��	  
	uart_init(9600);	 	//���ڳ�ʼ��Ϊ9600
	usart2_init(115200);
	LCD_Init();					//��ʼ��LCDҺ����ʾ��
	KEY_Init();					//������ʼ��
 	mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��(����Ҫ)
 	exfuns_init();			//Ϊfatfs��ر��������ڴ�  
  f_mount(0,fs[0]); 	//����SD�� 
	piclib_init();			//��ʼ����ͼ

	while(OV7725_Init() != 0);				//��ʼ��OV7725����ͷ
	
	POINT_COLOR = RED;
	LCD_ShowString(60,210,200,16,16,"OV7725 Init OK");
	//��Ч
  OV7725_Light_Mode(lightmode);
	OV7725_Color_Saturation(saturation);
	OV7725_Brightness(brightness);
	OV7725_Contrast(contrast);
	OV7725_Special_Effects(effect);
	
	//���������ʽ
	OV7725_Window_Set(OV7725_WINDOW_WIDTH,OV7725_WINDOW_HEIGHT,0);//QVGAģʽ���
	//���ʹ��
	OV7725_CS=0;
	
	EXTI8_Init();				//ʹ�ܶ�ʱ������
	
	LCD_Clear(BLACK);
	while(1)
	{
		camera_refresh();//������ʾ
		
		//����S2��ʾͼƬ
		if(KEY_Scan(1) == S2)
		{
			LCD_Clear(BLACK);
			ai_load_picfile("0:test1.bmp",0,0,lcddev.width,lcddev.height,1);//��ʾͼƬ
			delay_ms(5000);
			LCD_Clear(BLACK);//����֮����Է�ֹ���ָ�������
			continue;
		}
		
		//����S3ͼƬ����
		if(KEY_Scan(1) == S3)
		{
			LCD_Clear(WHITE);
			LCD_ShowString(60,210,200,16,16,"Graying......");
			
			//ͼ��ҶȻ�
			Graying("0:test1.bmp", "0:test2.bmp");
			
			//��ʾͼ������
			LCD_Clear(BLACK);
			ai_load_picfile("0:test2.bmp",0,0,lcddev.width,lcddev.height,1);//��ʾͼƬ
			delay_ms(5000);
			LCD_Clear(BLACK);//����֮����Է�ֹ���ָ�������

			LCD_Clear(WHITE);
			LCD_ShowString(60,210,200,16,16,"Ostu......");
			
			//ͼ���ֵ��
			Ostu("0:test2.bmp", "0:test3.bmp");
			//��ʾͼ������
			LCD_Clear(BLACK);
			ai_load_picfile("0:test3.bmp",0,0,lcddev.width,lcddev.height,1);//��ʾͼƬ
			delay_ms(5000);
			LCD_Clear(BLACK);//����֮����Է�ֹ���ָ�������
			
			//ͼ��ָ�(ͼƬ�ļ��е�·��Ϊ0:PICS/)
			//...
			
			LCD_Clear(WHITE);
			LCD_ShowString(60,210,200,16,16,"Image Recognition......");
			//ͼ��ʶ��
			res[0] = BP_Recongnization("0:PICS/0.bmp");
			res[1] = BP_Recongnization("0:PICS/1.bmp");
			res[2] = BP_Recongnization("0:PICS/2.bmp");
			res[3] = BP_Recongnization("0:PICS/3.bmp");
			res[4] = BP_Recongnization("0:PICS/4.bmp");
			
			r[0] = res[0] + '0';
			r[1] = res[1] + '0';
			r[2] = res[2] + '0';
			r[3] = res[3] + '0';
			r[4] = res[4] + '0';
			r[5] = '\0';
			//���ʶ����
			LCD_Clear(WHITE);
			LCD_ShowString(60,210,200,16,16, r);
			printf("ʶ����:%d%d%d%d%d", res[0], res[1], res[2], res[3], res[4]);
			
			//�����������ַ��͵�NB-IoTģ��
			do
			{
				//����ATָ��
				SendData();
				
				//�ȴ����ճɹ����Ӧ�Ľ��
				delay_ms(500);
				
				//�ж��Ƿ��ͳɹ�
				while(j < USART2_RX_STA)
				{
					if((USART2_RX_BUF[j] == 'O' || USART2_RX_BUF[j] == 'o') && (USART2_RX_BUF[j+1] == 'K' || USART2_RX_BUF[j+1] == 'k'))
					{
						flag = 1;//���ͳɹ�
						USART2_RX_STA = 0;
						j = 0;
						break;
					}
					j++;
					
					if(j == USART2_RX_STA)
					{
						USART2_RX_STA = 0;
						j = 0;
						break;
					}
				}
			}while(flag == 0);
			
			continue;
		}

		//����S4��ʾͼƬ������
		if(KEY_Scan(1) == S4)
		{
			LCD_Clear(BLACK);
			ai_load_picfile("0:test3.bmp",0,0,lcddev.width,lcddev.height,1);//��ʾͼƬ
			delay_ms(5000);
			LCD_Clear(BLACK);//����֮����Է�ֹ���ָ�������
			continue;
		}
		
		i++;
		if(i==15)//DS0��˸.
		{
			i=0;
			LED0=!LED0;
		}
	}
}