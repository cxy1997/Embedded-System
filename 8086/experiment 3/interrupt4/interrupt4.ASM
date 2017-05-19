;--------------------------------------------------------------------------
;
;              Build this with the "Source" menu using
;                     "Build All" option
;
;--------------------------------------------------------------------------
;
;                           实验三程序通用框架
;
;--------------------------------------------------------------------------
; 功能： 程序框架					                            				   |
; 编写：《嵌入式系统原理与实验》课程组                   				   |
; 版本：3.5
; 修订：B
;--------------------------------------------------------------------------
		DOSSEG
		.MODEL	SMALL		; 设定8086汇编程序使用Small model
		.8086				; 设定采用8086汇编指令集
;-------------------------------------------------------------------------
;	符号定义                                                              |
;-------------------------------------------------------------------------

; 8253芯片端口地址 （Port Address):
L8253T0			EQU	100H			; Timer0's port number in I/O space
L8253T1			EQU 	102H			; Timer1's port number in I/O space
L8253T2			EQU 	104H			; Timer2's port number in I/O space
L8253CS			EQU 	106H			; 8253 Control Register's port number in I/O space

; 8255芯片端口地址 （Port Address):
L8255PA			EQU	121H			; Port A's port number in I/O space
L8255PB			EQU 	123H			; Port B's port number in I/O space
L8255PC			EQU 	125H			; Port C's port number in I/O space
L8255CS			EQU 	127H			; 8255 Control Register's port number in I/O space

;  中断矢量号定义
IRQNum			EQU	147			; 中断矢量号,要根据学号计算得到后更新此定义。

Patch_Protues	EQU	IN AL, 0	;	Simulation Patch for Proteus, please ignore this line

;=======================================================================
; 宏定义
;=======================================================================

; 修补Proteus仿真的BUG，参见程序段中的使用说明
    WaitForHWInt MACRO INTNum		; INTNum is the HW INT number
		MOV AL, INTNum   			;
		OUT 0, AL					;
		STI
		CLI
    ENDM

;-----------------------------------------------------------
;	定义数据段                                             |
;-----------------------------------------------------------
		.data					; 定义数据段;
DelayShort	dw	300				; 短延时参量	
DelayLong	dw	8000			; 长延时参量
SEGTAB  	DB 3FH	; 7-Segment Tube
	        DB 06H	;
		DB 5BH	;            a a a
	        DB 4FH	;         f         b
		DB 66H	;         f         b
		DB 6DH	;         f         b
		DB 7DH	;            g g g 
		DB 07H	;         e         c
		DB 7FH	;         e         c
		DB 6FH	;         e         c
        	DB 77H	;            d d d     h h h
		DB 7CH	; ----------------------------------
		DB 39H	;       b7 b6 b5 b4 b3 b2 b1 b0
		DB 5EH	;       DP  g  f  e  d  c  b  a
		DB 79H	;
		DB 71H	;
CNT		DW 0
;-----------------------------------------------------------
;	定义代码段                                             |
;-----------------------------------------------------------
		.code						; Code segment definition
		.startup					; Entrance of this program
;------------------------------------------------------------------------
		Patch_Protues					; Simulation Patch for Proteus,
								; Please ignore the above code line.
;------------------------------------------------------------------------

START:								; Modify the following codes accordingly
		MOV AX, @DATA					;
		MOV DS, AX					; Initialize DS

		CALL INIT8255					; Initialize 8255 
		
		MOV  BL, IRQNum					; BL is used as a parameter to call the procedure INT_INIT
		CALL INT_INIT					; Procedure INT_INIT is used to set up the IVT
DISPLAY:		
		MOV BX, CNT
		AND BX, 0FH
		MOV AL, SEGTAB[BX]
		MOV DX, L8255PB
		OUT DX, AL
		MOV DX, L8255PA
		MOV AL, 11110111B
		OUT DX, AL
	        MOV CX, DelayShort
L1:		LOOP L1
	
		MOV BX, CNT
		MOV CL, 4
		SHR BX, CL
		AND BX, 0FH
		MOV AL, SEGTAB[BX]
		MOV DX, L8255PB
		OUT DX, AL
		MOV DX, L8255PA
		MOV AL, 11111011B
		OUT DX, AL
	        MOV CX, DelayShort
L2:		LOOP L2
		
		MOV BX, CNT
		MOV CL, 8
		SHR BX, CL
		AND BX, 0FH
		MOV AL, SEGTAB[BX]
		MOV DX, L8255PB
		OUT DX, AL
		MOV DX, L8255PA
		MOV AL, 11111101B
		OUT DX, AL
	        MOV CX, DelayShort
L3:		LOOP L3

		MOV BX, CNT
		MOV CL, 12
		SHR BX, CL
		AND BX, 0FH
		MOV AL, SEGTAB[BX]
		MOV DX, L8255PB
		OUT DX, AL
		MOV DX, L8255PA
		MOV AL, 11111110B
		OUT DX, AL
	        MOV CX, DelayShort
L4:		LOOP L4
		JMP DISPLAY

;===================================================================================
; Attention:
; The following codes is a Patching for Proteus 8086 Hardware Simulation Bug.
; Use these codes in the case you want the 8086 to halt and waiting for HW INT only! 
; You can treat it as if it doesn't exist. 
;
; If you need to use HW INT, please uncomment it, or
; Don't modify it, leave it here just as it is.
;		WaitForHWInt IRQNum				
;====================================================================================
		HLT						; 
;=====================================================================================

INIT8255 PROC
		MOV AL,10000001B
		MOV DX,L8255CS
		OUT DX, AL
		RET
INIT8255 ENDP

;-------------------------------------------------------------
;                                                             |                                                            |
; Function：INTERRUPT Vector Table INIT						  |
; Input: BL = Interrupt number								  |
; Output: None			                                	  |
;                                                             |
;-------------------------------------------------------------	
INT_INIT	PROC FAR
		CLI				; Disable interrupt
		MOV CL, 2
		SHL BX, CL
		MOV AX, 0
		MOV ES, AX			; To set up the interrupt vector table
		MOV WORD PTR ES:[BX], OFFSET MYIRQ
		MOV WORD PTR ES:[BX+2], SEG MYIRQ
		STI
	        RET	
INT_INIT	ENDP

		
;--------------------------------------------------------------
;                                                             |                                                            |
; FUNCTION: INTERRUPT SERVICE  Routine （ISR）				  | 
; Input::                                                     |
; Output:                                                     |
;                                                             |
;--------------------------------------------------------------	
		
MYIRQ 	PROC FAR
	        CLI
		
		PUSH AX
		MOV AX, CNT
		INC AX
		MOV CNT, AX
		POP AX
		
		STI
		RET
MYIRQ 	ENDP

	.stack 100h				; 定义256字节容量的堆栈
	END					