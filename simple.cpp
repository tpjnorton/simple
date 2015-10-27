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

		else if (word == "BLTH" || word == "MOVI" || word == "LOADI" || word == "NOT")
		{
			insMem[address].insName = word;
			word = f.getNextToken();
			insMem[address].arg1 = word;
			word = f.getNextToken();
			insMem[address].arg2 = word;
		}

		else if (word == "BRANCH" || word == "JUMP" || word == "CALL")
		{
			insMem[address].insName = word;
			word = f.getNextToken();
			insMem[address].arg1 = word;
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

int fdx(std::map<int,int> &dataMem, std::map<int,instructionMemory> insMem, regFile &r)
{
	
	//----------------Arithmetic Instructions----------------//

	if (insMem[r.pc].insName == "MOVI")
	{
		string word = insMem[r.pc].arg1;
		word.erase(0, 1);
		int dest = atoi(word.c_str());
		string imm = insMem[r.pc].arg2;
		int val = atoi(imm.c_str());
		r.store(dest,val);
	}

	else if (insMem[r.pc].insName == "ADDI")
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

	else if (insMem[r.pc].insName == "ADDR")
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

	else if (insMem[r.pc].insName == "SUBI")
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

	else if (insMem[r.pc].insName == "SUBR")
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

	else if (insMem[r.pc].insName == "MULI")
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

	else if (insMem[r.pc].insName == "MULR")
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

	else if (insMem[r.pc].insName == "AND")
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

	else if (insMem[r.pc].insName == "OR")
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

    else if (insMem[r.pc].insName == "XOR")
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

	else if (insMem[r.pc].insName == "NOT")
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

	else if (insMem[r.pc].insName == "SHIFTLL")
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

	else if (insMem[r.pc].insName == "SHIFTLA")
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

	else if (insMem[r.pc].insName == "SHIFTRL")
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

	else if (insMem[r.pc].insName == "SHIFTRA")
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

    else if (insMem[r.pc].insName == "LOADR")
	{
		string word1 = insMem[r.pc].arg1;
		word1.erase(0, 1);
		int dest = atoi(word1.c_str());
	    string word2 = insMem[r.pc].arg2;
		word2.erase(0, 1);
		int source = atoi(word2.c_str());
		string imm = insMem[r.pc].arg3;
		unsigned val = r.load(source) + atoi(imm.c_str());
		r.store(dest,dataMem[val && 0XFFFFFF00]);
	}

	if (insMem[r.pc].insName == "LOADI")
	{
		string word = insMem[r.pc].arg1;
		word.erase(0, 1);
		int dest = atoi(word.c_str());
		string imm = insMem[r.pc].arg2;
		int val = atoi(imm.c_str());
		r.store(dest,dataMem[val && 0XFFFFFF00]);
	}

	if (insMem[r.pc].insName == "STOREI")
	{
		string word = insMem[r.pc].arg1;
		word.erase(0, 1);
		int source = atoi(word.c_str());
		string addr = insMem[r.pc].arg2;
		int val = r.load(source) + atoi(addr.c_str());
		string imm = insMem[r.pc].arg3;
		dataMem[val && 0XFFFFFF00] = atoi(imm.c_str());
	}

	if (insMem[r.pc].insName == "STORER")
	{
		string word = insMem[r.pc].arg1;
		word.erase(0, 1);
		int source = atoi(word.c_str());

		string word2 = insMem[r.pc].arg2;
		int source2 = atoi(word2.c_str());

		string addr = insMem[r.pc].arg3;
		int val = r.load(source2) + atoi(addr.c_str());

		dataMem[val && 0XFFFFFF00] = r.load(source);
	}

	//---------------Control Flow Instructions-----------//

	else if (insMem[r.pc].insName == "CALL")
	{
		r.lr.push_back(r.pc);
		r.pc = atoi(insMem[r.pc].arg1.c_str());	
	}

    else if (insMem[r.pc].insName == "RETURN")
	{
		r.pc = r.lr.back();
		r.lr.pop_back();	
	}

	else if (insMem[r.pc].insName == "JUMP")
	{
		r.pc = atoi(insMem[r.pc].arg1.c_str());	
	}

	else if (insMem[r.pc].insName == "BRANCH")
	{
		string word = insMem[r.pc].arg1;
		word.erase(0,1);
		r.pc = atoi(word.c_str()) + atoi(insMem[r.pc].arg2.c_str());	
	}

	else if (insMem[r.pc].insName == "CMP")
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

	else if (insMem[r.pc].insName == "STOP")
	{
		return 1;
	}

	else if (insMem[r.pc].insName == "NOP") {}

    // cout << "pc = " << r.pc << endl;
	r.pc += 4;

	return 0;
}

int step(std::map<int,int> &dataMem, std::map<int,instructionMemory> insMem, regFile &r, int count)
{
  	for (int i = 0; i< count; i++)
  	{
  		int j = fdx(dataMem, insMem, r);

  		if (j == 1) return 1;
  	}

  	return 0;
}

void run(std::map<int,int> &dataMem, std::map<int,instructionMemory> insMem, regFile &r)
{
 	while (step(dataMem, insMem, r, 1) != 1) {};
}

void help()
{
    cout << "--------------------------------------------------------------" << endl;
    cout << "help  || HELP  - Displays all available commands." << endl << endl;
    cout << "run   || RUN   - executes instructions until STOP." << endl << endl;
    cout << "reset || RESET - clears registers and memory, resets pc and lr." << endl << endl;
    cout << "step [numSteps] || STEP [numSteps] - executes 'numSteps' instructions." << endl << endl;
    cout << "exit  || EXIT || quit || QUIT  - exits simulator." << endl << endl;
}

int parseInput(string s, std::map<int,int> &dataMem, std::map<int,instructionMemory> insMem, regFile &r, int regCount)
{
	if (s == "exit" || s == "EXIT" || s == "quit" || s == "QUIT")
	{
		return -1;
	}

    else if (s == "run" || s == "RUN")
	{
		run(dataMem, insMem, r);
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
          step(dataMem,insMem,r,atoi(s.c_str()));
        }
        else exitflag = parseInput(s, dataMem, insMem, r, regCount);
	}
	// cout << insMem[12].insName << endl;
	return 0;
}
