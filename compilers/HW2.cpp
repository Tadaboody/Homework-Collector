
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
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
class FunVar;
// Abstract Syntax Tree
#define data head->value
enum ControlScope{While,Switch,None};
void generatePCode(AST * ast, SymbolTable & symbolTable);
void code(AST * head, SymbolTable & table,int loop_num,ControlScope scope);
void execute_code(AST * head, SymbolTable & table,int loop_num,ControlScope scope);
void load_expression(AST * head, SymbolTable & table);
string load_variable(AST * head, SymbolTable & table);
int generate_cases(AST* head, SymbolTable& table,int switch_num);
void access_shift(AST* indexList, SymbolTable& table,const Array* array);
void call_function(AST* call,SymbolTable& table);
string toupper(const string& str);
int label_num = 0;

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
	address var_address;
	size_t size;
	int offset;
	int depth;
	string type;
	string argument_type;
	Variable() {}
	Variable(address open_address, string type,int size,int offset):
	var_address(open_address),offset(offset), type(type)
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
	Function* owner;
	SymbolTable(Function* owner):free_address(5),owner(owner)
	{	}
	Variable& operator[](string name) {return *this->variable_table[name];}
	const Variable* get_variable(const string& name);
	static SymbolTable generateSymbolTable(AST* tree); 
	map<string,int> size_table = 
	{
		{"int",1},
		{"bool",1},
		{"real",1},
		{"pointer",1}
	};

	static int fillSymbolTable(SymbolTable& table, AST* head,address scope_base);
	const Variable* get_variable(int address);
	// void merge(const SymbolTable& other_table) doesn't work because you can't copy unique pointers in copy constructors.
	// {
	// 	//https://stackoverflow.com/questions/3639741/merge-two-stl-maps
	// 	// variable_table.insert(other_table.variable_table.begin(),other_table.variable_table.end());//map::merge is c++17 https://stackoverflow.com/questions/3639741/merge-two-stl-maps
	// }
};
const Variable* SymbolTable::get_variable(int address)
{
	for (pair<string, Variable *> var : variable_table)
	{
		if (var.second->var_address == address)
			return var.second;
	}
	return nullptr;
}
class FunVar : public Variable
{
	public:
	const Function* static_link;
	FunVar(address open_address, string type,int size,int offset,const Function* static_link):Variable(open_address,type,size,offset),static_link(static_link)
	{}
};
class Function {//the function you declare, not the one you pass as a var
private:
  int find_extreme_pointer();
public:
	SymbolTable table;
	SymbolTable arguments;
	int depth;
	string fun_type;
	Function(AST* function_head,Function* static_link);
	Function* static_link;
	vector<Function*> children;
	enum ArgumentType{ByValue,ByReference};
	string name;
	AST* statementsListHead;
	void print_decleration();
	void print_body();
	const inline int stack_pointer() const {return table.free_address;}
	const inline int argument_size() const {return arguments.free_address - 5;}
	const Function* find_function(const string& name) const;
	inline string function_label() const {return toupper(name);}
	// void createCall();
};
const Function* Function::find_function(const string& name)const
{
	if(name == this->name)
		return this;
	// const Variable* var = table.get_variable(name);
	// const FunVar* fun = dynamic_cast<const FunVar*>(var);
	// if(fun != nullptr)
	// {
	// 	return fun->static_link;
	// }
	for(Function* fun:children)
	{
		if(name == fun->name)
			return fun;
	}
	if(static_link !=nullptr)
		return static_link->find_function(name);
	else
		return nullptr;
}
SymbolTable SymbolTable::generateSymbolTable(AST* tree) {
		// TODO: create SymbolTable from AST
		Function* main = new Function(tree,nullptr);
		return main->table;
	}
	int SymbolTable::fillSymbolTable(SymbolTable& table, AST* head,address scope_base=0)
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
		if(type=="identifier")
		{
			const Function* static_link = table.owner->find_function(var->right->left->value);
			if(static_link != nullptr)
			{
				new_var = new FunVar(var_address,type,2,offset,static_link);
			}
			else{
				new_var = new Variable(*table.get_variable(var->right->left->value));
				new_var->var_address = var_address;
			}
		}
		else if(type=="array")
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
		if(identifier==table.owner->name)
		{
			new_var->var_address = 0;
		}
		new_var->argument_type = var->value;//"var","byValue","byReference". quick fix so I don't change constructor. shows that I needed a factory all the while
		if(var->value == "byReference")
		{
			new_var->size = 1;
		}
		new_var->depth = table.owner->depth;
		table.variable_table[identifier] = (new_var);
		table.free_address += type=="record"||identifier==table.owner->name?0:table[identifier].size;
		table.size_table[identifier] = table[identifier].size;

		return table[identifier].size+scope_size;
	}
	inline int max(int a, int b)
	{
		return a>b?a:b;
	}

int Function::find_extreme_pointer()
	{
		int ret = 0;
		int sp = 0;
		map<string, int> command_known =
			{
				{"ldc", 1},
				{"sub", -1},
				{"add", -1},
				{"div", -1},
				{"mul", -1},
				{"and", -1},
				{"neg", 0},
				{"not", 0},
				{"geq", -1},
				{"leq", -1},
				{"les", -1},
				{"grt", -1},
				{"equ", -1},
				{"neq", -1},
				{"ujp", 0},
				{"fjp", -1},
				{"ixj", -1},
				{"print", -1},
				{"ind", 0},
				{"lda", 1},
				{"inc", 0},
				{"dec", 0},
				{"ixa", -1},
				{"mstf", 5},
				{"mst", 5},
				{"dpl", 1},
				{"lod", 1},
				{"lda", 1},
				{"str", -1},
				{"sto", -2}};
		//redirect stdout into output
		stringstream output;
		streambuf *backup_buf;
		backup_buf = cout.rdbuf();
		cout.rdbuf(output.rdbuf());
		code(statementsListHead, table, 0, None);
		label_num = 0;
		cout.rdbuf(backup_buf);

		string line;
		int MP; //temp sp when calling a function
		while (getline(output, line))
		{
			string command = line.substr(0,line.find(' ')); //str.split()
			if (command_known.find(command) != command_known.end())
			{
				sp += command_known[command];
			}
			else
			{
				string arg = line.substr(line.rfind(' ') + 1);
				if(command=="smp")
				{
					sp -= stoi(arg);
				}
				if(command == "movs")
				{
					sp+=stoi(arg)-1;
				}
				if(command == "cup")
				{
					string p1 = line.substr(line.find(' ') + 1);
					string p = p1.substr(0, p1.find(' '));
					sp -= stoi(p);//pop args
					locale loc;
					string fun_name = " ";
					fun_name[0] = tolower(arg[0],loc);
					const Function *callee = find_function(fun_name);
					if(callee->fun_type == "function")
					{
						sp+=1;//push return value;
					}
					sp -= 5; //pop descriptor
				}
				if(command == "cupi")
				{
					const Function *callee = dynamic_cast<const FunVar *>(table.get_variable(stoi(arg)))->static_link;
					if (callee->fun_type == "function")
						sp += 1;
				}
			}
			if (ret < sp)
				ret = sp;
		}
		return ret;
	}

	Function::Function(AST *function_head, Function *static_link) : table(this), arguments(this), depth(0), fun_type(function_head->value)
	{
		AST *id_and_parameters = function_head->left;
		AST *inOutParameters = id_and_parameters->right;
		AST *parameters_list = inOutParameters->left;
		name = id_and_parameters->left->left->value;
		this->static_link = static_link;
		table = SymbolTable(this);
		if (static_link != nullptr)
		{
			table.size_table = static_link->table.size_table; //"inherit" types
			arguments.size_table = table.size_table;
			depth = static_link->depth + 1;
		}
		AST *content = function_head->right;
		statementsListHead = content->right;
		AST *scope = content->left;
		if (parameters_list != nullptr)
		{
			SymbolTable::fillSymbolTable(arguments, parameters_list);
			SymbolTable::fillSymbolTable(table, parameters_list);
			//arguments
		}
		if (scope != nullptr)
		{
			if (scope->left != nullptr)
				SymbolTable::fillSymbolTable(table, scope->left);
			// if(function_head->value == "function")
			// {
			//create a fake node for the return value,insert it into the table and
			// 	table.insert()
			// }
			stack<AST *> backtrace = stack<AST *>();
			for (AST *functionsList = scope->right; functionsList != nullptr; functionsList = functionsList->left)
			{
				backtrace.push(functionsList);
			}
			for (; !backtrace.empty(); backtrace.pop())
			{
				AST *functionsList = backtrace.top();
				children.push_back(new Function(functionsList->right, this));
			}
		}
		if (fun_type == "function")
		{
			string type = inOutParameters->right->value;
			AST *declarationsList = new AST("declarationsList", new AST("var", new AST(type, nullptr, nullptr), new AST("identifier", nullptr, new AST(name, nullptr, nullptr))), nullptr);
			SymbolTable::fillSymbolTable(table, declarationsList);
			delete declarationsList;
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
	int sp = stack_pointer();
	int ep = find_extreme_pointer();
	cout << function_label() << ":" << endl;
	cout << "ssp " << sp << endl;
	cout << "sep " << ep << endl;
	cout << "ujp " << toupper(name) << "_begin" << endl;
	for(Function* fun:children)
	{
		fun->print_decleration();
	}
	print_body();
}

void Function::print_body()
{
	// for(Function* fun:children)
	// {
	// 	fun->print_body();
	// }	
	cout << function_label() << "_begin:" << endl;
	code(statementsListHead,table,0,None);
	map<string,string> stop = {
		{"program","stp"},
		{"function","retf"},
		{"procedure","retp"}
		};
	cout << stop[fun_type] << endl;
}
	const Variable* SymbolTable::get_variable(const string& name) {
		if(variable_table.find(name) != variable_table.end()){
			return this->variable_table[name];
		}else{
			if(owner->static_link == nullptr){return nullptr;}
			return owner->static_link->table.get_variable(name);
		}
		return nullptr;
	
	}
void generatePCode(AST* ast, SymbolTable& symbolTable) {
	// TODO: go over AST and print cod
	Function* main = symbolTable.owner;
	main->print_decleration();
	// main->print_body();
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


void execute_code(AST* head, SymbolTable& table,int loop_num,ControlScope scope)
{

	if (data == "if" && head->right->value == "else")
	{
		int if_label_num = label_num++;
		int else_label_num = label_num++;
		load_expression(head->left,table);
		head = head->right;//jump to else node
		cout << "fjp L" << if_label_num << endl;
		code(head->left,table,else_label_num,scope);
		cout << "ujp L" << else_label_num << endl;
		cout << "L" << if_label_num << ':' << endl;
		code(head->right, table,else_label_num,scope);
		cout << "L" << else_label_num << ':' << endl;
	}

	else if (data == "if")
	{
		int la = label_num++;
		load_expression(head->left, table);
		cout << "fjp " <<  "L" << la << endl;
		code(head->right, table,loop_num,scope);
		cout << "L" << la << ':' << endl;

	}
	if (data == "while")
	{
		int loop = label_num++;
		int after_loop = label_num++;
		cout << "L" << loop << ':' << endl;
		load_expression(head->left, table);
		cout << "fjp " << "L" << after_loop  << endl;
		code(head->right, table, after_loop,While);
		cout << "ujp " << "L" << loop << endl;
		cout << "L" << after_loop << ':' << endl;

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
			case None:cout << "Unbreakable" << endl;break;
		}
	}
	if(data == "call")
	{
		call_function(head,table);
	}
}

void load_args(AST* argument_list,SymbolTable& table,const Function* callee)
{
		map<string,Variable*> args = callee->arguments.variable_table;
		for(pair<const string,Variable*>& arg:args)
		{
			Variable* var = arg.second;
			if(var->argument_type == "byReference"){
				load_variable(argument_list->right,table);
			}
			if(var->argument_type == "byValue")
			{
				if (var->size > 1)
				{
					FunVar* fun = dynamic_cast<FunVar*>(var);
					if (fun != nullptr)
					{
						string arg_name = argument_list->right->left->value;
						if(table.get_variable(argument_list->right->left->value) != nullptr)
							load_variable(argument_list->right,table);
						else
							cout << "ldc " << callee->find_function(arg_name)->function_label() << endl;
						cout << "lda " << fun->static_link->depth -1 -table.owner->depth << " 0" << endl;
					}else{
						load_variable(argument_list->right, table);
						cout << "movs " << var->size << endl;
				}
				}
				else
					load_expression(argument_list->right,table);
			}
			argument_list = argument_list->left;
		}
}

void call_function(AST* call, SymbolTable& table)
{
	string fun_id = call->left->left->value;
	Function* caller = table.owner;
	const FunVar* function_variable = dynamic_cast<const FunVar*>(table.get_variable(fun_id));
	if(function_variable != nullptr)
	{
		const Function* callee = function_variable->static_link; 
		cout << "mstf " << caller->depth - function_variable->depth << " " << function_variable->var_address << endl;
		load_args(call->right,table, function_variable->static_link);
		cout << "smp " << callee->argument_size() << endl;
		cout << "cupi " << caller->depth - function_variable->depth << " " << function_variable->var_address << endl;
	}else{
	const Function* callee  = caller->find_function(fun_id);
	cout << "mst " << caller->depth - callee->depth+1 << endl;
	load_args(call->right,table,callee);
	cout << "cup " << callee->argument_size() << ' ' << callee->function_label() << endl;
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
	if(data == "call")
	{
		call_function(head,table);
	}

}
//codel


string load_variable(AST* head, SymbolTable& table) //TODO: put pointer/struct access here
{
	if(data == "identifier"){
		const Function* fun = table.owner->find_function(head->left->value);
		const Variable* var = table.get_variable(head->left->value);
		cout << "lda " << table.owner->depth - var->depth << ' ' <<var->var_address << endl;
		if (var->argument_type == "byReference")
			cout << "ind" << endl;
		return head->left->value;
	}
	else if(data == "pointer")
	{
		string type = load_variable(head->left,table);
		cout << "ind" << endl;
		return dynamic_cast<const Pointer*>(table.get_variable(type))->member_type;
	}else if(data == "array")
	{
		string type = load_variable(head->left,table);
		const Array* array = dynamic_cast<const Array*>(table.get_variable(type));
		access_shift(head->right,table,array);
		return array->member_type;
	}else if(data=="record")
	{
		string type = load_variable(head->left,table);
		string member_type = head->right->left->value;
		cout << "inc " << table.get_variable(member_type)->offset << endl;
		return member_type;
	}
	return "Shouldn't get here";
}


//codei
void access_shift(AST* indexList, SymbolTable& table,const Array* array)
{
	int accumilated_size = Array::calculate_size(*array->dimensions,array->member_type_size);
	stack<AST*> backtrace = stack<AST*>();
	for(int i=0;indexList!=nullptr;indexList=indexList->left,i++)
	{
		backtrace.push(indexList);
	}
	for(int i=backtrace.size()-1;!backtrace.empty();i--,backtrace.pop())
	{
		indexList = backtrace.top();
		accumilated_size/= ((*array->dimensions)[i].second -(*array->dimensions)[i].first +1);
		load_expression(indexList->right,table);
		cout << "ixa " << accumilated_size << endl;
	}
	cout << "dec " << array->subpart << endl;

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

int main(int argc, char **argv)
{

	AST *ast;
	ifstream myfile;
	if (argc > 1) //if there are command line arguments (argv[0] = program name)
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

