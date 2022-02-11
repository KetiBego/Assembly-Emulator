#include <bits/stdc++.h>

using namespace std;

string remove_spaces(string str);
void call_function(string name);
string get_function_name(string line);
int index_of(string str, char ch);
int to_int(string str);
void store(int* registers, char* stack, int& SP, string command);

const string filename = "assembly_code.txt";

int main() {
    int *registers = new int[100];
    char *stack = new char[400];
    int SP = 0;
    int PC = 0;
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
        } else if (index_of(current_command, 'M') != -1) {
            //load();
        } else {
            //perform_operation();
        }
        PC += 4;
    }

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

void store(int* registers, char* stack, int& SP, string command) {
    //M[...] = ...
    string location_addr = command.substr(2, index_of(command, ']') - index_of(command, '[') - 1);
    assert(location_addr.length() >= 2);
    string str_value = command.substr(index_of(command, '=') + 1);
    assert(str_value.length() != 0);
    if (location_addr[0] == 'R') {
        int storing_register_index = to_int(location_addr.substr(1));
        if (index_of(str_value, 'R') == -1) {
            registers[storing_register_index] = to_int(str_value);
        } else {
            int value_register_index = to_int(str_value.substr(1));
            registers[storing_register_index] = registers[value_register_index];
        }
    } else {
        int offset = SP;
        if (location_addr.length() > 2) {
            int num = to_int(location_addr.substr(3));
            cout << location_addr << endl;
            cout << num << " num" << endl;
            if (location_addr[2] == '+') {
                offset += num;
            } else if (location_addr[2] == '-') {
                offset -= num;
            } else {
                //invalid address to store memory
            }
        }
        if (index_of(str_value, 'R') == -1) {
            *(int*)(&stack[offset]) = to_int(str_value);
        } else {
            int value_register_index = to_int(str_value.substr(1));
            *(int*)(&stack[offset]) = registers[value_register_index];
        }
    }
}