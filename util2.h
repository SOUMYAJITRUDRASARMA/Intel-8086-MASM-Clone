#include <bits/stdc++.h>
using namespace std;

unordered_map<string, pair<vector<unsigned char>, bool>> createOpTab()
{
    // FORMAT OF VALUE TYPE -->
    // pair -->
    //     (i)  Stream of opcode bytes
    //     (ii) Whether last operand needs to be processed and enter by the user (eg :- immediate values or labels)
    unordered_map<string, pair<vector<unsigned char>, bool>> m;

    m["INT"] = {{0xCD, }, true};


    m["CALL"] = {{0xE8, }, true};


    m["RET"] = {{0xC3, }, true};


    m["JMP L"] = {{0xE9, }, true};
    m["JMP S"] = {{0xEB, }, true};


    m["LEA AX"] = {{0x8D, 0x06}, true};
    m["LEA BX"] = {{0x8D, 0x1E}, true};
    m["LEA CX"] = {{0x8D, 0x0E}, true};
    m["LEA DX"] = {{0x8D, 0x16}, true};
    m["LEA SI"] = {{0x8D, 0x36}, true};
    m["LEA DI"] = {{0x8D, 0x3E}, true};
    m["LEA BP"] = {{0x8D, 0x2E}, true};
    m["LEA SP"] = {{0x8D, 0x26}, true};


    m["MOV AL"] = {{0xB0, }, true};
    m["MOV AH"] = {{0xB4, }, true};
    m["MOV AX"] = {{0xB8, }, true};
    m["MOV BL"] = {{0xB3, }, true};
    m["MOV BH"] = {{0xB7, }, true};
    m["MOV BX"] = {{0xBB, }, true};
    m["MOV CL"] = {{0xB1, }, true};
    m["MOV CH"] = {{0xB5, }, true};
    m["MOV CX"] = {{0xB9, }, true};
    m["MOV DL"] = {{0xB2, }, true};
    m["MOV DH"] = {{0xB6, }, true};
    m["MOV DX"] = {{0xBA, }, true};
    m["MOV SI"] = {{0xBE, }, true};
    m["MOV DI"] = {{0xBF, }, true};
    m["MOV BP"] = {{0xBD, }, true};
    m["MOV SP"] = {{0xBC, }, true};

    // Complete all register pair <- RP  &&  register byte <- RB if u can ...
    m["MOV AX @DATA"] = {{0xB8, }, true};
    m["MOV DS AX"] = {{0x8E, 0xD8}, true};


    // PUSH ...
    m["PUSH"] = {{0x68, }, true};  // for immediate word

    // for registers (complete till GS -> it is vector, so use 1 or 2 unsigned chars as needed) ...
    m["PUSH AX"] = {{0x50, }, false};
    // m["PUSH "] = {{0x, }, false};



    // POP (complete) ...
    m["POP AX"] = {{0x58, }, false};
    // m["POP "] = {{0x, }, false};



    // m[""] = {{0x, }, true};

    return m;
}