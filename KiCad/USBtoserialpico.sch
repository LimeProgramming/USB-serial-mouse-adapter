EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Device:R R2
U 1 1 618EB846
P 6350 4300
F 0 "R2" H 6420 4346 50  0000 L CNN
F 1 "3K" H 6420 4255 50  0000 L CNN
F 2 "usbpico:R_1206_3216Metric" V 6280 4300 50  0001 C CNN
F 3 "~" H 6350 4300 50  0001 C CNN
	1    6350 4300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR013
U 1 1 618EDB47
P 6350 3400
F 0 "#PWR013" H 6350 3150 50  0001 C CNN
F 1 "GND" H 6355 3227 50  0000 C CNN
F 2 "" H 6350 3400 50  0001 C CNN
F 3 "" H 6350 3400 50  0001 C CNN
	1    6350 3400
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR014
U 1 1 618EE909
P 6350 4100
F 0 "#PWR014" H 6350 3850 50  0001 C CNN
F 1 "GND" H 6355 3927 50  0000 C CNN
F 2 "" H 6350 4100 50  0001 C CNN
F 3 "" H 6350 4100 50  0001 C CNN
	1    6350 4100
	-1   0    0    1   
$EndComp
Wire Wire Line
	6350 3450 6350 3400
Wire Wire Line
	6350 4150 6350 4100
Wire Wire Line
	6350 4500 6350 4450
Wire Wire Line
	5350 4500 6050 4500
Wire Wire Line
	5350 4300 6050 4300
Wire Wire Line
	6350 4500 6700 4500
Wire Wire Line
	6700 4500 6700 3850
Connection ~ 6350 4500
Wire Wire Line
	6350 3750 6350 3800
Wire Wire Line
	6650 3800 6350 3800
Wire Wire Line
	5350 3900 5900 3900
Wire Wire Line
	6650 3150 6650 3550
$Comp
L power:GND #PWR027
U 1 1 6191CCA4
P 9050 3250
F 0 "#PWR027" H 9050 3000 50  0001 C CNN
F 1 "GND" V 9055 3122 50  0000 R CNN
F 2 "" H 9050 3250 50  0001 C CNN
F 3 "" H 9050 3250 50  0001 C CNN
	1    9050 3250
	0    -1   -1   0   
$EndComp
$Comp
L power:+5V #PWR032
U 1 1 6191D633
P 9200 2300
F 0 "#PWR032" H 9200 2150 50  0001 C CNN
F 1 "+5V" H 9215 2473 50  0000 C CNN
F 2 "" H 9200 2300 50  0001 C CNN
F 3 "" H 9200 2300 50  0001 C CNN
	1    9200 2300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR028
U 1 1 6191E591
P 9050 3750
F 0 "#PWR028" H 9050 3500 50  0001 C CNN
F 1 "GND" V 9055 3622 50  0000 R CNN
F 2 "" H 9050 3750 50  0001 C CNN
F 3 "" H 9050 3750 50  0001 C CNN
	1    9050 3750
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR029
U 1 1 6191F400
P 9050 4250
F 0 "#PWR029" H 9050 4000 50  0001 C CNN
F 1 "GND" V 9055 4122 50  0000 R CNN
F 2 "" H 9050 4250 50  0001 C CNN
F 3 "" H 9050 4250 50  0001 C CNN
	1    9050 4250
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR030
U 1 1 6192040D
P 9050 4750
F 0 "#PWR030" H 9050 4500 50  0001 C CNN
F 1 "GND" V 9055 4622 50  0000 R CNN
F 2 "" H 9050 4750 50  0001 C CNN
F 3 "" H 9050 4750 50  0001 C CNN
	1    9050 4750
	0    -1   -1   0   
$EndComp
Wire Wire Line
	9000 4750 9050 4750
Wire Wire Line
	9000 4250 9050 4250
Wire Wire Line
	9000 3750 9050 3750
Wire Wire Line
	9000 3250 9050 3250
$Comp
L power:GND #PWR020
U 1 1 619265F9
P 7700 3750
F 0 "#PWR020" H 7700 3500 50  0001 C CNN
F 1 "GND" V 7705 3622 50  0000 R CNN
F 2 "" H 7700 3750 50  0001 C CNN
F 3 "" H 7700 3750 50  0001 C CNN
	1    7700 3750
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR019
U 1 1 61926BF7
P 7700 3250
F 0 "#PWR019" H 7700 3000 50  0001 C CNN
F 1 "GND" V 7705 3122 50  0000 R CNN
F 2 "" H 7700 3250 50  0001 C CNN
F 3 "" H 7700 3250 50  0001 C CNN
	1    7700 3250
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR021
U 1 1 6192788D
P 7700 4250
F 0 "#PWR021" H 7700 4000 50  0001 C CNN
F 1 "GND" V 7705 4122 50  0000 R CNN
F 2 "" H 7700 4250 50  0001 C CNN
F 3 "" H 7700 4250 50  0001 C CNN
	1    7700 4250
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR022
U 1 1 61928333
P 7700 4750
F 0 "#PWR022" H 7700 4500 50  0001 C CNN
F 1 "GND" V 7705 4622 50  0000 R CNN
F 2 "" H 7700 4750 50  0001 C CNN
F 3 "" H 7700 4750 50  0001 C CNN
	1    7700 4750
	0    1    1    0   
$EndComp
Wire Wire Line
	7750 3750 7700 3750
Wire Wire Line
	7750 4250 7700 4250
Wire Wire Line
	7750 4750 7700 4750
$Comp
L Device:LED_Small D2
U 1 1 6192CD08
P 7150 3150
F 0 "D2" H 7150 3385 50  0000 C CNN
F 1 "LED_Small" H 7150 3294 50  0000 C CNN
F 2 "usbpico:LED_D3.0mm" V 7150 3150 50  0001 C CNN
F 3 "~" V 7150 3150 50  0001 C CNN
	1    7150 3150
	0    1    1    0   
$EndComp
$Comp
L Device:LED_Small D1
U 1 1 6192F36D
P 6800 3150
F 0 "D1" H 6800 3385 50  0000 C CNN
F 1 "LED_Small" H 6800 3294 50  0000 C CNN
F 2 "usbpico:LED_D3.0mm" V 6800 3150 50  0001 C CNN
F 3 "~" V 6800 3150 50  0001 C CNN
	1    6800 3150
	0    1    1    0   
$EndComp
Wire Wire Line
	7150 3350 7150 3250
Wire Wire Line
	6800 3450 6800 3250
$Comp
L Device:R R4
U 1 1 6193D8A9
P 7150 2850
F 0 "R4" H 7220 2896 50  0000 L CNN
F 1 "R" H 7220 2805 50  0000 L CNN
F 2 "usbpico:R_1206_3216Metric" V 7080 2850 50  0001 C CNN
F 3 "~" H 7150 2850 50  0001 C CNN
	1    7150 2850
	1    0    0    -1  
$EndComp
$Comp
L Device:R R3
U 1 1 6193ECAC
P 6800 2850
F 0 "R3" H 6870 2896 50  0000 L CNN
F 1 "R" H 6870 2805 50  0000 L CNN
F 2 "usbpico:R_1206_3216Metric" V 6730 2850 50  0001 C CNN
F 3 "~" H 6800 2850 50  0001 C CNN
	1    6800 2850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR018
U 1 1 6194008B
P 7150 2650
F 0 "#PWR018" H 7150 2400 50  0001 C CNN
F 1 "GND" H 7155 2477 50  0000 C CNN
F 2 "" H 7150 2650 50  0001 C CNN
F 3 "" H 7150 2650 50  0001 C CNN
	1    7150 2650
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR015
U 1 1 61940B18
P 6800 2650
F 0 "#PWR015" H 6800 2400 50  0001 C CNN
F 1 "GND" H 6805 2477 50  0000 C CNN
F 2 "" H 6800 2650 50  0001 C CNN
F 3 "" H 6800 2650 50  0001 C CNN
	1    6800 2650
	-1   0    0    1   
$EndComp
Wire Wire Line
	7150 3050 7150 3000
Wire Wire Line
	6800 3050 6800 3000
Wire Wire Line
	6800 2700 6800 2650
Wire Wire Line
	7150 2700 7150 2650
Wire Wire Line
	7750 3250 7700 3250
$Comp
L Switch:SW_DIP_x06 SW1
U 1 1 619452DF
P 6350 5100
F 0 "SW1" H 6350 4633 50  0000 C CNN
F 1 "SW_DIP_x06" H 6350 4724 50  0000 C CNN
F 2 "usbpico:SW_DIP_SPSTx06_Slide_9.78x17.42mm_W7.62mm_P2.54mm" H 6350 5100 50  0001 C CNN
F 3 "~" H 6350 5100 50  0001 C CNN
	1    6350 5100
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR012
U 1 1 61963460
P 6000 5300
F 0 "#PWR012" H 6000 5050 50  0001 C CNN
F 1 "GND" V 6005 5172 50  0000 R CNN
F 2 "" H 6000 5300 50  0001 C CNN
F 3 "" H 6000 5300 50  0001 C CNN
	1    6000 5300
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR011
U 1 1 61964E7F
P 5950 5200
F 0 "#PWR011" H 5950 4950 50  0001 C CNN
F 1 "GND" V 5955 5072 50  0000 R CNN
F 2 "" H 5950 5200 50  0001 C CNN
F 3 "" H 5950 5200 50  0001 C CNN
	1    5950 5200
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR010
U 1 1 61965983
P 5900 5100
F 0 "#PWR010" H 5900 4850 50  0001 C CNN
F 1 "GND" V 5905 4972 50  0000 R CNN
F 2 "" H 5900 5100 50  0001 C CNN
F 3 "" H 5900 5100 50  0001 C CNN
	1    5900 5100
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR09
U 1 1 6196689D
P 5850 5000
F 0 "#PWR09" H 5850 4750 50  0001 C CNN
F 1 "GND" V 5855 4872 50  0000 R CNN
F 2 "" H 5850 5000 50  0001 C CNN
F 3 "" H 5850 5000 50  0001 C CNN
	1    5850 5000
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR07
U 1 1 6196742E
P 5800 4900
F 0 "#PWR07" H 5800 4650 50  0001 C CNN
F 1 "GND" V 5805 4772 50  0000 R CNN
F 2 "" H 5800 4900 50  0001 C CNN
F 3 "" H 5800 4900 50  0001 C CNN
	1    5800 4900
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR06
U 1 1 619681A1
P 5750 4800
F 0 "#PWR06" H 5750 4550 50  0001 C CNN
F 1 "GND" V 5755 4672 50  0000 R CNN
F 2 "" H 5750 4800 50  0001 C CNN
F 3 "" H 5750 4800 50  0001 C CNN
	1    5750 4800
	0    1    1    0   
$EndComp
Wire Wire Line
	6050 5300 6000 5300
Wire Wire Line
	6050 5200 5950 5200
Wire Wire Line
	6050 5100 5900 5100
Wire Wire Line
	6050 5000 5850 5000
Wire Wire Line
	6050 4900 5800 4900
Wire Wire Line
	6050 4800 5750 4800
$Comp
L Device:C_Small C2
U 1 1 61970911
P 3500 3700
F 0 "C2" H 3385 3654 50  0000 R CNN
F 1 "1uf" H 3385 3745 50  0000 R CNN
F 2 "usbpico:C_1206_3216Metric" H 3500 3700 50  0001 C CNN
F 3 "~" H 3500 3700 50  0001 C CNN
	1    3500 3700
	0    -1   -1   0   
$EndComp
$Comp
L Device:C_Small C3
U 1 1 619717D8
P 3550 3050
F 0 "C3" H 3665 3096 50  0000 L CNN
F 1 "1uf" H 3665 3005 50  0000 L CNN
F 2 "usbpico:C_1206_3216Metric" H 3550 3050 50  0001 C CNN
F 3 "~" H 3550 3050 50  0001 C CNN
	1    3550 3050
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C5
U 1 1 6197261F
P 5500 3050
F 0 "C5" H 5615 3096 50  0000 L CNN
F 1 "1uf" H 5615 3005 50  0000 L CNN
F 2 "usbpico:C_1206_3216Metric" H 5500 3050 50  0001 C CNN
F 3 "~" H 5500 3050 50  0001 C CNN
	1    5500 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3750 3200 3550 3200
Wire Wire Line
	5350 2900 5500 2900
Wire Wire Line
	5350 3200 5500 3200
$Comp
L Interface_UART:MAX232 U1
U 1 1 618E39A4
P 4550 3800
F 0 "U1" H 4550 5181 50  0000 C CNN
F 1 "MAX232" H 4550 5090 50  0000 C CNN
F 2 "usbpico:max232_SMD" H 4600 2750 50  0001 L CNN
F 3 "http://www.ti.com/lit/ds/symlink/max232.pdf" H 4550 3900 50  0001 C CNN
	1    4550 3800
	-1   0    0    -1  
$EndComp
$Comp
L Device:C_Small C1
U 1 1 61970F2B
P 3500 3350
F 0 "C1" H 3615 3396 50  0000 L CNN
F 1 "1uf" H 3615 3305 50  0000 L CNN
F 2 "usbpico:C_1206_3216Metric" H 3500 3350 50  0001 C CNN
F 3 "~" H 3500 3350 50  0001 C CNN
	1    3500 3350
	0    1    1    0   
$EndComp
Wire Wire Line
	3750 3400 3650 3400
Wire Wire Line
	3650 3400 3650 3350
Wire Wire Line
	3350 3350 3350 2600
Wire Wire Line
	3350 2600 4550 2600
$Comp
L power:GND #PWR03
U 1 1 61999C0C
P 3250 3700
F 0 "#PWR03" H 3250 3450 50  0001 C CNN
F 1 "GND" V 3255 3572 50  0000 R CNN
F 2 "" H 3250 3700 50  0001 C CNN
F 3 "" H 3250 3700 50  0001 C CNN
	1    3250 3700
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR04
U 1 1 6199D34F
P 4550 5100
F 0 "#PWR04" H 4550 4850 50  0001 C CNN
F 1 "GND" H 4555 4927 50  0000 C CNN
F 2 "" H 4550 5100 50  0001 C CNN
F 3 "" H 4550 5100 50  0001 C CNN
	1    4550 5100
	1    0    0    -1  
$EndComp
Wire Wire Line
	4550 5000 4550 5100
$Comp
L Connector:DB9_Female_MountingHoles rs232_1
U 1 1 619AA6CF
P 2300 4300
F 0 "rs232_1" H 2479 4209 50  0000 L CNN
F 1 "DB9_Female_MountingHoles" H 2479 4300 50  0000 L CNN
F 2 "Connector_Dsub:DSUB-9_Male_Horizontal_P2.77x2.84mm_EdgePinOffset7.70mm_Housed_MountingHolesOffset9.12mm" H 2300 4300 50  0001 C CNN
F 3 " ~" H 2300 4300 50  0001 C CNN
	1    2300 4300
	-1   0    0    1   
$EndComp
NoConn ~ 5350 4100
NoConn ~ 3750 4100
NoConn ~ 2600 4700
NoConn ~ 2600 4600
NoConn ~ 2600 4200
NoConn ~ 2600 4100
NoConn ~ 2600 4000
$Comp
L power:GND #PWR01
U 1 1 619C25CB
P 2700 3900
F 0 "#PWR01" H 2700 3650 50  0001 C CNN
F 1 "GND" V 2705 3772 50  0000 R CNN
F 2 "" H 2700 3900 50  0001 C CNN
F 3 "" H 2700 3900 50  0001 C CNN
	1    2700 3900
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2600 3900 2700 3900
Wire Wire Line
	7750 3650 6650 3650
Wire Wire Line
	6650 3550 7750 3550
Wire Wire Line
	7750 3450 6800 3450
Wire Wire Line
	7750 3350 7150 3350
$Comp
L pico_rp2040:PICO_RP2040 U2
U 1 1 618DD9EF
P 8400 4000
F 0 "U2" H 8375 5365 50  0000 C CNN
F 1 "PICO_RP2040" H 8375 5274 50  0000 C CNN
F 2 "usbpico:RPi_Pico_SMD_TH" H 8300 5450 50  0001 C CNN
F 3 "" H 8300 5450 50  0001 C CNN
	1    8400 4000
	1    0    0    -1  
$EndComp
Wire Wire Line
	6650 5300 7450 5300
Wire Wire Line
	7450 5300 7450 4850
Wire Wire Line
	7450 4850 7750 4850
Wire Wire Line
	6650 5200 7350 5200
Wire Wire Line
	7350 5200 7350 4650
Wire Wire Line
	7350 4650 7750 4650
Wire Wire Line
	6650 5100 7250 5100
Wire Wire Line
	7250 5100 7250 4550
Wire Wire Line
	7250 4550 7750 4550
Wire Wire Line
	6650 5000 7150 5000
Wire Wire Line
	7150 5000 7150 4450
Wire Wire Line
	7150 4450 7750 4450
Wire Wire Line
	6650 4900 7050 4900
Wire Wire Line
	7050 4350 7750 4350
Wire Wire Line
	6650 4800 6950 4800
Wire Wire Line
	6950 4150 7750 4150
Wire Wire Line
	9200 2300 9200 3050
Wire Wire Line
	9200 3050 9000 3050
Connection ~ 4550 2600
$Comp
L power:+5V #PWR08
U 1 1 6192A738
P 5850 2150
F 0 "#PWR08" H 5850 2000 50  0001 C CNN
F 1 "+5V" H 5865 2323 50  0000 C CNN
F 2 "" H 5850 2150 50  0001 C CNN
F 3 "" H 5850 2150 50  0001 C CNN
	1    5850 2150
	1    0    0    -1  
$EndComp
Wire Wire Line
	8900 2350 8900 2000
Wire Wire Line
	8900 2000 9450 2000
Wire Wire Line
	9450 2000 9450 3450
Wire Wire Line
	9450 3450 9000 3450
Wire Wire Line
	3550 2950 3550 2900
Wire Wire Line
	3550 3150 3550 3200
Wire Wire Line
	3550 2900 3750 2900
Wire Wire Line
	3650 3350 3600 3350
Wire Wire Line
	3400 3350 3350 3350
Wire Wire Line
	5500 3200 5500 3150
Wire Wire Line
	5500 2950 5500 2900
Wire Wire Line
	3600 3700 3750 3700
Wire Wire Line
	3250 3700 3400 3700
$Comp
L Jumper:SolderJumper_3_Open JP1
U 1 1 619597E0
P 6350 2350
F 0 "JP1" H 6350 2555 50  0000 C CNN
F 1 "SolderJumper_3_Open" H 6350 2464 50  0000 C CNN
F 2 "usbpico:SolderJumper-3_P1.3mm_Open_Pad1.0x1.5mm_NumberLabels" H 6350 2350 50  0001 C CNN
F 3 "~" H 6350 2350 50  0001 C CNN
	1    6350 2350
	1    0    0    -1  
$EndComp
Wire Wire Line
	8900 2350 6550 2350
Wire Wire Line
	5850 2350 5850 2150
Wire Wire Line
	6350 2600 6350 2500
Wire Wire Line
	4550 2600 5350 2600
Wire Wire Line
	5850 2350 6150 2350
Wire Wire Line
	5900 3150 6650 3150
Wire Wire Line
	5900 3900 5900 3150
Connection ~ 6350 3800
Wire Wire Line
	6050 4300 6050 3800
Wire Wire Line
	6700 3850 7750 3850
Wire Wire Line
	6650 3650 6650 3800
$Comp
L Connector:Conn_01x04_Male J1
U 1 1 61A0BD76
P 3250 4350
F 0 "J1" H 3222 4232 50  0000 R CNN
F 1 "Conn_01x04_Male" H 3222 4323 50  0000 R CNN
F 2 "usbpico:Serial_PinSocket_1x04_P2.54mm" H 3250 4350 50  0001 C CNN
F 3 "~" H 3250 4350 50  0001 C CNN
	1    3250 4350
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR02
U 1 1 61A1CDD9
P 2950 4150
F 0 "#PWR02" H 2950 3900 50  0001 C CNN
F 1 "GND" V 2955 4022 50  0000 R CNN
F 2 "" H 2950 4150 50  0001 C CNN
F 3 "" H 2950 4150 50  0001 C CNN
	1    2950 4150
	0    1    1    0   
$EndComp
Wire Wire Line
	3050 4150 2950 4150
Wire Wire Line
	3050 4250 3050 4200
Wire Wire Line
	3350 3900 3750 3900
Wire Wire Line
	3050 4350 3050 4300
Wire Wire Line
	3050 4450 3050 4400
Wire Wire Line
	3050 4400 3750 4400
Wire Wire Line
	3750 4400 3750 4500
Wire Wire Line
	2950 4200 3050 4200
Wire Wire Line
	3050 4400 2600 4400
Connection ~ 3050 4400
Wire Wire Line
	3050 4300 3750 4300
Wire Wire Line
	3350 4200 3350 3900
Connection ~ 3050 4200
Wire Wire Line
	3050 4200 3350 4200
Wire Wire Line
	2950 4200 2950 4300
Wire Wire Line
	2950 4300 2600 4300
Wire Wire Line
	3050 4300 3000 4300
Wire Wire Line
	3000 4300 3000 4500
Wire Wire Line
	2600 4500 3000 4500
Connection ~ 3050 4300
$Comp
L Device:C C4
U 1 1 61B04693
P 5350 2450
F 0 "C4" H 5465 2496 50  0000 L CNN
F 1 "10uf" H 5465 2405 50  0000 L CNN
F 2 "usbpico:C_1206_3216Metric" H 5388 2300 50  0001 C CNN
F 3 "~" H 5350 2450 50  0001 C CNN
	1    5350 2450
	1    0    0    -1  
$EndComp
Connection ~ 5350 2600
Wire Wire Line
	5350 2600 6350 2600
$Comp
L power:GND #PWR05
U 1 1 61B05FF4
P 5350 2300
F 0 "#PWR05" H 5350 2050 50  0001 C CNN
F 1 "GND" H 5355 2127 50  0000 C CNN
F 2 "" H 5350 2300 50  0001 C CNN
F 3 "" H 5350 2300 50  0001 C CNN
	1    5350 2300
	-1   0    0    1   
$EndComp
Wire Wire Line
	6950 4800 6950 4150
Wire Wire Line
	7050 4900 7050 4350
$Comp
L Switch:SW_MEC_5E SW2
U 1 1 61B1714F
P 9750 4100
F 0 "SW2" H 9750 4485 50  0000 C CNN
F 1 "SW_MEC_5E" H 9750 4394 50  0000 C CNN
F 2 "usbpico:SW_PUSH_6mm" H 9750 4400 50  0001 C CNN
F 3 "http://www.apem.com/int/index.php?controller=attachment&id_attachment=1371" H 9750 4400 50  0001 C CNN
	1    9750 4100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR033
U 1 1 61B18F1D
P 9950 4200
F 0 "#PWR033" H 9950 3950 50  0001 C CNN
F 1 "GND" H 9955 4027 50  0000 C CNN
F 2 "" H 9950 4200 50  0001 C CNN
F 3 "" H 9950 4200 50  0001 C CNN
	1    9950 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	9950 4000 9950 4100
Connection ~ 9950 4100
Wire Wire Line
	9950 4100 9950 4200
Wire Wire Line
	9550 4000 9550 4050
Wire Wire Line
	9000 4050 9550 4050
Connection ~ 9550 4050
Wire Wire Line
	9550 4050 9550 4100
$Comp
L power:GND #PWR031
U 1 1 61B92FF4
P 9200 700
F 0 "#PWR031" H 9200 450 50  0001 C CNN
F 1 "GND" H 9205 527 50  0000 C CNN
F 2 "" H 9200 700 50  0001 C CNN
F 3 "" H 9200 700 50  0001 C CNN
	1    9200 700 
	0    1    1    0   
$EndComp
Wire Wire Line
	9450 700  9200 700 
Wire Wire Line
	9550 700  9450 700 
Connection ~ 9450 700 
$Comp
L Connector:USB_A USB1
U 1 1 61B89299
P 9450 1100
F 0 "USB1" H 9220 997 50  0000 R CNN
F 1 "USB_A" H 9220 1088 50  0000 R CNN
F 2 "usbpico:USB_A_Wuerth_614004134726_Horizontal" H 9600 1050 50  0001 C CNN
F 3 " ~" H 9600 1050 50  0001 C CNN
	1    9450 1100
	-1   0    0    1   
$EndComp
$Comp
L power:+5V #PWR025
U 1 1 61C1CD84
P 8800 1450
F 0 "#PWR025" H 8800 1300 50  0001 C CNN
F 1 "+5V" H 8815 1623 50  0000 C CNN
F 2 "" H 8800 1450 50  0001 C CNN
F 3 "" H 8800 1450 50  0001 C CNN
	1    8800 1450
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR026
U 1 1 61C2CCD1
P 9000 1600
F 0 "#PWR026" H 9000 1350 50  0001 C CNN
F 1 "GND" H 9005 1427 50  0000 C CNN
F 2 "" H 9000 1600 50  0001 C CNN
F 3 "" H 9000 1600 50  0001 C CNN
	1    9000 1600
	1    0    0    -1  
$EndComp
$Comp
L Device:C C6
U 1 1 61C2CCD7
P 9000 1450
F 0 "C6" H 9115 1496 50  0000 L CNN
F 1 "10uf" H 9115 1405 50  0000 L CNN
F 2 "usbpico:C_1206_3216Metric" H 9038 1300 50  0001 C CNN
F 3 "~" H 9000 1450 50  0001 C CNN
	1    9000 1450
	1    0    0    -1  
$EndComp
Wire Wire Line
	9150 1300 9000 1300
Wire Wire Line
	8800 1300 8800 1450
Connection ~ 9000 1300
Wire Wire Line
	9000 1300 8800 1300
$Comp
L Connector:Conn_01x05_Male J3
U 1 1 61C5BFB2
P 7750 1100
F 0 "J3" H 7904 812 50  0000 R CNN
F 1 "Conn_01x05_Male" H 8150 750 50  0000 R CNN
F 2 "usbpico:PinSocket_1x05_P2.54mm_Vertical" H 7750 1100 50  0001 C CNN
F 3 "~" H 7750 1100 50  0001 C CNN
	1    7750 1100
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR023
U 1 1 61C75AF6
P 8050 700
F 0 "#PWR023" H 8050 550 50  0001 C CNN
F 1 "+5V" H 8065 873 50  0000 C CNN
F 2 "" H 8050 700 50  0001 C CNN
F 3 "" H 8050 700 50  0001 C CNN
	1    8050 700 
	1    0    0    -1  
$EndComp
Wire Wire Line
	7950 1300 7950 1200
Wire Wire Line
	7950 1200 8300 1200
Connection ~ 7950 1200
$Comp
L power:GND #PWR024
U 1 1 61C911F9
P 8300 1600
F 0 "#PWR024" H 8300 1350 50  0001 C CNN
F 1 "GND" H 8305 1427 50  0000 C CNN
F 2 "" H 8300 1600 50  0001 C CNN
F 3 "" H 8300 1600 50  0001 C CNN
	1    8300 1600
	1    0    0    -1  
$EndComp
Wire Wire Line
	8300 1200 8300 1600
Wire Wire Line
	7950 900  8050 900 
Wire Wire Line
	8050 900  8050 700 
Connection ~ 8750 1100
Wire Wire Line
	8750 1100 7950 1100
Connection ~ 8750 1000
Wire Wire Line
	8750 1000 7950 1000
Wire Wire Line
	9150 1000 8750 1000
Wire Wire Line
	9150 1100 8750 1100
$Comp
L Connector:TestPoint TP2
U 1 1 61B9DFDF
P 8750 1100
F 0 "TP2" H 8692 1126 50  0000 R CNN
F 1 "TestPoint" H 8692 1217 50  0000 R CNN
F 2 "usbpico:TestPoint_Pad_1.5x1.5mm" H 8950 1100 50  0001 C CNN
F 3 "~" H 8950 1100 50  0001 C CNN
	1    8750 1100
	-1   0    0    1   
$EndComp
$Comp
L Connector:TestPoint TP1
U 1 1 61B9D13B
P 8750 1000
F 0 "TP1" H 8808 1118 50  0000 L CNN
F 1 "TestPoint" H 8808 1027 50  0000 L CNN
F 2 "usbpico:TestPoint_Pad_1.5x1.5mm" H 8950 1000 50  0001 C CNN
F 3 "~" H 8950 1000 50  0001 C CNN
	1    8750 1000
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x02_Male J2
U 1 1 61A5DB58
P 6900 1050
F 0 "J2" H 7008 1231 50  0000 C CNN
F 1 "Conn_01x02_Male" H 7008 1140 50  0000 C CNN
F 2 "usbpico:PinSocket_1x02_P2.54mm_Vertical" H 6900 1050 50  0001 C CNN
F 3 "~" H 6900 1050 50  0001 C CNN
	1    6900 1050
	1    0    0    -1  
$EndComp
Wire Wire Line
	7100 900  7100 1050
Wire Wire Line
	7100 1300 7100 1150
$Comp
L power:+5V #PWR016
U 1 1 61D620E2
P 7100 900
F 0 "#PWR016" H 7100 750 50  0001 C CNN
F 1 "+5V" H 7115 1073 50  0000 C CNN
F 2 "" H 7100 900 50  0001 C CNN
F 3 "" H 7100 900 50  0001 C CNN
	1    7100 900 
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR017
U 1 1 61D67433
P 7100 1300
F 0 "#PWR017" H 7100 1050 50  0001 C CNN
F 1 "GND" H 7105 1127 50  0000 C CNN
F 2 "" H 7100 1300 50  0001 C CNN
F 3 "" H 7100 1300 50  0001 C CNN
	1    7100 1300
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 618EA60D
P 6350 3600
F 0 "R1" H 6420 3646 50  0000 L CNN
F 1 "3K" H 6420 3555 50  0000 L CNN
F 2 "usbpico:R_1206_3216Metric" V 6280 3600 50  0001 C CNN
F 3 "~" H 6350 3600 50  0001 C CNN
	1    6350 3600
	1    0    0    -1  
$EndComp
$Comp
L Device:R RB1
U 1 1 618EAF43
P 6200 3800
F 0 "RB1" V 5993 3800 50  0000 C CNN
F 1 "1.5K" V 6084 3800 50  0000 C CNN
F 2 "usbpico:R_1206_3216Metric_bridge" V 6130 3800 50  0001 C CNN
F 3 "~" H 6200 3800 50  0001 C CNN
	1    6200 3800
	0    1    1    0   
$EndComp
$Comp
L Device:R RB2
U 1 1 618EC13F
P 6200 4500
F 0 "RB2" V 6407 4500 50  0000 C CNN
F 1 "1.5K" V 6316 4500 50  0000 C CNN
F 2 "usbpico:R_1206_3216Metric" V 6130 4500 50  0001 C CNN
F 3 "~" H 6200 4500 50  0001 C CNN
	1    6200 4500
	0    -1   -1   0   
$EndComp
$EndSCHEMATC
