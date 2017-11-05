// Homework-Collector.cpp : Defines the entry point for the console application.
//
#ifdef _WIN32
	#include "stdafx.h"


#include <iostream>
#include <string>
#include <fstream>
#include <functional>
#include <map>
#include <vector>
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
		this->value = value;
		this->left = left;
		this->right = right;
	}

	static void printTree(AST* head)
	{
		if (head == nullptr)
		{
			cout << "~" << endl;
			return;
		}

		cout << head->value << endl;
		printTree(head->left);
		printTree(head->right);
	}

	static AST* createAST(fstream& input) {

		if (!(input))
			return nullptr;

		string line;
		getline(input, line);
		if(line.find('\r') != string::npos)//crutial for running on unix
			line.erase(line.end()-1);
		if (line == "~")
			return nullptr;
		AST* left = createAST(input);
		AST* right = createAST(input);
		return new AST(line, left, right);

	}
};
typedef string types;
typedef int address;
class Variable {
	// Think! what does a Variable contain?
	//"CORRECTER" enum types {Int,Real,Bool};
public:
	string type;
	address var_address;
	Variable() {}
	Variable(address open_address, string type)
	{
		this->var_address = open_address;
		this->type = type;
	}
};

class SymbolTable {
	// Think! what does a SymbolTable contain?
	//std::unordered_map<
public:
	address free_address;
	map<string, Variable> variable_table;
	SymbolTable()
	{
		variable_table = map<string, Variable>();
		free_address = 5;
	}
	Variable& operator[](string name) { return this->variable_table[name]; }
	static SymbolTable generateSymbolTable(AST* tree) {
		// TODO: create SymbolTable from AST
		if (tree->value == "program")
			return generateSymbolTable(tree->right);
		if (tree->value == "content")
		{
			SymbolTable return_table = SymbolTable();
			AST* head = tree->left->left;
			fillSymbolTable(return_table, head);
			return return_table;
		}
	}

	static void fillSymbolTable(SymbolTable& table, AST* head)
	{
		if (head->left != nullptr)
		{
			fillSymbolTable(table, head->left);
		}
		AST* var = head->right;
		string type = var->right->value;
		string identifier = var->left->left->value;
		table[identifier] = Variable(table.free_address++, type);
	}
	void print_table()
	{
		std::vector<string> firstv;
		std::vector<Variable> secondv;
		for (auto const& imap : variable_table)
		{
			cout << imap.first << ',' << imap.second.var_address << endl;
		}

	}
};
void generatePCode(AST* ast, SymbolTable symbolTable) {
	// TODO: go over AST and print code
}


int main(int argc, char** argv)
{

	AST* ast;
	int x = 0;
	fstream myfile;
	if(argc > 1)//if there are command line arguments (argv[0] = program name)
	{
		myfile = fstream(argv[1]);
	}
	else
	{
		string input;
		cin >> input;
		myfile = fstream(input);
	}
	if (myfile.is_open())
	{
		ast = AST::createAST(myfile);
		myfile.close();
		SymbolTable symbolTable = SymbolTable::generateSymbolTable(ast);
		symbolTable.print_table();
		generatePCode(ast, symbolTable);

	}
	else cout << "Unable to open file";

	cin >> x;
	return 0;
}
