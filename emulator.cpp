#include <bits/stdc++.h>

using namespace std;

string remove_spaces(string str);
void call_function(string name);
string get_function_name(string line);
int index_of(string str, char ch);
int to_int(string str);
bool is_number(string num);
void store(int* registers, char* stack, int SP, string command);
bool is_load_operation(string command);
bool is_special_register(string str);
void load(int* registers, char* stack, int SP, string command);

const string filename = "assembly_code.txt";

int main() {
    int *registers = new int[100];
    char *stack = new char[400];
    int SP = 0;
    int PC = 0;
    int RV = 0;
    vector<string> commands;
    map<string, int> functions;
    string line;
    ifstream my_file(filename);
    if (my_file.is_open()) {
        while (getline(my_file, line))
        {
            if (line.length() >= 2 && line[0] == '/' && line[1] == '/') continue;
            commands.push_back(line);
            //cout << line << '\n';
        }
        my_file.close();
    } else {
        cout << "Unable to open file";
    }

    string current_command;
    while (true) {
        current_command = commands[PC / 4];
        if (PC / 4 == commands.size()) break;
        if (current_command[current_command.length() - 1] == ':') {
            functions[current_command.substr(0, current_command.length() - 2)] = PC;
        }
        PC += 4;
    }

    PC = functions["main"];
    
    while (true) {
        current_command = remove_spaces(commands[PC / 4 + 1]);
        if (current_command == "RET") break;
        if (current_command[current_command.length() - 1] == '>') {
            call_function(get_function_name(current_command));
        } else if (index_of(current_command, 'M') == 0) {
            store(registers, stack, SP, current_command);
        } else if (is_load_operation(current_command)) {
            cout << current_command << endl;
            cout << "load" << endl;
            //load(registers, stack, SP, current_command);
        } else {
            //perform_operation();
        }
        PC += 4;
    }

    // cout << *(int*)(&stack[4]) << endl;

    return 0;
}

string remove_spaces(string str) {
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == ' ') {
            str = str.substr(0, i) + str.substr(i + 1);
            i--;
        }
    }

    return str;
}

void call_function(string name) {

}

string get_function_name(string line) {
    string fn_name = line.substr(5, line.length() - 6);
    return fn_name;
}

int index_of(string str, char ch) {
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == ch) return i;
    }

    return -1;
}

int to_int(string str) {
    if (str.length() == 0) return 0;
    return to_int(str.substr(0, str.length() - 1)) * 10 + str[str.length() - 1] - '0';
}

bool is_number(string num) {
    for (int i = 0; i < num.length(); i++) {
        if (num[i] < '0' || num[i] > '9') return false;
    }

    return true;
}

void store(int* registers, char* stack, int SP, string command) {
    //M[...] = ...
    string location_addr = command.substr(2, index_of(command, ']') - index_of(command, '[') - 1);
    assert(location_addr.length() >= 2);
    string str_value = command.substr(index_of(command, '=') + 1);
    assert(str_value.length() != 0);
    if (location_addr[0] == 'R') {
        int storing_register_index = to_int(location_addr.substr(1));
        if (index_of(str_value, 'R') == -1) {
            //storing a constant
            *(int*)(&stack[registers[storing_register_index]]) = to_int(str_value);
        } else {
            int value_register_index = to_int(str_value.substr(1));
            *(int*)(&stack[registers[storing_register_index]]) = registers[value_register_index];
        }
    } else {
        int offset = SP;
        if (location_addr.length() > 2) {
            int num = to_int(location_addr.substr(3));
            if (location_addr[2] == '+') {
                offset += num;
            } else if (location_addr[2] == '-') {
                offset -= num;
            } else {
                //invalid address to store memory
            }
        }
        if (index_of(str_value, 'R') == -1) {
            //storing a constant
            *(int*)(&stack[offset]) = to_int(str_value);
        } else {
            int value_register_index = to_int(str_value.substr(1));
            *(int*)(&stack[offset]) = registers[value_register_index];
        }
    }
}

bool is_load_operation(string command) {
    string left_side = command.substr(0, index_of(command, '='));
    string right_side = command.substr(index_of(command, '=') + 1);
    if (is_special_register(left_side)) {
        if (is_number(right_side)) return true;
        if (is_special_register(right_side)) return true;
        if (right_side[0] == 'M') {
            assert(right_side.length() >= 4);
            string between_brackets = right_side.substr(2, index_of(right_side, ']') - index_of(right_side, '[') - 1);
            if (is_number(between_brackets)) return true;
            if (is_special_register(between_brackets)) return true;
            if (index_of(between_brackets, '+') != -1) {
                string summand_1 = between_brackets.substr(0, index_of(between_brackets, '+'));
                string summand_2 = between_brackets.substr(index_of(between_brackets, '+') + 1);
                if (is_special_register(summand_1) && is_number(summand_2)) return true;
                if (is_number(summand_2) && is_special_register(summand_1)) return true;
            }
            if (index_of(between_brackets, '-') != -1) {
                string minuend = between_brackets.substr(0, index_of(between_brackets, '-'));
                string subrahend = between_brackets.substr(index_of(between_brackets, '-') + 1);
                if (is_special_register(minuend) && is_number(subrahend)) return true;
                if (is_number(subrahend) && is_special_register(minuend)) return true;
            }
        }
    }

    return false;
}

bool is_special_register(string str) {
    if (str == "SP") return true;
    if (str == "RV") return true;
    if (str.length() < 2) return false;
    if (str[0] == 'R' && is_number(str.substr(1))) return true;
    return false; 
}

void load(int* registers, char* stack, int SP, string command) {
    //R... = M[...]
    string destination_register = command.substr(0, index_of(command, '='));
    int dest_reg_index = to_int(destination_register.substr(1));
    string info_address = command.substr(index_of(command, '[') + 1, index_of(command, ']') - index_of(command, '[') - 1);
    if (info_address[0] == 'R') {
        int info_index = to_int(info_address.substr(1));
        registers[dest_reg_index] = *(int*)(&stack[registers[info_index]]);
    } else if (index_of(info_address, 'S') != -1) {
        assert(info_address.length() >= 2);
        if (info_address == "SP") {
            //R... = M[SP]
            registers[dest_reg_index] = *(int*)(&stack[SP]);
        } else {
            //R... = M[SP + ...]
            int offset = SP;
            int num = to_int(info_address.substr(3));
            if (info_address[2] == '+') {
                offset += num;
            } else if (info_address[2] == '-') {
                offset -= num;
            } else {
                //invalid address to store memory
            }
            registers[dest_reg_index] = *(int*)(&stack[offset]);
        }
    } else {
        //R... = M[constant]
        registers[dest_reg_index] = *(int*)(&stack[to_int(info_address)]);
    }
}