/*****************************************************************************
TITLE: asm.cpp																																
AUTHOR: SAIRAM PAILA (2101CS70)
Declaration of Authorship
This file is part of the miniproject of CS209/CS210 at the 
department of Computer Science and Engg, IIT Patna . 
*****************************************************************************/

#include<bits/stdc++.h>	    
using namespace std;

struct table{ 							// Store ASM code in following format					
	string label;						//  Label | Mnemonic | Operand | Operand Type (Hex, Oct etc) | If label is present in that line
	string mnemonic;
	string operand;
	int oprType;
	bool labelPresent;
};

vector<table> data;						//Data vector in Table form
map<string, pair<string, int>> OPC;		//Stores mnemonic and OPCode
vector<pair<int, string>> errors;		//Stores all the errors
vector<string> cleaned;					//Contains the code line in clean form w/o spaces
map<string, int> labels;				//Stores the labels and their declaration line
vector<pair<int, string>> machineCode;	//Stores the machine code and extra info for generating listCode
vector<int> pC;				//Maintains Program counter on every line
bool haltPresent = false;				//Check if HALT is present in the Code
string fileName;				
							
bool isOctal(string s);							
bool isHexa(string s);					
bool isDec(string s);						
bool validName(string cur);

string decToHex(int number, int add = 24){			// function to convert decimal to Hexadecimal
	if(add == 32){
		unsigned int num = number;
		stringstream ss; 
    	ss << hex << num; 
    	return ss.str();
	}
	if(number < 0)
		number += (1 << add);
	stringstream ss; 
    ss << hex << number; 
    return ss.str();
}

void init(){																		// Initialization of the Mnemonic, OPCode
    OPC["data"] = {"", 1}, OPC["ldc"] = {"00", 1}, OPC["adc"] = {"01", 1};			// 0 = Takes no operands like add
    OPC["ldl"] = {"02", 2}, OPC["stl"] = {"03", 2}, OPC["ldnl"] = {"04", 2};		// 1 = Takes 1 operand and is value
    OPC["stnl"] = {"05", 2}, OPC["add"] = {"06", 0}, OPC["sub"] = {"07", 0};		// 2 = Takes 1 operand and is offset
    OPC["shl"] = {"08", 0}, OPC["shr"] = {"09", 0}, OPC["adj"] = {"0A" , 1};		// No OPCODE for data and SET
    OPC["a2sp"] = {"0B" , 0}, OPC["sp2a"] = {"0C" , 0}, OPC["call"] = {"0D" , 2};	// SET is pseudo instruction
    OPC["return"] = {"0E" , 0}, OPC["brz"] = {"0F" , 2}, OPC["brlz"] = {"10" , 2};
    OPC["br"] = {"11", 2}, OPC["HALT"] = {"12", 0}, OPC["SET"] = {"" , 1};
}

void genErrors(int line, string type){												//Store the errors in errors vector
	errors.push_back({line + 1, "Error at line: " + to_string(line) + " -- Type: " + type});
}

string clean(string s, int line){													//Remove unnecessary spaces	
	for(int i = 0; i < 2; i++){
		reverse(s.begin(), s.end());
		while(s.back() == ' ' or s.back() == '\t'){
			s.pop_back();
		}
	}
	
	string temp;
	
	for(int i = 0; i < (int)s.size(); i++){
		if(s[i] == ';')
			break;
		if(s[i] == ':'){
			temp += ":";
			if(i == (int)s.size() - 1 or s[i + 1] != ' ')
				temp += " ";
			continue;
		}
		if(s[i] != ' ' and s[i] != '\t'){
			temp += s[i];
			continue;
		}
		temp += " ";
		int j = i;
		while(s[i] == s[j] and j < (int) s.size()) j++;
		i = j - 1;
	}
	
	while(!temp.empty() and (temp.back() == ' ' or temp.back() == '\t'))
		temp.pop_back();
	int spac = 0;
	for(auto to: temp)
		spac += (to == ' ');
	if(spac > 2)
		genErrors(line + 1, "Invalid syntax");
	return temp;
}

void pushSET(vector<string> &temp, string token, string s, int j){		// Following mnemonic used to implement SET mnemonic in assembler
	if(s.size() <= j + 5){
		return;
	}
	temp.push_back("adj 10000");			//Stack pointer at random memory
	temp.push_back("stl -1");				//Here to load A
	temp.push_back("stl 0");				//Here B
	temp.push_back("ldc " + s.substr(j + 6, s.size() - (j + 6)));
	temp.push_back("ldc " + token.substr(0, j));
	temp.push_back("stnl 0");				//Load A, B
	temp.push_back("ldl 0");				
	temp.push_back("ldl -1");
	temp.push_back("adj -10000");			//Adjust stack ptr
}

void impSET(){						// Implement SET mnemonic given. SET is pseudo instruction,so implement it with other mnemonic
	vector<string> temp;
	
	for(int i = 0; i < (int) cleaned.size(); i++){
		string cur;
		bool state = false;
		for(int j = 0; j < (int) cleaned[i].size(); j++){
			cur += cleaned[i][j];
			if(cleaned[i][j] == ':'){
				cur.pop_back();
				if(cleaned[i].size() > j + 5 and cleaned[i].substr(j + 2, 3) == "SET"){
					state = true;
					if(abs(labels[cur]) == i){
						labels[cur] = (int)temp.size() - 1;
						temp.push_back(cleaned[i].substr(0, j + 1) + " data " + cleaned[i].substr(j + 6, (int)cleaned[i].size() - (j + 6)));
					}
					else{
						pushSET(temp, cur, cleaned[i], j);
					}
					break;
				}
			}
		}
		if(!state and !cleaned[i].empty())
			temp.push_back(cleaned[i]);
	}
	cleaned = temp;
}
void processLabel(){														// process Lables and stores the labels map, with the postion they are declared
	for(int i = 0; i < (int) cleaned.size(); i++){
		string cur;
		for(int j = 0; j < (int) cleaned[i].size(); j++){
			if(cleaned[i][j] == ':'){
				bool is = validName(cur);
				if(!is){
					genErrors(i + 1, "Invalid label name" );
					break;
				}
				if(labels.count(cur)){
					if(cleaned[i].size() > j + 4 and cleaned[i].substr(j + 2, 3) == "SET"){
						continue;
					}
					if(cleaned[i].size() > j + 5 and cleaned[i].substr(j + 2, 4) == "data" and labels[cur] < 0){
						labels[cur] = i;
						continue;
					}
					genErrors(i + 1, "Multiple declaration of label: " + cur);
				}
				if(cleaned[i].size() > j + 4 and cleaned[i].substr(j + 2, 3) == "SET"){
					labels[cur] = -i;
					continue;
				}
				labels[cur] = i;
				break;
			}
			cur += cleaned[i][j];
		}
	}
}

void fillData(int i, string one, string two, string three, int type){			// fill the data vector to reduce code size
	data[i].label = one;
	data[i].mnemonic = two;
	data[i].operand = three;
	data[i].oprType = type;
}

int calType(string s){															//Return if the operand is Hex value/Decimal value/Octal value/label/ 
	if(s.empty()) return 0;
	if(s[0] == '+' or s[0] == '-'){
		reverse(s.begin(), s.end());
		s.pop_back();
		reverse(s.begin(), s.end());
	}
	if(s.empty())
		return -1;
	else if(isDec(s)) return 10;
	else if(isOctal(s)) return 8;
	else if(isHexa(s)) return 16;
	else if(validName(s)) return 1;
	return -1;
}

void tableForm(){															// Table to process the data as:
	int pc = 0;																//  Label | Mnemonic | Operand | Operand Type(Hex, Oct etc)
	for(int i = 0; i < (int) cleaned.size(); i++){							// Store the data in above form in table vector
		string ans[10] = {"", "", "", ""}, cur = "";
		int ptr = 1;
		for(int j = 0; j < (int) cleaned[i].size(); j++){
			if(cleaned[i][j] == ':'){
				ans[0] = cur;
				cur = "";
				j++;
				continue;
			}
			else if(cleaned[i][j] == ' '){
				ans[ptr++] = cur;
				cur = "";
				continue;
			}
			cur += cleaned[i][j];
			if(j == (int)cleaned[i].size() - 1)
				ans[ptr++] = cur;
		}
		if(!ans[1].empty()){
			data[i].labelPresent = true;
		}
		else{
			data[i].labelPresent = false;
		}
		if(ans[1] == "HALT")
			haltPresent = true;
		if(!ans[0].empty())
			labels[ans[0]] = pc;
		pC[i] = pc;
		if(ptr == 1){
			fillData(i, ans[0], "", "", 0);
			continue;
		}
		pc++;		
		if(!OPC.count(ans[1])){
			genErrors(i + 1, "Invalid Mnemonic");
			continue;
		}
		if(min(OPC[ans[1]].second, 1) != ptr - 2){
			genErrors(i + 1, "Invalid OPCode-Syntax combination");
			continue;
		}
		fillData(i, ans[0], ans[1], ans[2], calType(ans[2]));
		if(data[i].oprType == 1 and !labels.count(data[i].operand)){
			genErrors(i + 1, "No such data/label variable");
		}
		else if(data[i].oprType == -1){
			genErrors(i + 1, "Invalid number");
		}
	}
}

void makeDataSegment(){													// Separate the code in DATA segment to execute instructions properly
	vector<string> instr, dataseg;
	for(int i = 0; i < (int)cleaned.size(); i++){
		bool state = false;
		for(int j = 0; j < cleaned[i].size(); j++){
			if(cleaned[i].substr(j, 4) == "data" and j + 4 < cleaned[i].size()){
				dataseg.push_back(cleaned[i]);
				state = true;
				break;
			}
			if(cleaned[i].back() == ':' and i + 1 < (int)cleaned.size() and cleaned[i + 1].substr(0, 4) == "data"){
				dataseg.push_back(cleaned[i]);
				state = true;
				break;
			}
		}
		if(!state)
			instr.push_back(cleaned[i]);
	}
	instr.insert(instr.end(), dataseg.begin(), dataseg.end());
	cleaned = instr;
}

void firstPass() {													// 1st pass of assembler which uses the functions declared above
	ifstream infile;
	cout << "Enter file name:" << endl;
	cin >> fileName;
	infile.open(fileName);
	if(infile.fail()){
		cout << "Input file doesn't exist" << endl;
		exit(0);
	}
	string s;
	while(getline(infile, s)) {
		string cur = clean(s, (int) cleaned.size());
		cleaned.push_back(cur);
	}
	init();								// Initialize the mnemonics
	processLabel();						// Process labels in the code Ex: var1:
	if(errors.empty())
		impSET();					// Implement SET mnemonic
	data.resize((int) cleaned.size()); 	// Allocate memory for table
	pC.resize((int) cleaned.size());		//Allocate memory for program counter array
	makeDataSegment();					// Separate the code in data segment and code
	tableForm();						// Make the code in table form as states in the problem
}

bool seeErrors() {						// Store the errors or warnings in the file: logFile.log
	ofstream outErrors("logFile.log");
	outErrors << "Log code generated in: logFile.log" << endl;
	if(errors.empty()){
		cout << "No errors found!" << endl;
		if(haltPresent == false){
			cout << "1 warning detected" << endl;
			outErrors << "Warning: HALT not present!" << endl;
		}
		outErrors << "Machine code generated in: machineCode.o" << endl;
		outErrors << "Listing code generated in: listCode.l" << endl;
		outErrors.close();
		return true;
	}
	sort(errors.begin(), errors.end());
	cout << (int)errors.size() << " errors encountered! See logFile.log" << endl;
	for(auto to: errors){
		outErrors << to.second << endl;
	}
	outErrors.close();
	return false;
}

string appZero(string s, int sz = 6){			// Small function to append zero at end to make it of 24 bit
	reverse(s.begin(), s.end());
	while((int) s.size() < sz)
		s += '0';
	reverse(s.begin(), s.end());	
	return s;
}

void secPass() {								// 2nd pass of assembler,converts the code to machine code and also generates Listing code
	for(int i = 0; i < (int) data.size(); i++){
		if(cleaned[i].empty()){
			continue;
		}
		string location = appZero(decToHex(pC[i]));		
		if(data[i].mnemonic == ""){
			string curMacCode = "        ";
			machineCode.push_back({i, curMacCode});
			continue;
		}
		if(data[i].oprType == 1){
			int decForm;
			if(OPC[data[i].mnemonic].second == 2){
				int val = labels[data[i].operand];
				decForm = val - (pC[i] + 1);	
			}
			else{
				decForm = labels[data[i].operand];
			} 
			string curMacCode = appZero(decToHex(decForm)) + OPC[data[i].mnemonic].first;
			machineCode.push_back({i, curMacCode});
		}
		else if(data[i].oprType == 0){
			string curMacCode = "000000" + OPC[data[i].mnemonic].first;
			machineCode.push_back({i, curMacCode});
		}
		else{
			int sz = 6, add = 24;
			if(data[i].mnemonic == "data")
				sz = 8, add = 32;
			int decForm = stoi(data[i].operand, 0, data[i].oprType);
			string curMacCode = appZero(decToHex(decForm, add), sz) + OPC[data[i].mnemonic].first;
			machineCode.push_back({i, curMacCode});
		}
	}
}

void writeToFile(){														// 	Write machine code and to the file:
	ofstream outList("listCode.l");										// 1. Listing Code: listCode.txt
	for(auto to: machineCode){											// 2. Machine code: machineCode.txt
		outList << appZero(decToHex(pC[to.first])) << " " << to.second << " " << cleaned[to.first] << endl;
	}
	outList.close();
	ofstream outMachineCode;
	outMachineCode.open("machineCode.o",ios::binary | ios::out);
	for(auto to: machineCode){ 
		unsigned int x;
		if(to.second.empty() or to.second == "        ")
			continue;
    	stringstream ss;
    	ss << hex << to.second;
    	ss >> x; 
     	static_cast<int>(x);
 		outMachineCode.write((const char*)&x, sizeof(unsigned int));
	}
	outMachineCode.close();
	cout << "Log code generated in: logFile.log" << endl;
	cout << "Machine code generated in: machineCode.o" << endl;
	cout << "Listing code generated in: listCode.l" << endl;
}

int main() {	
	firstPass();
	if(seeErrors()){
		secPass();
		writeToFile();
	}
	system("pause");
	return 0;
}

bool isOctal(string s){									// Funtion to check whether the string is Octal or not
    if((int) s.size() < 2)
    	return false;
    bool ok = true;
    for(int i = 1; i < (int)s.size(); i++){
        ok &= (s[i] >= '0' and s[i] <= '7');
    }
    return ok & (s[0] == '0');
}
bool isHexa(string s){							// Check if the string is in HexaDecimal form 
    bool state = true ;
    if((int)s.size() < 3)
    	return false;
    state &= (s[0] == '0') & ((s[1] == 'x' or s[1] == 'X'));
    for(int i = 2; i < (int) s.size(); i++){
        bool st = (s[i] >= '0' and s[i] <= '9');
        st |= ((s[i] >= 'a' and s[i] <= 'f') or ((s[i] >= 'A' and s[i] <= 'F')));
        state &= st;
    }
    return state;
}
bool isDec(string s){								// Check if the string is in decimal form
	bool ok = true;
	for(int i = 0; i < (int)s.size(); i++)
		ok &= (s[i] >= '0' and s[i] <= '9');
	return ok;
}

bool validName(string cur){								// Check if the label name is valid. Ex: 1var is invalid, val1 is valid
	for(auto to: cur){
		if((to >= 'a' and to <= 'z') or (to >= 'A' and to <= 'Z') or (to >= '0' and to <= '9') or (to == '_'))
			continue;
		return false;
	}
	if((cur[0] >= 'a' and cur[0] <= 'z') or (cur[0] >= 'A' and cur[0] <= 'Z') or (cur[0] == '_'))
		return true;
	return false;
}