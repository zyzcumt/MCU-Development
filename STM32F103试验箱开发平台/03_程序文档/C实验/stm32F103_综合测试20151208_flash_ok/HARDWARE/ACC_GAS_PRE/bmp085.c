#include "bmp085.h"

#define OSS 0	// Oversampling Setting (note: code is not set up to use other OSS values)

//***BMP085??
unsigned char ge,shi,bai,qian,wan,shiwan;           //????
short ac1;
short ac2; 
short ac3; 
unsigned short ac4;
unsigned short ac5;
unsigned short ac6;
short b1; 
short b2;
short mb;
short mc;
short md;

long  temperature;
long  pressure;
float tempdat=0;
unsigned char outdata[6]={0};

void delay5ms(void);
void I2C_Stop();


//************十六进制转换成字符****************************
void string_shift(unsigned int dat) 
{ 
  outdata[3] =dat/1000+'0';      // 取千位 
  outdata[2] =(dat%1000)/100+'0'; //取百位 
  outdata[1] =(dat%100)/10+'0';  // 取十位 
  outdata[0] =dat%10+'0';        // 取个位 
} 

  /*******************************/
void DATA_printf(u8 *s,short temp_data)
{
	if(temp_data<0){
	temp_data=-temp_data;
    *s='-';
	}
	else *s=' ';
    *++s =temp_data/100+0x30;
    temp_data=temp_data%100;     //取余运算
    *++s =temp_data/10+0x30;
    temp_data=temp_data%10;      //取余运算
    *++s =temp_data+0x30; 	
}
/*
void I2C_Gpio_Init(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
		
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 
		
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6;				
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
		
	 GPIO_Init(GPIOB, &GPIO_InitStructure);	  				
	 I2C_Stop();	
}
*/
void I2C_Gpio_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	I2C_Stop();
}


void i2c_SDA_input(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 ;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
   GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void i2c_SDA_output(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; 
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	 GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	 GPIO_Init(GPIOB, &GPIO_InitStructure);
}
/*******************************************************************************
* Function Name  : I2C_delay
* Description    : Simulation IIC Timing series delay
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_delay(void)
{
   u16 i=5000; //这里可以优化速度	，经测试最低到5还能写入
   while(i) 
   { 
     i--; 
   }  
}

void delay5ms(void)
{
   int i=50000;  
   while(i) 
   { 
     i--; 
   }  
}
//*************************************************************************************************
//I2C起始信号
//*************************************************************************************************
void I2C_Start()
{
    SDA_H;                    //拉高数据线
    SCL_H;                    //拉高时钟线
    I2C_delay();                 //延时
    SDA_L;                    //产生下降沿
    I2C_delay();                 //延时
    SCL_L;                    //拉低时钟线
    I2C_delay();                 //延�	
}
//*************************************************************************************************
//I2C停止信号
//*************************************************************************************************
void I2C_Stop()
{
	  SCL_L;                    //拉高时钟线
    SDA_L;                    //拉低数据线
    I2C_delay();                 //延时	
    SCL_H;                    //拉高时钟线
    I2C_delay();                 //延时
    SDA_H;                    //产生上升沿
    I2C_delay();                 //延时
}
//**************************************************************************************************
//I2C发送应答信号
//入口参数:ack (0:ACK 1:NAK)
//**************************************************************************************************
void I2C_SendACK(char ack)
{
	if(ack == 1)
	  SDA_H;
	else
    SDA_L;                  //写应答信号
	SCL_H;                    //拉高时钟线
	I2C_delay();                 //延时
	SCL_L;                    //拉低时钟线
	I2C_delay();                 //延时
}
//****************************************************************************************************
//I2C接收应答信号
//****************************************************************************************************
char I2C_RecvACK()
{
	char CY;
	i2c_SDA_input();
	I2C_delay();                 //延时	
	SCL_H;                    //拉高时钟线
	I2C_delay();                 //延时
	if(SDA_read)
    CY = 1;                   //读应答信号
	else
		CY = 0;
	SCL_L;                    //拉低时钟线
	i2c_SDA_output();
	I2C_delay();                 //延时
	return CY;
}
//*****************************************************************************************************
//向I2C总线发送一个字节数据
//*****************************************************************************************************
void I2C_SendByte(u8 dat)
{
    u8 i;
    for (i=0; i<8; i++)         //8位计数器
    {
			  if(dat&0x80)
          SDA_H;               //送数据口
			  else
					SDA_L;
				
        dat <<= 1;              //移出数据的最高位			
        SCL_H;                //拉高时钟线
        I2C_delay();             //延时
        SCL_L;                //拉低时钟线
        I2C_delay();             //延时
    }
    I2C_RecvACK();
}
//*****************************************************************************************************
//从I2C总线接收一个字节数据
//******************************************************************************************************
u8 I2C_RecvByte(void)
{
    u8 i;
    u8 dat = 0;
		i2c_SDA_input();
    for (i=0; i<8; i++)         //8位计数器
    {
        SCL_L;                //拉低时钟线
        I2C_delay();             //延时 
        SCL_H;                //拉高时钟线
        I2C_delay();             //延时
  			dat <<= 1;			
			  if(SDA_read)
					dat |= 1;             //读数据               

    }
	  i2c_SDA_output();		
    return dat;
}



//*****************************************************************************************************
//向I2C设备写入一个字节数据
//*****************************************************************************************************
void Single_WriteI2C(u8 SlaveAddress, u8 REG_Address,u8 REG_data)
{
    I2C_Start();                  //起始信号
    I2C_SendByte(SlaveAddress);   //发送设备地址+写信号
    I2C_SendByte(REG_Address);    //内部寄存器地址，
    I2C_SendByte(REG_data);       //内部寄存器数据，
    I2C_Stop();                   //发送停止信号
}

//*******************************************************************************************************
//从I2C设备读取一个字节数据
//*******************************************************************************************************
u8 Single_ReadI2C(u8 SlaveAddress, u8 REG_Address)
{
	u8 REG_data;
	I2C_Start();                   //起始信号
	I2C_SendByte(SlaveAddress - 1);    //发送设备地址+写信号
	I2C_SendByte(REG_Address);     //发送存储单元地址，从0开始	
	I2C_Start();                   //起始信号
	I2C_SendByte(SlaveAddress);  //发送设备地址+读信号
	REG_data=I2C_RecvByte();       //读出寄存器数据
	I2C_SendACK(1);                //接收应答信号
	I2C_Stop();                    //停止信号
	return REG_data;
}				   

//*********************************************************
//??BMP085????,????
//*********************************************************
short Multiple_read(u8 SlaveAddress, u8 ST_Address)
{   
	u8 msb, lsb;
	short _data;
    I2C_Start();                          //????
    I2C_SendByte(SlaveAddress);    //??????+???
    I2C_SendByte(ST_Address);             //????????
    I2C_Start();                          //????
    I2C_SendByte(SlaveAddress+1);         //??????+???

    msb = I2C_RecvByte();                 //BUF[0]??
    I2C_SendACK(0);                       //??ACK
    lsb = I2C_RecvByte();     
	I2C_SendACK(1);                       //?????????NOACK

    I2C_Stop();                           //????
    //Delay5ms();
    _data = msb << 8;
	_data |= lsb;	
	return _data;
}
//******************************************************************************************************
//初始化MPU6050
//******************************************************************************************************
void InitBMP085(void)
{
	ac1 = Multiple_read(BMP085_SlaveAddress, 0xAA);
	ac2 = Multiple_read(BMP085_SlaveAddress, 0xAC);
	ac3 = Multiple_read(BMP085_SlaveAddress, 0xAE);
	ac4 = Multiple_read(BMP085_SlaveAddress, 0xB0);
	ac5 = Multiple_read(BMP085_SlaveAddress, 0xB2);
	ac6 = Multiple_read(BMP085_SlaveAddress, 0xB4);
	b1 =  Multiple_read(BMP085_SlaveAddress, 0xB6);
	b2 =  Multiple_read(BMP085_SlaveAddress, 0xB8);
	mb =  Multiple_read(BMP085_SlaveAddress, 0xBA);
	mc =  Multiple_read(BMP085_SlaveAddress, 0xBC);
	md =  Multiple_read(BMP085_SlaveAddress, 0xBE);
}
//********************************************************************
long bmp085ReadTemp(void)
{

    I2C_Start();                  //????
    I2C_SendByte(BMP085_SlaveAddress);   //??????+???
    I2C_SendByte(0xF4);	          // write register address
    I2C_SendByte(0x2E);       	// write register data for temp
    I2C_Stop();                   //??????
		I2C_delay();	// max time is 4.5ms
	
	return (long) Multiple_read(BMP085_SlaveAddress, 0xF6);
}
//*************************************************************
long bmp085ReadPressure(void)
{
	long pressure = 0;

    I2C_Start();                   //????
    I2C_SendByte(BMP085_SlaveAddress);   //??????+???
    I2C_SendByte(0xF4);	          // write register address
    I2C_SendByte(0x34);       	  // write register data for pressure
    I2C_Stop();                    //??????
	I2C_delay();    	                  // max time is 4.5ms
	
	pressure = Multiple_read(BMP085_SlaveAddress, 0xF6);
	pressure &= 0x0000FFFF;
	
	return pressure;	
	//return (long) bmp085ReadShort(0xF6);
}

//***********************************************************************
void bmp085Convert()
{
	unsigned int ut = 0;
	unsigned long up = 0;
	long x1, x2, b5, b6, x3, b3, p;
	unsigned long b4, b7;

	ut = bmp085ReadTemp();	   // change by enviornment
	up = bmp085ReadPressure();  // change by enviornment
	
	x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
	x2 = ((long) mc << 11) / (x1 + md);
	b5 = x1 + x2;
	temperature = ((b5 + 8) >> 4);
	
  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<< OSS) + 2)>>2;
  
  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;
  
  b7 = ((unsigned long)(up - b3) * (50000 >> OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;
    
  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  pressure = p+((x1 + x2 + 3791)>>4);
}

void bmp085_conversion(long temp_data, uint8_t *p)
{
	  *p++ = temp_data/100000+0x30;
    temp_data=temp_data%100000;   
		*p++=temp_data/10000+0x30;
    temp_data=temp_data%10000;   
		*p++=temp_data/1000+0x30;
    temp_data=temp_data%1000;    
    *p++=temp_data/100+0x30;
    temp_data=temp_data%100;     
    *p++=temp_data/10+0x30;
    temp_data=temp_data%10;     
    *p++=temp_data+0x30; 
}


void DATA_Diplay()
{ 
	u8 tmp[15]={"temperture:00.0"}, pres[19] = {"pressure:000000 Pa"};
	long temp_value;
//	LCD_ShowString(200, 18, bmp_tmp, White, Black);
	temp_value=temperature;

	bmp085_conversion(temp_value, outdata);	

	tmp[11]=outdata[3];
	tmp[12]=outdata[4];
	tmp[14]=outdata[5];

	LCD_ShowString(150, 150, tmp);
	//printf("temperature=%c%c.%c\n",outdata[3],outdata[4],outdata[5]);
	
	temp_value = pressure;
	bmp085_conversion(temp_value, outdata);
	
	pres[9]  = outdata[0];
	pres[10] = outdata[1];
	pres[11] = outdata[2];
	pres[12] = outdata[3];
	pres[13] = outdata[4];
	pres[14] = outdata[5];	
	
	LCD_ShowString(150, 180, pres);
	//printf("pressure=%c%c%c%c%c%c Pa\n",outdata[0],outdata[1],outdata[2],outdata[3],outdata[4],outdata[5]);
}

void BMP085_Test(void)
{

	
	I2C_Gpio_Init();
	InitBMP085();
  bmp085Convert();
  DATA_Diplay();		
}

