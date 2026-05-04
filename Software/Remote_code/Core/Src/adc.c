/**
 * @file    adc.c
 * @brief   ADC1外设配置与初始化
 * @author  langhz666
 * @date    2025-09-27
 *
 * @details 本文件配置ADC1用于采集摇杆4通道数据：
 *          - 4通道扫描模式
 *          - 连续转换使能
 *          - DMA循环传输
 *          - 12位分辨率
 *
 * ADC通道映射（摇杆）：
 * - Rank 1: CH1 (PA1) -> THR（油门）
 * - Rank 2: CH6 (PA6) -> YAW（偏航）
 * - Rank 3: CH2 (PA2) -> PIT（俯仰）
 * - Rank 4: CH3 (PA3) -> ROL（横滚）
 *
 * 采集参数：
 * - 分辨率：12位（0-4095）
 * - 采样时间：13.5个周期
 * - 转换模式：连续扫描
 * - DMA模式：循环传输
 *
 * @note   此文件由STM32CubeMX自动生成
 * @note   通道顺序由CubeMX中的Rank配置决定
 * @note   DMA自动将转换结果传输到adc_buff[]
 */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "adc.h"

/* USER CODE BEGIN 0 */
/**
 * ADC1通道映射说明：
 *
 * 通道分配：
 * - CH1 (PA1): THR（油门）- Rank 1
 * - CH6 (PA6): YAW（偏航）- Rank 2
 * - CH2 (PA2): PIT（俯仰）- Rank 3
 * - CH3 (PA3): ROL（横滚）- Rank 4
 *
 * 采集参数：
 * - 分辨率：12位（0-4095）
 * - 采样时间：13.5个周期
 * - 转换模式：连续扫描
 * - DMA模式：循环传输（DMA1_Channel1）
 *
 * 数据流向：
 * ADC1 -> DMA1_Channel1 -> adc_buff[4]（在Int_jystick.c中定义）
 *
 * 注意事项：
 * - 通道顺序由Rank配置决定，修改Rank会改变数据顺序
 * - DMA自动更新adc_buff[]，无需CPU干预
 * - 采样率由ADC时钟和采样时间决定
 */
/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* ADC1 init function */
/**
 * @brief  ADC1初始化函数
 *
 * @details 配置ADC1为4通道连续扫描模式：
 *          - 扫描模式：使能（多通道）
 *          - 连续转换：使能
 *          - 不连续模式：禁用
 *          - 触发源：软件触发
 *          - 数据对齐：右对齐
 *          - 转换通道数：4
 *
 * 通道配置：
 * - CH1 (PA1): Rank 1, 采样时间13.5周期
 * - CH6 (PA6): Rank 2, 采样时间13.5周期
 * - CH2 (PA2): Rank 3, 采样时间13.5周期
 * - CH3 (PA3): Rank 4, 采样时间13.5周期
 *
 * @note   此函数由STM32CubeMX自动生成
 * @note   修改后重新生成代码会覆盖手动更改
 */
void MX_ADC1_Init(void)
{

    /* USER CODE BEGIN ADC1_Init 0 */

    /* USER CODE END ADC1_Init 0 */

    ADC_ChannelConfTypeDef sConfig = {0};

    /* USER CODE BEGIN ADC1_Init 1 */

    /* USER CODE END ADC1_Init 1 */

    /** Common config
     */
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;          /* 扫描模式使能（多通道） */
    hadc1.Init.ContinuousConvMode = ENABLE;              /* 连续转换使能 */
    hadc1.Init.DiscontinuousConvMode = DISABLE;          /* 不连续模式禁用 */
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;    /* 软件触发 */
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;          /* 数据右对齐 */
    hadc1.Init.NbrOfConversion = 4;                      /* 转换通道数：4 */
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel - Rank 1: THR（油门）
     */
    sConfig.Channel = ADC_CHANNEL_1;                     /* 通道1 (PA1) */
    sConfig.Rank = ADC_REGULAR_RANK_1;                   /* Rank 1 */
    sConfig.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;    /* 采样时间13.5周期 */
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel - Rank 2: YAW（偏航）
     */
    sConfig.Channel = ADC_CHANNEL_6;                     /* 通道6 (PA6) */
    sConfig.Rank = ADC_REGULAR_RANK_2;                   /* Rank 2 */
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel - Rank 3: PIT（俯仰）
     */
    sConfig.Channel = ADC_CHANNEL_2;                     /* 通道2 (PA2) */
    sConfig.Rank = ADC_REGULAR_RANK_3;                   /* Rank 3 */
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel - Rank 4: ROL（横滚）
     */
    sConfig.Channel = ADC_CHANNEL_3;                     /* 通道3 (PA3) */
    sConfig.Rank = ADC_REGULAR_RANK_4;                   /* Rank 4 */
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN ADC1_Init 2 */

    /* USER CODE END ADC1_Init 2 */
}

/**
 * @brief  ADC1 MSP初始化回调函数
 *
 * @details 配置ADC1的底层硬件资源：
 *          - 使能ADC1时钟
 *          - 配置GPIO引脚为模拟输入
 *          - 配置DMA传输
 *
 * GPIO配置：
 * - PA1: ADC1_IN1（THR）
 * - PA2: ADC1_IN2（PIT）
 * - PA3: ADC1_IN3（ROL）
 * - PA6: ADC1_IN6（YAW）
 *
 * DMA配置：
 * - DMA1_Channel1: ADC1 -> 内存
 * - 方向：外设到内存
 * - 数据宽度：半字（16位）
 * - 模式：循环传输
 * - 优先级：低
 *
 * @note   此函数由HAL_ADC_Init()自动调用
 * @note   此函数由STM32CubeMX自动生成
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef *adcHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (adcHandle->Instance == ADC1)
    {
        /* USER CODE BEGIN ADC1_MspInit 0 */

        /* USER CODE END ADC1_MspInit 0 */
        /* ADC1 clock enable */
        __HAL_RCC_ADC1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**ADC1 GPIO Configuration
        PA1     ------> ADC1_IN1 (THR)
        PA2     ------> ADC1_IN2 (PIT)
        PA3     ------> ADC1_IN3 (ROL)
        PA6     ------> ADC1_IN6 (YAW)
        */
        GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* ADC1 DMA Init */
        /* ADC1 Init */
        hdma_adc1.Instance = DMA1_Channel1;
        hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;           /* 外设到内存 */
        hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;               /* 外设地址不递增 */
        hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;                   /* 内存地址递增 */
        hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;  /* 外设数据宽度：半字 */
        hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;     /* 内存数据宽度：半字 */
        hdma_adc1.Init.Mode = DMA_CIRCULAR;                             /* 循环模式 */
        hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;                     /* 低优先级 */
        if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc1);

        /* USER CODE BEGIN ADC1_MspInit 1 */

        /* USER CODE END ADC1_MspInit 1 */
    }
}

/**
 * @brief  ADC1 MSP反初始化回调函数
 *
 * @details 释放ADC1的底层硬件资源：
 *          - 禁用ADC1时钟
 *          - 反初始化GPIO引脚
 *          - 反初始化DMA
 *
 * @note   此函数由HAL_ADC_DeInit()自动调用
 * @note   此函数由STM32CubeMX自动生成
 */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef *adcHandle)
{

    if (adcHandle->Instance == ADC1)
    {
        /* USER CODE BEGIN ADC1_MspDeInit 0 */

        /* USER CODE END ADC1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_ADC1_CLK_DISABLE();

        /**ADC1 GPIO Configuration
        PA1     ------> ADC1_IN1
        PA2     ------> ADC1_IN2
        PA3     ------> ADC1_IN3
        PA6     ------> ADC1_IN6
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6);

        /* ADC1 DMA DeInit */
        HAL_DMA_DeInit(adcHandle->DMA_Handle);
        /* USER CODE BEGIN ADC1_MspDeInit 1 */

        /* USER CODE END ADC1_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
