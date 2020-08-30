STM32工程由Keil5编写，用于读取气压传感器MS5611的数据，并通过蓝牙模块HT05发送给树莓派。对于此工程，Keil5执行编译需要一定的工程配置，现将Option for Target选项卡中的配置罗列如下。
① Device应选择STM32F103C8；
② Output中，勾选Create HEX File；
③ C/C++中，Define中应填写USE_STDPERIPH_DRIVER,STM32F10X_MD，保证某些特定宏命令的执行。
