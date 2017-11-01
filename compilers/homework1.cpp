#include <iostream>
#include <string>
#include <fstream>
#include <functional>
#include <unordered_map>
using namespace std;

// Abstract Syntax Tree
class AST
{
	string _value;
	AST* _left; // can be null
	AST* _right; // can be null

public:

	AST(string value, AST* left, AST* right) {
		_value = value;
		_left = left;
		_right = right;
	}
	static AST* createAST(ifstream& input) {
		if (!(input))
			return nullptr;

		string line;
		getline(input, line);
		if (line == "~")
			return nullptr;

		return new AST(line, createAST(input), createAST(input));
	}
};
typedef string types;
typedef unsigned int address;
class Variable {
	// Think! what does a Variable contain?
	//"CORRECTER" enum types {Int,Real,Bool};
public:
	string type;
	address var_address;
	Variale(address address,string type):address(address),type(type)
	{

	}
};

class SymbolTable {
	// Think! what does a SymbolTable contain?
	//std::unordered_map<
	void
public:
	static SymbolTable generateSymbolTable(AST* tree) {
		// TODO: create SymbolTable from AST
		unsigned int free_adress = 5;
		unordered_map<string,Variable> variavle_table;
	}
};
unordered_map<string,function> commands = {
	{"program", [] () { cout << "a";}}
}
void generatePCode(AST* ast, SymbolTable symbolTable) {
	// TODO: go over AST and print code

}


int main(int argc,char** argv)
{
	AST* ast;
	cout << argv[1]  << ' ' << endl;
	ifstream myfile(argv[1]);
	if (myfile.is_open())
	{
		ast = AST::createAST(myfile);
		myfile.close();
		SymbolTable symbolTable = SymbolTable::generateSymbolTable(ast);
		generatePCode(ast, symbolTable);
	}
	else cout << "Unable to open file";
	return 0;
}
