#include "App_transmit_data.h"

extern Remote_Data remote_data;
uint8_t transmit_buff[TX_PLOAD_WIDTH] = {0};

uint8_t post_buff[TX_PLOAD_WIDTH] = {0};

/**
 * @brief 自动切换SI24R1的模式 => 将采集完成的遥控数据打包发送到飞机
 *
 */
void App_transmit_data(void)
{

    // 2. 发送数据 => 唯一性和可靠性
    // (1) => 帧头校验指定发送给对应的设备 唯一性
    // (2) => 发送数据的结尾 => 添加检验和(将发送的数据累加得到的值添加到数据的末尾) 可靠性

    uint32_t sum = 0;
    //  帧头校验设置为3字节 => 数据本身是10字节 => 校验和设置为4字节
    transmit_buff[0] = FRAME_HEAD_CHECK_1;
    transmit_buff[1] = FRAME_HEAD_CHECK_2;
    transmit_buff[2] = FRAME_HEAD_CHECK_3;

    // 高8位在前
    transmit_buff[3] = (remote_data.thr >> 8) & 0xFF;
    transmit_buff[4] = remote_data.thr & 0xFF;

    transmit_buff[5] = (remote_data.yaw >> 8) & 0xFF;
    transmit_buff[6] = remote_data.yaw & 0xFF;

    transmit_buff[7] = (remote_data.pit >> 8) & 0xFF;
    transmit_buff[8] = remote_data.pit & 0xFF;

    transmit_buff[9] = (remote_data.rol >> 8) & 0xFF;
    transmit_buff[10] = remote_data.rol & 0xFF;

    // 此处的关机和定高其实是会有0或1
    taskENTER_CRITICAL();
    transmit_buff[11] = remote_data.shutdown; // 正好在此处运行完成之后切换任务
    remote_data.shutdown = 0;                 // 此时shutdown=1  但是上一行代码没有记录下来
    transmit_buff[12] = remote_data.fix_height;
    remote_data.fix_height = 0;
    taskEXIT_CRITICAL();

    for (uint8_t i = 0; i < 13; i++)
    {
        sum += transmit_buff[i];
    }
    // 高位在前
    transmit_buff[13] = (sum >> 24) & 0xFF;
    transmit_buff[14] = (sum >> 16) & 0xFF;
    transmit_buff[15] = (sum >> 8) & 0xFF;
    transmit_buff[16] = sum & 0xFF;

    // debug_printf(":%d,%d,%d,%d,%d,%d\n",remote_data.thr,remote_data.yaw,remote_data.pit,remote_data.rol,remote_data.shutdown,remote_data.fix_height);

    // 1. 切换到发送模式
    Int_SI24R1_TX_Mode();
    uint8_t res = Int_SI24R1_TxPacket(transmit_buff);
    // 3. 切换回接收模式
    Int_SI24R1_RX_Mode();
    // 判断是否发送成功 => 发送成功接收回传数据

    if (res == 0)
    {
        // 2.4G同步发送逻辑 => 必须严格按照双端发送的时序才能数据
        while (Int_SI24R1_RxPacket(post_buff) == 1)
        {
        }
        // debug_printf("post_buff:%s\n", post_buff);
    }
}
