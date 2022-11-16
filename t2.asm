.MODEL SMALL
.STACK 100H

.DATA
    NEWL DB 13, 10, "$"

    STR1 DB "Lol Hello !!!$"
    STR2 DB "Second Line Baby !!!$"

    

.CODE
    MOV AX, @DATA
    MOV DS, AX

    JMP startLabel


func: MOV AX, 0002H

    LEA DX, STR1
    MOV AH, 09H
    INT 21H

    LEA DX, NEWL
    MOV AH, 09H
    INT 21H

    LEA DX, STR2
    MOV AH, 09H
    INT 21H

    RET


startLabel: MOV AX, 0002H

    CALL func

    LEA DX, STR1
    MOV AH, 09H
    INT 21H

    LEA DX, NEWL
    MOV AH, 09H
    INT 21H

    LEA DX, STR2
    MOV AH, 09H
    INT 21H




endingLabel: MOV AH, 4CH
    INT 21H

END