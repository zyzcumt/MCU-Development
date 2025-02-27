#include "PCF8574.h"
#include  <math.h>    //Keil library

//PA0(23) - SCL
//PA1(24) - SDA

/*******************************************************************************
* Function Name  : I2C_GPIO_Config
* Description    : Configration Simulation IIC GPIO
* Input          : None 
* Output         : None
* Return         : None
****************************************************************************** */
void PCF8574_I2C_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	PCF8574_I2C_Stop();
}

void PCF8574_i2c_SDA_input(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 ;
   //GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	
	 //GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
   //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; 
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
   GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void PCF8574_i2c_SDA_output(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; 
	//GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
/*******************************************************************************
* Function Name  : PCF8574_I2C_delay
* Description    : Simulation IIC Timing series delay
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void PCF8574_I2C_delay(void)
{
		
   u16 i=5000; //这里可以优化速度	，经测试最低到5还能写入
   while(i) 
   { 
     i--; 
   }  
}

void PCF8574_delay5ms(void)
{
		
   int i=50000;  
   while(i) 
   { 
     i--; 
   }  
}

void PCF8574_delay50us(void)
{
		
   int i=500;  
   while(i) 
   { 
     i--; 
   }  
}
//*************************************************************************************************
//I2C起始信号
//*************************************************************************************************
void PCF8574_I2C_Start(void)
{
    SDA_H;                    //拉高数据线
    SCL_H;                    //拉高时钟线
    PCF8574_I2C_delay();                 //延时
    SDA_L;                    //产生下降沿
    PCF8574_I2C_delay();                 //延时
    SCL_L;                    //拉低时钟线
    PCF8574_I2C_delay();                 //延�	
}
//*************************************************************************************************
//I2C停止信号
//*************************************************************************************************
void PCF8574_I2C_Stop(void)
{
	  SCL_L;                    //拉高时钟线
    SDA_L;                    //拉低数据线
    PCF8574_I2C_delay();                 //延时	
    SCL_H;                    //拉高时钟线
    PCF8574_I2C_delay();                 //延时
    SDA_H;                    //产生上升沿
    PCF8574_I2C_delay();                 //延时
}
//**************************************************************************************************
//I2C发送应答信号
//入口参数:ack (0:ACK 1:NAK)
//**************************************************************************************************
void PCF8574_I2C_SendACK(char ack)
{
	if(ack == 1)
	  SDA_H;
	else
    SDA_L;                  //写应答信号
	SCL_H;                    //拉高时钟线
	PCF8574_I2C_delay();                 //延时
	SCL_L;                    //拉低时钟线
	PCF8574_I2C_delay();                 //延时
}
//****************************************************************************************************
//I2C接收应答信号
//****************************************************************************************************
char PCF8574_I2C_RecvACK(void)
{
	char CY;
	PCF8574_i2c_SDA_input();
	PCF8574_I2C_delay();                 //延时	
	SCL_H;                    //拉高时钟线
	PCF8574_I2C_delay();                 //延时
	if(SDA_read)
    CY = 1;                   //读应答信号
	else
		CY = 0;
	SCL_L;                    //拉低时钟线
	PCF8574_i2c_SDA_output();
	PCF8574_I2C_delay();                 //延时
	return CY;
}
//*****************************************************************************************************
//向I2C总线发送一个字节数据
//*****************************************************************************************************
void PCF8574_I2C_SendByte(uchar dat)
{
    uchar i;
    for (i=0; i<8; i++)         //8位计数器
    {
			  if(dat&0x80)
          SDA_H;               //送数据口
			  else
					SDA_L;
				
        dat <<= 1;              //移出数据的最高位			
        SCL_H;                //拉高时钟线
        PCF8574_I2C_delay();             //延时
        SCL_L;                //拉低时钟线
        PCF8574_I2C_delay();             //延时
    }
    PCF8574_I2C_RecvACK();
}
//*****************************************************************************************************
//从I2C总线接收一个字节数据
//******************************************************************************************************
uchar PCF8574_I2C_RecvByte(void)
{
    uchar i;
    uchar dat = 0;
		PCF8574_i2c_SDA_input();
    for (i=0; i<8; i++)         //8位计数器
    {
        SCL_L;                //拉低时钟线
        PCF8574_I2C_delay();             //延时 
        SCL_H;                //拉高时钟线
        PCF8574_I2C_delay();             //延时
  			dat <<= 1;			
			  if(SDA_read)
					dat |= 1;             //读数据               

    }
	  PCF8574_i2c_SDA_output();		
    return dat;
}

//*****************************************************************************************************
//向I2C设备写入一个字节数据
//*****************************************************************************************************

//REG_Address : SALVE ADDRESS
//REG_data    : data
void PCF8574_Single_WriteI2C(uchar REG_Address,uchar REG_data)
{  uchar t;
	 t=(REG_Address<<1);
	
    PCF8574_I2C_Start();       //起始信号
	  PCF8574_I2C_SendByte(t);   //发送设备地址+写信号
    PCF8574_I2C_SendByte(REG_data);   //内部寄存器地址
    PCF8574_I2C_Stop();        //发送停止信号
}

//*******************************************************************************************************
//从I2C设备读取一个字节数据
//*******************************************************************************************************
uchar PCF8574_Single_ReadI2C(uchar REG_Address)
{
	uchar REG_data;
	 uchar t;
	t=((REG_Address<<1)|0x01);	
	
	PCF8574_I2C_Start();                   //起始信号
	PCF8574_I2C_SendByte(t);  //发送设备地址+读信号
	REG_data=PCF8574_I2C_RecvByte();       //读出寄存器数据
	PCF8574_I2C_SendACK(1);                //接收应答信号
	PCF8574_I2C_Stop();                    //停止信号
	return REG_data;
}					      

