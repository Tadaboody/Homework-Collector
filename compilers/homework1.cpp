#include <iostream>
#include <string>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <algorithm>
using namespace std;

// Abstract Syntax Tree
class AST
{
public:
	string value;
	AST* left; // can be null
	AST* right; // can be null


	AST(string value, AST* left, AST* right) {
		value = value;
		left = left;
		right = right;
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
	Variable(address open_address,string type):var_address(open_address),type(type)
	{ }
};

class SymbolTable {
	// Think! what does a SymbolTable contain?
	//std::unordered_map<
public:
		address free_address;
		unordered_map<string,Variable> variable_table;
		SymbolTable()
		{
			variable_table = unordered_map<string,Variable>();
			free_address = 5;
		}
		Variable& operator[](string name){return this->variable_table[name];}
	static SymbolTable generateSymbolTable(AST* tree) {
		// TODO: create SymbolTable from AST
			if(tree->value == "program")
			return generateSymbolTable(tree->right->left);
			if(tree->value == "scope")
			{
				SymbolTable return_table = SymbolTable();
				for(AST* head=tree->left;head->left != nullptr; head = head->left)
				{
					AST* var = head->right;
					string identifier = var->left->left->value;//var->identifier->a
					string type = var->right->value;//var->real
					return_table[identifier] = Variable(return_table.free_address++,type);
				}
				return return_table;
			}
	}
	void print_table()
	{
	  for (std::unordered_map<string,Variable>::iterator it=variable_table.begin(); it!=variable_table.end(); ++it)
	    std::cout << it->first << " => " << it->second.var_address << '\n';
	}
};
void generatePCode(AST* ast, SymbolTable symbolTable) {
	// TODO: go over AST and print code
}


int main(int argc,char** argv)
{
	AST* ast;
	ifstream myfile(argv[1]);
	if (myfile.is_open())
	{
		ast = AST::createAST(myfile);
		myfile.close();
		SymbolTable symbolTable = SymbolTable::generateSymbolTable(ast);
		symbolTable.print_table();
		generatePCode(ast, symbolTable);
	}
	else cout << "Unable to open file";
	return 0;
}
