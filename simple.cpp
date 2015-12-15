#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <fstream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <map>

using namespace std;

int currentLine = 1;
int baseAddress = 0x00;

struct instructionMemory
{
	string insName;
	string arg1;
	string arg2;
	string arg3;
};

struct pipelineStage
{
    string insName;
    int arg1;
    int arg2;
    int arg3;
};

struct stringMetadata
{
	string line;
	int lineNo;
};

class pipeline
{
    public:
        int length;
        instructionMemory fetchStage;
        instructionMemory decodeStagePart1;
        pipelineStage     decodeStagePart2;
        pipelineStage     executeStagePart1;
        pipelineStage     executeStagePart2;
        pipelineStage     wbStage;

        pipeline()
        {
            length = 4;
        }

        void advance()
        {
            wbStage.insName = executeStagePart2.insName;
            wbStage.arg1 = executeStagePart2.arg1;
            wbStage.arg2 = executeStagePart2.arg2;
            wbStage.arg3 = executeStagePart2.arg3;

            executeStagePart1.insName = decodeStagePart2.insName;
            executeStagePart1.arg1    = decodeStagePart2.arg1;
            executeStagePart1.arg2    = decodeStagePart2.arg2;
            executeStagePart1.arg3    = decodeStagePart2.arg3;

            decodeStagePart1.insName = fetchStage.insName;
            decodeStagePart1.arg1    = fetchStage.arg1;
            decodeStagePart1.arg2    = fetchStage.arg2;
            decodeStagePart1.arg3    = fetchStage.arg3;
        }

        void flush()
        {
            executeStagePart1.insName     = "NOP";
            executeStagePart2.insName     = "NOP";
            decodeStagePart1.insName      = "NOP";
            decodeStagePart2.insName      = "NOP";
            fetchStage.insName            = "NOP";
        }
};

class regFile
{
	public:

		int pc;
		vector<int> lr;

        regFile(int regCount)
        {
            regBank = new int[regCount];
            lr.push_back(0);
            defRegCount = regCount;
            pc = baseAddress;
        }

        regFile()
        {
            defRegCount = 16;
            regBank = new int[defRegCount];
            lr.push_back(0);
            pc = baseAddress;
        }

        void store(int regNum, int value)
        {
            regBank[regNum] = value;
        }

        int load(int regNum)
        {
            
            return regBank[regNum];
        }

        void clear()
        {
            for (int i = 0; i< defRegCount; i++)
            {
                regBank[i] = 0;
            }

            lr.clear();
            lr.push_back(0);
            pc = baseAddress;   
        }

        ~regFile()
        {
            delete regBank;
        }

    private:
        int* regBank;
        int defRegCount;
};

class fileReader
{
	public:
		fileReader(char* s) : i((s)) { currentTokenPos = currentLinePos = 0;}

		void getContents()
		{
			while (i.peek() != EOF)
			{
				getline(i,line);
				stringstream linebuf(line);
				stringMetadata lineStruct;
				lineStruct.line = line;
				lineStruct.lineNo = currentLine;
				lineList.push_back(lineStruct);

				while (linebuf.good())
				{
					stringMetadata wordStruct;
					getline(linebuf,word,' ');

					if (word != "")
					{			
						wordStruct.line = word;
						wordStruct.lineNo = currentLine;
						tokenList.push_back(wordStruct);
					}
				}
				
				currentLine++;
			}
			i.close();

		}

		string getNextToken()
		{
			string gettableWord;
			gettableWord = tokenList[currentTokenPos].line;
			currentTokenPos++;
			return gettableWord;
		}

		string getNextLine()
		{
			string gettableLine;
			gettableLine = lineList[currentLinePos].line;
			currentLinePos++;
			return gettableLine;
		}

		string getPrevLine()
		{
			currentLinePos--;
			string gettableLine;
			gettableLine = lineList[currentLinePos].line;
			return gettableLine;
		}

		string getPrevToken()
		{
			currentTokenPos--;
			string gettableWord;
			gettableWord = tokenList[currentTokenPos].line;
			return gettableWord;
		}

		string peekNextLine()
		{
			string gettableLine;
			gettableLine = lineList[currentLinePos].line;
			return gettableLine;
		}

		string peekNextToken()
		{
			string gettableWord;
			gettableWord = tokenList[currentTokenPos].line;
			return gettableWord;
		}

		int getLineNo()
		{
			int lineNo;
			lineNo = tokenList[currentTokenPos].lineNo;
			return lineNo;
		}


		bool hasMoreTokens()
		{
			if (currentTokenPos < static_cast<int>(tokenList.size())) return true;
			else return false;
		}
		
		bool hasMoreLines()
		{
			if (currentLinePos <  static_cast<int>(lineList.size())) return true;
			else return false;
		}

	private:
		ifstream i;
		vector<stringMetadata> lineList;
		vector<stringMetadata> tokenList;
		string word,line;
		int currentTokenPos;
		int currentLinePos;
};

void usage()
{
	cout << endl << "Usage: ./simple [inputfile]" << endl;
	cout << endl << "No input arguments specified!" << endl << endl;
	exit(1);
}

void fillInstructionMemory(fileReader &f, map<int,instructionMemory> &insMem)
{
	int address = baseAddress;
	while (f.hasMoreTokens())
	{
		string word = f.getNextToken();
		// cout << word << endl;

		if (word == "ADDI" || word == "ADDR" || word == "SUBI" || word == "SUBR" || word == "CMP" || word == "LOADR")
		{
			insMem[address].insName = word;
			word = f.getNextToken();
			insMem[address].arg1 = word;
			word = f.getNextToken();
			insMem[address].arg2 = word;
			word = f.getNextToken();
			insMem[address].arg3 = word;
		}

		else if (word == "AND" || word == "OR" || word == "XOR" || word == "SHIFTLA" || word == "SHIFTLL" || word == "SHIFTRA" || word == "SHIFTRL" || word == "STOREI")
		{
			insMem[address].insName = word;
			word = f.getNextToken();
			insMem[address].arg1 = word;
			word = f.getNextToken();
			insMem[address].arg2 = word;
			word = f.getNextToken();
			insMem[address].arg3 = word;
		}

		else if (word == "MULI" || word == "MULR" || word == "XOR" || word == "SHIFTLA" || word == "SHIFTLL" || word == "SHIFTRA" || word == "SHIFTRL" || word == "STORER")
		{
			insMem[address].insName = word;
			word = f.getNextToken();
			insMem[address].arg1 = word;
			word = f.getNextToken();
			insMem[address].arg2 = word;
			word = f.getNextToken();
			insMem[address].arg3 = word;
		}

		else if (word == "BEQ" || word == "MOVI" || word == "LOADI" || word == "NOT")
		{
			insMem[address].insName = word;
			word = f.getNextToken();
			insMem[address].arg1 = word;
			word = f.getNextToken();
			insMem[address].arg2 = word;
		}

		else if (word == "JUMP" || word == "CALL")
		{
			insMem[address].insName = word;
			word = f.getNextToken();
			insMem[address].arg1 = word;
            cout << word << endl;
		}

		else if (word == "NOP" || word == "STOP" || word == "RETURN")
		{
			insMem[address].insName = word;
		}

		address += 4;
	}
}

void regs(regFile &r, int regCount)
{
	int num;
	cout << "------------Register Contents------------" << endl;

	for (num = 0; num < regCount; num++)
	{
		if (num < 10) cout << "r" << num << "     == " << r.load(num) << endl;
		else cout << "r" << num << "    == " << r.load(num) << endl;
	}

	cout << "pc" << "     == " << r.pc << endl;
	cout << "lr(top)" << "== " << r.lr.back() << endl;

	cout << "------------------////-------------------" << endl;
}

void fetch(std::map<int,instructionMemory> insMem, regFile &r, pipeline &p)
{
    p.fetchStage.insName = insMem[r.pc].insName;
    p.fetchStage.arg1 = insMem[r.pc].arg1;
    p.fetchStage.arg2 = insMem[r.pc].arg2;
    p.fetchStage.arg3 = insMem[r.pc].arg3;
    r.pc += 4;
}

int decode(regFile &r, pipeline &p)
{
    p.decodeStagePart2.insName = p.decodeStagePart1.insName;
    p.decodeStagePart2.arg1 = 0;
    p.decodeStagePart2.arg2 = 0;
    p.decodeStagePart2.arg3 = 0;  
    string instructionName = p.decodeStagePart2.insName;

    //----------------Arithmetic Instructions----------------//

    if (instructionName == "MOVI" || instructionName == "LOADI")
    {
        string word = p.decodeStagePart1.arg1;
        word.erase(0, 1);
        p.decodeStagePart2.arg1 = atoi(word.c_str());

        string imm = p.decodeStagePart1.arg2;
        p.decodeStagePart2.arg2 = atoi(imm.c_str());
    }

    else if (instructionName == "ADDI" || instructionName == "SUBI" || instructionName == "MULI")
    {
        string word1 = p.decodeStagePart1.arg1;
        word1.erase(0, 1);
        p.decodeStagePart2.arg1 = atoi(word1.c_str());
        string word2 = p.decodeStagePart1.arg2;
        word2.erase(0, 1);
        p.decodeStagePart2.arg2 = r.load(atoi(word2.c_str()));
        string imm = p.decodeStagePart1.arg3;
        p.decodeStagePart2.arg3 = atoi(imm.c_str());  
    }

    else if (instructionName == "ADDR" || instructionName == "SUBR" || instructionName == "MULR" || instructionName == "AND"  \
             || instructionName == "OR" || instructionName == "XOR" || instructionName == "CMP" )
    {
        string word1 = p.decodeStagePart1.arg1;
        word1.erase(0, 1);
        p.decodeStagePart2.arg1 = atoi(word1.c_str());

        string word2 = p.decodeStagePart1.arg2;
        word2.erase(0, 1);
        p.decodeStagePart2.arg2  = r.load(atoi(word2.c_str()));

        string word3 = p.decodeStagePart1.arg3;
        word3.erase(0, 1);
        p.decodeStagePart2.arg3 = r.load(atoi(word3.c_str()));
        
    }


    //----------------Logical Operators----------------//


    else if (instructionName == "NOT")
    {
        string word = p.decodeStagePart1.arg1;
        word.erase(0, 1);
        p.decodeStagePart2.arg1 = atoi(word.c_str());

        string imm = p.decodeStagePart1.arg2;
        p.decodeStagePart2.arg2 = r.load(atoi(imm.c_str()));
        
    }

    else if (instructionName == "SHIFTLL" || instructionName == "SHIFTLA" || instructionName == "SHIFTRL" \
              || instructionName == "SHIFTRA")
    {
        string word1 = p.decodeStagePart1.arg1;
        word1.erase(0, 1);
        p.decodeStagePart2.arg1 = atoi(word1.c_str());
        string word2 = p.decodeStagePart1.arg2;
        word2.erase(0, 1);
        p.decodeStagePart2.arg2 = r.load(atoi(word2.c_str()));
        string imm = p.decodeStagePart1.arg3;
        p.decodeStagePart2.arg3 = atoi(imm.c_str());      
    }

    //----------------LOAD/STORE Instructions----------------//

    else if (instructionName == "STOREI")
    {
        string word1 = p.decodeStagePart1.arg1;
        word1.erase(0, 1);
        p.decodeStagePart2.arg1 = r.load(atoi(word1.c_str()));
        string word2 = p.decodeStagePart1.arg2;
        p.decodeStagePart2.arg2 = atoi(word2.c_str());
        string imm = p.decodeStagePart1.arg3;
        p.decodeStagePart2.arg3 = atoi(imm.c_str());  
    }

    else if (instructionName == "STORER")
    {
        string word1 = p.decodeStagePart1.arg1;
        word1.erase(0, 1);
        p.decodeStagePart2.arg1 = r.load(atoi(word1.c_str()));

        string word2 = p.decodeStagePart1.arg2;
        word2.erase(0, 1);
        p.decodeStagePart2.arg2 = r.load(atoi(word2.c_str()));
        string imm = p.decodeStagePart1.arg3;
        imm.erase(0, 1);
        p.decodeStagePart2.arg3 = r.load(atoi(imm.c_str()));
    }

    else if (instructionName == "LOADR")
    {
        string word1 = p.decodeStagePart1.arg1;
        word1.erase(0, 1);
        p.decodeStagePart2.arg1 = atoi(word1.c_str());
        string word2 = p.decodeStagePart1.arg2;
        word2.erase(0, 1);
        p.decodeStagePart2.arg2 = r.load(atoi(word2.c_str()));
        string imm = p.decodeStagePart1.arg3;
        cout << "japoigjadog: " << p.decodeStagePart1.arg3 << endl;
        p.decodeStagePart2.arg3 = atoi(imm.c_str());
    }

    //---------------Control Flow Instructions-----------//

    else if (instructionName == "BEQ")
    {
        string word = p.decodeStagePart1.arg1;
        word.erase(0,1);
        int reg = atoi(word.c_str());
        p.decodeStagePart2.arg1 = r.load(reg);
        p.decodeStagePart2.arg2 = atoi(p.decodeStagePart1.arg2.c_str());   
    }

    else if (instructionName == "CALL")
    { 
        p.decodeStagePart2.arg1 = atoi(p.decodeStagePart1.arg1.c_str()); 
    }

    else if (instructionName == "RETURN") {}

    else if (instructionName == "JUMP")
    {
        p.decodeStagePart2.arg1 = atoi(p.decodeStagePart1.arg1.c_str()); 
    }

    else if (instructionName == "NOP" || instructionName == "STOP" ) {}

    return 0;
}

int execute(std::map<int,int> &dataMem, regFile &r, pipeline &p)
{
    p.executeStagePart2.insName = p.executeStagePart1.insName;
    p.executeStagePart2.arg1 = 0;
    p.executeStagePart2.arg2 = 0;
    p.executeStagePart2.arg3 = 0;  
    string instructionName = p.executeStagePart2.insName;
    int retVal = 0;

    //----------------Arithmetic Instructions----------------//

    if (instructionName == "MOVI")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2;
    }

    else if (instructionName == "ADDI")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2 + p.executeStagePart1.arg3;
    }

    else if (instructionName == "ADDR")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2 + p.executeStagePart1.arg3;
    }

    else if (instructionName == "SUBI")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2 - p.executeStagePart1.arg3;
    }

    else if (instructionName == "SUBR")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2 - p.executeStagePart1.arg3;
    }

    else if (instructionName == "MULI")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2 * p.executeStagePart1.arg3;
    }

    else if (instructionName == "MULR")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2 * p.executeStagePart1.arg3;
    }

    //----------------Logical Operators----------------//

    else if (instructionName == "AND")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2 & p.executeStagePart1.arg3;
    }

    else if (instructionName == "OR")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2 | p.executeStagePart1.arg3;
    }

    else if (instructionName == "XOR")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2 ^ p.executeStagePart1.arg3;
    }

    else if (instructionName == "NOT")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = ~(p.executeStagePart1.arg2); 
    }

    else if (instructionName == "SHIFTLL")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = (unsigned)p.executeStagePart1.arg2 << p.executeStagePart1.arg3;
    }

    else if (instructionName == "SHIFTLA")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2 << p.executeStagePart1.arg3;
    }

    else if (instructionName == "SHIFTRL")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = (unsigned)p.executeStagePart1.arg2 >> p.executeStagePart1.arg3;
    }

    else if (instructionName == "SHIFTRA")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2 >> p.executeStagePart1.arg3;
    }

    //----------------LOAD/STORE Instructions----------------//

    else if (instructionName == "LOADR")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2 + p.executeStagePart1.arg3;
        cout << "address to load from: " << p.executeStagePart2.arg2 << endl;
        p.executeStagePart2.arg2    = dataMem[p.executeStagePart2.arg2];
        cout << "value loaded: " << p.executeStagePart2.arg2 << endl;
    }

    if (instructionName == "LOADI")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = dataMem[p.executeStagePart1.arg2];
    }

    if (instructionName == "STOREI")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2 + p.executeStagePart1.arg3;
        dataMem[p.executeStagePart2.arg1] = p.executeStagePart2.arg2;
    }

    if (instructionName == "STORER")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2 + p.executeStagePart1.arg3;
        cout << "address to store in: " << p.executeStagePart2.arg2 << endl;
        dataMem[p.executeStagePart2.arg2] = p.executeStagePart2.arg1;
        cout << "value stored: " << p.executeStagePart2.arg1 << endl;
    }

    //---------------Control Flow Instructions-----------//

    else if (instructionName == "CALL")
    {
        r.lr.push_back(r.pc);
        r.pc = p.executeStagePart1.arg2; 
        p.flush();
    }

    else if (instructionName == "RETURN")
    {
        r.pc = r.lr.back();
        r.lr.pop_back();
        p.flush();
    }

    else if (instructionName == "JUMP")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        r.pc = p.executeStagePart2.arg1;
        p.flush();
    }

    else if (instructionName == "BEQ")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        p.executeStagePart2.arg2    = p.executeStagePart1.arg2;
        if (p.executeStagePart2.arg1 == 0) 
        {
            r.pc = p.executeStagePart2.arg2;  
            p.flush(); 
        }
    }

    else if (instructionName == "CMP")
    {
        p.executeStagePart2.arg1    = p.executeStagePart1.arg1;
        
        int cmp1   = p.executeStagePart1.arg2;
        int cmp2   = p.executeStagePart1.arg3;

        if (cmp1 < cmp2)
        {
            p.executeStagePart2.arg2 = -1;
        }

        else if (cmp1 == cmp2)
        {
            p.executeStagePart2.arg2 = 0;
        }

        else
        {
            p.executeStagePart2.arg2 = 1;
        }

    }

    else if (instructionName == "STOP")
    {
        retVal = 1;
        p.flush();
    }

    else if (instructionName == "NOP") {}


    return retVal;

}

void writeback(regFile &r, pipeline &p)
{

    string instructionName = p.wbStage.insName;
    p.wbStage.arg3 = 0;
    cout << endl;

    //----------------Arithmetic Instructions----------------//

    if (instructionName == "MOVI")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    else if (instructionName == "ADDI")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    else if (instructionName == "ADDR")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    else if (instructionName == "SUBI")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    else if (instructionName == "SUBR")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    else if (instructionName == "MULI")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    else if (instructionName == "MULR")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    //----------------Logical Operators----------------//

    else if (instructionName == "AND")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    else if (instructionName == "OR")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    else if (instructionName == "XOR")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); ;
    }

    else if (instructionName == "NOT")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    else if (instructionName == "SHIFTLL")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    else if (instructionName == "SHIFTLA")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    else if (instructionName == "SHIFTRL")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    else if (instructionName == "SHIFTRA")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    //----------------LOAD/STORE Instructions----------------//

    else if (instructionName == "CMP")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    else if (instructionName == "LOADR")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }

    if (instructionName == "LOADI")
    {
        r.store(p.wbStage.arg1, p.wbStage.arg2); 
    }    
}

/*
int fdx(std::map<int,int> &dataMem, std::map<int,instructionMemory> insMem, regFile &r, pipeline &p)
{
	string instructionName = insMem[r.pc].insName;
    // cout << instructionName << endl;

	//----------------Arithmetic Instructions----------------//

	if (instructionName == "MOVI")
	{
		string word = insMem[r.pc].arg1;
		word.erase(0, 1);
		int dest = atoi(word.c_str());
		string imm = insMem[r.pc].arg2;
		int val = atoi(imm.c_str());
		r.store(dest,val);
	}

	else if (instructionName == "ADDI")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());
	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source = atoi(word2.c_str());
		string imm = insMem[r.pc].arg3;
		int val = r.load(source) + atoi(imm.c_str());
		r.store(dest,val);
	}

	else if (instructionName == "ADDR")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());

	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source1 = atoi(word2.c_str());

		string word3 = insMem[r.pc].arg3;
		word3.erase(0, 1);
		int source2 = atoi(word3.c_str());

		int val = r.load(source1) + r.load(source2);
		r.store(dest,val);
	}

	else if (instructionName == "SUBI")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());
	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source = atoi(word2.c_str());
		string imm = insMem[r.pc].arg3;
		int val = r.load(source) - atoi(imm.c_str());
		r.store(dest,val);
	}

	else if (instructionName == "SUBR")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());

	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source1 = atoi(word2.c_str());

		string word3 = insMem[r.pc].arg3;
		word3.erase(0, 1);
		int source2 = atoi(word3.c_str());

		int val = r.load(source1) - r.load(source2);
		r.store(dest,val);
	}

	else if (instructionName == "MULI")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());
	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source = atoi(word2.c_str());
		string imm = insMem[r.pc].arg3;
		int val = r.load(source) * atoi(imm.c_str());
		r.store(dest,val);
	}

	else if (instructionName == "MULR")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());

	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source1 = atoi(word2.c_str());

		string word3 = insMem[r.pc].arg3;
		word3.erase(0, 1);
		int source2 = atoi(word3.c_str());

		int val = r.load(source1) * r.load(source2);
		r.store(dest,val);
	}

	//----------------Logical Operators----------------//

	else if (instructionName == "AND")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());

	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source1 = atoi(word2.c_str());

		string word3 = insMem[r.pc].arg3;
		word3.erase(0, 1);
		int source2 = atoi(word3.c_str());

		int val = r.load(source1) & r.load(source2);
		r.store(dest,val);
	}

	else if (instructionName == "OR")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());

	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source1 = atoi(word2.c_str());

		string word3 = insMem[r.pc].arg3;
		word3.erase(0, 1);
		int source2 = atoi(word3.c_str());

		int val = r.load(source1) | r.load(source2);
		r.store(dest,val);
	}

    else if (instructionName == "XOR")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());

	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source1 = atoi(word2.c_str());

		string word3 = insMem[r.pc].arg3;
		word3.erase(0, 1);
		int source2 = atoi(word3.c_str());

		int val = r.load(source1) ^ r.load(source2);
		r.store(dest,val);
	}

	else if (instructionName == "NOT")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());

	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source1 = atoi(word2.c_str());

		int val = ~(r.load(source1));
		r.store(dest,val);
	}

	else if (instructionName == "SHIFTLL")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());
	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source = atoi(word2.c_str());
		string imm = insMem[r.pc].arg3;
		unsigned val = r.load(source) << atoi(imm.c_str());
		r.store(dest,(int)val);
	}

	else if (instructionName == "SHIFTLA")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());
	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source = atoi(word2.c_str());
		string imm = insMem[r.pc].arg3;
		int val = r.load(source) << atoi(imm.c_str());
		r.store(dest,val);
	}

	else if (instructionName == "SHIFTRL")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());
	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source = atoi(word2.c_str());
		string imm = insMem[r.pc].arg3;
		unsigned val = r.load(source) >> atoi(imm.c_str());
		r.store(dest,(int)val);
	}

	else if (instructionName == "SHIFTRA")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());
	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source = atoi(word2.c_str());
		string imm = insMem[r.pc].arg3;
		int val = r.load(source) >> atoi(imm.c_str());
		r.store(dest,val);
	}

	//----------------LOAD/STORE Instructions----------------//

    else if (instructionName == "LOADR")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());
	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source = atoi(word2.c_str());
		string imm = insMem[r.pc].arg3;
		unsigned val = r.load(source) + atoi(imm.c_str());
        cout << val << endl;
		r.store(dest,dataMem[val]);
	}

	if (instructionName == "LOADI")
	{
		string word = insMem[r.pc].arg1;
		word.erase(0, 1);
		int dest = atoi(word.c_str());
		string imm = insMem[r.pc].arg2;
		int val = atoi(imm.c_str());
		r.store(dest,dataMem[val]);
	}

	if (instructionName == "STOREI")
	{
		string word = insMem[r.pc].arg1;
		word.erase(0, 1);
		int source = atoi(word.c_str());
		string addr = insMem[r.pc].arg2;
		int val = r.load(source) + atoi(addr.c_str());
        // cout << val << endl;
		string imm = insMem[r.pc].arg3;
		dataMem[val] = atoi(imm.c_str());
	}

	if (instructionName == "STORER")
	{
		string word = insMem[r.pc].arg1;
		word.erase(0, 1);
		int source = atoi(word.c_str());

		string word2 = insMem[r.pc].arg2;
        word2.erase(0, 1);
		int source2 = atoi(word2.c_str());

		string addr = insMem[r.pc].arg3;
		int val = r.load(source2) + atoi(addr.c_str());

		dataMem[val] = r.load(source);
	}

	//---------------Control Flow Instructions-----------//

	else if (instructionName == "CALL")
	{
		r.lr.push_back(r.pc);
		r.pc = atoi(insMem[r.pc].arg1.c_str()) - 4;	
	}

    else if (instructionName == "RETURN")
	{
		r.pc = r.lr.back();
		r.lr.pop_back();	
	}

	else if (instructionName == "JUMP")
	{
		cout << insMem[r.pc].arg1 << endl;
        int x = atoi(insMem[r.pc].arg1.c_str()) - 4;
        cout << x << endl;	
        r.pc = x;
	}

	else if (instructionName == "BEQ")
	{
		string word = insMem[r.pc].arg1;
		word.erase(0,1);
        int reg = atoi(word.c_str());
        int val = r.load(reg);
        if (val == 0) r.pc = atoi(insMem[r.pc].arg2.c_str()) - 4;	
	}

	else if (instructionName == "CMP")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());

	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source1 = atoi(word2.c_str());

		string word3 = insMem[r.pc].arg3;
		word3.erase(0, 1);
		int source2 = atoi(word3.c_str());

		int val;

		if (r.load(source1) < r.load(source2))
		{
			val = -1;
		}

		else if (r.load(source1) == r.load(source2))
		{
			val = 0;
		}

		else
		{
			val = 1;
		}

		r.store(dest,val);
	}

	else if (instructionName == "STOP")
	{
		return 1;
	}

	else if (instructionName == "NOP") {}

    // cout << "pc = " << r.pc << endl;
	r.pc += 4;

	return 0;
}
*/

int tick(std::map<int,int> &dataMem, std::map<int,instructionMemory> insMem, regFile &r, pipeline &p)
{
    fetch(insMem,r,p);
    decode(r,p);
    int x = execute(dataMem,r,p);
    writeback(r,p);

    // system("clear"); 
    cout << p.fetchStage.insName << " " << p.decodeStagePart1.insName<< " " <<  p.executeStagePart1.insName << " "<<  p.wbStage.insName << endl;
    cout << p.fetchStage.arg1 << " " << p.decodeStagePart1.arg1 << " " <<  p.executeStagePart1.arg1 << " "<<  p.wbStage.arg1 << endl;    
    cout << p.fetchStage.arg2 << " " << p.decodeStagePart1.arg2 << " " <<  p.executeStagePart1.arg2 << " "<<  p.wbStage.arg2 << endl;  
    cout << p.fetchStage.arg3 << " " << p.decodeStagePart1.arg3 << " " <<  p.executeStagePart1.arg3 << " "<<  p.wbStage.arg3 << endl;
 
    p.advance();
    return x;
}

int step(std::map<int,int> &dataMem, std::map<int,instructionMemory> insMem, regFile &r, int count, pipeline &p)
{
  	for (int i = 0; i< count; i++)
  	{
  		int j = tick(dataMem, insMem, r, p);

  		if (j == 1) return 1;
  	}

  	return 0;
}

void run(std::map<int,int> &dataMem, std::map<int,instructionMemory> insMem, regFile &r, pipeline &p)
{
 	while (step(dataMem, insMem, r, 1, p) != 1) {};
}

void help()
{
    cout << "--------------------------------------------------------------" << endl;
    cout << "help  || HELP  - Displays all available commands." << endl << endl;
    cout << "run   || RUN   - executes instructions until STOP." << endl << endl;
    cout << "reset || RESET - clears registers and memory, resets pc and lr." << endl << endl;
    cout << "step [numSteps] || STEP [numSteps] - executes 'numSteps' instructions." << endl << endl;
    cout << "exit  || EXIT || quit || QUIT  - exits simulator." << endl;
    cout << "--------------------------------------------------------------" << endl;
}

int parseInput(string s, std::map<int,int> &dataMem, std::map<int,instructionMemory> insMem, regFile &r, int regCount, pipeline &p)
{
	if (s == "exit" || s == "EXIT" || s == "quit" || s == "QUIT")
	{
		return -1;
	}

    else if (s == "run" || s == "RUN")
	{
		run(dataMem, insMem, r, p);
	}

	else if (s == "regs" || s == "REGS")
	{
		regs(r, regCount);
	}

	else if (s == "help" || s == "HELP")
	{
		help();
	}

    else if (s == "reset" || s == "reset")
    {
        dataMem.clear();
        r.clear();
    }

    else
    {
        cout << "Unrecognized command, type 'help' for a list of available commands." << endl;
    } 

	return 0;

}

int main(int argc, char* argv[])
{
	if (argc <2)
	{
		usage();
		exit(1);
	}
	int regCount = 16;
	std::map<int,instructionMemory> insMem;
	std::map<int,int> dataMem;
    pipeline p;
	regFile	   r(regCount);
	fileReader f(argv[1]);
	f.getContents();
	fillInstructionMemory(f,insMem);
	cout << "-----------Simple: A simple scalar processor simulator---------" << endl;
	int exitflag = 0;
	string s;
	while (exitflag == 0)
	{
		cout << ">> ";
        cin >> s;
        if (s == "step" || s == "STEP")
        {
          cin >> s;
          step(dataMem,insMem,r,atoi(s.c_str()), p);
        }
        else exitflag = parseInput(s, dataMem, insMem, r, regCount,p);
	}
    cout << "Goodbye!" << endl;
	// cout << insMem[12].insName << endl;
	return 0;
}
