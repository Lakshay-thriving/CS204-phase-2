#ifndef U_H
#define U_H
#include <bits/stdc++.h>
#include <string>
#include "R.h"
#include "I.h"
#include "utils.h" // Include utilities header file
using namespace std;
extern unordered_map<int, int> registers;


// U-format instruction processing with binary formatting
pair<string, string> U(string s, int j, string Mnemonic)
{
    string ans = "", rd = "", imm = "";
    int flag = 0;

    // Extracting `rd` and `imm` fields
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
        else if (flag == 1)
        {
            i++;
            while (s[i] == ' ') i++; // Skip spaces
            while (i < s.size() && s[i] != ' ')
            {
                imm += s[i];
                i++;
            }
            flag = 0;
            break; // Stop after extracting the values
        }
    }

    // Determine opcode
    string opcode;
    if (Mnemonic == "auipc")
        opcode = "0010111";
    else if (Mnemonic == "lui")
        opcode = "0110111";
    else
    {
        printf("Error: Invalid U-format instruction\n");
        exit(0);
    }

    // Convert rd to 5-bit binary
    int rd_num = stoi(rd);
    if (rd_num > 31 || rd_num < 0)
    {
        printf("Error: Register not found\n");
        exit(0);
    }
    string rd_bin = dectobin(rd_num, 5);

    // Convert immediate to 20-bit binary (Shift left by 12 bits)
    long long imm_num = stoll(imm);  // Do not shift left
    string imm_bin = dectobin(imm_num, 20);


    // Construct binary instruction
    ans = imm_bin + rd_bin + opcode;

    // Convert binary to hexadecimal
    string hex = bintodec(ans);


    // Format binary instruction with hyphens
    string formatted_binary = opcode + "-NULL-NULL-" + rd_bin + "-NULL-NULL-" + imm_bin;


    return {hex, formatted_binary};
}

#endif
tuple<string, int, int> decode_U(const string& line) {
    stringstream ss(line);
    string mnemonic, rd_str, imm_str;

    ss >> mnemonic >> rd_str >> imm_str;

    if (!rd_str.empty() && rd_str.back() == ',') rd_str.pop_back();

    int rd = stoi(rd_str.substr(1));
    long long imm = stoll(imm_str);  // U-type supports 32-bit values, upper 20 bits used

    cout << "Decode Stage: Decoded U-type instruction.\n";
    cout << "   Mnemonic: " << mnemonic << ", rd: x" << rd << ", imm (upper 20 bits): " << imm << "\n";

    return {mnemonic, rd, imm};
}

int execute_U(const string& mnemonic, int imm, int PC) {
    if (mnemonic == "lui") {
        cout << "Execute Stage: Loading upper immediate = " << (imm << 12) << "\n";
        return imm << 12;
    } else if (mnemonic == "auipc") {
        int result = PC + (imm << 12);
        cout << "Execute Stage: Adding PC + (imm << 12) = " << PC << " + " << (imm << 12) << " = " << result << "\n";
        return result;
    } else {
        cerr << "Unknown U-type mnemonic: " << mnemonic << "\n";
        exit(1);
    }
}
void memoryAccessU(const string& mnemonic) {
    cout << "Memory Access Stage: No memory operation for \"" << mnemonic << "\" instruction.\n";
}
void writeBackU(const string& mnemonic, int rd, int value) {
    registers[rd] = value;
    cout << "Writeback Stage: x" << rd << " = " << value << " (Result of " << mnemonic << ")\n";
}
