# pi-display
**测试环境 Linux/arm 4.14.95 kernel**

## dht11 温度传感器
|GPIO|设备节点|读取值|
|----|--------|-------|
|DATA:20|/dev/dht11|4bytes:<br>	[0].[1]湿度xx.xx%RH<br>	[2].[3]温度xx.xx°C|

## hc-sr04 超声波距离传感器
|GPIO|设备节点|读取值|
|----|--------|-------|
|Trig:18<br>Echo:24|/dev/hc-sr04|u32:距离(mm)

## ssd1306 128x64-oled显示模块
### spi & i2c
使用`config_spi=1; config_i2c=1`启用spi或i2c设备
|Interface|GPIO|设备节点|
|---------|----|---------|
|spi(32Mhz)|CS:cs0/cs1<br>DC:22/23<br>RESET:27/17|/dev/fbx|
|i2c(default)|RESET:4|/dev/fbx|

模块使用spi_board_info/i2c_board_info注册设备，或遵循ARM-Linux规范使用设备树
**arch/arm/boot/dts/bcm2708-rpi-0-w.dts(树莓派zero w)**
不改动spidev，增加cs2(gpio5) cs3(gpio6)
```
spi0_cs_pins: spi0_cs_pins {
		brcm,pins = <8 7 5 6>;
		brcm,function = <1>; /* output */
	};
```
增加spi设备:
```
display0: display@0{
		compatible = "ssd1306";
		reg = <2>;	/* CE2 */
		#address-cells = <1>;
		#size-cells = <0>;
		spi-max-frequency = <32000000>;
		display-reset = <27>;
		display-dc = <22>;
	};
```
### backlight设备
```
echo [0~255] > /sys/class/backlight/bl-fb1/brightness
cat /sys/class/backlight/bl-fb1/brightness
```
### 更新udev
修改backlight为video组，添加同组读写权限
/etc/udev/rules.d/

### 多个display设备
使用`display_horizontal=2; display_vertical=1`设置屏幕排列
![2个ssd1306](https://github.com/mumumusuc/PiDisplay/images/?)

### 使用xorg和tty
con2fbmap 1 1
sudo startx 

## 都试试看
cmake项目[pi-freetype](https://github.com/mumumusuc/pi-freetype)
编译[freetype](https://www.freetype.org/download.html)支持中文字符
使用display*2（256*64）
使用hc-sr04调节背光
使用卡尔曼滤波器
