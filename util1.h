#include <bits/stdc++.h>
using namespace std;

string MEMORY_BYTE("RMB");
string REGISTER_BYTE("RB");
string REGISTER_OR_MEMORY_BYTE("RMB");
string MEMORY_WORD("MW");
string REGISTER_WORD("RW");
string REGISTER_OR_MEMORY_WORD("RMW");
string MEMORY_DOUBLE_WORD("MD");
string REGISTER_DOUBLE_WORD("RD");
string MEMORY_QUAD_WORD("MQ");
string SHORT_LABEL("SL");
string LONG_LABEL("LL");
string NEAR_POINTER("NP");
string FAR_POINTER("FP");
string IMMEDIATE_BYTE("IB");
string IMMEDIATE_WORD("IW");
string MEMORY_WORD_REAL("MWR");
string MEMORY_DOUBLE_WORD_REAL("MDR");
string MEMORY_QUAD_WORD_REAL("MQR");
string MEMORY_TEN_WORD_REAL("MTR");
string CONTROL_REGISTER("CR");
string DEBUG_REGISTER("DR");
string TEST_REGISTER("TR");
string SPECIAL_REGISTER("SR");

class Operator
{
public:
    string opName;
    vector<string> mnemonics;

    Operator(string opName, vector<string> mnemonics)
    {
        this->opName = opName;
        this->mnemonics = mnemonics;
    }

    bool operator ==(const Operator & o) const
    {
        return opName == o.opName && mnemonics == o.mnemonics;
    }
};

class OperatorHasher 
{
public:
    size_t operator()(const Operator& o) const
    {
        return (hash<string>()(o.opName));
    }
};


unordered_map<Operator, char, OperatorHasher> createOpTab()
{
    unordered_map<Operator, char, OperatorHasher> m;

    m[{"INT", {IMMEDIATE_BYTE}}] = 0xCD;
    m[{"INT", {"3"}}] = 0xCC;

    m[{"LEA", {REGISTER_WORD, MEMORY_BYTE}}] = 0x8D;

    m[{"PUSH", {"AX"}}] = 0x50;
    m[{"PUSH", {"CX"}}] = 0x51;
    m[{"PUSH", {"DX"}}] = 0x52;
    m[{"PUSH", {"BX"}}] = 0x53;
    m[{"PUSH", {"SP"}}] = 0x54;
    m[{"PUSH", {"BP"}}] = 0x55;
    m[{"PUSH", {"SI"}}] = 0x56;
    m[{"PUSH", {"DI"}}] = 0x57;
    m[{"PUSH", {IMMEDIATE_BYTE}}] = 0x6A;
    m[{"PUSH", {IMMEDIATE_WORD}}] = 0x68;
    m[{"PUSH", {REGISTER_OR_MEMORY_WORD}}] = 0xFF;

    m[{"POP", {"AX"}}] = 0x58;
    m[{"POP", {"CX"}}] = 0x59;
    m[{"POP", {"DX"}}] = 0x5A;
    m[{"POP", {"BX"}}] = 0x5B;
    m[{"POP", {"SP"}}] = 0x5C;
    m[{"POP", {"BP"}}] = 0x5D;
    m[{"POP", {"SI"}}] = 0x5E;
    m[{"POP", {"DI"}}] = 0x5F;
    m[{"POP", {REGISTER_OR_MEMORY_WORD}}] = 0x8F;

    m[{"MOV", {"AL", REGISTER_OR_MEMORY_BYTE}}] = 0xA0;
    m[{"MOV", {"AX", REGISTER_OR_MEMORY_WORD}}] = 0xA1;
    m[{"MOV", {"AL", IMMEDIATE_BYTE}}] = 0xB0;
    m[{"MOV", {"AH", IMMEDIATE_BYTE}}] = 0xB4;
    m[{"MOV", {"AX", IMMEDIATE_WORD}}] = 0xB8;
    m[{"MOV", {"CL", IMMEDIATE_BYTE}}] = 0xB1;
    m[{"MOV", {"CH", IMMEDIATE_BYTE}}] = 0xB5;
    m[{"MOV", {"CX", IMMEDIATE_WORD}}] = 0xB9;
    m[{"MOV", {"DL", IMMEDIATE_BYTE}}] = 0xB2;
    m[{"MOV", {"DH", IMMEDIATE_BYTE}}] = 0xB6;
    m[{"MOV", {"DX", IMMEDIATE_WORD}}] = 0xBA;
    m[{"MOV", {"BL", IMMEDIATE_BYTE}}] = 0xB3;
    m[{"MOV", {"BH", IMMEDIATE_BYTE}}] = 0xB7;
    m[{"MOV", {"BX", IMMEDIATE_WORD}}] = 0xBB;
    m[{"MOV", {"SP", IMMEDIATE_WORD}}] = 0xBC;
    m[{"MOV", {"BP", IMMEDIATE_WORD}}] = 0xBD;
    m[{"MOV", {"SI", IMMEDIATE_WORD}}] = 0xBE;
    m[{"MOV", {"DI", IMMEDIATE_WORD}}] = 0xBF;

    m[{"MOV", {REGISTER_BYTE, REGISTER_OR_MEMORY_BYTE}}] = 0x8A;
    m[{"MOV", {REGISTER_OR_MEMORY_BYTE, REGISTER_BYTE}}] = 0x88;
    m[{"MOV", {REGISTER_OR_MEMORY_BYTE, "AL"}}] = 0xA2;
    m[{"MOV", {REGISTER_OR_MEMORY_WORD, "AX"}}] = 0xA3;
    m[{"MOV", {REGISTER_OR_MEMORY_BYTE, IMMEDIATE_BYTE}}] = 0xC6;
    m[{"MOV", {REGISTER_OR_MEMORY_WORD, IMMEDIATE_WORD}}] = 0xC7;
    m[{"MOV", {REGISTER_OR_MEMORY_WORD, REGISTER_WORD}}] = 0x89;
    m[{"MOV", {REGISTER_WORD, REGISTER_OR_MEMORY_WORD}}] = 0x8B;
    m[{"MOV", {REGISTER_OR_MEMORY_WORD, SPECIAL_REGISTER}}] = 0x8C;
    m[{"MOV", {SPECIAL_REGISTER, REGISTER_OR_MEMORY_WORD}}] = 0x8E;

    m[{"ADD", {"AL", IMMEDIATE_BYTE}}] = 0x04;
    m[{"ADD", {"AX", IMMEDIATE_WORD}}] = 0x05;
    m[{"ADD", {REGISTER_BYTE, REGISTER_OR_MEMORY_BYTE}}] = 0x02;
    m[{"ADD", {REGISTER_WORD, REGISTER_OR_MEMORY_WORD}}] = 0x03;
    m[{"ADD", {REGISTER_OR_MEMORY_BYTE, IMMEDIATE_BYTE}}] = 0x80;
    m[{"ADD", {REGISTER_OR_MEMORY_WORD, IMMEDIATE_WORD}}] = 0x81;
    m[{"ADD", {REGISTER_OR_MEMORY_WORD, IMMEDIATE_BYTE}}] = 0x83;
    m[{"ADD", {REGISTER_OR_MEMORY_BYTE, REGISTER_BYTE}}] = 0x00;
    m[{"ADD", {REGISTER_OR_MEMORY_WORD, REGISTER_WORD}}] = 0x01;

    // m[{"", {}}] = 0x;

    return m;
}


string getOperandType(string operand)
{
    
}