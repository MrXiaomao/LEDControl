#pragma once

#include "afxwin.h"
// Order Set of Lighting Control 
class Order
{
public:
	Order(void);
	~Order(void);

	//复位
	static BYTE Reset[]; 

	//LED发光宽度，单位为×10ns，例如300ns对应的指令为：12 02 00 1E DD
	static BYTE LightingWidth[]; 

	//LED发光延迟时间，单位us，默认值9999us
	static BYTE LightingDelay[];

	//配置移位寄存器时钟频率， 不可更改
	static BYTE RegisterClockRate[];

	//配置硬件触发高电平点数,单位×10ns，默认值10ns
	static BYTE TriggerPointsSet[];

	//配置移位寄存器数据,控制各个LED是否发光
	static BYTE LightingSwitch[];

	//开启移位寄存器配置
	static BYTE ShiftRegisterOn[];

	//配置DAC参考电压,不可更改
	static BYTE ReferenceVolt_DAC[];

	//配置DAC数据,控制灯光电压
	static BYTE VoltA_Lighting[];
	static BYTE VoltB_Lighting[];

	//写入DAC数据
	static BYTE WriteData_DAC[];
	
	//关闭DAC配置
	static BYTE DAC_Off[];

	//开启电源(外设上电使能)
	static BYTE CommonVolt_On[];
	
	//关闭电源（外设使能）
	static BYTE CommonVolt_Off[];

	//开启A灯组硬件触发
	static BYTE TriggerOn_A[];

	//开启B灯组硬件触发
	static BYTE TriggerOn_B[];

	//开启AB灯组硬件触发
	static BYTE TriggerOn_AB[];

	//关闭硬件触发,同时关闭A、B组触发
	static BYTE TriggerOff[];
	
};

