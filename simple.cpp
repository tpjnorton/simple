#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>

using namespace std;

int currentLine = 1;

struct stringMetadata
{
	string line;
	int lineNo;
};

class readFile
{
	public:
		readFile(char* s) : i((s)) { currentTokenPos = currentLinePos = 0;}

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
						// cout << currentLine << endl;
						tokenList.push_back(wordStruct);
					}
			    	// cout << word << endl;
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

void usage(int argc, char* argv[])
{
	cout << "Usage: ./simple [inputfile]" << endl;
	exit(1);
}

int main()
{
	return 0;
}