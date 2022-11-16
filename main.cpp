#include "utility.h"
#include "util2.h"

using namespace std;


// Allowed OPCODES --> INT , LEA , MOV , JMP (1 byte version) [not implemented yet] , CALL (always 2 byte oriented)

class TwoPassAssembler
{
public:
    string filename;
    string errorCodeMain;

    unordered_map<string, pair<vector<unsigned char>, bool>> opTab;
    unordered_map<string, pair<int, bool>> symTab;  // true means from data section ...

    TModuleHeader *tModuleHeader;
    Lnames *lnames;
    Segdef *segdefCode, *segdefData, *segdefStack;
    Grpdef *grpdefDgroup;
    Coment *coment_Data;
    LeData *leDataText, *leDataCode;
    Fixupp *fixupp;
    EndRecord *endRecord;

    TwoPassAssembler(string filename)
    {
        this->filename = filename;
        errorCodeMain = "All OK";

        opTab = createOpTab();

        tModuleHeader = new TModuleHeader(filename);
        lnames = new Lnames();

        // Need first pass for these segment sizes ...
        segdefCode = nullptr;
        segdefData = nullptr;
        segdefStack = nullptr;

        grpdefDgroup = new Grpdef();
        coment_Data = new Coment();

        // Need first pass for these ...
        leDataText = nullptr;
        leDataCode = nullptr;

        fixupp = new Fixupp();
        endRecord = new EndRecord();
    }

    ~TwoPassAssembler()
    {
        if(tModuleHeader) delete tModuleHeader;
        if(lnames) delete lnames;
        if(segdefCode) delete segdefCode;
        if(segdefData) delete segdefData;
        if(segdefStack) delete segdefStack;
        if(grpdefDgroup) delete grpdefDgroup;
        if(coment_Data) delete coment_Data;
        if(leDataText) delete leDataText;
        if(leDataCode) delete leDataCode;
        if(fixupp) delete fixupp;
    }

    bool firstPass(bool printDebug = false)
    {
        // Initializing LeData modules as they will be needed later ...
        // cout << "lol1" << "{ ";
        leDataText = new LeData(LeData::TEXT);
        // cout << "lol2" << " }" << endl;


        int DATA = 1, 
            CODE = 2, 
            OTHERS = 3;
            
        int curState = OTHERS, locctr = 0, locctrData = 0;


        // File parsing data ...
        char label[MAX_BUFFER_SIZE], 
             opcode[MAX_BUFFER_SIZE], 
             operands[MAX_NUM_OPERANDS][MAX_BUFFER_SIZE], 
             actualLineRead[MAX_BUFFER_SIZE], 
             errorCode[MAX_BUFFER_SIZE];
        int lenOperands, index = 1;
        bool sectionMarker, empty, res;
        fstream fp;
        fp.open(filename, ios::in);


        // Reading file ...
        while(!fp.eof())
        {
            res = readLineUtility(label, opcode, operands, &lenOperands, actualLineRead, &sectionMarker, fp, &empty, errorCode);

            if(empty) continue;  // empty line ...

            if(!res)
            {
                cout << "(Parsing) Error Code: " << errorCode << endl;
                fp.close();
                return false;
            }

            if(printDebug)
            {
                cout << "Line #" << index << endl;
                cout << "actual line read: " << actualLineRead << endl;
                cout << "curState: " << ((curState == DATA)? "DATA": (curState == CODE)? "CODE": "OTHERS") << endl << endl;

                cout << "label: " << label << endl;
                cout << "opcode: " << opcode << endl;
                cout << "section marker: " << ((sectionMarker)? "true": "false") << endl;
                cout << "# operands = " << lenOperands << endl;
                cout << "Operands --> ";
                for(int i = 0; i < lenOperands; i++) cout << operands[i] << ", ";

                cout << endl << "_________________________________________________" << endl << endl;
                index++;
            }


            // Logic for processing ...
            if(sectionMarker)
            {
                if(!strcmp(opcode, "MODEL"))
                {
                    curState = OTHERS;
                    // ekhane ki hobe ???
                }
                else if(!strcmp(opcode, "STACK"))
                {
                    curState = OTHERS;
                    if(segdefStack)
                    {
                        errorCodeMain = "Multiple defination of stack segments found ...";
                        fp.close();
                        return false;
                    }
                    else segdefStack = new Segdef(Segdef::STACK, processHexOperandIntoNumber(operands[0]));
                }
                else if(!strcmp(opcode, "DATA"))
                {
                    curState = DATA;
                }
                else if(!strcmp(opcode, "CODE"))
                {
                    curState = CODE;
                }
                else
                {
                    errorCodeMain = "Invalid segment type found ...";
                    fp.close();
                    return false;
                }
            }
            else if(!strcmp(opcode, "END")) break;
            else
            {
                if(curState == DATA)
                {
                    if(lenOperands < 2 || strcmp(operands[0], "DB"))
                    {
                        errorCodeMain = "Data define format not correct (note: only DB can be used) ...";
                        fp.close();
                        return false;
                    }

                    vector<uchar> curDataline;
                    for(int i = 1; i < lenOperands; i++)
                    {
                        if(operands[i][0] == '\"')
                        {
                            // String data
                            char ch;
                            for(char *ptr = operands[i] + 1; (ch = *ptr) != '\"'; ptr++) curDataline.push_back((uchar) ch);
                        }
                        else
                        {
                            // Numeric Data
                            curDataline.push_back((uchar) processDecimalOperandIntoNumber(operands[i]));  // decimal in our program ...
                        }
                    }

                    leDataText->append(curDataline);
                    // cout << opcode << " --> " << curDataline.size() << endl;

                    // Logic for SYMTAB ...
                    if(symTab.find(string(opcode)) == symTab.end()) symTab[string(opcode)] = {locctrData, true};
                    else
                    {
                        errorCodeMain = "Multiple defination of symbol in SYMTAB (found in DATA section) ...";
                        fp.close();
                        return false;
                    }
                    locctrData += curDataline.size();
                }
                else if(curState == CODE)
                {
                    if(strlen(label))
                    {
                        // Label is present ...
                        if(symTab.find(string(label)) == symTab.end()) symTab[string(label)] = {locctr, false};
                        else
                        {
                            errorCodeMain = "Multiple defination of symbol in SYMTAB (found in CODE section) ...";
                            fp.close();
                            return false;
                        }
                    }

                    // Not processing code part yet -> done in 2nd pass -> after we have our symtab
                    // eg :- JMP label (maybe in 1 byte (signed hexadecimal relative dispalcement) /OR/ in 2 bytes (needs fixupp)) )
                    if(!strcmp(opcode, "INT"))
                    {
                        locctr += 2;
                    }
                    else if(!strcmp(opcode, "LEA"))
                    {
                        locctr += 4;
                    }
                    else if(!strcmp(opcode, "MOV"))
                    {
                        if((!strcmp(operands[0], "AX")) && (!strcmp(operands[1], "@DATA")))
                        {
                            locctr += 3;
                        }
                        else if(operands[1][0] >= '0' && operands[1][0] <= '9')
                        {
                            // Immediate value is numeric
                            if(isRegisterWord(operands[0])) locctr += 3;
                            else locctr += 2;
                        }
                        else
                        {
                            // Immediate value is register
                            locctr += 2;
                        }
                    }
                    else if(!strcmp(opcode, "JMP"))
                    {
                        // With our 2 pass construct , determining relative distance here is impossible
                        // Need 1 pass -> ll of labels -> then resolving closest inwards to farthest logic ...
                        // Soln -> Add NOP to compensate ...
                        locctr += 3;
                    }
                    else if(!strcmp(opcode, "CALL"))
                    {
                        locctr += 3;
                    }
                    else if(!strcmp(opcode, "RET"))
                    {
                        locctr += 1;
                    }
                    else
                    {
                        errorCodeMain = "Invalid opcode ...";
                        fp.close();
                        return false;
                    }
                }
                else
                {
                    errorCodeMain = "Normal code found without DATA or CODE section defined ...";
                    fp.close();
                    return false;
                }
            }
        }

        fp.close();

        // Initializing Segdef records ...
        segdefData = new Segdef(Segdef::DATA, leDataText->headerSize - 4);

        return true;
    }

    bool secondPass(bool printDebug = false)
    {
        // Initializing LeData modules as they will be needed later ...
        // cout << "lol1" << "{ ";
        leDataCode = new LeData(LeData::CODE);
        // cout << "lol2" << " }" << endl;


        int DATA = 1, 
            CODE = 2, 
            OTHERS = 3;
            
        int curState = OTHERS, locctr = 0, locctrData = 0;


        // File parsing data ...
        char label[MAX_BUFFER_SIZE], 
             opcode[MAX_BUFFER_SIZE], 
             operands[MAX_NUM_OPERANDS][MAX_BUFFER_SIZE], 
             actualLineRead[MAX_BUFFER_SIZE], 
             errorCode[MAX_BUFFER_SIZE];
        int lenOperands, index = 1;
        bool sectionMarker, empty, res;
        fstream fp;
        fp.open(filename, ios::in);


        // Reading file ...
        while(!fp.eof())
        {
            res = readLineUtility(label, opcode, operands, &lenOperands, actualLineRead, &sectionMarker, fp, &empty, errorCode);

            if(empty) continue;  // empty line ...

            if(!res)
            {
                cout << "(Parsing) Error Code: " << errorCode << endl;
                fp.close();
                return false;
            }

            if(printDebug)
            {
                cout << "Line #" << index << endl;
                cout << "actual line read: " << actualLineRead << endl;
                cout << "curState: " << ((curState == DATA)? "DATA": (curState == CODE)? "CODE": "OTHERS") << endl << endl;

                cout << "label: " << label << endl;
                cout << "opcode: " << opcode << endl;
                cout << "section marker: " << ((sectionMarker)? "true": "false") << endl;
                cout << "# operands = " << lenOperands << endl;
                cout << "Operands --> ";
                for(int i = 0; i < lenOperands; i++) cout << operands[i] << ", ";

                cout << endl << "_________________________________________________" << endl << endl;
                index++;
            }


            // Logic for processing ...
            if(sectionMarker)
            {
                if(!strcmp(opcode, "MODEL"))
                {
                    curState = OTHERS;
                    // processed earlier in first pass ...
                }
                else if(!strcmp(opcode, "STACK"))
                {
                    curState = OTHERS;
                    // processed earlier in first pass ...
                }
                else if(!strcmp(opcode, "DATA"))
                {
                    curState = DATA;
                }
                else if(!strcmp(opcode, "CODE"))
                {
                    curState = CODE;
                }
                else
                {
                    errorCodeMain = "Invalid segment type found ...";
                    fp.close();
                    return false;
                }
            }
            else if(!strcmp(opcode, "END")) break;
            else
            {
                if(curState == DATA)
                {
                    if(lenOperands < 2 || strcmp(operands[0], "DB"))
                    {
                        errorCodeMain = "Data define format not correct (note: only DB can be used) ...";
                        fp.close();
                        return false;
                    }

                    // processed earlier in first pass ...
                }
                else if(curState == CODE)
                {
                    if(strlen(label))
                    {
                        // processed earlier in first pass ...
                    }

                    if(!strcmp(opcode, "INT"))
                    {
                        leDataCode->append( { 0xCD, ((uchar) processHexOperandIntoNumber(operands[0])) } );
                        locctr += 2;
                    }
                    else if(!strcmp(opcode, "LEA"))
                    {
                        leDataCode->append( opTab["LEA " + string(operands[0])].first );
                        leDataCode->append( { 0x00, 0x00 } );
                        if(symTab.find(string(operands[1])) == symTab.end())
                        {
                            // From code section -> labels ...
                            errorCodeMain = "Symbol not found in SYMTAB to LEA ...";
                            fp.close();
                            return false;
                        }
                        if(!symTab[string(operands[1])].second)
                        {
                            // From code section -> labels ...
                            errorCodeMain = "Cannot LEA labels from Code Section ...";
                            fp.close();
                            return false;
                        }

                        fixupp->addDataRecord(locctr + 2, symTab[string(operands[1])].first);
                        locctr += 4;
                    }
                    else if(!strcmp(opcode, "MOV"))
                    {
                        if((!strcmp(operands[0], "AX")) && (!strcmp(operands[1], "@DATA")))
                        {
                            leDataCode->append( { 0xB8, 0x00, 0x00 } );
                            fixupp->addDataPointerRecord(locctr + 1);
                            locctr += 3;
                        }
                        else if(operands[1][0] >= '0' && operands[1][0] <= '9')
                        {
                            // Immediate value is numeric
                            leDataCode->append( opTab["MOV " + string(operands[0])].first );
                            if(isRegisterWord(operands[0])) leDataCode->append(getBinarizedSize(processHexOperandIntoNumber(operands[1]))), locctr += 3;
                            else leDataCode->append( { (uchar) processHexOperandIntoNumber(operands[1]) } ), locctr += 2;
                        }
                        else
                        {
                            // Immediate value is register
                            leDataCode->append( opTab["MOV " + string(operands[0]) + string(" ") + string(operands[1])].first );
                            locctr += 2;
                        }
                    }
                    else if(!strcmp(opcode, "JMP"))
                    {
                        if(symTab.find(string(operands[0])) == symTab.end())
                        {
                            // From code section -> labels ...
                            errorCodeMain = "Symbol not found in SYMTAB to JMP ...";
                            fp.close();
                            return false;
                        }
                        if(symTab[string(operands[0])].second)
                        {
                            // From data section -> variables ...
                            errorCodeMain = "Cannot JMP variables from Data Section ...";
                            fp.close();
                            return false;
                        }

                        int relativeDisplacement = symTab[string(operands[0])].first - locctr - 2;  // EB dd NOP
                        if(relativeDisplacement >= -128 && relativeDisplacement <= 127)
                        {
                            // Relative Signed Hexadecimal Displacement (1 byte) ...
                            leDataCode->append( {0xEB} );
                            if(relativeDisplacement < 0) relativeDisplacement = 256 + relativeDisplacement;
                            leDataCode->append( { (uchar) (relativeDisplacement), 0x90 } );  // Adding extra NOP to compensate
                            locctr += 3;
                        }
                        else
                        {
                            // Absolute Displacement (2 bytes) -> needs fixupp ...
                            leDataCode->append( {0xE9} );
                            leDataCode->append( { 0x00, 0x00 } );
                            fixupp->addCodeRecord(locctr + 1, symTab[string(operands[0])].first);
                            locctr += 3;
                        }
                    }
                    else if(!strcmp(opcode, "CALL"))
                    {
                        leDataCode->append( {0xE8} );
                        leDataCode->append( { 0x00, 0x00 } );
                        if(symTab.find(string(operands[0])) == symTab.end())
                        {
                            // From code section -> labels ...
                            errorCodeMain = "Symbol not found in SYMTAB to CALL ...";
                            fp.close();
                            return false;
                        }
                        if(symTab[string(operands[0])].second)
                        {
                            // From data section -> variables ...
                            errorCodeMain = "Cannot CALL variables from Data Section ...";
                            fp.close();
                            return false;
                        }

                        fixupp->addCodeRecord(locctr + 1, symTab[string(operands[0])].first);
                        locctr += 3;
                    }
                    else if(!strcmp(opcode, "RET"))
                    {
                        leDataCode->append( {0xC3} );
                        locctr += 1;
                    }
                    else
                    {
                        errorCodeMain = "Invalid opcode ...";
                        fp.close();
                        return false;
                    }
                }
                else
                {
                    errorCodeMain = "Normal code found without DATA or CODE section defined ...";
                    fp.close();
                    return false;
                }
            }
        }

        fp.close();

        // Initializing Segdef records ...
        segdefCode = new Segdef(Segdef::CODE, leDataCode->headerSize - 4);

        return true;
    }
};

int main()
{
    string filename;
    cout << "Enter filename to assemble: ";
    cin >> filename;

    cout << endl;

    // Testing if file exists ...
    ifstream in(filename);
    if(!in)
    {
        cout << "Error, File does not exist !!!";
        return 0;
    }
    else in.close();

    TwoPassAssembler tpa(filename);

    if(!tpa.firstPass())
    {
        cout << "Error Code: " << tpa.errorCodeMain << endl;
        return 0;
    }
    cout << "First Pass Done !!!" << endl;

    cout << "\n\nSYMTAB -->" << endl << endl;
    for(pair<string, pair<int, bool>> p: tpa.symTab)
    cout << p.first << " -> " << p.second.first << " | " << p.second.second << endl;
    cout << endl << endl;

    if(!tpa.secondPass())
    {
        cout << "Error Code: " << tpa.errorCodeMain << endl;
        return 0;
    }
    cout << "Second Pass Done !!!" << endl;





    // Testing
    fstream fp;
    fp.open("lol.obj", ios::out | ios::trunc | ios::binary);
    for(char ch: tpa.tModuleHeader->getBinaryRepresentation()) fp << ch;
    for(char ch: tpa.lnames->getBinaryRepresentation()) fp << ch;
    for(char ch: tpa.segdefCode->getBinaryRepresentation()) fp << ch;
    for(char ch: tpa.segdefData->getBinaryRepresentation()) fp << ch;
    for(char ch: tpa.segdefStack->getBinaryRepresentation()) fp << ch;
    for(char ch: tpa.grpdefDgroup->getBinaryRepresentation()) fp << ch;
    for(char ch: tpa.coment_Data->getBinaryRepresentation()) fp << ch;
    for(char ch: tpa.leDataText->getBinaryRepresentation()) fp << ch;
    for(char ch: tpa.leDataCode->getBinaryRepresentation()) fp << ch;
    for(char ch: tpa.fixupp->getBinaryRepresentation()) fp << ch;
    for(char ch: tpa.endRecord->getBinaryRepresentation()) fp << ch;
    fp.close();

    cout << "All Done !!!" << endl;








    /* // // Another Testing ...
    // char label[MAX_BUFFER_SIZE], 
    //      opcode[MAX_BUFFER_SIZE], 
    //      operands[MAX_NUM_OPERANDS][MAX_BUFFER_SIZE], 
    //      actualLineRead[MAX_BUFFER_SIZE], 
    //      errorCode[MAX_BUFFER_SIZE];
    // int lenOperands;
    // bool sectionMarker, empty, res;
    // fstream fp;
    // // fp.open("A1Q1.asm", ios::in);
    // fp.open("lol.asm", ios::in);
    // // fp.open("q1.asm", ios::in);
    // int index = 1;

    // cout << "Starting ..." << endl << endl;

    // while(!fp.eof())
    // {
    //     res = readLineUtility(label, opcode, operands, &lenOperands, actualLineRead, &sectionMarker, fp, &empty, errorCode);

    //     if(empty) continue;

    //     cout << "Line #" << index << endl;
    //     cout << "actual line read: " << actualLineRead << endl;

    //     if(!res)
    //     {
    //         cout << "Error Code: " << errorCode << endl;
    //         break;
    //     }

    //     cout << "label: " << label << endl;
    //     cout << "opcode: " << opcode << endl;
    //     cout << "section marker: " << ((sectionMarker)? "true": "false") << endl;
    //     cout << "# operands = " << lenOperands << endl;
    //     cout << "Operands --> ";
    //     for(int i = 0; i < lenOperands; i++) cout << operands[i] << ", ";

    //     cout << endl << "_________________________________________________" << endl << endl;
    //     index++;
    // }

    // cout << "DONE !!!" << endl;
    // fp.close(); */

    return 0;
}