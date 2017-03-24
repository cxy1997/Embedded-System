;-----------------------------------------------------------
;实验一 程序2
;功能：控制不同颜色的LED模拟交通信号灯
;74LS273带有8个发光二极管，地址为88H or 8AH or 8CH or 8EH
;Designer：MasterofDarkArts
;-----------------------------------------------------------
.MODEL SMALL; 设定8086汇编程序使用Small model
.8086 ; 设定采用8086汇编指令集
;-----------------------------------------------------------
;	符号定义                                               |
;-----------------------------------------------------------
PortOut	EQU 88h	;定义输出端口号
INITIALSTATE EQU 0F6h ;灯光初始化, 南北红, 东西红
STATE_1 EQU 0F3h; 南北绿, 东西红
STATE_2 EQU 0F7h; 南北不亮, 东西红
STATE_3 EQU 0F5h; 南北黄, 东西红
STATE_4 EQU 0DEh; 南北红, 东西绿
STATE_5 EQU 0FEh; 南北红, 东西不亮
STATE_6 EQU 0EEh; 南北红, 东西黄
ITERATION_BLINK EQU 6000h; 灯闪烁间隔
;-----------------------------------------------------------
;	定义堆栈段                                             |
;-----------------------------------------------------------
.stack 100h; 定义256字节容量的堆栈

;-----------------------------------------------------------
;	定义数据段                                             |
;-----------------------------------------------------------
.data; 定义数据段
;MyVar	DB	?					; 定义MyVar变量   
;-----------------------------------------------------------
;	定义代码段                                             |
;-----------------------------------------------------------
.code; Code segment definition
.startup; 定义汇编程序执行入口点
;
;以下开始放置用户指令代码
;
;-----------------------------------------------------------
;主程序部分, 模拟交通信号灯                    |
;-----------------------------------------------------------
    MOV AL, INITIALSTATE; 灯光初始化
    OUT PortOut, AL; 送显示
    MOV CX, 0; 循环次数最大化
Stage1:
    LOOP Stage1
    MOV AL, STATE_1
    OUT PortOut, AL
Stage2:
    LOOP Stage2
    MOV AL, STATE_2
    OUT PortOut, AL
    MOV CX, ITERATION_BLINK
Stage3:
    LOOP Stage3
    MOV AL, STATE_1
    OUT PortOut, AL
    MOV CX, ITERATION_BLINK
Stage4:
    LOOP Stage4
    MOV AL, STATE_2
    OUT PortOut, AL
    MOV CX, ITERATION_BLINK
Stage5:
    LOOP Stage5
    MOV AL, STATE_3
    OUT PortOut, AL
Stage6:
    LOOP Stage6
    MOV AL, STATE_4
    OUT PortOut, AL
Stage7:
    LOOP Stage7
    MOV AL, STATE_5
    OUT PortOut, AL
    MOV CX, ITERATION_BLINK
Stage8:
    LOOP Stage8
    MOV AL, STATE_4
    OUT PortOut, AL
    MOV CX, ITERATION_BLINK
Stage9:
    LOOP Stage9
    MOV AL, STATE_5
    OUT PortOut, AL
    MOV CX, ITERATION_BLINK
Stage10:
    LOOP Stage10
    MOV AL, STATE_6
    OUT PortOut, AL
    JMP Stage1
    END; 指示汇编程序结束编译