#include "Int_SI24R1.h"

// 定义一个静态的发送地址  => 发送地址与接收地址相同
uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x0A, 0x01, 0x06, 0x1E, 0x01}; // 定义一个静态发送地址

// SPI读写一个字节 => 写入的字节是传入的参数  读取的字节是返回值
static uint8_t SPI_RW(uint8_t byte)
{
	uint8_t rx_data = 0;
	HAL_SPI_TransmitReceive(&hspi1, &byte, &rx_data, 1, 1000);
	return rx_data;
}

/********************************************************
函数功能：写寄存器的值（单字节）
入口参数：reg:寄存器映射地址（格式：SI24R1_WRITE_REG｜reg）
					value:寄存器的值
返回  值：状态寄存器的值
*********************************************************/
uint8_t Int_SI24R1_Write_Reg(uint8_t reg, uint8_t value)
{
	uint8_t status;

	CS_LOW;
	status = SPI_RW(reg);
	SPI_RW(value);
	CS_HIGH;

	return (status);
}

/********************************************************
函数功能：写寄存器的值（多字节）
入口参数：reg:寄存器映射地址（格式：SI24R1_WRITE_REG｜reg）
					pBuf:写数据首地址
					bytes:写数据字节数
返回  值：状态寄存器的值
*********************************************************/
uint8_t Int_SI24R1_Write_Buf(uint8_t reg, const uint8_t *pBuf, uint8_t size)
{
	uint8_t status, byte_ctr;

	CS_LOW;
	status = SPI_RW(reg);
	for (byte_ctr = 0; byte_ctr < size; byte_ctr++)
	{
		SPI_RW(*pBuf++);
	}

	CS_HIGH;

	return (status);
}

/********************************************************
函数功能：读取寄存器的值（单字节）
入口参数：reg:寄存器映射地址（格式：SI24R1_READ_REG｜reg）
返回  值：寄存器值
*********************************************************/
uint8_t Int_SI24R1_Read_Reg(uint8_t reg)
{
	uint8_t value;

	CS_LOW;
	SPI_RW(reg);
	value = SPI_RW(0);
	CS_HIGH;

	return (value);
}

/********************************************************
函数功能：读取寄存器的值（多字节）
入口参数：reg:寄存器映射地址（SI24R1_READ_REG｜reg）
					pBuf:接收缓冲区的首地址
					bytes:读取字节数
返回  值：状态寄存器的值
*********************************************************/
uint8_t Int_SI24R1_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t size)
{
	uint8_t status, byte_ctr;

	CS_LOW;
	status = SPI_RW(reg);
	for (byte_ctr = 0; byte_ctr < size; byte_ctr++)
	{
		pBuf[byte_ctr] = SPI_RW(0); // 读取数据，低字节在前
	}
	CS_HIGH;

	return (status);
}

/********************************************************
函数功能：SI24R1接收模式初始化
入口参数：无
返回  值：无
*********************************************************/
void Int_SI24R1_RX_Mode(void)
{
	CE_LOW;
	Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // 接收设备接收通道0使用和发送设备相同的发送地址
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_AA, 0x01);						   // 使能接收通道0自动应答
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_RXADDR, 0x01);					   // 使能接收通道0
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_CH, CHANNEL);					   // 选择射频通道40
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RX_PW_P0, TX_PLOAD_WIDTH);			   // 接收通道0选择和发送通道相同有效数据宽度
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_SETUP, 0x06);					   // 数据传输率1Mbps，发射功率4dBm
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + CONFIG, 0x0f);						   // CRC使能，16位CRC校验，上电，接收模式
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + STATUS, 0xff);						   // 清除所有的中断标志位
	CE_HIGH;																	   // 拉高CE启动接收设备
}

/********************************************************
函数功能：SI24R1发送模式初始化
入口参数：无
返回  值：无
*********************************************************/
void Int_SI24R1_TX_Mode(void)
{
	CE_LOW;
	Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);	   // 写入发送地址
	Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // 为了应答接收设备，接收通道0地址和发送地址相同
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_AA, 0x01);						   // 使能接收通道0自动应答
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_RXADDR, 0x01);					   // 使能接收通道0
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + SETUP_RETR, 0x0a);					   // 自动重发延时等待250us+86us，自动重发10次
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_CH, CHANNEL);					   // 选择射频通道0x40
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_SETUP, 0x06);					   // 数据传输率1Mbps，发射功率4dBm
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + CONFIG, 0x0e);						   // CRC使能，16位CRC校验，上电
	CE_HIGH;
}

/********************************************************
函数功能：读取接收数据   硬件直接接收数据保存到 FIFO队列中 => 通过状态标志位判断队列中是否有数据
入口参数：rxbuf:接收数据存放首地址
返回  值：0:接收到数据
		  1:没有接收到数据
*********************************************************/
uint8_t Int_SI24R1_RxPacket(uint8_t *rxbuf)
{
	uint8_t state;
	// 将读取到的值 原封不动再写回状态寄存器  => 因为状态寄存器中的标志位设计为写1清除
	state = Int_SI24R1_Read_Reg(STATUS);					// 读取状态寄存器的值
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + STATUS, state); // 清除RX_DS中断标志

	if (state & RX_DR) // 接收到数据
	{
		Int_SI24R1_Read_Buf(RD_RX_PLOAD, rxbuf, TX_PLOAD_WIDTH); // 读取数据
		Int_SI24R1_Write_Reg(FLUSH_RX, 0xff);					 // 清除RX FIFO寄存器
		return 0;
	}
	return 1; // 没收到任何数据
}

/********************************************************
函数功能：发送一个数据包
入口参数：txbuf:要发送的数据
返回  值: 0: 发送成功 1: 发送失败
*********************************************************/
uint8_t Int_SI24R1_TxPacket(uint8_t *txbuf)
{
	uint8_t state;
	CE_LOW;													  // CE拉低，使能SI24R1配置
	Int_SI24R1_Write_Buf(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH); // 写数据到TX FIFO,32个字节
	CE_HIGH;												  // CE置高，使能发送

	// 没有使用中断判断是否发送完成  => 使用轮询读取状态标志位
	// while (IRQ == 1)
	// 	;										 // 等待发送完成

	state = Int_SI24R1_Read_Reg(STATUS); // 读取状态寄存器的值
	while (((state & TX_DS) == 0) && ((state & MAX_RT) == 0))
	{
		state = Int_SI24R1_Read_Reg(STATUS);
		vTaskDelay(1);
	}

	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + STATUS, state); // 清除TX_DS或MAX_RT中断标志
	if (state & MAX_RT)										// 达到最大重发次数
	{
		Int_SI24R1_Write_Reg(FLUSH_TX, 0xff); // 清除TX FIFO寄存器
		return 1;
	}
	if (state & TX_DS) // 发送完成
	{
		return 0;
	}
	return 1; // 发送失败
}

uint8_t si24r1_rx_buff[5] = {0};

/**
 * @brief SI24R1的初始化检测
 *
 * @return uint8_t  0:检测成功  1:检测失败
 */
uint8_t Int_SI24R1_Check(void)
{

	// 1. 测试SPI通信能够正常读写寄存器
	// 1.0 SI24R1芯片需要先读取一次  保证SPI正常之后再写入
	Int_SI24R1_Read_Buf(SI24R1_READ_REG + TX_ADDR, si24r1_rx_buff, TX_ADR_WIDTH);

	// 1.1 写入发送地址
	Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);

	// 1.2 读取同样的数据
	Int_SI24R1_Read_Buf(SI24R1_READ_REG + TX_ADDR, si24r1_rx_buff, TX_ADR_WIDTH);

	for (uint8_t i = 0; i < TX_ADR_WIDTH; i++)
	{
		if (si24r1_rx_buff[i] != TX_ADDRESS[i])
		{
			return 1;
		}
	}
	return 0;
}

/**
 * @brief 硬件接口层SI24R1的初始化
 *
 */
void Int_SI24R1_Init(void)
{
	// 上电之后的芯片延迟 >100ms
	HAL_Delay(200);
	// 校验检测
	while (Int_SI24R1_Check() == 1)
	{
		// 每两次检测间隔10ms
		HAL_Delay(10);
	}

	// 设置默认的状态为接收模式  => 每次发送数据的时候  切换到发送状态 
	Int_SI24R1_RX_Mode();
	debug_printf("SI24R1 Init Success!\r\n");
}
