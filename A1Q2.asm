.MODEL SMALL
.STACK 100H

.DATA
    ASK1 DB "Enter a valid uppercase character: $"
    TELL1 DB "The corresponding lowercase character: $"
    TELL2 DB "Entered character is not a valid uppercase character ...$"
    NEWL DB 13, 10, "$"

.CODE
    MOV AX, @DATA
    MOV DS, AX

    LEA DX, ASK1
    MOV AH, 09H
    INT 21H

    MOV AH, 01H
    INT 21H
    
    LEA DX, NEWL
    MOV AH, 09H
    INT 21H

    CMP AL, 'A'
    JL invalidInputLabel
    CMP AL, 'Z'
    JG invalidInputLabel

    ADD AL, 20H

    LEA DX, TELL1
    MOV AH, 09H
    INT 21H

    MOV DL, AL
    MOV AH, 02H
    INT 21H
    JMP endingLabel

invalidInputLabel: LEA DX, TELL2
    MOV AH, 09H
    INT 21H

endingLabel: MOV AH, 4CH
    INT 21H

END