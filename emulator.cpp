#include <bits/stdc++.h>

using namespace std;

string remove_spaces(string str);
void call_function(string name);
string get_function_name(string line);
int index_of(string str, char ch);
int to_int(string str);
bool is_number(string num);
bool is_store_operation (string command);
void store(int* registers, char* stack, int& SP, int& RV, string command);
bool is_load_operation(string command);
bool is_special_register(string str);
void load(int* registers, char* stack, int& SP, int& RV, string command);
int get_value_of_special_register(string special_register, int* registers, int SP, int RV);
int get_memory_from_stack(string address, int* registers, char* stack, int SP, int RV);
void update_register(string special_register, int value, int* registers, int& SP, int& RV);
bool is_ALU_operation(string command);
void perform_ALU(int* registers, char* stack, int& SP, int& RV, string command);

const string filename = "assembly_code.txt";

int main() {
    int *registers = new int[100];
    char *stack = new char[400];
    int SP = 400;
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
        if (PC / 4 == commands.size()) break;
        current_command = commands[PC / 4];
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
        } else if (is_store_operation(current_command)) {
            store(registers, stack, SP, RV, current_command);
        } else if (is_load_operation(current_command)) {
            load(registers, stack, SP, RV, current_command);
        } else if (is_ALU_operation(current_command)) {
            perform_ALU(registers, stack, SP, RV, current_command);
        } else {
            //perform_operation();
        }
        PC += 4;
    }

    cout << *(int*)(&stack[SP]) << endl;

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

bool is_store_operation (string command) {
    if (command[0] != 'M') return false;
    if (command.length() < 7) return false;
    if (command[1] != '[') return true;
    string left_side = command.substr(0, index_of(command, '='));
    string right_side = command.substr(index_of(command, '=') + 1);
    if (left_side[left_side.length() - 1] != ']') return false;
    if (is_number(right_side) || is_special_register(right_side)) return true;
    return false;
}

void store(int* registers, char* stack, int& SP, int& RV, string command) {
    string left_side = command.substr(0, index_of(command, '='));
    string right_side = command.substr(index_of(command, '=') + 1);
    string between_brackets = left_side.substr(2, index_of(right_side, ']') - index_of(right_side, '[') - 1);
    int value;
    if (is_number(right_side)) {
        value = to_int(right_side);
    } else {
        value = get_value_of_special_register(right_side, registers, SP, RV);
    }
    
    if (is_number(between_brackets)) {
        *(int*)(&stack[to_int(between_brackets)]) = value;
    } else if (is_special_register(between_brackets)) {
        update_register(between_brackets, value, registers, SP, RV);
    } else if (index_of(between_brackets, '+') != -1) {
        string summand_1 = between_brackets.substr(0, index_of(between_brackets, '+'));
        string summand_2 = between_brackets.substr(index_of(between_brackets, '+') + 1);
        if (is_special_register(summand_1) && is_number(summand_2)) {
            int index = get_value_of_special_register(summand_1, registers, SP, RV) + to_int(summand_2);
            *(int*)(&stack[index]) = value;
        }
        if (is_number(summand_1) && is_special_register(summand_2)) {
            int index = to_int(summand_1) + get_value_of_special_register(summand_2, registers, SP, RV);
            *(int*)(&stack[index]) = value;
        }
    } else if (index_of(between_brackets, '-') != -1) {
        string minuend = between_brackets.substr(0, index_of(between_brackets, '-'));
            string subrahend = between_brackets.substr(index_of(between_brackets, '-') + 1);
            if (is_special_register(minuend) && is_number(subrahend)) {
                int index = get_value_of_special_register(minuend, registers, SP, RV) - to_int(subrahend);
                if (index < 0) {
                    //error
                } else {
                    *(int*)(&stack[index]) = value;
                }
            }
            if (is_number(minuend) && is_special_register(subrahend)) {
                int index = to_int(minuend) - get_value_of_special_register(subrahend, registers, SP, RV);
                if (index < 0) {
                    //error
                } else {
                    *(int*)(&stack[index]) = value;
                }
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
            assert(right_side.length() >= 4);
            assert(right_side[1] == '[');
            assert(right_side[right_side.length() - 1] == ']');
            string between_brackets = right_side.substr(2, index_of(right_side, ']') - index_of(right_side, '[') - 1);
            if (is_number(between_brackets)) return true;
            if (is_special_register(between_brackets)) return true;
            if (index_of(between_brackets, '+') != -1) {
                string summand_1 = between_brackets.substr(0, index_of(between_brackets, '+'));
                string summand_2 = between_brackets.substr(index_of(between_brackets, '+') + 1);
                if (is_special_register(summand_1) && is_number(summand_2)) return true;
                if (is_number(summand_1) && is_special_register(summand_2)) return true;
            }
            if (index_of(between_brackets, '-') != -1) {
                string minuend = between_brackets.substr(0, index_of(between_brackets, '-'));
                string subrahend = between_brackets.substr(index_of(between_brackets, '-') + 1);
                if (is_special_register(minuend) && is_number(subrahend)) return true;
                if (is_number(minuend) && is_special_register(subrahend)) return true;
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

void load(int* registers, char* stack, int& SP, int& RV, string command) {
    string left_side = command.substr(0, index_of(command, '='));
    string right_side = command.substr(index_of(command, '=') + 1);
    int value;
    if (is_number(right_side)) {
        value = to_int(right_side);
    } else if (is_special_register(right_side)) {
        value = get_value_of_special_register(right_side, registers, SP, RV);
    } else if (right_side[0] == 'M') {
        string between_brackets = right_side.substr(2, index_of(right_side, ']') - index_of(right_side, '[') - 1);
        value = get_memory_from_stack(between_brackets, registers, stack, SP, RV);
    }
    update_register(left_side, value, registers, SP, RV);
}

int get_value_of_special_register(string special_register, int* registers, int SP, int RV) {
    if (special_register == "SP") return SP;
    if (special_register == "RV") return RV;
    if (special_register[0] == 'R') {
        int index = to_int(special_register.substr(1));
        return registers[index];
    }
    // else error
    return INT_MIN;
}

int get_memory_from_stack(string address, int* registers, char* stack, int SP, int RV) {
    if (is_number(address)) {
        return *(int*)(&stack[to_int(address)]);
    } else if (is_special_register(address)) {
        return *(int*)(&stack[get_value_of_special_register(address, registers, SP, RV)]);
    } else if (index_of(address, '+') != -1) {
        string summand_1 = address.substr(0, index_of(address, '+'));
        string summand_2 = address.substr(index_of(address, '+') + 1);
        if (is_special_register(summand_1) && is_number(summand_2)) {
            int index = get_value_of_special_register(summand_1, registers, SP, RV) + to_int(summand_2);
            return *(int*)(&stack[index]);
        }
        if (is_number(summand_1) && is_special_register(summand_2)) {
            int index = to_int(summand_1) + get_value_of_special_register(summand_2, registers, SP, RV);
            return *(int*)(&stack[index]);
        }
    } else if (index_of(address, '-') != -1) {
            string minuend = address.substr(0, index_of(address, '-'));
            string subrahend = address.substr(index_of(address, '-') + 1);
            if (is_special_register(minuend) && is_number(subrahend)) {
                int index = get_value_of_special_register(minuend, registers, SP, RV) - to_int(subrahend);
                if (index < 0) {
                    //error
                } else {
                    return *(int*)(&stack[index]);
                }
            }
            if (is_number(minuend) && is_special_register(subrahend)) {
                int index = to_int(minuend) - get_value_of_special_register(subrahend, registers, SP, RV);
                if (index < 0) {
                    //error
                } else {
                    return *(int*)(&stack[index]);
                }
            }
    } else {
        //error
    }
    return INT_MIN;
}

void update_register(string special_register, int value, int* registers, int& SP, int& RV) {
    if (special_register == "SP") {
        SP = value;
    } else if (special_register == "RV") {
        RV = value;
    } else {
        int register_index = to_int(special_register.substr(1));
        registers[register_index] = value;
    }
}

bool is_ALU_operation(string command) {
    string left_side = command.substr(0, index_of(command, '='));
    string right_side = command.substr(index_of(command, '=') + 1);
    if (is_special_register(left_side)) {
        string first, second;
        if (index_of(right_side, '+') != -1) {
            first = right_side.substr(0, index_of(right_side, '+'));
            second = right_side.substr(index_of(right_side, '+') + 1);
            assert(index_of(right_side, '-') == -1 && index_of(right_side, '*') == -1 && index_of(right_side, '/') == -1);
            return true;
        } else if (index_of(right_side, '-') != -1) {
            first = right_side.substr(0, index_of(right_side, '-'));
            second = right_side.substr(index_of(right_side, '-') + 1);
            assert(index_of(right_side, '+') == -1 && index_of(right_side, '*') == -1 && index_of(right_side, '/') == -1);
            return true;
        } else if (index_of(right_side, '*') != -1) {
            first = right_side.substr(0, index_of(right_side, '*'));
            second = right_side.substr(index_of(right_side, '*') + 1);
            assert(index_of(right_side, '-') == -1 && index_of(right_side, '+') == -1 && index_of(right_side, '/') == -1);
            return true;
        } else if (index_of(right_side, '/') != -1) {
            first = right_side.substr(0, index_of(right_side, '/'));
            second = right_side.substr(index_of(right_side, '/') + 1);
            assert(index_of(right_side, '-') == -1 && index_of(right_side, '+') == -1 && index_of(right_side, '*') == -1);
            return true;
        }
    }
    return false;
}

void perform_ALU(int* registers, char* stack, int& SP, int& RV, string command) {
    string left_side = command.substr(0, index_of(command, '='));
    string right_side = command.substr(index_of(command, '=') + 1);
    int value;
    string first, second;
    int first_value, second_value;
    if (index_of(right_side, '+') != -1) {
        first = right_side.substr(0, index_of(right_side, '+'));
        if (is_special_register(first)) {
            first_value = get_value_of_special_register(first, registers, SP, RV);
        } else {
            first_value = to_int(first);
        }

        second = right_side.substr(index_of(right_side, '+') + 1);
        if (is_special_register(second)) {
            second_value = get_value_of_special_register(second, registers, SP, RV);
        } else {
            second_value = to_int(second);
        }

        value = first_value + second_value;
    } else if (index_of(right_side, '-') != -1) {
        first = right_side.substr(0, index_of(right_side, '-'));
        if (is_special_register(first)) {
            first_value = get_value_of_special_register(first, registers, SP, RV);
        } else {
            first_value = to_int(first);
        }

        second = right_side.substr(index_of(right_side, '-') + 1);
        if (is_special_register(second)) {
            second_value = get_value_of_special_register(second, registers, SP, RV);
        } else {
            second_value = to_int(second);
        }

        value = first_value - second_value;
    } else if (index_of(right_side, '*') != -1) {
        first = right_side.substr(0, index_of(right_side, '*'));
        if (is_special_register(first)) {
            first_value = get_value_of_special_register(first, registers, SP, RV);
        } else {
            first_value = to_int(first);
        }

        second = right_side.substr(index_of(right_side, '*') + 1);
        if (is_special_register(second)) {
            second_value = get_value_of_special_register(second, registers, SP, RV);
        } else {
            second_value = to_int(second);
        }

        value = first_value * second_value;
    } else if (index_of(right_side, '/') != -1) {
        first = right_side.substr(0, index_of(right_side, '/'));
        if (is_special_register(first)) {
            first_value = get_value_of_special_register(first, registers, SP, RV);
        } else {
            first_value = to_int(first);
        }

        second = right_side.substr(index_of(right_side, '/') + 1);
        if (is_special_register(second)) {
            second_value = get_value_of_special_register(second, registers, SP, RV);
        } else {
            second_value = to_int(second);
        }

        value = first_value / second_value;
    }
    update_register(left_side, value, registers, SP, RV);
}