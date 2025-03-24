#ifndef I_H
#define I_H
#include <bits/stdc++.h>
#include <string>
#include "R.h"
extern unordered_map<int, int> registers;

using namespace std;

// Convert decimal to binary of given bit size (Handles Two's Complement for negative numbers)
#include "utils.h" // Include the new utilities header file

// FUNC3 for I-format instructions
string func3I(string Mnemonic)
{
    if (Mnemonic == "lb" || Mnemonic == "jalr" || Mnemonic == "addi")
        return "000";
    else if (Mnemonic == "lh")
        return "001";
    else if (Mnemonic == "lw")
        return "010";
    else if (Mnemonic == "ld")
        return "011";
    else if (Mnemonic == "andi")
        return "111";
    else if (Mnemonic == "ori")
        return "110";
    else
    {
        printf("Error: Invalid I-format instruction\n");
        exit(0);
    }
}

// I-format instruction processing with binary formatting
pair<string, string> I(string s, int j, string Mnemonic)
{
    string ans = "", rd = "", rs1 = "", imm = "";
    int flag = 0;

    // Extract `rd`, `rs1`, `imm` fields
    if (Mnemonic == "addi" || Mnemonic == "andi" || Mnemonic == "ori" || Mnemonic == "jalr")
    {
        for (int i = j; i < s.size(); i++)
        {
            if (s[i] == 'x')
            {
                i++;
                if (flag == 0)
                {
                    while (s[i] != ',' && s[i] != ' ')
                    {
                        rd += s[i];
                        i++;
                    }
                    flag++;
                }
                else if (flag == 1)
                {
                    while ((s[i] != ',' && s[i] != ' ') || rs1 == "")
                    {
                        rs1 += s[i];
                        i++;
                    }
                    flag++;
                }
            }
            if (flag == 2)
            {
                i++;
                while (s[i] == ' ') i++; // Skip spaces
                while (i < s.size() && s[i] != ' ')
                {
                    imm += s[i];
                    i++;
                }
                flag = 0;
                break;
            }
        }
    }
    else
    {
        for (int i = j; i < s.size(); i++)
        {
            if (s[i] == 'x' && flag == 0)
            {
                i++;
                while (s[i] != ',' && s[i] != ' ')
                {
                    rd += s[i];
                    i++;
                }
                flag++;
            }
            else if (flag == 1 && s[i] != ' ')
            {
                while (s[i] != '(')
                {
                    imm += s[i]; // Extract offset
                    i++;
                }
                flag++;
            }
            else if (flag == 2 && s[i] != ' ')
            {
                if (s[i] == 'x')
                {
                    i++;
                    while (s[i] != ')' && s[i] != ' ')
                    {
                        rs1 += s[i];
                        i++;
                    }
                    break; // Stop after extracting the values
                }
            }
        }
    }

    // Ensure rd is valid
    if (rd.empty())
    {
        printf("Error: Missing rd in instruction\n");
        exit(0);
    }

    // Ensure rs1 is valid
    if (rs1.empty())
    {
        printf("Error: Missing rs1 in instruction\n");
        exit(0);
    }

    // Ensure imm is valid
    if (imm.empty())
    {
        printf("Error: Missing immediate value in instruction\n");
        exit(0);
    }

    // Determine opcode
    string opcode;
    if (Mnemonic == "addi" || Mnemonic == "andi" || Mnemonic == "ori")
        opcode = "0010011";
    else if (Mnemonic == "jalr")
        opcode = "1100111";
    else
        opcode = "0000011";

    // Convert rd to 5-bit binary
    int rd_num = stoi(rd);
    if (rd_num > 31 || rd_num < 0)
    {
        printf("Error: Register not found\n");
        exit(0);
    }
    string rd_bin = dectobin(rd_num, 5);

    // Get funct3
    string funct3 = func3I(Mnemonic);

    // Convert rs1 to 5-bit binary
    int rs1_num = stoi(rs1);
    if (rs1_num > 31 || rs1_num < 0)
    {
        printf("Error: Register not found\n");
        exit(0);
    }
    string rs1_bin = dectobin(rs1_num, 5);

    // Convert immediate to 12-bit binary (Handles negatives correctly)
    int imm_num = stoi(imm);
    string imm_bin = dectobin(imm_num, 12);

    // Construct binary instruction
    ans = imm_bin + rs1_bin + funct3 + rd_bin + opcode;

    // Convert binary to hexadecimal
    string hex = bintodec(ans);

    // Format binary instruction with hyphens
    string formatted_binary = opcode + "-" + funct3 + "-NULL-" + rd_bin + "-" + rs1_bin + "-" + "NULL-" + imm_bin;


    return {hex, formatted_binary};
}

tuple<string, int, int, int> decode_I(const string& line) {
    stringstream ss(line);
    string mnemonic, rd_str, arg2, arg3;

    ss >> mnemonic >> rd_str >> arg2;

    if (rd_str.empty() || arg2.empty()) {
        cerr << "Decode Error: Incomplete instruction: " << line << "\n";
        exit(1);
    }

    if (rd_str.back() == ',') rd_str.pop_back();
    if (arg2.back() == ',') arg2.pop_back();

    int rd = 0, rs1 = 0, imm = 0;

    try {
        rd = stoi(rd_str.substr(1));
    } catch (...) {
        cerr << "Decode Error: Invalid rd = " << rd_str << " in line: " << line << "\n";
        exit(1);
    }

    // Case 1: offset(base) form (like ld/lw)
    if (arg2.find('(') != string::npos && arg2.find(')') != string::npos) {
        size_t open = arg2.find('(');
        size_t close = arg2.find(')');
        string imm_str = arg2.substr(0, open);
        string rs1_str = arg2.substr(open + 1, close - open - 1);

        try {
            imm = stoi(imm_str);
            rs1 = stoi(rs1_str.substr(1));
        } catch (...) {
            cerr << "Decode Error: Invalid offset or rs1 in: " << line << "\n";
            exit(1);
        }
    }
    // Case 2: regular I-type like addi x5, x2, 10
    else {
        ss >> arg3;
        if (arg3.empty()) {
            cerr << "Decode Error: Missing immediate in line: " << line << "\n";
            exit(1);
        }
        if (arg3.back() == ',') arg3.pop_back();

        try {
            rs1 = stoi(arg2.substr(1));
            imm = stoi(arg3);
        } catch (...) {
            cerr << "Decode Error: Invalid rs1 or imm in line: " << line << "\n";
            exit(1);
        }
    }

    cout << "Decode Stage: Decoded I-type instruction.\n";
    cout << "   Mnemonic: " << mnemonic << ", rd (Ry): x" << rd << ", rs1: x" << rs1 << ", imm: " << imm << "\n";

    return {mnemonic, rd, rs1, imm};
}




int execute_I(const string& mnemonic, int rs1_val, int imm) {
    if (mnemonic == "addi") {
        cout << "Adding immediate " << imm << " to x" << rs1_val << "\n";
        return rs1_val + imm;
    }
    if (mnemonic == "andi") return rs1_val & imm;
    if (mnemonic == "ori")  return rs1_val | imm;
    if (mnemonic == "jalr") return rs1_val + imm;  // target address

    // For load instructions, result = effective memory address
    if (mnemonic == "lw" || mnemonic == "lb" || mnemonic == "lh" || mnemonic == "ld") {
        return rs1_val + imm;
    }

    cerr << "Unknown I-type mnemonic: " << mnemonic << endl;
    exit(1);
}
// You can simulate memory as: unordered_map<int, int> memory;
void memoryAccessI(const string& mnemonic, int address, int& loadData) {
    if (mnemonic == "lw" || mnemonic == "lb" || mnemonic == "lh" || mnemonic == "ld") {
        loadData = execMemory[address];
        cout << "Memory Access Stage: Loaded value " << loadData
             << " from address 0x" << hex << address << dec << "\n";
    } else {
        cout << "Memory Access Stage: No memory access for \"" << mnemonic << "\" instruction.\n";
    }
}

void writeBackI(const string& mnemonic, int rd, int value) {
    registers[rd] = value;
    cout << "Writeback Stage: x" << rd << " = " << value << " (Result of " << mnemonic << ")\n";
}

#endif
