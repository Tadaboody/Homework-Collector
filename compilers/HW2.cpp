
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <utility>
#include <memory>
#include <stack>
#include <locale>
using namespace std;
class SymbolTable;
class Function;
class AST;
class Variable;
class Array;
class Pointer;
// Abstract Syntax Tree
#define data head->value
enum ControlScope{While,Switch,None};
void generatePCode(AST * ast, SymbolTable & symbolTable);
void code(AST * head, SymbolTable & table,int loop_num,ControlScope scope);
void execute_code(AST * head, SymbolTable & table,int loop_num,ControlScope scope);
void load_expression(AST * head, SymbolTable & table);
string load_variable(AST * head, SymbolTable & table);
int generate_cases(AST* head, SymbolTable& table,int switch_num);
void access_shift(AST* indexList, SymbolTable& table,const Array& array);

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
public:
	string type;
	address var_address;
	size_t size;
	int offset;
	string passing_type;
	Variable() {}
	Variable(address open_address, string type,int size,int offset):
	var_address(open_address),type(type),offset(offset)
	{
		this->var_address = open_address;
		this->type = type;
		this->size = size;
	}
	virtual ~Variable(){}
};

class Array : public Variable
{
	public:
		int subpart;
		int member_type_size;
		string member_type;
		vector<pair<int,int>>* dimensions;
		Array(address open_address,string type,int offset, string member_type ,int member_type_size, vector<pair<int,int>>* dimensions)
		:Variable(open_address,type,calculate_size(*dimensions,member_type_size),offset)
		{
			this->member_type = member_type;
			this->dimensions = dimensions;
			this->member_type_size = member_type_size;
			calculate_subpart();
		}
		static int calculate_size(const vector<pair<int,int>>& dimensions,int member_type_size)
		{
			int size =member_type_size;
			for(const pair<int,int>& range : dimensions)
			{
				size*= (range.second - range.first + 1);
			}
			return size;
		}
	private:
	inline int calculate_subpart()
	{
		subpart = 0;
		int volume = member_type_size;
		for(const pair<int,int>& range : *this->dimensions)
		{
			subpart+=range.first*volume;
			volume*= (range.second - range.first +1);
		}
		return subpart;
	}
};
class Pointer : public Variable
{
	public:
	string member_type;
	Pointer(address open_address, string type,int size,int offset,string member_type)
		:Variable(open_address,type,size,offset)
		,member_type(member_type)
		{}
};

class SymbolTable {
public:
	address free_address;
	map<string, Variable*> variable_table;
	Function* pred_tree;
	SymbolTable(Function* owner):free_address(5),pred_tree(owner)
	{	}
	Variable& operator[](string name) { return *this->variable_table[name]; }
	static SymbolTable generateSymbolTable(AST* tree); 
	map<string,int> size_table = 
	{
		{"int",1},
		{"bool",1},
		{"real",1},
		{"pointer",1}
	};

	static int fillSymbolTable(SymbolTable& table, AST* head,address scope_base=0)
	{
		int scope_size = 0;
		if (head->left != nullptr)
		{
			scope_size += fillSymbolTable(table, head->left,scope_base);
		}
		AST* var = head->right;
		string type = var->right->value;
		string identifier = var->left->left->value;
		address var_address = table.free_address;
		int offset = var_address - scope_base;
		Variable* new_var;
		if(type=="array")
		{
			string member_type = var->right->right->value;
			if(member_type == "identifier")
				member_type = var->right->right->left->value;//botchy
			int member_type_size = table.size_table[member_type];
			vector<pair<int,int>>* dimensions= new vector<pair<int,int>>();
			for(AST* rangeList = var->right->left;rangeList!=nullptr;rangeList=rangeList->left)
			{
				AST* range = rangeList->right;
				int start = stoi(range->left->left->value);
				int end = stoi(range->right->left->value);
				dimensions->push_back(make_pair(start,end));
			}
			new_var =  new Array(table.free_address,type,offset,member_type,member_type_size,dimensions);
		}else if(type=="record")
		{
			int size = fillSymbolTable(table,var->right->left,var_address);
			new_var = new Variable(var_address,type,size,offset);
		}
		else if(type=="pointer")
		{
			string member_type = var->right->left->left->value;
			new_var = new Pointer(table.free_address,type,table.size_table[type],offset,member_type);
		}
		else
		{
			new_var = new Variable(table.free_address,type,table.size_table[type],offset);
		}
		new_var->passing_type = var->value;//"var","byValue","byReference". quick fix so I don't change constructor. shows that I needed a factory all the while
		table.variable_table[identifier] = (new_var);
		table.free_address += type=="record"?0:table[identifier].size;
		table.size_table[identifier] = table[identifier].size;
		return table[identifier].size+scope_size;
	}
	// void merge(const SymbolTable& other_table) doesn't work because you can't copy unique pointers in copy constructors.
	// {
	// 	//https://stackoverflow.com/questions/3639741/merge-two-stl-maps
	// 	// variable_table.insert(other_table.variable_table.begin(),other_table.variable_table.end());//map::merge is c++17 https://stackoverflow.com/questions/3639741/merge-two-stl-maps
	// }
};
class Function {//the function you declare, not the one you pass as a var
private:
int find_extreme_pointer();
public:
	Function(AST* function_head,Function* static_link);
	Function* static_link;
	vector<Function*> children;
	enum ArgumentType{ByValue,ByReference};
	string name;
	SymbolTable table;
	SymbolTable arguments;
	AST* statementsListHead;
	void print_decleration();
	void print_body();
	// void createCall();
};
SymbolTable SymbolTable::generateSymbolTable(AST* tree) {
		// TODO: create SymbolTable from AST
		Function* main = new Function(tree,nullptr);
		return main->table;
	}
int Function::find_extreme_pointer()
{
	return 1;
}
Function::Function(AST* function_head,Function* static_link):table(this),arguments(this)
{
	AST* id_and_parameters = function_head->left;
	AST* parameters_list = id_and_parameters->right->left;
	name = id_and_parameters->left->left->value;
	this->static_link = static_link;
	table = SymbolTable(this);
	if(static_link != nullptr){
		table.size_table = static_link->table.size_table;//"inherit" types
		arguments.size_table = table.size_table;
	}
	AST* content = function_head->right;
	statementsListHead = content->right;
	AST* scope = content->left;
	if(scope != nullptr){
		SymbolTable::fillSymbolTable(table,scope->left);
	if(parameters_list != nullptr)
	{
		SymbolTable::fillSymbolTable(arguments,parameters_list);
		SymbolTable::fillSymbolTable(table,parameters_list);//is fine because I never use the size of this table, just the argument table
		//arguments
	}
	// if(function_head->value == "function")
	// {
		//create a fake node for the return value,insert it into the table and 
	// 	table.insert()
	// }
	AST* functionsList = scope->right;
	for(AST* functionsList = scope->right;functionsList!=nullptr;functionsList=functionsList->left)
	{
		children.push_back(new Function(functionsList->right,this));
	}
}
}

string toupper(const string& str)
{
	string ret = string(str);
	locale loc;
	for (std::string::size_type i=0; i<str.length(); ++i)
		ret[i] = toupper(ret[i],loc);//http://www.cplusplus.com/reference/locale/toupper/
	return ret;
}

void Function::print_decleration()
{
	int sp = arguments.free_address;
	int ep = find_extreme_pointer();
	cout << toupper(name) << ":" << endl;
	cout << "ssp " << sp << endl;
	cout << "sep " << ep << endl;
	cout << "ujp " << toupper(name) << "_begin" << endl;
	for(Function* fun:children)
	{
		fun->print_decleration();
	}
}

void Function::print_body()
{
	cout << toupper(name) << "_begin:" << endl;
	
	for(Function* fun:children)
	{
		fun->print_body();
	}	
}
void generatePCode(AST* ast, SymbolTable& symbolTable) {
	// TODO: go over AST and print cod
	Function* main = symbolTable.pred_tree;
	main->print_decleration();
	main->print_body();
}
void code(AST* head, SymbolTable& table,int loop_num,ControlScope scope) //gets StatementList
{
	if (head == nullptr)
	{
		return;
	}

	if (head->left != nullptr)
		code(head->left, table,loop_num,scope);
	execute_code(head->right, table,loop_num,scope);

}
int label_num = 0;


void execute_code(AST* head, SymbolTable& table,int loop_num,ControlScope scope)
{

	if (data == "if" && head->right->value == "else")
	{
		int if_label_num = label_num++;
		int else_label_num = label_num++;
		load_expression(head->left,table);
		head = head->right;//jump to else node
		cout << "fjp skip_if_" << if_label_num << endl;
		code(head->left,table,else_label_num,scope);
		cout << "ujp skip_else_" << else_label_num << endl;
		cout << "skip_if_" << if_label_num << ':' << endl;
		code(head->right, table,else_label_num,scope);
		cout << "skip_else_" << else_label_num << ':' << endl;
	}

	else if (data == "if")
	{
		int la = label_num++;
		load_expression(head->left, table);
		cout << "fjp " <<  "skip_if_" << la << endl;
		code(head->right, table,loop_num,scope);
		cout << "skip_if_" << la << ':' << endl;

	}
	if (data == "while")
	{
		int loop = label_num++;
		int after_loop = label_num++;
		cout << "while_" << loop << ':' << endl;
		load_expression(head->left, table);
		cout << "fjp " << "while_out_" << after_loop  << endl;
		code(head->right, table, after_loop,While);
		cout << "ujp " << "while_" << loop << endl;
		cout << "while_out_" << after_loop << ':' << endl;

	}
	if(data == "switch")
	{
		int global_switch_num = label_num;
		int switch_num = global_switch_num++;
		load_expression(head->left,table);
		cout << "neg" << endl;
		cout << "ixj switch_end_" << switch_num << endl;
		int number_of_cases = generate_cases(head->right,table,switch_num);
		for(;number_of_cases>0;number_of_cases--)
		{
			cout << "ujp case_" << switch_num << '_' << number_of_cases << endl;
		}
		
		cout << "switch_end_" << switch_num << ':' << endl;
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
	if(data == "break")
	{
		switch(scope)
		{
			case While: cout << "ujp while_out_" << loop_num << endl;break;
			case Switch:cout << "ujp switch_end_" << loop_num << endl; break;
		}
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

	if (data == "identifier"||data == "array" || data=="pointer" || data=="record")
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
//codel
string load_variable(AST* head, SymbolTable& table) //TODO: put pointer/struct access here
{
	if(data == "identifier"){
		cout << "ldc " << table[head->left->value].var_address << endl;
		return head->left->value;
	}
	else if(data == "pointer")
	{
		string type = load_variable(head->left,table);
		cout << "ind" << endl;
		return dynamic_cast<Pointer&>(table[type]).member_type;
	}else if(data == "array")
	{
		string type = load_variable(head->left,table);
		// string identifier = head-
		const Array& array = dynamic_cast<Array&>(table[type]);
		access_shift(head->right,table,array);
		return array.member_type;
	}else if(data=="record")
	{
		string type = load_variable(head->left,table);
		string member_type = head->right->left->value;
		cout << "inc " << table[member_type].offset << endl;
		return member_type;
	}
	return "Shouldn't get here";
}
//codei
void access_shift(AST* indexList, SymbolTable& table,const Array& array)
{
	int accumilated_size = Array::calculate_size(*array.dimensions,array.member_type_size);
	stack<AST*> backtrace = stack<AST*>();
	for(int i=0;indexList!=nullptr;indexList=indexList->left,i++)
	{
		backtrace.push(indexList);
	}
	for(int i=backtrace.size()-1;!backtrace.empty();i--,backtrace.pop())
	{
		indexList = backtrace.top();
		accumilated_size/= ((*array.dimensions)[i].second -(*array.dimensions)[i].first +1);
		load_expression(indexList->right,table);
		cout << "ixa " << accumilated_size << endl;
	}
	cout << "dec " << array.subpart << endl;

}
int generate_cases(AST* head, SymbolTable& table,int switch_num)
{
	int number_of_cases;
	if(head->left != nullptr)
		number_of_cases = generate_cases(head->left,table,switch_num) + 1;
	else
		number_of_cases = 1;
	AST* case_node = head->right;
	cout << "case_" << switch_num << '_' <<  case_node->left->left->value << ':' << endl;//case->constInt->value
	code(case_node->right,table,switch_num,Switch);
	cout << "ujp switch_end_" << switch_num << endl;
	return number_of_cases;
}
/*
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
*/

int main(int argc, char** argv)
{

	AST* ast;
	ifstream myfile;
	if(argc > 1)//if there are command line arguments (argv[0] = program name)
	{
		myfile = ifstream(argv[1]);
	}
	else
	{
		string input;
		cout << "No CL arguments,enter tree path:";
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
	else cout << "Unable to open file" << endl;
	return 0;
}

