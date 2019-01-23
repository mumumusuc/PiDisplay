//
// Created by mumumusuc on 19-1-14.
//

#ifndef PI_DISPLAY__SSD1306_H
#define PI_DISPLAY__SSD1306_H


//调试模式
#define SSD1306_DEBUG
#define SSD1306_I2C_ADDR 0x3C
#define SSD1306_I2C_RATE 1000000

#define SCREEN_ROWS 64
#define SCREEN_COLUMNS 128

/**
 * 基本常量
 */
#define SSD1306_TRUE 0x01
#define SSD1306_FALSE 0x00

/**
 * 显示屏幕
 */
#define CMD_DISPLAY_ON 0xAF

/**
 * 关闭屏幕
 */
#define CMD_DISPLAY_OFF 0xAE

/**
 * 正常显示
 */
#define CMD_DISPLAY_NORMAL 0xA6

/**
 * 反转显示
 */
#define CMD_DISPLAY_INVERSE 0xA7

/**
 * 根据内存点亮
 */
#define CMD_DISPLAY_RAM 0xA4

/**
 * 全屏点亮
 */
#define CMD_DISPLAY_ALL 0xA5

/**
 * 对比度设定(双字节命令)
 * 1 - 256级(默认 0x7F)
 * 示例: 0x81 0x7F
 */
#define CMD_DISPLAY_CONTRAST 0x81

/** 屏幕显示类命令 ************************************************************/

/**
 * 页偏移参数
 */
#define SCROLL_PAGE_MASK 0x00
#define SCROLL_PAGE0 0x00
#define SCROLL_PAGE1 0x01
#define SCROLL_PAGE2 0x02
#define SCROLL_PAGE3 0x03
#define SCROLL_PAGE4 0x04
#define SCROLL_PAGE5 0x05
#define SCROLL_PAGE6 0x06
#define SCROLL_PAGE7 0x07

/**
 * 帧数参数
 */
#define SCROLL_FRAME_5      0x00
#define SCROLL_FRAME_64     0x01
#define SCROLL_FRAME_128    0x02
#define SCROLL_FRAME_256    0x03
#define SCROLL_FRAME_3      0x04
#define SCROLL_FRAME_4      0x05
#define SCROLL_FRAME_25     0x06
#define SCROLL_FRAME_2      0x07


/**
 * 水平移动特效设置
 *
 * 命令格式：
 *      CMD_SCROLL_RIGHT/CMD_SCROLL_LEFT
 *      A ==> 固定(0x00)
 *      B ==> 起始页地址, 如 SCROLL_PAGE0
 *      C ==> 帧率, 如 SCROLL_FRAME_5
 *      D ==> 结束页地址(需大于等于页起始地址), 如 SCROLL_PAGE0
 *      E ==> 固定(0x00)
 *      F ==> 固定(0xFF)
 * 示例:
 *      {CMD_SCROLL_RIGHT,0x00,SCROLL_PAGE0,SCROLL_FRAME_5,SCROLL_PAGE1,0x00,0xFF}
 *      从页0到页1区域以5帧(列)每秒的速度向右移动
 */
#define CMD_SCROLL_RIGHT 0x26
#define CMD_SCROLL_LEFT 0x27
#define SCROLL_DIRECT_LEFT CMD_SCROLL_LEFT
#define SCROLL_DIRECT_RIGHT CMD_SCROLL_RIGHT

/**
 * 水平向下移动
 *
 * 命令格式:
 *      CMD_SCROLL_DOWN_RIGHT/CMD_SCROLL_DOWN_LEFT
 *      A ==> 固定(0x00)
 *      B ==> 起始页地址, 如 SCROLL_PAGE0
 *      C ==> 帧率, 如 SCROLL_FRAME_5
 *      D ==> 结束页地址(需大于等于页起始地址), 如 SCROLL_PAGE0
 *      E ==> 纵向偏移值, 取值范围 0x01(偏移1行) - 0x3F(偏移63行)
 * 示例:
 *      {CMD_SCROLL_DOWN_RIGHT,0x00,SCROLL_PAGE0,SCROLL_FRAME_5,SCROLL_PAGE1,0x02}
 *      从页0到页1区域以5帧(列)每秒的速度向右移动,并以10行(5*2)每秒的速度向下移动
 */
#define CMD_SCROLL_DOWN_RIGHT 0x29
#define CMD_SCROLL_DOWN_LEFT 0x2A
#define SCROLL_DOWN_DIRECT_RIGHT CMD_SCROLL_DOWN_RIGHT
#define SCROLL_DOWN_DIRECT_LEFT CMD_SCROLL_DOWN_LEFT

/**
 * 关闭移动特效
 */
#define CMD_SCROLL_DISABLE 0x2E

/**
 * 使能移动特效
 */
#define CMD_SCROLL_ENABLE 0x2F

/**
 * 向下滚动范围设置
 *
 * 命令格式:
 *      CMD_SCROLL_DOWN_AREA
 *      A ==> 固定的行数,取值范围 0x00 - 0x3F
 *      B ==> 滚动的行数,取值范围 0x00 - 0x3F;
 *              设定值 > [水平向下移动]中[纵向偏移值]
 *              设定值 > 显示开始行
 * 示例:
 *      0xA3,0x00,0x3F ==> 全屏滚动
 *      0xA3,0x00,0x03 ==> 顶部3行滚动
 */
#define CMD_SCROLL_DOWN_AREA 0xA3

/**
 * 特效模式参数
 */
#define FADE_OFF    0x00
#define FADE_ONCE     0x20
#define FADE_LOOP   0x30 //循环渐隐

/**
 * 渐隐特效帧率
 */
#define FADE_FRAME_8    0x00
#define FADE_FRAME_16   0x01
#define FADE_FRAME_24   0x02
#define FADE_FRAME_32   0x03
#define FADE_FRAME_40   0x04
#define FADE_FRAME_48   0x05
#define FADE_FRAME_56   0x06
#define FADE_FRAME_64   0x07
#define FADE_FRAME_72   0x08
#define FADE_FRAME_80   0x09
#define FADE_FRAME_88   0x0A
#define FADE_FRAME_96   0x0B
#define FADE_FRAME_104  0x0C
#define FADE_FRAME_112  0x0D
#define FADE_FRAME_120  0x0E
#define FADE_FRAME_128  0x0F

/**
 * 渐隐特效
 *
 * 命令格式:
 *      CMD_GRAPHIC_FADE
 *      A ==> (默认) FADE_OFF | FADE_FRAME_8 特效开关与帧率设置组合位
 *          A[5:4]模式选项
 *          A[3:0]帧数选项
 */
#define CMD_GRAPHIC_FADE 0x23

/**
 * 缩放特效参数
 */
#define ZOOM_ON     0x01
#define ZOOM_OFF    0x00
/**
 * 缩放特效
 *
 * 命令格式:
 *      CMD_GRAPHIC_ZOOM
 *      A ==> 开关参数 默认(ZOOM_OFF)
 */
#define CMD_GRAPHIC_ZOOM 0xD6

/** 特效命令 ******************************************************************/

/**
 * 地址模式
 */
#define MODE_HORIZONTAL 0x00
#define MODE_VERTICAL   0x01
#define MODE_PAGE       0x02
#define MODE_INVALID    0x03

/**
 * 内存地址映射模式
 *
 * 命令格式:
 *      CMD_ADDR_MODE
 *      A ==> 模式设定,如 MODE_PAGE(默认)
 */
#define CMD_ADDR_MODE 0x20

/**
 * 设置列可编辑区域(仅适用于水平和垂直模式)
 *
 * 命令格式:
 *      CMD_ADDR_COL_RANGE
 *      A ==> 开始列, 取值范围 0x00 - 0x7F（0-127）默认0x00
 *      B ==> 结束列, 取值范围 0x00 - 0x7F（0-127）默认0x3F
 */
#define CMD_ADDR_COL_RANGE 0x21

/**
 * 设置页可编辑区域(仅适用于水平和垂直模式)
 *
 * 命令格式:
 *      CMD_ADDR_PAGE_RANGE
 *      A ==> 开始页, 取值范围 0x00 - 0x07 默认0x00
 *      B ==> 结束页, 取值范围 0x00 - 0x07 默认0x07
 */
#define CMD_ADDR_PAGE_RANGE 0x22

/**
 * 数据写入起始页设定(仅适用于页模式)
 */
#define CMD_ADDR_PAGE_START_MASK 0xB0
#define CMD_ADDR_PAGE_START_0 0xB0
#define CMD_ADDR_PAGE_START_1 0xB1
#define CMD_ADDR_PAGE_START_2 0xB2
#define CMD_ADDR_PAGE_START_3 0xB3
#define CMD_ADDR_PAGE_START_4 0xB4
#define CMD_ADDR_PAGE_START_5 0xB5
#define CMD_ADDR_PAGE_START_6 0xB6
#define CMD_ADDR_PAGE_START_7 0xB7

/**
 * 数据写入起始列设定,分别为高4位与低4位掩码(仅适用于页模式)
 * 默认0x0, {0x10,0x03}==>从第4列开始写数据
 */
#define CMD_ADDR_COL_START_HIGH_MASK 0x10
#define CMD_ADDR_COL_START_LOW_MASK  0x00

/** 地址设定命令 **************************************************************/

/**
 * 设置显示内存起始位置
 *
 * 取值范围: 0x00 - 0x3F(0-63) 默认为0x00
 * 命令范围: CMD_HARD_START_LINE_MASK | 0x00
 */
#define CMD_HARD_START_LINE_MASK 0x40

/**
 * 设置列映射SEG0的地址为0(默认)
 */
#define CMD_HARD_MAP_COL_0      0xA0
/**
 * 设置列映射SEG0的地址为127
 */
#define CMD_HARD_MAP_COL_127    0xA1

/**
 * 设置有效内存行
 *
 * 命令格式:
 *      CMD_HARD_MUX
 *      A ==> 复用率, 取值范围 0x0F - 0x3F(15-63) 默认0x3F
 */
#define CMD_HARD_MUX 0xA8

/**
 * 设置公共端扫描方向 正向(默认)
 */
#define CMD_HARD_SCAN_DIRECT_NORMAL 0xC0

/**
 * 设置公共端扫描方向 反向
 */
#define CMD_HARD_SCAN_DIRECT_INVERSE 0xC8

/**
 * 设置纵向公共端偏移量
 *
 * 命令格式:
 *      CMD_HARD_VERTICAL_OFFSET
 *      A ==> 偏移量, 取值范围 0x00 - 0x3F(0-63) 默认0x00
 */
#define CMD_HARD_VERTICAL_OFFSET 0xD3

/**
 * 配置公共端口属性
 *
 * 命令格式:
 *      CMD_HARD_COM_CONFIG
 *      A ==> 命令序列 0 0 A5 A4 0 0 1 0
 *          A5 => 0:禁用公共端口左右交换(默认)  1:公共端口左右交换
 *          A4 => 0:连续 1:间隔(默认)
 */
#define CMD_HARD_COM_CONFIG 0xDA

/** 硬件设定命令 **************************************************************/

/**
 * 时钟设定
 *
 * 命令格式:
 *      CMD_TIME_CLOCK
 *      A ==> A7 A6 A5 A4 A3 A2 A1 A0
 *          A[3:0] 分频计数器,显示频率=时钟频率/(设定值+1),取值范围 0-15(1-16的时钟周期) 默认 0
 *          A[7:4] 时钟周期,值越大频率越高,取值范围 0-15(1-16) 默认 (0x8 << 4)
 */
#define CMD_TIME_CLOCK 0xD5

/**
 * 供电时间设置
 *
 * 命令格式:
 *      CMD_POWER_PRECHARGE
 *      A ==> A7 A6 A5 A4 A3 A2 A1 A0
 *          A[3:0] 第一阶段供电时钟周期数,取值范围 1-15 默认 2
 *          A[7:4] 第二阶段供电时钟周期数,取值范围 1-15 默认 2
 */
#define CMD_POWER_PRECHARGE 0xD9

/**
 * 电压设定参数
 */
#define VOLTAGE_0_DOT_65X 0x00
#define VOLTAGE_0_DOT_77X 0x20
#define VOLTAGE_0_DOT_83X 0x30

/**
 * 电压设定
 *
 * 命令格式:
 *      CMD_POWER_VOLTAGE
 *      A ==> 电压值,默认VOLTAGE_0_DOT_77X 0.77Vcc
 */
#define CMD_POWER_VOLTAGE 0xDB

/**
 * 空指令
 */
#define CMD_NOP 0xE3

/**
 * 电荷泵参数
 */
#define CHARGE_PUMP_ON  0x14
#define CHARGE_PUMP_OFF 0x10

/**
 * 电荷泵设定
 *
 * 命令格式:
 *      CMD_POWER_CHARGE_PUMP
 *      A ==> CHARGE_PUMP_OFF(默认值)
 */
#define CMD_POWER_CHARGE_PUMP 0x8D

/** 时钟/供电设定命令 *********************************************************/

/**
 * 读取命令状态指示
 */
#define CMD_READ_DISPLAY_ON     0x00
#define CMD_READ_DISPLAY_OFF    0x40
/** 状态读取命令 ***××××*******************************************************/


#endif //PI_DISPLAY_SSD1306_H