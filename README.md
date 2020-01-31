# freeRTOS_TwrK60D
freeRTOS project to run on TWR_K60D hardware with FTM0 PWM operation
This code should activate a compined PWM signals on FTM0 channels 0 & 1
and be able to modify the duty cycle on FTM0 Channel 2

UART connection to Primary Tower"
UART0 TX: B67
UART0 RX: B68

FTM0 channel 0 (PTC1) is connected to Primary Tower pin A40
FTM0 channel 1 (PTC2) is connected to Primary Tower pin A39
FTM0 channel 2 (PTC3) is connected to Primary Tower pin A38

CLI commands that can be issued on UART0 port:
pwm -?  ==> PWM help screen
pwm -c 0 50 0 8 ==> will set FTM0 Channels 0&1 to 50% duty cycle, 0 dead time and 8Khz period
pwm -c 0 50 0 16 ==> will set FTM0 Channels 0&1 to 50% duty cycle, 0 dead time and 16Khz period
pwm -c 1 25 0 0 ==> should set FTM1 channel 2 to 25% duty cycle
pwm -c 1 50 0 0 ==> should set FTM1 channel 2 to 50% duty cycle
pwm -c 1 90 0 0 ==> should set FTM1 channel 2 to 90% duty cycle
