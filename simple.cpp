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
    int arg1RegNum;
    int arg2RegNum;
    int arg3RegNum;


    pipelineStage()
    {
        insName = "NOP";
        arg1 = arg2 = arg3 = 0;
        arg1RegNum = arg2RegNum = arg3RegNum = -2;
    }

};

struct stringMetadata
{
    string line;
    int lineNo;
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

        regFile *rPoint;
        std::map<int,instructionMemory> *insMemPoint;
        std::map<int,int> *dataMemPoint;

        // void fetch();
        // int decode();
        // int execute();
        // void writeback();


        pipeline(std::map<int,instructionMemory> *insMem, std::map<int,int> *dataMem, regFile *r)
        {
            rPoint = r;
            insMemPoint = insMem;
            dataMemPoint = dataMem;
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

            executeStagePart1.arg1RegNum    = decodeStagePart2.arg1RegNum;
            executeStagePart1.arg2RegNum    = decodeStagePart2.arg2RegNum;
            executeStagePart1.arg3RegNum    = decodeStagePart2.arg3RegNum;

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

        void fetch()
        {
            fetchStage.insName = (*insMemPoint)[rPoint->pc].insName;
            fetchStage.arg1 =    (*insMemPoint)[rPoint->pc].arg1;
            fetchStage.arg2 =    (*insMemPoint)[rPoint->pc].arg2;
            fetchStage.arg3 =    (*insMemPoint)[rPoint->pc].arg3;
            rPoint->pc += 4;
        }

        int decode()
        {
            decodeStagePart2.insName = decodeStagePart1.insName;
            decodeStagePart2.arg1 = 0;
            decodeStagePart2.arg2 = 0;
            decodeStagePart2.arg3 = 0;
            decodeStagePart2.arg1RegNum = -2;
            decodeStagePart2.arg2RegNum = -2;
            decodeStagePart2.arg3RegNum = -2;   
            string instructionName = decodeStagePart2.insName;

            //----------------Arithmetic Instructions----------------//

            if (instructionName == "MOVI" || instructionName == "LOADI")
            {
                string word = decodeStagePart1.arg1;
                word.erase(0, 1);
                decodeStagePart2.arg1 = atoi(word.c_str());

                string imm = decodeStagePart1.arg2;
                decodeStagePart2.arg2 = atoi(imm.c_str());
            }

            else if (instructionName == "ADDI" || instructionName == "SUBI" || instructionName == "MULI")
            {
                string word1 = decodeStagePart1.arg1;
                word1.erase(0, 1);
                decodeStagePart2.arg1 = atoi(word1.c_str());
                string word2 = decodeStagePart1.arg2;
                word2.erase(0, 1);
                decodeStagePart2.arg2 = rPoint->load(atoi(word2.c_str()));
                cout << "value of register: " << decodeStagePart2.arg2 << endl;
                decodeStagePart2.arg2RegNum = atoi(word2.c_str());
                string imm = decodeStagePart1.arg3;
                decodeStagePart2.arg3 = atoi(imm.c_str());  
            }

            else if (instructionName == "ADDR" || instructionName == "SUBR" || instructionName == "MULR" || instructionName == "AND"  \
                     || instructionName == "OR" || instructionName == "XOR" || instructionName == "CMP" )
            {
                string word1 = decodeStagePart1.arg1;
                word1.erase(0, 1);
                decodeStagePart2.arg1 = atoi(word1.c_str());

                string word2 = decodeStagePart1.arg2;
                word2.erase(0, 1);
                decodeStagePart2.arg2  = rPoint->load(atoi(word2.c_str()));
                decodeStagePart2.arg2RegNum = atoi(word2.c_str());

                string word3 = decodeStagePart1.arg3;
                word3.erase(0, 1);
                decodeStagePart2.arg3 = rPoint->load(atoi(word3.c_str()));
                decodeStagePart2.arg3RegNum = atoi(word3.c_str());
                
            }


            //----------------Logical Operators----------------//


            else if (instructionName == "NOT")
            {
                string word = decodeStagePart1.arg1;
                word.erase(0, 1);
                decodeStagePart2.arg1 = atoi(word.c_str());

                string imm = decodeStagePart1.arg2;
                decodeStagePart2.arg2 = rPoint->load(atoi(imm.c_str()));
                decodeStagePart2.arg2RegNum = atoi(imm.c_str());
                
            }

            else if (instructionName == "SHIFTLL" || instructionName == "SHIFTLA" || instructionName == "SHIFTRL" \
                      || instructionName == "SHIFTRA")
            {
                string word1 = decodeStagePart1.arg1;
                word1.erase(0, 1);
                decodeStagePart2.arg1 = atoi(word1.c_str());
                string word2 = decodeStagePart1.arg2;
                word2.erase(0, 1);
                decodeStagePart2.arg2 = rPoint->load(atoi(word2.c_str()));
                decodeStagePart2.arg2RegNum = atoi(word2.c_str());
                string imm = decodeStagePart1.arg3;
                decodeStagePart2.arg3 = atoi(imm.c_str());      
            }

            //----------------LOAD/STORE Instructions----------------//

            else if (instructionName == "STOREI")
            {
                string word1 = decodeStagePart1.arg1;
                word1.erase(0, 1);
                decodeStagePart2.arg1 = rPoint->load(atoi(word1.c_str()));
                decodeStagePart2.arg1RegNum = atoi(word1.c_str());
                string word2 = decodeStagePart1.arg2;
                decodeStagePart2.arg2 = atoi(word2.c_str());
                string imm = decodeStagePart1.arg3;
                decodeStagePart2.arg3 = atoi(imm.c_str());  
            }

            else if (instructionName == "STORER")
            {
                string word1 = decodeStagePart1.arg1;
                word1.erase(0, 1);
                decodeStagePart2.arg1 = rPoint->load(atoi(word1.c_str()));
                decodeStagePart2.arg1RegNum = atoi(word1.c_str());

                string word2 = decodeStagePart1.arg2;
                word2.erase(0, 1);
                decodeStagePart2.arg2 = rPoint->load(atoi(word2.c_str()));
                decodeStagePart2.arg2RegNum = atoi(word2.c_str());

                string imm = decodeStagePart1.arg3;
                imm.erase(0, 1);
                decodeStagePart2.arg3 = rPoint->load(atoi(imm.c_str()));
                decodeStagePart2.arg3RegNum = atoi(imm.c_str());
            }

            else if (instructionName == "LOADR")
            {
                string word1 = decodeStagePart1.arg1;
                word1.erase(0, 1);
                decodeStagePart2.arg1 = atoi(word1.c_str());

                string word2 = decodeStagePart1.arg2;
                word2.erase(0, 1);
                decodeStagePart2.arg2 = rPoint->load(atoi(word2.c_str()));
                decodeStagePart2.arg2RegNum = atoi(word2.c_str());

                string imm = decodeStagePart1.arg3;
                decodeStagePart2.arg3 = atoi(imm.c_str());
            }

            //---------------Control Flow Instructions-----------//

            else if (instructionName == "BEQ")
            {
                string word = decodeStagePart1.arg1;
                word.erase(0,1);
                int reg = atoi(word.c_str());
                decodeStagePart2.arg1 = rPoint->load(reg);
                decodeStagePart2.arg1RegNum = reg;
                decodeStagePart2.arg2 = atoi(decodeStagePart1.arg2.c_str());   
            }

            else if (instructionName == "CALL")
            { 
                decodeStagePart2.arg1 = atoi(decodeStagePart1.arg1.c_str()); 
            }

            else if (instructionName == "RETURN") {}

            else if (instructionName == "JUMP")
            {
                decodeStagePart2.arg1 = atoi(decodeStagePart1.arg1.c_str()); 
            }

            else if (instructionName == "NOP" || instructionName == "STOP" ) {}

            return 0;
        }

        int execute()
        {
            
            if (executeStagePart2.insName != "NOP" && executeStagePart2.insName != "STOP" && executeStagePart2.insName != "RETURN")
            {
                if (executeStagePart1.arg1RegNum == executeStagePart2.arg1)
                {
                    executeStagePart1.arg1 = executeStagePart2.arg2;
                    cout << "Dependency issue, reloading register in argument 1" << endl;
                }

                if (executeStagePart1.arg2RegNum == executeStagePart2.arg1)
                {
                    executeStagePart1.arg2 = executeStagePart2.arg2;
                    cout << "Dependency issue, reloading register in argument 2" << endl;
                }

                if (executeStagePart1.arg3RegNum == executeStagePart2.arg1)
                {
                    executeStagePart1.arg3 = executeStagePart2.arg2;
                    cout << "Dependency issue, reloading register in argument 3" << endl;
                }    

            }

            // else if (executeStagePart2.insName != "BEQ" && executeStagePart2.insName != "STORER" && executeStagePart2.insName != "STOREI")

            executeStagePart2.insName = executeStagePart1.insName;
            executeStagePart2.arg1 = 0;
            executeStagePart2.arg2 = 0;
            executeStagePart2.arg3 = 0;  
            string instructionName = executeStagePart2.insName;
            int retVal = 0;

            //----------------Arithmetic Instructions----------------//

            if (instructionName == "MOVI")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = executeStagePart1.arg2;
            }

            else if (instructionName == "ADDI")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = executeStagePart1.arg2 + executeStagePart1.arg3;
            }

            else if (instructionName == "ADDR")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = executeStagePart1.arg2 + executeStagePart1.arg3;
            }

            else if (instructionName == "SUBI")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = executeStagePart1.arg2 - executeStagePart1.arg3;
            }

            else if (instructionName == "SUBR")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = executeStagePart1.arg2 - executeStagePart1.arg3;
            }

            else if (instructionName == "MULI")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = executeStagePart1.arg2 * executeStagePart1.arg3;
            }

            else if (instructionName == "MULR")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = executeStagePart1.arg2 * executeStagePart1.arg3;
            }

            //----------------Logical Operators----------------//

            else if (instructionName == "AND")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = executeStagePart1.arg2 & executeStagePart1.arg3;
            }

            else if (instructionName == "OR")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = executeStagePart1.arg2 | executeStagePart1.arg3;
            }

            else if (instructionName == "XOR")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = executeStagePart1.arg2 ^ executeStagePart1.arg3;
            }

            else if (instructionName == "NOT")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = ~(executeStagePart1.arg2); 
            }

            else if (instructionName == "SHIFTLL")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = (unsigned)executeStagePart1.arg2 << executeStagePart1.arg3;
            }

            else if (instructionName == "SHIFTLA")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = executeStagePart1.arg2 << executeStagePart1.arg3;
            }

            else if (instructionName == "SHIFTRL")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = (unsigned)executeStagePart1.arg2 >> executeStagePart1.arg3;
            }

            else if (instructionName == "SHIFTRA")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = executeStagePart1.arg2 >> executeStagePart1.arg3;
            }

            //----------------LOAD/STORE Instructions----------------//

            else if (instructionName == "LOADR")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = executeStagePart1.arg2 + executeStagePart1.arg3;
                cout << "address to load from: " << executeStagePart2.arg2 << endl;
                executeStagePart2.arg2    = (*dataMemPoint)[executeStagePart2.arg2];
                cout << "value loaded: " << executeStagePart2.arg2 << endl;
            }

            if (instructionName == "LOADI")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = (*dataMemPoint)[executeStagePart1.arg2];
            }

            if (instructionName == "STOREI")
            {
                executeStagePart2.arg1          = executeStagePart1.arg1;
                executeStagePart2.arg2          = executeStagePart1.arg1 + executeStagePart1.arg2;
                cout << "address to store in: "   << executeStagePart2.arg2 << endl;
                (*dataMemPoint)[executeStagePart2.arg2] = executeStagePart1.arg3;
                cout << "value stored: "          << executeStagePart1.arg3 << endl;
            }

            if (instructionName == "STORER")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = executeStagePart1.arg2 + executeStagePart1.arg3;
                cout << "address to store in: " << executeStagePart2.arg2 << endl;
                (*dataMemPoint)[executeStagePart2.arg2] = executeStagePart2.arg1;
                cout << "value stored: " << executeStagePart2.arg1 << endl;
            }

            //---------------Control Flow Instructions-----------//

            else if (instructionName == "CALL")
            {
                rPoint->lr.push_back(rPoint->pc - 4);
                rPoint->pc = executeStagePart1.arg1; 
                flush();
            }

            else if (instructionName == "RETURN")
            {
                rPoint->pc = rPoint->lr.back();
                rPoint->lr.pop_back();
                flush();
            }

            else if (instructionName == "JUMP")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                rPoint->pc = executeStagePart2.arg1;
                flush();
            }

            else if (instructionName == "BEQ")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                executeStagePart2.arg2    = executeStagePart1.arg2;
                if (executeStagePart2.arg1 == 0) 
                {
                    rPoint->pc = executeStagePart2.arg2;  
                    flush(); 
                }
            }

            else if (instructionName == "CMP")
            {
                executeStagePart2.arg1    = executeStagePart1.arg1;
                
                int cmp1   = executeStagePart1.arg2;
                int cmp2   = executeStagePart1.arg3;

                if (cmp1 < cmp2)
                {
                    executeStagePart2.arg2 = -1;
                }

                else if (cmp1 == cmp2)
                {
                    executeStagePart2.arg2 = 0;
                }

                else
                {
                    executeStagePart2.arg2 = 1;
                }

            }

            else if (instructionName == "STOP")
            {
                retVal = 1;
                flush();
            }

            else if (instructionName == "NOP") {}

            executeStagePart2.arg1RegNum = executeStagePart1.arg1RegNum;
            executeStagePart2.arg2RegNum = executeStagePart1.arg2RegNum; 
            executeStagePart2.arg3RegNum = executeStagePart1.arg3RegNum;  


            return retVal;

        }

        void writeback()
        {

            string instructionName = wbStage.insName;
            wbStage.arg3 = 0;
            cout << endl;

            //----------------Arithmetic Instructions----------------//

            if (instructionName == "MOVI")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            else if (instructionName == "ADDI")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            else if (instructionName == "ADDR")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            else if (instructionName == "SUBI")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            else if (instructionName == "SUBR")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            else if (instructionName == "MULI")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            else if (instructionName == "MULR")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            //----------------Logical Operators----------------//

            else if (instructionName == "AND")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            else if (instructionName == "OR")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            else if (instructionName == "XOR")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); ;
            }

            else if (instructionName == "NOT")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            else if (instructionName == "SHIFTLL")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            else if (instructionName == "SHIFTLA")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            else if (instructionName == "SHIFTRL")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            else if (instructionName == "SHIFTRA")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            //----------------LOAD/STORE Instructions----------------//

            else if (instructionName == "CMP")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            else if (instructionName == "LOADR")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }

            if (instructionName == "LOADI")
            {
                rPoint->store(wbStage.arg1, wbStage.arg2); 
            }    
        }
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
            // cout << word << endl;
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

int tick(pipeline &p)
{
    p.writeback();
    int x = p.execute();
    p.decode();
    p.fetch();

    cout << "------------------////-------------------" << endl;
    cout << "fetch     insname: " << p.fetchStage.insName << endl;
    cout << "decode    insname: " << p.decodeStagePart1.insName << endl;
    cout << "execute   insname: " << p.executeStagePart1.insName << endl;
    cout << "writeback insname: " << p.wbStage.insName << endl;
    cout << "fetch args:"  << " " << p.fetchStage.arg1 << " " << p.fetchStage.arg2 << " " << p.fetchStage.arg3 << endl << endl;
    cout << "decode stage 1 args:" << " " << p.decodeStagePart1.arg1 << " " << p.decodeStagePart1.arg2 << " " << p.decodeStagePart1.arg3 << endl;
    cout << "decode stage 2 args:" << " " << p.decodeStagePart2.arg1 << " " << p.decodeStagePart2.arg2 << " " << p.decodeStagePart2.arg3 << endl << endl;
    cout << "execute stage 1 args:" << " " << p.executeStagePart1.arg1 << " " << p.executeStagePart1.arg2 << " " << p.executeStagePart1.arg3 << endl;
    cout << "execute stage 2 args:" << " " << p.executeStagePart2.arg1 << " " << p.executeStagePart2.arg2 << " " << p.executeStagePart2.arg3 << endl << endl;
    cout << "writeback args:"  << " " << p.wbStage.arg1 << " " << p.wbStage.arg2 << " " << p.wbStage.arg3 << endl;
    cout << "pc: " << p.rPoint->pc << endl;
    cout << "------------------////-------------------" << endl;

 
    p.advance();
    return x;
}

int step(std::map<int,int> &dataMem, std::map<int,instructionMemory> insMem, regFile &r, int count, pipeline &p)
{
  	for (int i = 0; i< count; i++)
  	{
  		int j = tick(p);

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
    regFile    r(regCount);
    fileReader f(argv[1]);
    pipeline p(&insMem, &dataMem, &r);
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
