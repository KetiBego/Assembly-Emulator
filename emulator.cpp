#include <bits/stdc++.h>

using namespace std;

string remove_spaces(string str);
bool is_call_operation(string command);
void call_function(int* registers, char* stack, map<string, int> functions, int& PC, int& SP, int& RV, string command);
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
void update_register(string special_register, int value, int* registers, int& SP, int& RV, string type);
void update_stack(int value, char* stack, int index, string type);
bool is_ALU_operation(string command);
void perform_ALU(int* registers, char* stack, int& SP, int& RV, string command);
bool is_branch_operation(string command);
bool statement_is_true(int* registers, char* stack, int SP, int RV, string command);
void change_address(string addr, int& PC);
bool is_jump_operation(string command);
int count_operation(string operation, int* registers, char* stack, int SP, int RV);

const int stack_size = 10000;

int main() {
    int *registers = new int[100000];
    char *stack = new char[stack_size];
    int SP = stack_size;
    int PC = 0;
    int RV = 0;
    vector<string> commands;
    map<string, int> functions;
    string filename;
    cout << "Enter the filename: ";
    cin >> filename;
    string line;
    ifstream my_file(filename);
    if (my_file.is_open()) {
        while (getline(my_file, line))
        {
            if (line.length() >= 2 && line[0] == '/' && line[1] == '/') continue;
            commands.push_back(line);
        }
        my_file.close();
    } else {
        cout << "Unable to open file\n";
        return 0;
    }

    string current_command;
    while (true) {
        if (PC / 4 == commands.size()) break;
        current_command = commands[PC / 4];
        if (current_command[current_command.length() - 1] == ':') {
            functions[current_command.substr(0, current_command.length() - 1)] = PC;
        }
        PC += 4;
    }

    PC = functions["main"];
    
    while (true) {
        current_command = remove_spaces(commands[PC / 4 + 1]);
        if (current_command == "RET") {
            if (SP == stack_size) break;
            PC = get_memory_from_stack(to_string(SP), registers, stack, SP, RV);
            SP = SP + 4;
        } else if (is_call_operation(current_command)) {
            call_function(registers, stack, functions, PC, SP, RV, current_command);
        } else if (is_store_operation(current_command)) {
            store(registers, stack, SP, RV, current_command);
        } else if (is_load_operation(current_command)) {
            load(registers, stack, SP, RV, current_command);
        } else if (is_ALU_operation(current_command)) {
            perform_ALU(registers, stack, SP, RV, current_command);
        } else if (is_branch_operation(current_command)) {
            if (statement_is_true(registers, stack, SP, RV, current_command)) {
                string after_first_comma = current_command.substr(index_of(current_command, ',') + 1);
                string after_second_comma = after_first_comma.substr(index_of(after_first_comma, ',') + 1);
                change_address(after_second_comma, PC);
                PC -= 4;
            }
        } else if (is_jump_operation(current_command)) {
            change_address(current_command.substr(3), PC);
            PC -= 4;
        } else {
            cout << "Could not recognize command " << commands[PC / 4 + 1] << ", exiting the program." << endl;
            break;
        }
        PC += 4;
    }

    cout << RV << endl;
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

bool is_call_operation(string command) {
    if (command.length() < 6) return false;
    if (command.substr(0, 4) == "CALL") {
        string fn = command.substr(4);
        if (is_special_register(fn) || is_number(fn)) return true;
        if (fn[0] == '<' && fn[fn.length() - 1] == '>') return true;
    }
    return false;
}

void call_function(int* registers, char* stack, map<string, int> functions, int& PC, int& SP, int& RV, string command) {
    SP = SP - 4;
    update_stack(PC, stack, SP, "int");

    string fn = command.substr(4);
    int fn_addr;
    if (is_number(fn)) {
        PC = to_int(fn);
    } else if (fn[0] == '<') {
        PC = functions[get_function_name(fn)];
    } else if (is_special_register(fn)) {
        PC = get_value_of_special_register(fn, registers, SP, RV);
    }
    PC -= 4;
}

string get_function_name(string line) {
    string fn_name = line.substr(index_of(line, '<') + 1);
    fn_name = fn_name.substr(0, fn_name.length() - 1);
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
    if (str[0] == '-') return ((-1) * to_int(str.substr(1)));
    return to_int(str.substr(0, str.length() - 1)) * 10 + str[str.length() - 1] - '0';
}

bool is_number(string num) {
    if (num.length() == 0) return false;
    if (num[0] == '-') num = num.substr(1);
    for (int i = 0; i < num.length(); i++) {
        if (num[i] < '0' || num[i] > '9') return false;
    }

    return true;
}

bool is_store_operation (string command) {
    if (command[0] != 'M') return false;
    if (command.length() < 7) return false;
    if (command[1] != '[') return false;
    string left_side = command.substr(0, index_of(command, '='));
    string right_side = command.substr(index_of(command, '=') + 1);
    if (right_side[0] == '.') {
        if (right_side[1] == '1' || right_side[1] == '2') return is_store_operation(left_side + '=' + right_side.substr(2));
        return false;
    }
    if (left_side[left_side.length() - 1] != ']') return false;
    if (!(is_number(right_side) || is_special_register(right_side))) return false;
    string between_brackets = left_side.substr(2, index_of(left_side, ']') - index_of(left_side, '[') - 1);
    if (is_number(between_brackets) || is_special_register(between_brackets)) return true;
    string first, second;
    if (index_of(between_brackets, '+') != -1) {
        first = between_brackets.substr(0, index_of(between_brackets, '+'));
        second = between_brackets.substr(index_of(between_brackets, '+') + 1);
    } else if (index_of(between_brackets, '-') != -1) {
        first = between_brackets.substr(0, index_of(between_brackets, '-'));
        second = between_brackets.substr(index_of(between_brackets, '-') + 1);
    } else if (index_of(between_brackets, '*') != -1) {
        first = between_brackets.substr(0, index_of(between_brackets, '*'));
        second = between_brackets.substr(index_of(between_brackets, '*') + 1);
    }  else if (index_of(between_brackets, '/') != -1) {
        first = between_brackets.substr(0, index_of(between_brackets, '/'));
        second = between_brackets.substr(index_of(between_brackets, '/') + 1);
    }

    if (is_special_register(first) && is_special_register(second)) return false;
    if ((is_number(first) || is_special_register(first)) && (is_number(second) || is_special_register(second))) return true;
    return false;
}

void store(int* registers, char* stack, int& SP, int& RV, string command) {
    string left_side = command.substr(0, index_of(command, '='));
    string right_side = command.substr(index_of(command, '=') + 1);
    string between_brackets = left_side.substr(2, index_of(left_side, ']') - index_of(left_side, '[') - 1);
    int value;
    string type = "int";
    if (right_side[0] == '.') {
        if(right_side[1] == '1') type = "char";
        if(right_side[1] == '2') type = "short";
        right_side = right_side.substr(2);
    }

    if (is_number(right_side)) {
        value = to_int(right_side);
    } else if (is_special_register(right_side)) {
        value = get_value_of_special_register(right_side, registers, SP, RV);
    }
    
    if (is_number(between_brackets)) {
        update_stack(value, stack, to_int(between_brackets), type);
    } else if (is_special_register(between_brackets)) {
        update_stack( value, stack, get_value_of_special_register(between_brackets, registers, SP, RV), type);
    } else {
        int index = count_operation(between_brackets, registers, stack, SP, RV);
        update_stack(value, stack, index, type);
    }
}

bool is_load_operation(string command) {
    string left_side = command.substr(0, index_of(command, '='));
    string right_side = command.substr(index_of(command, '=') + 1);
    if (is_special_register(left_side)) {
        if (is_number(right_side)) return true;
        if (is_special_register(right_side)) return true;
        if (right_side[0] == '.') {
            if (right_side[1] == '1' || right_side[1] == '2') return is_load_operation(left_side + '=' + right_side.substr(2));
            return false;
        }
        if (right_side[0] == 'M') {
            if (right_side.length() < 4) return false;
            if (right_side[1] != '[') return false;
            if (right_side[right_side.length() - 1] != ']') return false;
            string between_brackets = right_side.substr(2, index_of(right_side, ']') - index_of(right_side, '[') - 1);
            if (is_number(between_brackets)) return true;
            if (is_special_register(between_brackets)) return true;
            string first, second;
            if (index_of(between_brackets, '+') != -1) {
                first = between_brackets.substr(0, index_of(between_brackets, '+'));
                second = between_brackets.substr(index_of(between_brackets, '+') + 1);
            } else if (index_of(between_brackets, '-') != -1) {
                first = between_brackets.substr(0, index_of(between_brackets, '-'));
                second = between_brackets.substr(index_of(between_brackets, '-') + 1);
            } else if (index_of(between_brackets, '*') != -1) {
                first = between_brackets.substr(0, index_of(between_brackets, '*'));
                second = between_brackets.substr(index_of(between_brackets, '*') + 1);
            }  else if (index_of(between_brackets, '/') != -1) {
                first = between_brackets.substr(0, index_of(between_brackets, '/'));
                second = between_brackets.substr(index_of(between_brackets, '/') + 1);
            }

            if (is_special_register(first) && is_special_register(second)) return false;
            if ((is_number(first) || is_special_register(first)) && (is_number(second) || is_special_register(second))) return true;
        }
    }

    return false;
}

bool is_special_register(string str) {
    if (str == "SP") return true;
    if (str == "RV") return true;
    if (str.length() < 2) return false;
    if (str[0] == 'R' && is_number(str.substr(1)) && to_int(str.substr(1)) > 0) return true;
    return false; 
}

void load(int* registers, char* stack, int& SP, int& RV, string command) {
    string left_side = command.substr(0, index_of(command, '='));
    string right_side = command.substr(index_of(command, '=') + 1);
    int value;
    string type = "int";
    if (right_side[0] == '.') {
        if(right_side[1] == '1') type = "char";
        if(right_side[1] == '2') type = "short";
        right_side = right_side.substr(2);
    }

    if (is_number(right_side)) {
        value = to_int(right_side);
    } else if (is_special_register(right_side)) {
        value = get_value_of_special_register(right_side, registers, SP, RV);
    } else if (right_side[0] == 'M') {
        string between_brackets = right_side.substr(2, index_of(right_side, ']') - index_of(right_side, '[') - 1);
        value = get_memory_from_stack(between_brackets, registers, stack, SP, RV);
    }
    update_register(left_side, value, registers, SP, RV, type);
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
    } else {
        int index = count_operation(address, registers, stack, SP, RV);
        return *(int*)(&stack[index]);
    }
    
    return INT_MIN;
}

void update_register(string special_register, int value, int* registers, int& SP, int& RV, string type) {
    if (type == "char") {
        value = (char) value;
    } else if (type == "short") {
        value = (short) value;
    }

    if (special_register == "SP") {
        SP = value;
    } else if (special_register == "RV") {
        RV = value;
    } else {
        int register_index = to_int(special_register.substr(1));
        registers[register_index] = value;
    }
}

void update_stack(int value, char* stack, int index, string type) {
    if (type == "char") {
        stack[index] = (char) value;
    } else if (type == "short") {
        *(short*)(&stack[index]) = (short) value;
    } else {
        *(int*)(&stack[index]) = value;
    }
}

bool is_ALU_operation(string command) {
    string left_side = command.substr(0, index_of(command, '='));
    string right_side = command.substr(index_of(command, '=') + 1);
    if (is_special_register(left_side)) {
        if (right_side[0] == '.') {
            if (right_side[1] == '1' || right_side[1] == '2') return is_store_operation(left_side + '=' + right_side.substr(2));
            return false;
        }
        string first, second;
        if (index_of(right_side, '+') != -1) {
            first = right_side.substr(0, index_of(right_side, '+'));
            second = right_side.substr(index_of(right_side, '+') + 1);
            if (index_of(right_side, '-') != -1 || index_of(right_side, '*') != -1 || index_of(right_side, '/') != -1) return false;
        } else if (index_of(right_side, '-') != -1) {
            first = right_side.substr(0, index_of(right_side, '-'));
            second = right_side.substr(index_of(right_side, '-') + 1);
            if (index_of(right_side, '+') != -1 || index_of(right_side, '*') != -1 || index_of(right_side, '/') != -1) return false;
        } else if (index_of(right_side, '*') != -1) {
            first = right_side.substr(0, index_of(right_side, '*'));
            second = right_side.substr(index_of(right_side, '*') + 1);
            if (index_of(right_side, '+') != -1 || index_of(right_side, '-') != -1 || index_of(right_side, '/') != -1) return false;
        } else if (index_of(right_side, '/') != -1) {
            first = right_side.substr(0, index_of(right_side, '/'));
            second = right_side.substr(index_of(right_side, '/') + 1);
            if (index_of(right_side, '+') != -1 || index_of(right_side, '-') != -1 || index_of(right_side, '*') != -1) return false;
        }
        return (is_number(first) || is_special_register(first)) && (is_number(second) || is_special_register(second));;
    }
    return false;
}

void perform_ALU(int* registers, char* stack, int& SP, int& RV, string command) {
    string left_side = command.substr(0, index_of(command, '='));
    string right_side = command.substr(index_of(command, '=') + 1);
    int value = count_operation(right_side, registers, stack, SP, RV);
    string type = "int";
    if (right_side[0] == '.') {
        if(right_side[1] == '1') type = "char";
        if(right_side[1] == '2') type = "short";
        right_side = right_side.substr(2);
    }
    
    update_register(left_side, value, registers, SP, RV, type);
}

bool is_branch_operation(string command) {
    if (command.length() < 8) return false;
    string branch_variant = command.substr(0, 3);
    if (branch_variant != "BLT" && branch_variant != "BLE" && branch_variant != "BGT" && branch_variant != "BGE" && branch_variant != "BEQ" && branch_variant != "BNE") return false;
    if (index_of(command, ',') == -1) return false;
    if (index_of(command.substr(index_of(command, ',') + 1), ',') == -1) return false;

    return true;
}

bool statement_is_true(int* registers, char* stack, int SP, int RV, string command) {
    string branch_case = command.substr(1, 2);
    string first_value_str = command.substr(3, index_of(command, ',') - 3);
    string after_first_comma = command.substr(index_of(command, ',') + 1);
    string second_value_str = after_first_comma.substr(0, index_of(after_first_comma, ','));

    int first_value, second_value;

    if (is_number(first_value_str)) {
        first_value = to_int(first_value_str);
    } else if (is_special_register(first_value_str)) {
        first_value = get_value_of_special_register(first_value_str, registers, SP, RV);
    }

    if (is_number(second_value_str)) {
        second_value = to_int(second_value_str);
    } else if (is_special_register(second_value_str)) {
        second_value = get_value_of_special_register(second_value_str, registers, SP, RV);
    }

    if (branch_case == "LT") return (first_value < second_value);
    if (branch_case == "LE") return (first_value <= second_value);
    if (branch_case == "GT") return (first_value > second_value);
    if (branch_case == "GE") return (first_value >= second_value);
    if (branch_case == "EQ") return (first_value == second_value);
    if (branch_case == "NE") return (first_value != second_value);

    return false;
}

void change_address(string addr, int& PC) {
    if (is_number(addr)) {
        PC = to_int(addr);
    } else if (index_of(addr, '+') != -1) {
        PC += to_int(addr.substr(index_of(addr, '+') + 1));
    } else if (index_of(addr, '-') != -1) {
        PC -= to_int(addr.substr(index_of(addr, '-') + 1));
    }
}

bool is_jump_operation(string command) {
    if (command.length() < 4) return false;
    if (command.substr(0, 3) == "JMP") return true;

    return false;
}

int count_operation(string operation, int* registers, char* stack, int SP, int RV) {
    int value;
    string first, second;
    int first_value, second_value;
    if (index_of(operation, '+') != -1) {
        first = operation.substr(0, index_of(operation, '+'));
        if (is_special_register(first)) {
            first_value = get_value_of_special_register(first, registers, SP, RV);
        } else {
            first_value = to_int(first);
        }

        second = operation.substr(index_of(operation, '+') + 1);
        if (is_special_register(second)) {
            second_value = get_value_of_special_register(second, registers, SP, RV);
        } else {
            second_value = to_int(second);
        }

        value = first_value + second_value;
    } else if (index_of(operation, '-') != -1) {
        first = operation.substr(0, index_of(operation, '-'));
        if (is_special_register(first)) {
            first_value = get_value_of_special_register(first, registers, SP, RV);
        } else {
            first_value = to_int(first);
        }

        second = operation.substr(index_of(operation, '-') + 1);
        if (is_special_register(second)) {
            second_value = get_value_of_special_register(second, registers, SP, RV);
        } else {
            second_value = to_int(second);
        }

        value = first_value - second_value;
    } else if (index_of(operation, '*') != -1) {
        first = operation.substr(0, index_of(operation, '*'));
        if (is_special_register(first)) {
            first_value = get_value_of_special_register(first, registers, SP, RV);
        } else {
            first_value = to_int(first);
        }

        second = operation.substr(index_of(operation, '*') + 1);
        if (is_special_register(second)) {
            second_value = get_value_of_special_register(second, registers, SP, RV);
        } else {
            second_value = to_int(second);
        }

        value = first_value * second_value;
    } else if (index_of(operation, '/') != -1) {
        first = operation.substr(0, index_of(operation, '/'));
        if (is_special_register(first)) {
            first_value = get_value_of_special_register(first, registers, SP, RV);
        } else {
            first_value = to_int(first);
        }

        second = operation.substr(index_of(operation, '/') + 1);
        if (is_special_register(second)) {
            second_value = get_value_of_special_register(second, registers, SP, RV);
        } else {
            second_value = to_int(second);
        }

        value = first_value / second_value;
    }
    return value;
}