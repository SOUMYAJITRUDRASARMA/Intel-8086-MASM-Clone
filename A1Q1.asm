.MODEL SMALL
.STACK 100H

.DATA
    NAME1 DB "Name: Soumyajit Rudra Sarma$"
    PROGTITLE DB "Program title: A1q1.asm$"
.CODE
    MOV AX, @DATA
    MOV DS, AX

    LEA DX, NAME1
    MOV AH, 09H
    INT 21H

    MOV AH, 02H
    MOV DL, 0DH
    INT 21H

    MOV DL, 0AH
    INT 21H

    LEA DX, PROGTITLE
    MOV AH, 09H
    INT 21H

    MOV AH, 4CH
    INT 21H
END
