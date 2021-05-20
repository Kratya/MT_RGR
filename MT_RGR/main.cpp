#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stack>
#include <queue>
#include <map>

using namespace std;

struct VarTableElem {
	VarTableElem(string name, int value, int typeLevel) :name(name), valueLevel(-1), typeLevel(-1) {}
	VarTableElem() : name(""), valueLevel(-1), typeLevel(-1) {}
	string name;
	int valueLevel;//уровень задания значения переменной, -1 - не задано
	int typeLevel; //уровень действия переменной, -1 - не объявлена
};
struct place {
	int i;
	int j;
	place(int i, int j) :i(i), j(j) {}
	place() :i(), j() {}
};
struct token {
	int table;
	int i;
	int j;
	token(int _table, int _i, int _j) {
		table = _table;
		i = _i;
		j = _j;
	}
	token() {}
};

class ConstTable
{
public:
	vector<string> array;
	ConstTable(string fileName)
	{
		readTable(fileName);
	};
	~ConstTable()
	{
		array.~vector();
	}

	int findElem(string Name)
	{
		auto res = find(array.begin(), array.end(), Name);
		if (res == array.end())
			return -1;
		else
			return res - array.begin();
	}
	string getElem(int index) {
		return array[index];
	}

private:
	void readTable(string fileName)
	{
		string tmp;
		fstream in(fileName);
		while (!(in.eof())) {
			in >> tmp;
			array.push_back(tmp);
		}
		in.close();
	}
};

class TableVar
{
public:
	vector <vector<VarTableElem>> array;

	TableVar()
	{
		array.resize(52);
	};
	~TableVar()
	{
		array.~vector();
	}

	place findElem(string name)
	{
		place pl;
		auto hash = getHash(name);
		pl.i = hash;
		if (array[hash].size()) {
			auto res = find_if(array[hash].begin(), array[hash].end(), [&](const VarTableElem& s)-> bool {return s.name == name; });
			if (res == array[hash].end())
			{
				array[hash].push_back(VarTableElem(name, 0, 0));
				pl.j = array[hash].size() - 1;
			}

			else pl.j = res - array[hash].begin();
		}

		else {
			array[hash].push_back(VarTableElem(name, 0, 0));
			pl.j = 0;
		}
		return pl;
	}

	VarTableElem getElem(place pl) {
		return array[pl.i][pl.j];
	}

private:
	int getHash(string h) {
		char len;
		if (h[0] >= 'A' && h[0] <= 'Z')
			len = h[0] - 'A';
		if (h[0] >= 'a' && h[0] <= 'z')
			len = h[0] - 'a' + 26;
		return (int)len;
	}
};

class TableInt
{
public:
	vector<vector<int>> array;

	TableInt()
	{
		array.resize(19);
	};
	~TableInt() {
		array.~vector();
	}

	place findElem(string value)
	{
		int tmp = stoi(value);
		place pl;
		auto hash = getHash(value);
		pl.i = hash;
		if (array[hash].size()) {
			auto res = find(array[hash].begin(), array[hash].end(), tmp);
			if (res == array[hash].end())
			{
				array[hash].push_back(tmp);
				pl.j = array[hash].size() - 1;
			}

			else pl.j = res - array[hash].begin();
		}

		else {
			array[hash].push_back(tmp);
			pl.j = 0;
		}
		return pl;
	}

	int getElem(place pl) {
		return array[pl.i][pl.j];
	}

private:
	int getHash(string h) {
		int len;
		if (h[0] == '-')
			len = (int)h[1] - '0' + 9;
		else len = (int)h[0] - '0';
		return len;
	}
};

class Lexeme
{
public:
	ConstTable* keyWords = new ConstTable("KeyWords.txt");
	ConstTable* operations = new ConstTable("Operation.txt");
	ConstTable* delimiters = new ConstTable("Delimiter.txt");
	TableInt* tableInt = new TableInt();
	TableVar* tableVar = new TableVar();
	bool syntaxSuccess = false;
	Lexeme(string fileName)
	{
		syntaxSuccess = createTokens(fileName, "tokenFile.txt", "errorFile.txt");
	}
	Lexeme() {}

	string str;
	int value;
	VarTableElem var;

	bool createTokens(string codeName, string tokenName, string errorName)
	{
		token tmp;
		int flag, strCount = 1;
		ofstream tokenFile(tokenName), errorFile(errorName);
		ifstream code(codeName);
		char ch;
		string str;
		bool unexpected = true, flStrl = false;
		place pl;
		if (code.peek() != EOF)
		{
			code.get(ch);
			while (1)
			{
				string errTmp;
				unexpected = true;
				if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z')
				{
					do
					{
						str += ch;
						errTmp += ch;
						if (code.peek() == EOF) { return true; }
						code.get(ch);

						if (!(ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch >= '0' && ch <= '9' || ch == '_' || ch == ':' || ch == ',' || ch == '/' ||
							ch == '-' || ch == '+' || ch == '*' || ch == '=' || ch == ';' || ch == '<' || ch == '>' || ch == '(' || ch == '{' || ch == ' ' || ch == ')' || ch == '!')) {

							if (code.peek() == EOF) { return true; }

							errTmp += ch;
							code.get(ch);
							flStrl = true;
						}

					} while (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch >= '0' && ch <= '9' || ch == '_');

					if (flStrl == true) {
						errorFile << "Lexical error in line " << strCount << ": " << errTmp << "." << " Maybe you mean " << str << endl;
						errTmp.clear();
						return false;
					}

					flag = keyWords->findElem(str);
					if (flag != -1)
					{
						tmp.table = 0;
						tmp.i = flag;
						tmp.j = -1;
						tokenFile << endl << tmp.table << " " << tmp.i << " " << tmp.j;
						unexpected = false;
					}
					else
					{
						pl = tableVar->findElem(str);
						tmp.table = 4;
						tmp.i = pl.i;
						tmp.j = pl.j;
						tokenFile << endl << tmp.table << " " << tmp.i << " " << tmp.j;
						unexpected = false;
					}
					str.clear();
				}

				if (ch >= '0' && ch <= '9')
				{
					flStrl = false;
					do
					{
						errTmp += ch;
						str += ch;
						if (code.peek() == EOF) { return true; }
						code.get(ch);

						if (!(ch >= '0' && ch <= '9' || ch == '_' || ch == ':' || ch == '-' || ch == '+' || ch == '*' || ch == '=' || ch == '!'
							|| ch == ';' || ch == '<' || ch == '>' || ch == '(' || ch == '{' || ch == ' ' || ch == ')' || ch == ',' || ch == '/')) {

							errTmp += ch;
							code.get(ch);
							flStrl = true;

						}
					} while (ch >= '0' && ch <= '9');
					if (flStrl) {
						errorFile << "Lexical error in line " << strCount << ": unexpected const " << errTmp << ". " << "Maybe you mean: " << str << endl;
						errTmp.clear();
						return false;
					}

					pl = tableInt->findElem(str);
					tmp.table = 3;
					tmp.i = pl.i;
					tmp.j = pl.j;
					tokenFile << endl << tmp.table << " " << tmp.i << " " << tmp.j;
					unexpected = false;
					str.clear();
				}

				if (ch == '=')
				{
					str = ch;
					if (code.peek() == EOF) { return true; }
					code.get(ch);
					tmp.table = 1;
					tmp.i = operations->findElem(str);
					tmp.j = -1;
					tokenFile << endl << tmp.table << " " << tmp.i << " " << tmp.j;
					unexpected = false;
					str.clear();
					while (ch == ' ')
					{
						if (code.peek() == EOF) { return true; }
						code.get(ch);
					}
					if (ch == '-' || ch == '+')
					{
						if (ch == '-') str = ch;
						if (code.peek() == EOF) { return true; }
						code.get(ch);
					}
				}

				if (ch == '+' || ch == '-' || ch == '*' || ch == '<' || ch == '>' || ch == '=' || ch == '!')
				{
					str += ch;
					if (code.peek() == EOF) { return true; }
					code.get(ch);
					if (ch == '+' || ch == '-' || ch == '*' || ch == '!' || ch == '<' || ch == '>' || ch == '=')
					{
						str += ch;
						if (code.peek() == EOF) { return true; }
						//code.get(ch);
					}
					flag = operations->findElem(str);
					if (flag != -1)
					{
						tmp.table = 1;
						tmp.i = flag;
						tmp.j = -1;
						tokenFile << endl << tmp.table << " " << tmp.i << " " << tmp.j;
						unexpected = false;
					}
					else { errorFile << "Lexical error in line " << strCount << ": two operation in round " << str << ". Maybe you mean " << str[0] << " or " << str[1] << endl; return false; }
					str.clear();
				}

				if (ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == ':' || ch == ';' || ch == ',')
				{
					str += ch;
					flag = delimiters->findElem(str);
					if (flag != -1)
					{
						tmp.table = 2;
						tmp.i = flag;
						tmp.j = -1;
						tokenFile << endl << tmp.table << " " << tmp.i << " " << tmp.j;
						unexpected = false;
					}
					else { errorFile << "Lexical error in line " << strCount << ": unexpected symbol " << str << endl; return false; }
					str.clear();
					if (code.peek() == EOF) { return true; }
					code.get(ch);
				}

				if (ch == '/')
				{
					if (code.peek() == EOF) { errorFile << "Lexical error in line " << strCount << ": unexpected symbol " << ch << endl; return false; }
					code.get(ch);
					if (ch == '/')
					{
						do
						{
							if (code.peek() == EOF) { return true; }
							code.get(ch);
						} while (ch != '\n');
						strCount++;
					}
					else if (ch == '*')
					{
						char ch2 = ' ';
						do
						{
							if (code.peek() == EOF) { errorFile << "Lexical error in line " << strCount << ": unclosed comment" << endl; return false; }
							code.get(ch);
							if (ch == '\n')
								strCount++;
							if (ch == '*')
							{
								if (code.peek() == EOF) { errorFile << "Lexical error in line " << strCount << ": unclosed comment" << endl; return false; }
								code.get(ch2);
							}
						} while (!(ch == '*' && ch2 == '/'));

					}
					else { errorFile << "Lexical error in line " << strCount << ":  open comment error" << endl; return false; }
					unexpected = false;
					if (code.peek() == EOF) { return true; }
					code.get(ch);
				}

				if (ch == '\n')
					strCount++;

				while (ch == '\n' || ch == '\t' || ch == ' ')
				{
					unexpected = false;
					if (code.peek() == EOF) { return true; }
					code.get(ch);
				}

				if (ch >= 'а' && ch <= 'я' || ch >= 'А' && ch <= 'Я' || ch == 'Ё' || ch == 'ё') {
					errorFile << "Lexical error in line " << strCount << ": russian letter " << ch << endl; return false;
				}

				if (unexpected)
				{
					errorFile << "Lexical error in line " << strCount << ": unexpected symbol " << ch << endl; return false;
				}
			}
		}

		tokenFile.close();
		errorFile.close();
		code.close();
	}

	void getElemByToken(token tmp) {
		switch (tmp.table) {
		case 0: str = keyWords->getElem(tmp.i); break;
		case 1: str = operations->getElem(tmp.i); break;
		case 2: str = delimiters->getElem(tmp.i); break;
		case 3: value = tableInt->getElem(place(tmp.i, tmp.j)); break;
		case 4: var = tableVar->getElem(place(tmp.i, tmp.j)); break;
		}
	}
private:

};

class Translator
{
public:

	Lexeme* oneLexeme;

	Translator(string ParseTable, string tokenFile, string postfixFile, string asmFile) {
		oneLexeme = new Lexeme("code.txt");
		if (oneLexeme->syntaxSuccess) {
			readParseTable(ParseTable);
			if (LLParse(tokenFile)) {

				ofstream postfix;
				postfix.open(postfixFile);
				for (int i = 0; i < toPostfixFile.size() - 1; i++)
					postfix << toPostfixFile[i] << " ";
				postfix << toPostfixFile[toPostfixFile.size() - 1];
				postfix.close();

				generateCode(postfixFile, asmFile);
			}
		}
	}

	vector<string> toPostfixFile;

	map<string, int> priority = { {"+", 2}, {"-",2}, {"*",3},{"=",0},{"==",1},{"!=",1 }, {"<",1}, {">",1},{",",0} };


	stack<int> States;

	// Структура элемент таблицы разбора
	struct tableParseElem
	{
		vector<string> terminal_; // Терминалы
		int jump_;                // Переход
		int accept_;              // Принимать или нет
		int stack_;               // Cтек
		int return_;              // Возвращать или нет
		int error_;               // Oшибка
	};

	vector<tableParseElem> tableParse;

	void readParseTable(string fileName) {
		string tmp;
		fstream in(fileName);
		while (!(in.eof())) {
			getline(in, tmp);

			string t;
			istringstream ss(tmp);
			vector<string> v;
			while (ss >> t)
				v.push_back(t);
			int size = v.size();
			tableParseElem one;

			int i = 0;
			for (i = 0; i < size - 5; i++)
				one.terminal_.push_back(v[i]);
			i = size - 5;
			one.jump_ = stoi(v[i]);
			one.accept_ = stoi(v[i + 1]);
			one.stack_ = stoi(v[i + 2]);
			one.return_ = stoi(v[i + 3]);
			one.error_ = stoi(v[i + 4]);

			tableParse.push_back(one);
		}
		in.close();

	}

	string readToken(int tableNum, int i, int j) {
		token oneToken(tableNum, i, j);

		oneLexeme->getElemByToken(oneToken);
		string str;
		if (tableNum == 0 || tableNum == 1 | tableNum == 2) {
			str = oneLexeme->str;
		}
		if (tableNum == 3) str = "const";
		if (tableNum == 4) str = "var";
		return str;
	}

	string makePostfix(vector<token> infix) {
		stack<string> tmp;
		string tmpStr;
		token curElem;
		queue<string> postfix;
		for (int i = 0; i < infix.size(); i++) {
			curElem = infix[i];
			oneLexeme->getElemByToken(curElem);
			if (curElem.table == 3) {
				int value = oneLexeme->value;
				string str = to_string(value);
				postfix.push(str);
			}
			else {
				if (curElem.table == 4) {
					string str = oneLexeme->var.name;
					postfix.push(str);
				}
				else { //не числа и не переменные
					oneLexeme->getElemByToken(curElem);
					string curStr = oneLexeme->str;


					if (curStr == "=") {
						tmp.push(curStr);
					}
					else {
						if (curStr == "(") {
							tmp.push(curStr);
						}
						else {
							if (curStr == ")") {
								while (tmp.top() != "(") {
									postfix.push(tmp.top());
									tmp.pop();
								}
								tmp.pop();
							}

							else {
								//все операторы кроме скобок
								if (tmp.empty() || tmp.top() == "(")
									tmp.push(curStr);
								else {
									int p_in = priority[curStr];
									int p_top = priority[tmp.top()];
									if (p_in > p_top) tmp.push(curStr);
									else {
										while ((tmp.top() != "(" && priority[tmp.top()] >= p_in)) {
											postfix.push(tmp.top());
											tmp.pop();
											if (tmp.empty()) break;
										}
										tmp.push(curStr);
									}
								}
							}
						}
					}
				}

			}

		}

		//перегрузка остатков из стека в очередь
		while (!tmp.empty()) {
			postfix.push(tmp.top());
			tmp.pop();
		}

		string str;

		//печать постфиксной очереди
		while (!postfix.empty()) {
			str = str + postfix.front();
			str = str + " ";
			postfix.pop();
		}

		return str;
	}

	bool LLParse(string tokenFile) {
		bool errorFlag = false;
		int curState = 0;
		ifstream in(tokenFile);
		ofstream err;
		err.open("errorFile.txt", ios::app);
		string tmp;
		int tableNum, i, j;
		if (in.peek() == EOF)
		{
			return true;
		}

		in >> tableNum >> i >> j;
		tmp = readToken(tableNum, i, j);
		vector<token> infix;
		bool postfixFlag = false;

		//структуры для обработки switch
		int globalCaseCount = 0;//для номера метки
		stack<string>switchCond;//для выражений из switch
		stack<string> endCase; //окончания case
		stack<int>countCase; //количество case для каждого switch

		int curLevel = 0;//текущий уровень действия переменных
		stack<vector<int>> admitLevels;//список допустимых уровней для переменных на каждом текущем уровне
		vector<int> tmpLevelVec;//для допустимых уровней переменных на текущем уровне
		tmpLevelVec.push_back(0);
		admitLevels.push(tmpLevelVec);

		//обработка состояний таблицы разбора
		do {
			//проверка столбца терминалов
			auto res = find(tableParse[curState].terminal_.begin(), tableParse[curState].terminal_.end(), tmp);
			if (res == tableParse[curState].terminal_.end()) { //если не найден
				if (tableParse[curState].error_) {
					err << "Syntax error: unexpected symbol, expected: ";
					for (i = 0; i < tableParse[curState].terminal_.size(); i++)
						err << "'" << tableParse[curState].terminal_[i] << "' ";
					return false;
				}
				else {
					curState++;
				}
			}
			else {//если найден
				if (tableParse[curState].accept_) {
					if (curState == 46 || curState == 30) {
						auto curVar = oneLexeme->tableVar->getElem(place(i, j));
						if (curState == 46) {
							//если текущий левел есть в списке на вершине стека
							if (curVar.typeLevel != -1)
								//уже задан - curVar.typeLevel принадлежит admitLevels.top()
							{
								err << "Syntax error: redefinition of type of variable '" << curVar.name << "'" << endl;
								errorFlag = true;
							}
							else oneLexeme->tableVar->array[i][j].typeLevel = admitLevels.top()[admitLevels.top().size() - 1];

							//задание типа переменные
						}
						else {
							//проверка типа
							if (find(admitLevels.top().begin(), admitLevels.top().end(), curVar.typeLevel) == admitLevels.top().end())
								//еще не задан - curVar.typeLevel не принадлежит admitLevels.top()
							{
								err << "Syntax error: undeclared variable '" << curVar.name << "'" << endl;
								errorFlag = true;
							}
						}
						string tmp2;
						int tableNum2, i2, j2;
						//считать новый токен
						if (in.peek() != EOF) {

							in >> tableNum2 >> i2 >> j2;
							tmp2 = readToken(tableNum2, i2, j2);
						}
						if (tmp2 == "=") {
							if (oneLexeme->tableVar->array[i][j].valueLevel == -1)//только если значение не было задано
								oneLexeme->tableVar->array[i][j].valueLevel = curLevel;
							infix.push_back(token(tableNum, i, j));
							postfixFlag = true;

						}
						tmp = tmp2;
						i = i2;
						j = j2;
						tableNum = tableNum2;
					}

					else {
						if (curState == 81) {
							auto curVar = oneLexeme->tableVar->getElem(place(i, j));
							if (find(admitLevels.top().begin(), admitLevels.top().end(), curVar.typeLevel) == admitLevels.top().end())
								//еще не объявлено - curVar.typeLevel не принадлежит admitLevels.top()
							{
								err << "Syntax error: undeclared variable '" << curVar.name << "'" << endl;
								errorFlag = true;
							}
							else {
								if (find(admitLevels.top().begin(), admitLevels.top().end(), curVar.valueLevel) == admitLevels.top().end())
									//еще не задано значение - curVar.typeLevel не принадлежит admitLevels.top()

								{
									err << "Syntax error: using variable without definition '" << curVar.name << "'" << endl;
									errorFlag = true;
								}
							}
						}
						if (curState == 25) {
							if (postfixFlag)
								toPostfixFile.push_back(makePostfix(infix));
							postfixFlag = false;
							infix.clear();
						}

						//скобка от switch
						if (curState == 36) {
							if (postfixFlag)
								switchCond.push(makePostfix(infix));
							postfixFlag = false;
							infix.clear();
						}

						//новый case
						if (curState == 51) {
							oneLexeme->getElemByToken(token(tableNum, i, j));
							toPostfixFile.push_back(switchCond.top() + to_string(oneLexeme->value) + " == ");
							countCase.top()++;
							toPostfixFile.push_back(" L" + to_string(globalCaseCount) + " CJ");
							endCase.push(" L" + to_string(globalCaseCount + 1) + ":");
							endCase.push(" L" + to_string(globalCaseCount) + ":");
							endCase.push(" UJ");
							endCase.push(" L" + to_string(globalCaseCount + 1));

							globalCaseCount += 2;
							curLevel++;
							tmpLevelVec.clear();
							tmpLevelVec = admitLevels.top();
							tmpLevelVec.push_back(curLevel);
							admitLevels.push(tmpLevelVec);
						}

						//break от текущего case
						if (curState == 56) {
							toPostfixFile.push_back(endCase.top());
							endCase.pop();
							toPostfixFile.push_back(endCase.top());
							endCase.pop();
							toPostfixFile.push_back(endCase.top());
							endCase.pop();
							admitLevels.pop();//убрать из стека список допустимых уровней
						}

						//начало default
						if (curState == 39) {
							curLevel++;
							tmpLevelVec.clear();
							tmpLevelVec = admitLevels.top();
							tmpLevelVec.push_back(curLevel);
							admitLevels.push(tmpLevelVec);
						}

						//break от default текущего switch
						if (curState == 42) {
							for (int ii = 0; ii < countCase.top(); ii++) {
								toPostfixFile.push_back(endCase.top());
								endCase.pop();
							}
							countCase.pop();//удалить подсчет case на текущем switch
							switchCond.pop();//удалить условие текущего switch
							admitLevels.pop();//убрать из стека список допустимых уровней
						}

						//записать в инфикс
						if (postfixFlag) infix.push_back(token(tableNum, i, j));

						//скобка от switch
						if (curState == 34) {
							postfixFlag = true;
							countCase.push(0);
						}
						//считать новый токен
						if (in.peek() != EOF) {
							in >> tableNum >> i >> j;
							tmp = readToken(tableNum, i, j);
						}
					}
				}
				if (tableParse[curState].stack_) {
					int stTmp;
					switch (curState) {
					case 19: stTmp = 52; break;
					case 21: stTmp = 53; break;
					default: stTmp = curState + 1;
					}
					States.push(stTmp);
				}
				if (tableParse[curState].jump_ > 0) {
					curState = tableParse[curState].jump_;
				}
				else {
					if (tableParse[curState].return_) {
						if (!States.empty()) {
							curState = States.top();
							States.pop();
							if (curState == 52 || curState == 53) {
								if (postfixFlag)
									toPostfixFile.push_back(makePostfix(infix));
								postfixFlag = false;
								infix.clear();
							}
						}
						else {
							err << "Syntax error: stack is empty!";
						}
					}
				}
			}
		} while (in.peek() != EOF && !errorFlag);

		if (errorFlag) {
			in.close();
			err.close();
			return false;
		}

		if (!errorFlag) {
			if (curState == 13) {
				err << "Success: correct end of making postfix!";
				in.close();
				err.close();
				return true;
			}
			else {
				err << "Syntax error: uncorrect end";
				in.close();
				err.close();
				return false;
			}
		}

	}

	bool generateCode(string postfixFile, string asmFile) {

		int markNum = 0;

		ofstream asmOut(asmFile);

		asmOut << ".386; директива, предписывающая Ассемблеру использовать" << endl;
		asmOut << "; набор операций для процессора 80386." << endl;

		asmOut << ".MODEL FLAT, STDCALL" << endl;
		asmOut << "EXTRN ExitProcess@4:PROC; выход" << endl;

		asmOut << ".DATA" << endl;
		asmOut << "; Переменные" << endl; //объявление переменных и констант
		for (int i = 0; i < 52; i++) {
			if (!oneLexeme->tableVar->array[i].empty()) {
				for (int j = 0; j < oneLexeme->tableVar->array[i].size(); j++) {
					asmOut << oneLexeme->tableVar->array[i][j].name << "\tdd ?" << endl;
				}
			}
		}

		asmOut << endl << "; Константы" << endl;
		for (int i = 0; i < 19; i++) {
			if (!oneLexeme->tableInt->array[i].empty()) {
				for (int j = 0; j < oneLexeme->tableInt->array[i].size(); j++) {
					asmOut << "const_" << i << "_" << j << "\t dd " << oneLexeme->tableInt->array[i][j] << endl;
				}
			}
		}

		asmOut << endl << ".CODE ; Сегмент кода" << endl;
		asmOut << "MAIN PROC; Метка точки входа" << endl;
		asmOut << "FINIT" << endl;

		string str, str1;
		bool flagAsm = false;
		string strAsm;

		ifstream postfix(postfixFile);

		//обработка постфиксной записи
		while (postfix >> str) {
			//проверить метки
			if (str[0] == 'L' && str[1] >= '0' && str[1] <= '9') {

				asmOut << "FINIT" << endl;
				flagAsm = false;
				if (str[str.length() - 1] == ':') {//если метка (не переход)
					asmOut << str << endl;
				}
				else {//если переход
					postfix >> str1;
					if (str1[0] == 'C') {//условный переход
						asmOut << "JNE " << str << endl;
					}
					else {//безусловный переход
						asmOut << "JMP " << str << endl;
					}
				}
			}
			else {
				//операнды - константы 
				if ((str[0] >= '0' && str[0] <= '9') || (str[0] == '-' && str.length() > 1)) {
					place constAsm = oneLexeme->tableInt->findElem(str);
					asmOut << "FILD " << "const_" << constAsm.i << "_" << constAsm.j << endl;
				}

				//операнды - переменные
				if ((str[0] >= 'a' && str[0] <= 'z') || (str[0] >= 'A' && str[0] <= 'Z')) {
					//проверочка для обработки присваивания
					if (!flagAsm) {
						strAsm = str;
						flagAsm = true;
					}
					asmOut << "FILD " << str << endl;
				}
			}

			//операция +
			if (str[0] == '+') {
				//2 элемента лежат в стеке				
				asmOut << "FADD" << endl;
			}

			//операция - 
			if (str[0] == '-' && str.length() == 1) {
				//2 элемента лежат в стеке				
				asmOut << "FSUB" << endl;
			}

			//операция *
			if (str[0] == '*') {
				//2 элемента лежат в стеке				
				asmOut << "FMUL" << endl;
			}

			//операция =
			if (str[0] == '=' && str.length() == 1) {
				//2 элемента лежат в стеке
				asmOut << "FISTP " << strAsm << endl;
				asmOut << "FINIT " << endl;

				flagAsm = false;
			}

			//операция ==
			if (str[0] == '=' && str.length() > 1) {
				//2 элемента лежат в стеке
				asmOut << "FCOM" << endl;
				asmOut << "FSTSW AX" << endl;
				asmOut << "SAHF" << endl;
				asmOut << "JE JMP_EQ_" << markNum << endl;
				asmOut << "FLDZ" << endl;
				asmOut << "JMP JMP_EQ_" << markNum << "_END" << endl;
				asmOut << "JMP_EQ_" << markNum << ":" << endl;
				asmOut << "FLD1" << endl;
				asmOut << "JMP_EQ_" << markNum << "_END:" << endl;

				markNum++;
			}

			//операция !=
			if (str[0] == '!') {
				//2 элемента лежат в стеке
				asmOut << "FCOM" << endl;
				asmOut << "FSTSW AX" << endl;
				asmOut << "SAHF" << endl;
				asmOut << "JNE JMP_NOTEQ_" << markNum << endl;
				asmOut << "FLDZ" << endl;
				asmOut << "JMP JMP_NOTEQ_" << markNum << "_END" << endl;
				asmOut << "JMP_NOTEQ_" << markNum << ":" << endl;
				asmOut << "FLD1" << endl;
				asmOut << "JMP_NOTEQ_" << markNum << "_END:" << endl;

				markNum++;
			}

			//операция <
			if (str[0] == '<') {
				//2 элемента лежат в стеке
				asmOut << "FCOM" << endl;
				asmOut << "FSTSW AX" << endl;
				asmOut << "SAHF" << endl;
				asmOut << "JG JMP_LESS_" << markNum << endl;
				asmOut << "FLD1" << endl;
				asmOut << "JMP JMP_LESS_" << markNum << "_END" << endl;
				asmOut << "JMP_LESS_" << markNum << ":" << endl;
				asmOut << "FLDZ" << endl;
				asmOut << "JMP_LESS_" << markNum << "_END:" << endl;

				markNum++;
			}

			//операция >
			if (str[0] == '>') {

				asmOut << "FCOM" << endl;
				asmOut << "FSTSW AX" << endl;
				asmOut << "SAHF" << endl;
				asmOut << "JL JMP_GREAT_" << markNum << endl;
				asmOut << "FLD1" << endl;
				asmOut << "JMP JMP_GREAT_" << markNum << "_END" << endl;
				asmOut << "JMP_GREAT_" << markNum << ":" << endl;
				asmOut << "FLDZ" << endl;
				asmOut << "JMP_GREAT_" << markNum << "_END:" << endl;

				markNum++;
			}
		}

		asmOut << endl << "; Выход из программы" << endl;
		asmOut << "PUSH 0; Параметр: код выхода" << endl;
		asmOut << "CALL ExitProcess@4" << endl;
		asmOut << "MAIN ENDP" << endl;
		asmOut << "END MAIN" << endl;


		postfix.close();
		asmOut.close();
		return true;
	}
};

void main()
{
	auto trans = new Translator("ParseTable.txt", "tokenFile.txt", "postfix.txt", "asmOut.txt");

	system("pause");
}