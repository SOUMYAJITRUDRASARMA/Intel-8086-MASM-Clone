#include <bits/stdc++.h>
using namespace std;

#define MAX_NUM_OPERANDS 10
#define MAX_BUFFER_SIZE 100

typedef unsigned char uchar;

bool readLineUtility(char *label, char *opcode, char operands[][MAX_BUFFER_SIZE], int *lenOperands, char *actualLineRead, bool *sectionMarker, fstream &fp, bool *empty, char *errorCode)
{
    label[0] = opcode[0] = *lenOperands = 0;

    string str;
    getline(fp, str);
    // str.pop_back();  // Do only for linux ...
    bool emptyLine = true;
    for(char ch: str) if(ch != ' '){ emptyLine = false; break; }
    if(emptyLine){ *empty = true; return true; }
    else *empty = false;

    strcpy(actualLineRead, str.c_str());
    char token[MAX_BUFFER_SIZE];
    char ch, *ptrToken = token;
    int BEG = 0, LABEL_READ = 1, OPCODE_READ = 2;
    int curState = BEG, curOperandIndex = 0;
    *sectionMarker = false;
    for(char *ptr = actualLineRead; (ch = *ptr) != '\0'; ptr++)
    {
        if((ch == ' ' && (token[0] != '\"' || ptrToken == token)) || (ch == ' ' && token[0] == '\"' && *(ptrToken - 1) == '\"'))
        {
            if(ptrToken != token)
            {
                // Case when actually some token has been read, no consecutive spaces
                *ptrToken = '\0';
                if(token[0] == '.')
                {
                    if(curState == BEG)
                    {
                        *sectionMarker = true;
                        strcpy(opcode, token + 1);
                        curState = OPCODE_READ;
                    }
                    else{ strcpy(errorCode, "Section Marker Found but not at BEG"); return false; }
                }
                else if(*(ptrToken - 1) == ':')
                {
                    *(ptrToken - 1) = '\0';

                    if(curState != BEG){ strcpy(errorCode, "Label found but not at BEG"); cout << token << endl; return false; }  // cannot allow multiple labels in my assembler
                    if(ptrToken - 1 == token){ strcpy(errorCode, "Empty label found"); return false; }  // cannot allow empty label

                    strcpy(label, token);
                    curState = LABEL_READ;
                }
                else
                {
                    if(curState == BEG || curState == LABEL_READ)
                    {
                        strcpy(opcode, token);
                        curState = OPCODE_READ;
                    }
                    else
                    {
                        if(*(ptrToken - 1) == ',') *(ptrToken - 1) = '\0';
                        strcpy(operands[curOperandIndex++], token);
                    }
                }
            }

            ptrToken = token;
        }
        else
        {
            *ptrToken = ch;
            ptrToken++;
        }
    }

    if(ptrToken != token)
    {
        // Case when actually some token has been read, no consecutive spaces
        *ptrToken = '\0';
        if(token[0] == '.')
        {
            if(curState == BEG)
            {
                *sectionMarker = true;
                strcpy(opcode, token + 1);
                curState = OPCODE_READ;
            }
            else{ strcpy(errorCode, "Section Marker Found but not at BEG"); return false; }
        }
        else if(*(ptrToken - 1) == ':')
        {
            if(curState != BEG){ strcpy(errorCode, "Label found but not at BEG"); return false; }  // cannot allow multiple labels in my assembler
            if(ptrToken - 1 == token){ strcpy(errorCode, "Empty label found"); return false; }  // cannot allow empty label

            *(ptrToken - 1) = '\0';
            strcpy(label, token);
            curState = LABEL_READ;
        }
        else
        {
            if(curState == BEG || curState == LABEL_READ)
            {
                strcpy(opcode, token);
                curState = OPCODE_READ;
            }
            else
            {
                strcpy(operands[curOperandIndex++], token);
            }
        }
    }


    if(curState < OPCODE_READ){ strcpy(errorCode, "No Opcode found"); return false; }  // cannot allow empty label

    *lenOperands = curOperandIndex;
    return true;
}




uchar computeChecksum(vector<vector<uchar>> streamList)
{
    long sum = 0;
    for(vector<uchar> &stream: streamList)
        for(uchar ch: stream)
            sum += ch;  // sum

    while(sum >= 256) sum = sum / 256 + sum % 256;  // wrapped sum

    return (uchar)(255 - sum);  // 1'C is checksum
}




inline void appendVector(vector<uchar> &v1, vector<uchar> v2)
{
    for(uchar ch: v2) v1.push_back(ch);
}




inline vector<uchar> getBinarizedSize(int size)
{
    if(size >= 256 * 256)
    {
        cout << "Size exceeded limit: " << size << "  |  limiting ..." << endl;
        size %= 256 * 256;
    }

    return {(uchar)(size % 256), (uchar)(size / 256)};
}




class TModuleHeader
{
public:
    static uchar recordType;
    string filename;
    int headerSize;
    int filenameSize;
    uchar checksum;

    TModuleHeader(string filename)
    {
        this->filename = filename;
        this->filenameSize = filename.size();
        this->headerSize = this->filenameSize + 2;
        this->checksum = computeChecksum({{TModuleHeader::recordType}, 
                                          getBinarizedSize(headerSize), 
                                          {(uchar) filenameSize}, 
                                          {filename.begin(), filename.end()}});
    }

    vector<uchar> getBinaryRepresentation()
    {
        vector<uchar> ans;
        ans.push_back(TModuleHeader::recordType);
        appendVector(ans, getBinarizedSize(headerSize));
        ans.push_back((uchar) filenameSize);
        appendVector(ans, {filename.begin(), filename.end()});
        ans.push_back((uchar) checksum);

        return ans;
    }
};




class Lnames
{
public:
    static uchar recordType;
    int headerSize;
    int indexLnamesRecord;
    vector<string> components;
    uchar checksum;

    Lnames()
    {
        headerSize = 0x25;
        indexLnamesRecord = 0;
        components = {"DGROUP", "DATA", "CODE", "STACK", "_DATA", "_TEXT"};
        checksum = 0x8F;
    }

    vector<uchar> getBinaryRepresentation()
    {
        vector<uchar> ans;
        ans.push_back(Lnames::recordType);
        appendVector(ans, getBinarizedSize(headerSize));
        ans.push_back((uchar) indexLnamesRecord);
        for(string component: components) ans.push_back((uchar) component.size()), appendVector(ans, {component.begin(), component.end()});
        ans.push_back((uchar) checksum);

        return ans;
    }
};




class Segdef
{
public:
    static uchar recordType;
    int type;  // for CODE , DATA , STACK ...
    int headerSize;
    uchar lol1;
    int segmentSize;  // this is stack size for STACK type
    int segmentNameIndex;
    uchar lol2, lol3;
    uchar checksum;

    static int CODE, DATA, STACK;

    Segdef(int type, int segmentSize)
    {
        this->type = type;
        headerSize = 7;
        this->segmentSize = segmentSize;
        lol3 = 0x01;

        if(type == Segdef::CODE)
        {
            lol1 = 0x48;
            segmentNameIndex = 7;
            lol2 = 0x04;
        }
        else if(type == Segdef::DATA)
        {
            lol1 = 0x48;
            segmentNameIndex = 6;
            lol2 = 0x03;
        }
        else if(type == Segdef::STACK)
        {
            lol1 = 0x74;
            segmentNameIndex = 5;
            lol2 = 0x05;
        }
        else
        {
            cout << "Segdef type is not of right type ..." << endl;
        }

        checksum = computeChecksum({{Segdef::recordType}, 
                                    getBinarizedSize(headerSize)});  // lol ...
    }

    vector<uchar> getBinaryRepresentation()
    {
        vector<uchar> ans;
        ans.push_back(Segdef::recordType);
        appendVector(ans, getBinarizedSize(headerSize));
        ans.push_back((uchar) lol1);
        appendVector(ans, getBinarizedSize(segmentSize));
        ans.push_back((uchar) segmentNameIndex);
        ans.push_back((uchar) lol2), ans.push_back((uchar) lol3);
        ans.push_back((uchar) checksum);

        return ans;
    }
};




class Grpdef
{
public:
    static uchar recordType;
    int headerSize;
    vector<uchar> lol1;
    uchar checksum;

    Grpdef()
    {
        // use for DGROUP ...
        headerSize = 6;
        lol1 = {0x02 ,0xFF ,0x02 ,0xFF ,0x03};
        checksum = 0x5B;
    }

    vector<uchar> getBinaryRepresentation()
    {
        vector<uchar> ans;
        ans.push_back(Grpdef::recordType);
        appendVector(ans, getBinarizedSize(headerSize));
        appendVector(ans, lol1);
        ans.push_back((uchar) checksum);

        return ans;
    }
};




class Coment
{
public:
    static uchar recordType;
    int headerSize;
    vector<uchar> lol1;
    uchar checksum;

    Coment()
    {
        // use for _DATA ...
        headerSize = 4;
        lol1 = {0x00 ,0xA2 ,0x01};
        checksum = 0xD1;
    }

    vector<uchar> getBinaryRepresentation()
    {
        vector<uchar> ans;
        ans.push_back(Coment::recordType);
        appendVector(ans, getBinarizedSize(headerSize));
        appendVector(ans, lol1);
        ans.push_back((uchar) checksum);

        return ans;
    }
};




class LeData
{
public:
    static uchar recordType;
    int headerSize;
    uchar segmentIndex;
    int enumeratedDataOffset;
    vector<uchar> data;  // actual data ...
    uchar checksum;

    static int TEXT, CODE;

    LeData(int type)
    {
        headerSize = 4;  // initial size when empty
        if(type == LeData::TEXT) segmentIndex = 0x02;
        else if(type == LeData::CODE) segmentIndex = 0x01;
        else cout << "LeData type is not of right type ..." << endl;
        enumeratedDataOffset = 0;
        checksum = computeChecksum({{LeData::recordType}, 
                                    getBinarizedSize(headerSize)});  // lol ...
    }

    void append(vector<uchar> v)
    {
        headerSize += v.size();
        appendVector(data, v);
    }

    vector<uchar> getBinaryRepresentation()
    {
        vector<uchar> ans;
        ans.push_back(LeData::recordType);
        appendVector(ans, getBinarizedSize(headerSize));
        ans.push_back(segmentIndex);
        appendVector(ans, getBinarizedSize(enumeratedDataOffset));
        appendVector(ans, data);
        ans.push_back((uchar) checksum);

        return ans;
    }
};




class Fixupp
{
public:
    static uchar recordType;
    int headerSize;
    vector<uchar> fixuppData;  // All records combined together
    uchar checksum;

    Fixupp()
    {
        headerSize = 1;  // Initially when no internal record is present (for checksum) ...
        checksum = 0;
    }

    // TYPES OF RECORDS -->
    // 1. Special @DATA pointer
    // 2. From DATA Section (2 cases -> 1st data (0 displacement) or others)
    // 3. From CODE Section (JMP / CALL -> same structure)

    void addDataPointerRecord(int locUpdate)
    {
        // FIXUPP.LOCATE
        int headerVersion = 0xC8 / 4;  // 6 bits
        int locationToUpdate = locUpdate;  // 10 bits

        // FIXUPP.FIXDAT
        uchar fixdat = 0x15;  // 8 bits

        int targetDataM = 0x0101;  // 16 bits (CODE -> 01 01 (0x0101)  |  DATA -> 01 02 (0x0201))
        // int targetDisplacement = targetDisp;  // 16 bits (optional) (where to find with what to update)


        // ADDING ______________________________________________________________________
        // FIXUPP.LOCATE
        fixuppData.push_back((uchar)(headerVersion * 4 + locationToUpdate / 256));
        fixuppData.push_back((uchar)(locationToUpdate % 256));

        // FIXUPP.FIXDAT
        fixuppData.push_back(fixdat);

        // DATAM and TargetDisplacement
        appendVector(fixuppData, getBinarizedSize(targetDataM));
        // appendVector(fixuppData, getBinarizedSize(targetDisplacement));
    }

    void addDataRecord(int locUpdate, int targetDisp)
    {
        // FIXUPP.LOCATE
        int headerVersion = 0xC4 / 4;  // 6 bits
        int locationToUpdate = locUpdate;  // 10 bits

        // FIXUPP.FIXDAT
        uchar fixdat = (targetDisp)? 0x10: 0x14;  // 8 bits

        int targetDataM = 0x0201;  // 16 bits (CODE -> 01 01 (0x0101)  |  DATA -> 01 02 (0x0201))
        int targetDisplacement = targetDisp;  // 16 bits (optional) (where to find with what to update)


        // ADDING ______________________________________________________________________
        // FIXUPP.LOCATE
        fixuppData.push_back((uchar)(headerVersion * 4 + locationToUpdate / 256));
        fixuppData.push_back((uchar)(locationToUpdate % 256));

        // FIXUPP.FIXDAT
        fixuppData.push_back(fixdat);

        // DATAM and TargetDisplacement
        appendVector(fixuppData, getBinarizedSize(targetDataM));
        if(targetDisplacement) appendVector(fixuppData, getBinarizedSize(targetDisplacement));
    }

    void addCodeRecord(int locUpdate, int targetDisp)
    {
        // FIXUPP.LOCATE
        int headerVersion = 0x84 / 4;  // 6 bits
        int locationToUpdate = locUpdate;  // 10 bits

        // FIXUPP.FIXDAT
        uchar fixdat = 0x00;  // 8 bits

        int targetDataM = 0x0101;  // 16 bits (CODE -> 01 01 (0x0101)  |  DATA -> 01 02 (0x0201))
        int targetDisplacement = targetDisp;  // 16 bits (optional) (where to find with what to update)


        // ADDING ______________________________________________________________________
        // FIXUPP.LOCATE
        fixuppData.push_back((uchar)(headerVersion * 4 + locationToUpdate / 256));
        fixuppData.push_back((uchar)(locationToUpdate % 256));

        // FIXUPP.FIXDAT
        fixuppData.push_back(fixdat);

        // DATAM and TargetDisplacement
        appendVector(fixuppData, getBinarizedSize(targetDataM));
        appendVector(fixuppData, getBinarizedSize(targetDisplacement));
    }

    /* void add_Record(int locUpdate, int targetDisp)
    {
        // FIXUPP.LOCATE
        int headerVersion;  // 6 bits
        int locationToUpdate = locUpdate;  // 10 bits

        // FIXUPP.FIXDAT
        uchar fixdat;  // 8 bits

        int targetDataM;  // 16 bits (CODE -> 01 01 (0x0101)  |  DATA -> 01 02 (0x0201))
        int targetDisplacement;  // 16 bits (optional) (where to find with what to update)


        // ADDING ______________________________________________________________________
        // FIXUPP.LOCATE
        fixuppData.push_back((uchar)(headerVersion * 4 + locationToUpdate / 256));
        fixuppData.push_back((uchar)(locationToUpdate % 256));

        // FIXUPP.FIXDAT
        fixuppData.push_back(fixdat);

        // DATAM and TargetDisplacement
        appendVector(fixuppData, getBinarizedSize(targetDataM));
        appendVector(fixuppData, getBinarizedSize(targetDisplacement));
    } */

    vector<uchar> getBinaryRepresentation()
    {
        vector<uchar> ans;
        ans.push_back(Fixupp::recordType);
        appendVector(ans, getBinarizedSize(headerSize + fixuppData.size()));
        for(uchar data: fixuppData) ans.push_back(data);
        ans.push_back((uchar) checksum);

        return ans;
    }
};




class EndRecord
{
public:
    static uchar recordType;
    vector<uchar> data;

    EndRecord()
    {
        data = {0x02, 0x00, 0x00, 0x74};
    }

    vector<uchar> getBinaryRepresentation()
    {
        vector<uchar> ans;
        ans.push_back(EndRecord::recordType);
        for(uchar e: data) ans.push_back(e);

        return ans;
    }
};




uchar TModuleHeader::recordType = 0x80;
uchar Lnames::recordType = 0x96;
uchar Segdef::recordType = 0x98;
uchar Grpdef::recordType = 0x9A;
uchar Coment::recordType = 0x88;
uchar LeData::recordType = 0xA0;
uchar Fixupp::recordType = 0x9C;
uchar EndRecord::recordType = 0x8A;


int Segdef::CODE  = 1;
int Segdef::DATA  = 2;
int Segdef::STACK = 3;


int LeData::TEXT = 1;
int LeData::CODE = 2;




int processHexOperandIntoNumber(char num[MAX_BUFFER_SIZE])
{
    int sum = 0;
    char ch;
    for(char *ptr = num; (ch = *ptr) != '\0'; ptr++)
        if(ch == 'H') break;
        else sum = sum * 16 + ((ch >= 'A')? (ch - 'A' + 10): (ch - '0'));

    return sum;
}




int processDecimalOperandIntoNumber(char num[MAX_BUFFER_SIZE])
{
    int sum = 0;
    char ch;
    for(char *ptr = num; (ch = *ptr) != '\0'; ptr++)
        sum = sum * 10 + (ch - '0');

    return sum;
}




bool isRegisterWord(char *s)
{
    if(!strcmp(s, "AX") || 
       !strcmp(s, "BX") || 
       !strcmp(s, "CX") || 
       !strcmp(s, "DX") || 
       !strcmp(s, "SI") || 
       !strcmp(s, "DI") || 
       !strcmp(s, "BP") || 
       !strcmp(s, "SP")) return true;
    else return false;
}