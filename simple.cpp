#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <map>

using namespace std;

int currentLine = 1;

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
		regFile(int regCount)
		{
			regBank = new int[regCount];
		}

	    regFile()
		{
			regBank = new int[16];
		}

		void store(int regNum, int value)
		{
			regBank[regNum] = value;
		}

		int load(int regNum)
		{
			
			return regBank[regNum];
		}

		~regFile()
		{
			delete regBank;
		}

	private:
		int* regBank;
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
	int address = 0x00;
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

		else if (word == "AND" || word == "OR" || word == "XOR" || word == "SHIFTLA" || word == "SHIFTLL" || word == "SHIFTRA" || word == "SHIFTRL")
		{
			insMem[address].insName = word;
			word = f.getNextToken();
			insMem[address].arg1 = word;
			word = f.getNextToken();
			insMem[address].arg2 = word;
			word = f.getNextToken();
			insMem[address].arg3 = word;
		}

		else if (word == "MULI" || word == "MULR" || word == "XOR" || word == "SHIFTLA" || word == "SHIFTLL" || word == "SHIFTRA" || word == "SHIFTRL")
		{
			insMem[address].insName = word;
			word = f.getNextToken();
			insMem[address].arg1 = word;
			word = f.getNextToken();
			insMem[address].arg2 = word;
			word = f.getNextToken();
			insMem[address].arg3 = word;
		}

		else if (word == "BLTH" || word == "MOVI" || word == "LOADI" || word == "STOREI" || word == "STORER" || word == "NOT")
		{
			insMem[address].insName = word;
			word = f.getNextToken();
			insMem[address].arg1 = word;
			word = f.getNextToken();
			insMem[address].arg2 = word;
		}

		else if (word == "B" || word == "J" || word == "LOADI" || word == "CALL")
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

int main(int argc, char* argv[])
{
	if (argc <2)
	{
		usage();
		exit(1);
	}

	std::map<int,instructionMemory> insMem;
	std::map<int,int> dataMem;
	regFile	   r;
	fileReader f(argv[1]);
	f.getContents();
	fillInstructionMemory(f,insMem);
	// cout << insMem[12].insName << endl;
	return 0;
}
