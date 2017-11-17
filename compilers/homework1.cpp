
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>


using namespace std;



// Abstract Syntax Tree
class AST
{
public:
	string value;
	AST* left; // can be null
	AST* right; // can be null


	AST(string value, AST* right, AST* left) {
		this->value = value;
		this->left = left;
		this->right = right;
	}
	static AST* createAST(ifstream& input) {
		if (!(input))
			return nullptr;

		string line;
		getline(input, line);
		if (line == "~")
			return nullptr;
		AST* left = createAST(input);
		AST* right = createAST(input);
		return new AST(line, right, left);
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
	size_t size;
	Variable() {}
	Variable(address open_address, string type)
	{
		this->var_address = open_address;
		this->type = type;
		if (type == "int" || type == "real" || type == "bool")
		{
			this->size = 1;
		}
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
		SymbolTable return_table = SymbolTable();
		if (tree->value == "program")
			return generateSymbolTable(tree->right);
		if (tree->value == "content")
		{
			if(tree->left != nullptr)
			{
				AST* head = tree->left->left;
				fillSymbolTable(return_table, head);
			}
		}
		return return_table;
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
		table[identifier] = Variable(table.free_address, type);
		table.free_address += table[identifier].size;
	}
	void print_table()
	{
		for (auto const& imap : variable_table)
		{
			cout << imap.first << ',' << imap.second.var_address << endl;
		}
	}
};


void generatePCode(AST * ast, SymbolTable & symbolTable);
void code(AST * head, SymbolTable & table);
void execute_code(AST * head, SymbolTable & table);
void load_expression(AST * head, SymbolTable & table);
void load_variable(AST * head, SymbolTable & table);

void generatePCode(AST* ast, SymbolTable& symbolTable) {
	// TODO: go over AST and print code
	code(ast->right->right,symbolTable);
}
void code(AST* head, SymbolTable& table) //gets StatementList
{
	if (head == nullptr)
	{
		return;
	}

	if (head->left != nullptr)
		code(head->left, table);
	execute_code(head->right, table);

}
#define data head->value
int label_num = 0;
inline void print_label(const int& label_num)
{
	cout << "L" << label_num << ':' << endl;
}
void execute_code(AST* head, SymbolTable& table)
{

	if (data == "if" && head->right->value == "else")
	{
		int if_label_num = label_num++;
		int else_label_num = label_num++;
		load_expression(head->left,table);
		head = head->right;//jump to else node
		cout << "fjp L" << if_label_num << endl;
		code(head->left,table);
		cout << "ujp L" << else_label_num << endl;
		print_label(if_label_num);
		code(head->right, table);
		print_label(else_label_num);
	}

	else if (data == "if")
	{
		int la = label_num++;
		load_expression(head->left, table);
		cout << "fjp " <<  'L' << la << endl;
		code(head->right, table);
		print_label(la);

	}
	if (data == "while")
	{
		int loop = label_num++;
		int after_loop = label_num++;
		print_label(loop);
		load_expression(head->left, table);
		cout << "fjp " << 'L' << after_loop << endl;
		code(head->right, table);
		cout << "ujp " << 'L' << loop << endl;
		print_label(after_loop);

	}

	if (data == "print")
	{
		load_expression(head->left, table);
		cout << "print" << endl;
	}
	if (data == "assignment")
	{
		load_variable(head->left,table);
		load_expression(head->right,table);
		cout << "sto" << endl;
	}
}

void load_expression(AST* head, SymbolTable& table)
{
	map<string, string> operators =
	{
		{ "plus", "add" },
		{ "minus", "sub" },
		{ "multiply", "mul" },
		{ "divide", "div" },
		{ "lessThan" , "les" },
		{ "greaterThan" , "grt" },
		{ "lessOrEquals", "leq" },
		{ "greaterOrEquals", "geq" },
		{ "equals" , "equ" },
		{ "notEquals", "neq" },
		{ "and", "and" },
		{ "or" , "or" },
		{ "negative", "neg" },
		{ "not", "not" }
	};


	if (data.find("const") != string::npos)
		cout << "ldc " << head->left->value << endl;
	if (data == "true")
		cout << "ldc 1" << endl;
	if (data == "false")
		cout << "ldc 0" << endl;

	if (data == "identifier")
	{
		load_variable(head, table);
		cout << "ind" << endl;
	}

	if(operators.find(data) != operators.end() )
	{
		load_expression(head->left,table);
		if(head->right != nullptr) // binary operator
			load_expression(head->right, table);
		cout << operators[data] << endl;
	}

}

void load_variable(AST* head, SymbolTable& table) //TODO: put pointer/struct access here
{
	if(data == "identifier")
		cout << "ldc " << table[head->left->value].var_address << endl;
}

int main()
{
	AST* ast;
	SymbolTable symbolTable;
	ifstream myfile("tree7.txt");
	if (myfile.is_open())
	{
		ast = AST::createAST(myfile);
		myfile.close();
		symbolTable = SymbolTable::generateSymbolTable(ast);
		generatePCode(ast, symbolTable);
	}
	else cout << "Unable to open file";
	return 0;
}

/*
int main(int argc, char** argv)
{

	AST* ast;
	int x = 0;
	ifstream myfile;
	if(argc > 1)//if there are command line arguments (argv[0] = program name)
	{
		myfile = ifstream(argv[1]);
	}
	else
	{
		string input;
		cin >> input;
		myfile = ifstream(input);
	}
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
*/
