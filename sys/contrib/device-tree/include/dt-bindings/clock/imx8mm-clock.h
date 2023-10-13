/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2017-2018 NXP
 */

#ifndef __DT_BINDINGS_CLOCK_IMX8MM_H
#define __DT_BINDINGS_CLOCK_IMX8MM_H

#define IMX8MM_CLK_DUMMY			0
#define IMX8MM_CLK_32K				1
#define IMX8MM_CLK_24M				2
#define IMX8MM_OSC_HDMI_CLK			3
#define IMX8MM_CLK_EXT1				4
#define IMX8MM_CLK_EXT2				5
#define IMX8MM_CLK_EXT3				6
#define IMX8MM_CLK_EXT4				7
#define IMX8MM_AUDIO_PLL1_REF_SEL		8
#define IMX8MM_AUDIO_PLL2_REF_SEL		9
#define IMX8MM_VIDEO_PLL1_REF_SEL		10
#define IMX8MM_DRAM_PLL_REF_SEL			11
#define IMX8MM_GPU_PLL_REF_SEL			12
#define IMX8MM_VPU_PLL_REF_SEL			13
#define IMX8MM_ARM_PLL_REF_SEL			14
#define IMX8MM_SYS_PLL1_REF_SEL			15
#define IMX8MM_SYS_PLL2_REF_SEL			16
#define IMX8MM_SYS_PLL3_REF_SEL			17
#define IMX8MM_AUDIO_PLL1			18
#define IMX8MM_AUDIO_PLL2			19
#define IMX8MM_VIDEO_PLL1			20
#define IMX8MM_DRAM_PLL				21
#define IMX8MM_GPU_PLL				22
#define IMX8MM_VPU_PLL				23
#define IMX8MM_ARM_PLL				24
#define IMX8MM_SYS_PLL1				25
#define IMX8MM_SYS_PLL2				26
#define IMX8MM_SYS_PLL3				27
#define IMX8MM_AUDIO_PLL1_BYPASS		28
#define IMX8MM_AUDIO_PLL2_BYPASS		29
#define IMX8MM_VIDEO_PLL1_BYPASS		30
#define IMX8MM_DRAM_PLL_BYPASS			31
#define IMX8MM_GPU_PLL_BYPASS			32
#define IMX8MM_VPU_PLL_BYPASS			33
#define IMX8MM_ARM_PLL_BYPASS			34
#define IMX8MM_SYS_PLL1_BYPASS			35
#define IMX8MM_SYS_PLL2_BYPASS			36
#define IMX8MM_SYS_PLL3_BYPASS			37
#define IMX8MM_AUDIO_PLL1_OUT			38
#define IMX8MM_AUDIO_PLL2_OUT			39
#define IMX8MM_VIDEO_PLL1_OUT			40
#define IMX8MM_DRAM_PLL_OUT			41
#define IMX8MM_GPU_PLL_OUT			42
#define IMX8MM_VPU_PLL_OUT			43
#define IMX8MM_ARM_PLL_OUT			44
#define IMX8MM_SYS_PLL1_OUT			45
#define IMX8MM_SYS_PLL2_OUT			46
#define IMX8MM_SYS_PLL3_OUT			47
#define IMX8MM_SYS_PLL1_40M			48
#define IMX8MM_SYS_PLL1_80M			49
#define IMX8MM_SYS_PLL1_100M			50
#define IMX8MM_SYS_PLL1_133M			51
#define IMX8MM_SYS_PLL1_160M			52
#define IMX8MM_SYS_PLL1_200M			53
#define IMX8MM_SYS_PLL1_266M			54
#define IMX8MM_SYS_PLL1_400M			55
#define IMX8MM_SYS_PLL1_800M			56
#define IMX8MM_SYS_PLL2_50M			57
#define IMX8MM_SYS_PLL2_100M			58
#define IMX8MM_SYS_PLL2_125M			59
#define IMX8MM_SYS_PLL2_166M			60
#define IMX8MM_SYS_PLL2_200M			61
#define IMX8MM_SYS_PLL2_250M			62
#define IMX8MM_SYS_PLL2_333M			63
#define IMX8MM_SYS_PLL2_500M			64
#define IMX8MM_SYS_PLL2_1000M			65

/* core */
#define IMX8MM_CLK_A53_SRC			66
#define IMX8MM_CLK_M4_SRC			67
#define IMX8MM_CLK_VPU_SRC			68
#define IMX8MM_CLK_GPU3D_SRC			69
#define IMX8MM_CLK_GPU2D_SRC			70
#define IMX8MM_CLK_A53_CG			71
#define IMX8MM_CLK_M4_CG			72
#define IMX8MM_CLK_VPU_CG			73
#define IMX8MM_CLK_GPU3D_CG			74
#define IMX8MM_CLK_GPU2D_CG			75
#define IMX8MM_CLK_A53_DIV			76
#define IMX8MM_CLK_M4_DIV			77
#define IMX8MM_CLK_VPU_DIV			78
#define IMX8MM_CLK_GPU3D_DIV			79
#define IMX8MM_CLK_GPU2D_DIV			80

/* bus */
#define IMX8MM_CLK_MAIN_AXI			81
#define IMX8MM_CLK_ENET_AXI			82
#define IMX8MM_CLK_NAND_USDHC_BUS		83
#define IMX8MM_CLK_VPU_BUS			84
#define IMX8MM_CLK_DISP_AXI			85
#define IMX8MM_CLK_DISP_APB			86
#define IMX8MM_CLK_DISP_RTRM			87
#define IMX8MM_CLK_USB_BUS			88
#define IMX8MM_CLK_GPU_AXI			89
#define IMX8MM_CLK_GPU_AHB			90
#define IMX8MM_CLK_NOC				91
#define IMX8MM_CLK_NOC_APB			92

#define IMX8MM_CLK_AHB				93
#define IMX8MM_CLK_AUDIO_AHB			94
#define IMX8MM_CLK_IPG_ROOT			95
#define IMX8MM_CLK_IPG_AUDIO_ROOT		96

#define IMX8MM_CLK_DRAM_ALT			97
#define IMX8MM_CLK_DRAM_APB			98
#define IMX8MM_CLK_VPU_G1			99
#define IMX8MM_CLK_VPU_G2			100
#define IMX8MM_CLK_DISP_DTRC			101
#define IMX8MM_CLK_DISP_DC8000			102
#define IMX8MM_CLK_PCIE1_CTRL			103
#define IMX8MM_CLK_PCIE1_PHY			104
#define IMX8MM_CLK_PCIE1_AUX			105
#define IMX8MM_CLK_DC_PIXEL			106
#define IMX8MM_CLK_LCDIF_PIXEL			107
#define IMX8MM_CLK_SAI1				108
#define IMX8MM_CLK_SAI2				109
#define IMX8MM_CLK_SAI3				110
#define IMX8MM_CLK_SAI4				111
#define IMX8MM_CLK_SAI5				112
#define IMX8MM_CLK_SAI6				113
#define IMX8MM_CLK_SPDIF1			114
#define IMX8MM_CLK_SPDIF2			115
#define IMX8MM_CLK_ENET_REF			116
#define IMX8MM_CLK_ENET_TIMER			117
#define IMX8MM_CLK_ENET_PHY_REF			118
#define IMX8MM_CLK_NAND				119
#define IMX8MM_CLK_QSPI				120
#define IMX8MM_CLK_USDHC1			121
#define IMX8MM_CLK_USDHC2			122
#define IMX8MM_CLK_I2C1				123
#define IMX8MM_CLK_I2C2				124
#define IMX8MM_CLK_I2C3				125
#define IMX8MM_CLK_I2C4				126
#define IMX8MM_CLK_UART1			127
#define IMX8MM_CLK_UART2			128
#define IMX8MM_CLK_UART3			129
#define IMX8MM_CLK_UART4			130
#define IMX8MM_CLK_USB_CORE_REF			131
#define IMX8MM_CLK_USB_PHY_REF			132
#define IMX8MM_CLK_ECSPI1			133
#define IMX8MM_CLK_ECSPI2			134
#define IMX8MM_CLK_PWM1				135
#define IMX8MM_CLK_PWM2				136
#define IMX8MM_CLK_PWM3				137
#define IMX8MM_CLK_PWM4				138
#define IMX8MM_CLK_GPT1				139
#define IMX8MM_CLK_WDOG				140
#define IMX8MM_CLK_WRCLK			141
#define IMX8MM_CLK_DSI_CORE			142
#define IMX8MM_CLK_DSI_PHY_REF			143
#define IMX8MM_CLK_DSI_DBI			144
#define IMX8MM_CLK_USDHC3			145
#define IMX8MM_CLK_CSI1_CORE			146
#define IMX8MM_CLK_CSI1_PHY_REF			147
#define IMX8MM_CLK_CSI1_ESC			148
#define IMX8MM_CLK_CSI2_CORE			149
#define IMX8MM_CLK_CSI2_PHY_REF			150
#define IMX8MM_CLK_CSI2_ESC			151
#define IMX8MM_CLK_PCIE2_CTRL			152
#define IMX8MM_CLK_PCIE2_PHY			153
#define IMX8MM_CLK_PCIE2_AUX			154
#define IMX8MM_CLK_ECSPI3			155
#define IMX8MM_CLK_PDM				156
#define IMX8MM_CLK_VPU_H1			157
#define IMX8MM_CLK_CLKO1			158

#define IMX8MM_CLK_ECSPI1_ROOT			159
#define IMX8MM_CLK_ECSPI2_ROOT			160
#define IMX8MM_CLK_ECSPI3_ROOT			161
#define IMX8MM_CLK_ENET1_ROOT			162
#define IMX8MM_CLK_GPT1_ROOT			163
#define IMX8MM_CLK_I2C1_ROOT			164
#define IMX8MM_CLK_I2C2_ROOT			165
#define IMX8MM_CLK_I2C3_ROOT			166
#define IMX8MM_CLK_I2C4_ROOT			167
#define IMX8MM_CLK_OCOTP_ROOT			168
#define IMX8MM_CLK_PCIE1_ROOT			169
#define IMX8MM_CLK_PWM1_ROOT			170
#define IMX8MM_CLK_PWM2_ROOT			171
#define IMX8MM_CLK_PWM3_ROOT			172
#define IMX8MM_CLK_PWM4_ROOT			173
#define IMX8MM_CLK_QSPI_ROOT			174
#define IMX8MM_CLK_NAND_ROOT			175
#define IMX8MM_CLK_SAI1_ROOT			176
#define IMX8MM_CLK_SAI1_IPG			177
#define IMX8MM_CLK_SAI2_ROOT			178
#define IMX8MM_CLK_SAI2_IPG			179
#define IMX8MM_CLK_SAI3_ROOT			180
#define IMX8MM_CLK_SAI3_IPG			181
#define IMX8MM_CLK_SAI4_ROOT			182
#define IMX8MM_CLK_SAI4_IPG			183
#define IMX8MM_CLK_SAI5_ROOT			184
#define IMX8MM_CLK_SAI5_IPG			185
#define IMX8MM_CLK_SAI6_ROOT			186
#define IMX8MM_CLK_SAI6_IPG			187
#define IMX8MM_CLK_UART1_ROOT			188
#define IMX8MM_CLK_UART2_ROOT			189
#define IMX8MM_CLK_UART3_ROOT			190
#define IMX8MM_CLK_UART4_ROOT			191
#define IMX8MM_CLK_USB1_CTRL_ROOT		192
#define IMX8MM_CLK_GPU3D_ROOT			193
#define IMX8MM_CLK_USDHC1_ROOT			194
#define IMX8MM_CLK_USDHC2_ROOT			195
#define IMX8MM_CLK_WDOG1_ROOT			196
#define IMX8MM_CLK_WDOG2_ROOT			197
#define IMX8MM_CLK_WDOG3_ROOT			198
#define IMX8MM_CLK_VPU_G1_ROOT			199
#define IMX8MM_CLK_GPU_BUS_ROOT			200
#define IMX8MM_CLK_VPU_H1_ROOT			201
#define IMX8MM_CLK_VPU_G2_ROOT			202
#define IMX8MM_CLK_PDM_ROOT			203
#define IMX8MM_CLK_DISP_ROOT			204
#define IMX8MM_CLK_DISP_AXI_ROOT		205
#define IMX8MM_CLK_DISP_APB_ROOT		206
#define IMX8MM_CLK_DISP_RTRM_ROOT		207
#define IMX8MM_CLK_USDHC3_ROOT			208
#define IMX8MM_CLK_TMU_ROOT			209
#define IMX8MM_CLK_VPU_DEC_ROOT			210
#define IMX8MM_CLK_SDMA1_ROOT			211
#define IMX8MM_CLK_SDMA2_ROOT			212
#define IMX8MM_CLK_SDMA3_ROOT			213
#define IMX8MM_CLK_GPT_3M			214
#define IMX8MM_CLK_ARM				215
#define IMX8MM_CLK_PDM_IPG			216
#define IMX8MM_CLK_GPU2D_ROOT			217
#define IMX8MM_CLK_MU_ROOT			218
#define IMX8MM_CLK_CSI1_ROOT			219

#define IMX8MM_CLK_DRAM_CORE			220
#define IMX8MM_CLK_DRAM_ALT_ROOT		221

#define IMX8MM_CLK_NAND_USDHC_BUS_RAWNAND_CLK	222

#define IMX8MM_CLK_GPIO1_ROOT			223
#define IMX8MM_CLK_GPIO2_ROOT			224
#define IMX8MM_CLK_GPIO3_ROOT			225
#define IMX8MM_CLK_GPIO4_ROOT			226
#define IMX8MM_CLK_GPIO5_ROOT			227

#define IMX8MM_CLK_SNVS_ROOT			228
#define IMX8MM_CLK_GIC				229

#define IMX8MM_SYS_PLL1_40M_CG			230
#define IMX8MM_SYS_PLL1_80M_CG			231
#define IMX8MM_SYS_PLL1_100M_CG			232
#define IMX8MM_SYS_PLL1_133M_CG			233
#define IMX8MM_SYS_PLL1_160M_CG			234
#define IMX8MM_SYS_PLL1_200M_CG			235
#define IMX8MM_SYS_PLL1_266M_CG			236
#define IMX8MM_SYS_PLL1_400M_CG			237
#define IMX8MM_SYS_PLL2_50M_CG			238
#define IMX8MM_SYS_PLL2_100M_CG			239
#define IMX8MM_SYS_PLL2_125M_CG			240
#define IMX8MM_SYS_PLL2_166M_CG			241
#define IMX8MM_SYS_PLL2_200M_CG			242
#define IMX8MM_SYS_PLL2_250M_CG			243
#define IMX8MM_SYS_PLL2_333M_CG			244
#define IMX8MM_SYS_PLL2_500M_CG			245

#define IMX8MM_CLK_M4_CORE			246
#define IMX8MM_CLK_VPU_CORE			247
#define IMX8MM_CLK_GPU3D_CORE			248
#define IMX8MM_CLK_GPU2D_CORE			249

#define IMX8MM_CLK_CLKO2			250

#define IMX8MM_CLK_A53_CORE			251

#define IMX8MM_CLK_CLKOUT1_SEL			252
#define IMX8MM_CLK_CLKOUT1_DIV			253
#define IMX8MM_CLK_CLKOUT1			254
#define IMX8MM_CLK_CLKOUT2_SEL			255
#define IMX8MM_CLK_CLKOUT2_DIV			256
#define IMX8MM_CLK_CLKOUT2			257

#define IMX8MM_CLK_END				258

#endif
