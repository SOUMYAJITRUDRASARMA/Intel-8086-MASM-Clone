#include "utility.h"
using namespace std;

int main()
{
    char label[MAX_BUFFER_SIZE], 
         opcode[MAX_BUFFER_SIZE], 
         operands[MAX_NUM_OPERANDS][MAX_BUFFER_SIZE], 
         actualLineRead[MAX_BUFFER_SIZE], 
         errorCode[MAX_BUFFER_SIZE];
    int lenOperands;
    bool sectionMarker, empty, res;
    fstream fp;
    // fp.open("A1Q1.asm", ios::in);
    fp.open("A1Q2.asm", ios::in);
    // fp.open("lol.asm", ios::in);
    // fp.open("q1.asm", ios::in);
    int index = 1;

    cout << "Starting ..." << endl << endl;

    while(!fp.eof())
    {
        res = readLineUtility(label, opcode, operands, &lenOperands, actualLineRead, &sectionMarker, fp, &empty, errorCode);

        if(empty) continue;

        cout << "Line #" << index << endl;
        cout << "actual line read: " << actualLineRead << endl;

        if(!res)
        {
            cout << "Error Code: " << errorCode << endl;
            break;
        }

        cout << "label: " << label << endl;
        cout << "opcode: " << opcode << endl;
        cout << "section marker: " << ((sectionMarker)? "true": "false") << endl;
        cout << "# operands = " << lenOperands << endl;
        cout << "Operands --> ";
        for(int i = 0; i < lenOperands; i++) cout << operands[i] << ", ";

        cout << endl << "_________________________________________________" << endl << endl;
        index++;
    }

    cout << "DONE !!!" << endl;
    fp.close();

    return 0;
}