
#include <bits/stdc++.h>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include "R.h"
#include "I.h"
#include "S.h"
#include "U.h"
#include "SB.h"
#include "J.h"
#include <utility>
#include <iomanip>

#include <unordered_map>
using namespace std;
int PC = 0;
int PC_2 = 0; // for label
unsigned int dat = 268435456;
int factor = 0;
int clock_cycles = 0; // Global clock variable
unordered_map<int, int> registers;
vector<string> text_segment; // Stores text (instruction) segment
vector<string> data_segment; // Stores data (variables in .data)
map<string, string> memory;  // Stores final memory dump (address -> value)
unordered_map<int, int> execMemory;
unordered_map<int, int> pc_to_line;  // Maps PC value to line number

// R format
vector<string> Rf;
// I format
vector<string> If;
// S format
vector<string> Sf;
// UJ format
vector<string> Uf;
// SB format
vector<string> Sbf;
// J format
vector<string> Jf;
// Function to determine id .data type is word or other
// Convert register names (like x5) to 5-bit binary representation
string hex_to_binary(string hex_str)
{
    stringstream ss;
    ss << hex << hex_str;
    unsigned int n;
    ss >> n;
    return bitset<32>(n).to_string(); // Convert to 32-bit binary
}

string register_to_binary(string reg) {
    int reg_num = stoi(reg.substr(1)); // Remove 'x' and convert to number
    return bitset<5>(reg_num).to_string();
}

// Convert immediate values to binary with specified bit length
string immediate_to_binary(string imm, int bits) {
    int imm_num = stoi(imm);
    return bitset<32>(imm_num).to_string().substr(32 - bits);
}

// Convert a binary string to hexadecimal
string binary_to_hex(string binary_str) {
    bitset<32> b(binary_str);
    stringstream ss;
    ss << hex << uppercase << b.to_ulong();
    return ss.str();
}

void data_func(string data_type)
{
    if (data_type == "byte")
        factor = 1;
    else if (data_type == "half")
        factor = 2;
    else if (data_type == "word")
        factor = 4;
    else
        factor = 8;
}




// Function to convert PC into hexdecimal
string hexa_convert(int y) {
    stringstream ss;
    ss << hex << uppercase << setw(8) << setfill('0') << (y & 0xFFFFFFFF); // Ensure 8-digit hex format
    return ss.str();
}
void printPipelineStages(string instruction, string binaryAns, int instructionType) {
    cout << "Cycle " << ++clock_cycles << ": Fetching instruction...\n";
    cout << "   PC: 0x" << hexa_convert(PC) << " | Instruction: " << instruction << "\n";

    cout << "Cycle " << ++clock_cycles << ": Decoding instruction...\n";
    cout << "   Binary Representation: " << binaryAns << "\n";

    cout << "Cycle " << ++clock_cycles << ": Executing ALU operation / branch evaluation...\n";
    if (instructionType == 0) // Example: R-type instruction
        cout << "   ALU performing operation on registers...\n";

    cout << "Cycle " << ++clock_cycles << ": Memory Access (if required)...\n";
    if (instructionType == 1) // Example: Load instruction
        cout << "   Accessing memory location...\n";

    cout << "Cycle " << ++clock_cycles << ": Writeback to register/memory.\n";
    cout << "   Updated Register File / Memory.\n";

    cout << "Total Clock Cycles: " << clock_cycles << "\n\n";
}

void writeMemoryBlock(ofstream &output, string addr, string machineCode, string originalLine, string description) {
    output << addr << " " << machineCode 
           << " , " << originalLine 
           << " # " << description << "\n";
}

// Fetch stage: simply logs the instruction being fetched at a given PC

void fetch(const string& instruction) {
    cout << "Fetch Stage: Fetching instruction from instruction memory.\n";
    cout << "   >> " << instruction << "\n";
}


int main()
{
    string binary_ans;
    // Opening the assembly file
    ifstream file("input.asm");
    if (!file.is_open())
    {
        cout << "Error opening file." << endl;
        return 0;
    }
    // Taking every line of assembly code as strings in vector
    vector<string> lines;
    // Preprocessing: Split inline labels like "label: addi x1, x0, 5" into two separate lines
vector<string> processed_lines;
for (string& line : lines) {
    size_t colonPos = line.find(':');
    if (colonPos != string::npos && colonPos + 1 < line.size() && line[colonPos + 1] != ' ') {
        string label = line.substr(0, colonPos + 1);
        string rest = line.substr(colonPos + 1);
        processed_lines.push_back(label);
        processed_lines.push_back(rest);
    } else {
        processed_lines.push_back(line);
    }
}
lines = processed_lines; // Replace with cleaned version

    string s;
    while (getline(file, s))
    {
        lines.push_back(s);
    }
    // Completed taking file as input
    file.close();
    // Adding R format instructions
    Rf.push_back("add");
    Rf.push_back("and");
    Rf.push_back("or");
    Rf.push_back("sll");
    Rf.push_back("slt");
    Rf.push_back("sra");
    Rf.push_back("srl");
    Rf.push_back("sub");
    Rf.push_back("xor");
    Rf.push_back("mul");
    Rf.push_back("div");
    Rf.push_back("rem");
    // Adding I format Instruction
    If.push_back("addi");
    If.push_back("andi");
    If.push_back("ori");
    If.push_back("lb");
    If.push_back("ld");
    If.push_back("lh");
    If.push_back("lw");
    If.push_back("jalr");
    // Adding S format
    Sf.push_back("sb");
    Sf.push_back("sw");
    Sf.push_back("sd");
    Sf.push_back("sh");
    // Addig Sb format
    Sbf.push_back("beq");
    Sbf.push_back("bne");
    Sbf.push_back("bge");
    Sbf.push_back("blt");
    // Adding U format
    Uf.push_back("auipc");
    Uf.push_back("lui");
    // Adding UJ format
    Jf.push_back("jal");
    // END of inserting Mnemonics

    int flag = 0;
    // Creating output file
    ofstream output("output.mc");
    int counter_2 = 0;
    // Initializing map for label
    unordered_map<string, int> m_label;
    for (int i = 0; i < lines.size(); i++)
    {
        string ans = "";
        string Mnemonic = "";
        int j = 0;
        // Getting the opcode until space and avoiding space in from of the opcode
        while ((lines[i][j] != ' ' || Mnemonic == "") && j < lines[i].size())
        {
            // Checking if the instruction is comment or not
            if (lines[i][j] == '#')
            {
                flag++;
                break;
            }
            Mnemonic = Mnemonic + lines[i][j];
            j++;
        }
        // Checking if its a comment
        if (flag)
        {
            flag = 0;
            continue;
        }
        // If not a comment then proceeding with instruction
        if (Mnemonic == ".data:")
        {
            i++;
            // the assumption that .data is in a separate line and anything following it is on the next line
            int j = 0;
            while ((lines[i] != ".text:") && i < lines.size()) // the assumption that while starting the text segment it should start exactly with the line ".text"
                                                               // meaning no other char before or after this exact word
            {
                string data_type = "";
                while (lines[i][j] != '.')
                {
                    j++;
                }
                j++;
                while (lines[i][j] != ' ')
                {
                    data_type += lines[i][j];
                    j++;
                }

                if (data_type == "asciiz" || data_type == "string") {
                    string str = "";
                    
                    // Move to the opening quote
                    while (j < lines[i].size() && lines[i][j] != '\"') j++; 
                    j++;
                
                    // Process each character inside the string
                    while (j < lines[i].size() && lines[i][j] != '\"') {
                        if (lines[i][j] == '\\' && j + 1 < lines[i].size()) {
                            // Handle escape sequences (e.g., \n, \t)
                            j++;
                            switch (lines[i][j]) {
                                case 'n': str += '\n'; break;
                                case 't': str += '\t'; break;
                                case 'r': str += '\r'; break;
                                case '0': str += '\0'; break;
                                default: str += lines[i][j]; break;
                            }
                        } else {
                            str += lines[i][j];
                        }
                        j++;
                    }
                
                    // Ensure closing quote is present
                    if (j >= lines[i].size() || lines[i][j] != '\"') {
                        cout << "Warning: Missing closing quote for string at line " << i + 1 << endl;
                    }
                
                    // Store string characters in memory
                    // Store string characters in memory
                        string assembled_string = "";
                        for (char ch : str) {
                        string h = hexa_convert(int(ch));
                        string addr = "0x" + hexa_convert(dat);
                        string data_output = addr + " 0x" + h + " , " + lines[i] + " # Char: " + string(1, ch);

                        data_segment.push_back(data_output);  // Add to data segment
                        memory[addr] = "0x" + h;  // Store for final memory dump
                        assembled_string += ch;
                        dat++;
}

// Store NULL terminator (0x00) for string termination
                string addr = "0x" + hexa_convert(dat);
    string data_output = addr + " 0x0 , " + lines[i] + " # String termination";
data_segment.push_back(data_output);  // Add termination to data segment
memory[addr] = "0x0";  // Store for final memory dump

dat++;  // Move past null terminator
// Move past null terminator
                }
                
                else
                {

                    data_func(data_type);
                    vector<string> arr;
                    while (lines[i][j] == ' ')
                        j++;
                    while (j < lines[i].size())
                    {
                        string id = "";
                        while (lines[i][j] != ' ' && j < lines[i].size())
                        {
                            id = id + lines[i][j];
                            j++;
                        }
                        arr.push_back(id);
                        j++;
                    }
                    for (int t = 0; t < arr.size(); t++)
                    {
                        int yt = stoi(arr[t]);
                        string h = hexa_convert(yt);
                        string addr = "0x" + hexa_convert(dat);
                        string data_output = addr + " 0x" + h + " , " + lines[i] + " # Data (" + data_type + "): " + arr[t];
                        data_segment.push_back(data_output);
                        memory[addr] = "0x" + h; // Store for final memory dump

                        dat += factor;
                    }
                }
                i++;
                j = 0;
            }
        }
        else
        {

            // Creating map function for labels
            if (!counter_2) {
                int temp_PC = 0;
            
                for (int ot = i; ot < lines.size(); ot++) {
                    string line = lines[ot];
            
                    // Skip empty lines or comment-only lines
                    if (line.empty() || line[0] == '#') continue;
            
                    // Handle label (must end with ':')
                    if (line.find(':') != string::npos && line.back() == ':') {
                        string lab = "";
                        for (char ch : line) {
                            if (ch == ':') break;
                            if (ch != ' ') lab += ch;
                        }
                        m_label[lab] = temp_PC;
                        continue; // ✅ IMPORTANT: skip incrementing PC for label-only lines
                    }
            
                    // Handle instruction lines
                    if (line.find('.') == string::npos && !line.empty() && line[0] != '#') {
                        pc_to_line[temp_PC] = ot;  // ✅ map PC to line index
                        temp_PC += 4;
                    }
                }
            
                PC_2 = temp_PC;  // total instruction bytes mapped
                counter_2++;
            } 

            // Checking if its R_format Mnemonics
            if (ans == "")
{
    for (int k = 0; k < Rf.size(); k++)
    {
        if (Mnemonic == Rf[k]) {
            // --- Pipeline Stage 1: Fetch ---
            cout << "Cycle " << ++clock_cycles << ":\n";
            fetch(lines[i]);
        
            // --- Pipeline Stage 2: Decode ---
            cout << "Cycle " << ++clock_cycles << ":\n";
            string mnemonic;
            int rd, rs1, rs2;
            tie(mnemonic, rd, rs1, rs2) = decode_R(lines[i]);
        
            // Get values from the register file
            int rs1_val = registers[rs1];
            int rs2_val = registers[rs2];
        
            // --- Pipeline Stage 3: Execute ---
            cout << "Cycle " << ++clock_cycles << ":\n";
            int result = execute_R(mnemonic, rs1_val, rs2_val);
        
            // --- Pipeline Stage 4: Memory Access (No-op for R-type) ---
            cout << "Cycle " << ++clock_cycles << ":\n";
            memoryAccessR(mnemonic);
        
            // --- Pipeline Stage 5: Write Back ---
            cout << "Cycle " << ++clock_cycles << ":\n";
            writeBackR(rd, result, mnemonic);
        
            // Generate binary & hex representation
            tie(ans, binary_ans) = R(lines[i], j, Mnemonic);
        
            // Final output formatting
            string instruction_output = "0x" + hexa_convert(PC) + " " + ans + " , " + lines[i] + " # " + binary_ans;
            text_segment.push_back(instruction_output);
        
            PC += 4;
            break;
        }
        
        
    }
}


            // Checking if its I format Mnemonic
            if (ans == "")
{
    for (int k = 0; k < If.size(); k++) {
        if (Mnemonic == If[k]) {
            cout << "Cycle " << ++clock_cycles << ":\n";
            fetch(lines[i]);
    
            cout << "Cycle " << ++clock_cycles << ":\n";
            string mnemonic;
            int rd, rs1, imm;
            tie(mnemonic, rd, rs1, imm) = decode_I(lines[i]);
            int rs1_val = registers[rs1];
            cout << "Decode Stage: Decoded I-type instruction.\n";
            cout << "   Mnemonic: " << mnemonic << ", rd (Ry): x" << rd << ", rs1: x" << rs1
                 << " = " << rs1_val << ", imm: " << imm << "\n";
    
            cout << "Cycle " << ++clock_cycles << ":\n";
            int aluResult = execute_I(mnemonic, rs1_val, imm);
            cout << "Execute Stage: ALU Result = " << aluResult << " → will be written to Ry (x" << rd << ")\n";
    
            cout << "Cycle " << ++clock_cycles << ":\n";
            int loadData = 0;
            memoryAccessI(mnemonic, aluResult, loadData);
    
            cout << "Cycle " << ++clock_cycles << ":\n";
            if (mnemonic == "lw" || mnemonic == "lb" || mnemonic == "lh" || mnemonic == "ld") {
                if (rd != 0) writeBackI(mnemonic, rd, loadData);
            } else {
                if (rd != 0) writeBackI(mnemonic, rd, aluResult);
            }
    
            tie(ans, binary_ans) = I(lines[i], j, Mnemonic);
            string instruction_output = "0x" + hexa_convert(PC) + " " + ans + " , " + lines[i] + " # " + binary_ans;
            text_segment.push_back(instruction_output);
    
            // 🧠 Handle PC update: if jalr, jump to address in aluResult
            if (mnemonic == "jalr") {
    if (aluResult % 4 != 0) {
        cerr << "Runtime Error: Unaligned jump target address " << aluResult << " in jalr.\n";
        exit(1);
    }
    if (pc_to_line.find(aluResult) != pc_to_line.end()) {
        i = pc_to_line[aluResult] - 1;  // go to corresponding instruction line
        PC = aluResult;
    } else {
        cerr << "Runtime Error: Invalid jump target address " << aluResult << " in jalr.\n";
        exit(1);
    }
}

    
            break;  // Done processing this instruction
        }
    }
    
    
}

            // Checking if its S format Mnemonic
            if (ans == "")
{
    for (int k = 0; k < Sf.size(); k++)
    {
        if (Mnemonic == Sf[k]) {
            cout << "Cycle " << ++clock_cycles << ":\n";
            fetch(lines[i]);
        
            cout << "Cycle " << ++clock_cycles << ":\n";
            string mnemonic;
            int rs1, rs2, imm;
            tie(mnemonic, rs1, rs2, imm) = decode_S(lines[i]);
        
            int rs1_val = registers[rs1];
            int rs2_val = registers[rs2];
        
            cout << "Cycle " << ++clock_cycles << ":\n";
            int effectiveAddr = execute_S(mnemonic, rs1_val, imm);
        
            cout << "Cycle " << ++clock_cycles << ":\n";
            memoryAccessS(mnemonic, effectiveAddr, rs2_val);
        
            cout << "Cycle " << ++clock_cycles << ":\n";
            writeBackS(mnemonic);
        
            tie(ans, binary_ans) = S(lines[i], j, Mnemonic);
            string instruction_output = "0x" + hexa_convert(PC) + " " + ans + " , " + lines[i] + " # " + binary_ans;
            text_segment.push_back(instruction_output);
        
            PC += 4;
            break;
        }
    }
}

            // Checking if its SB format Mnemonics
            if (ans == "")
{
    for (int k = 0; k < Sbf.size(); k++) {
        if (Mnemonic == Sbf[k]) {
            int originalPC = PC;
    
            cout << "Cycle " << ++clock_cycles << ":\n";
            fetch(lines[i]);
    
            cout << "Cycle " << ++clock_cycles << ":\n";
            string mnemonic;
            int rs1, rs2, offset;
            tie(mnemonic, rs1, rs2, offset) = decode_SB(lines[i], m_label, originalPC);
    
            int rs1_val = registers[rs1];
            int rs2_val = registers[rs2];
    
            cout << "Cycle " << ++clock_cycles << ":\n";
            bool branchTaken = execute_SB(mnemonic, rs1_val, rs2_val);
    
            cout << "Cycle " << ++clock_cycles << ":\n";
            memoryAccessSB(mnemonic);
    
            cout << "Cycle " << ++clock_cycles << ":\n";
            writeBackSB(mnemonic, branchTaken, originalPC + offset);
    
            tie(ans, binary_ans) = SB(lines[i], j, Mnemonic, m_label, originalPC);
            string instruction_output = "0x" + hexa_convert(originalPC) + " " + ans + " , " + lines[i] + " # " + binary_ans;
            text_segment.push_back(instruction_output);
    
            // ✅ Correct PC update and line jump
            if (branchTaken) {
                PC = originalPC + offset;
            
                // ✅ Update the instruction pointer `i` to reflect the new PC
                if (pc_to_line.find(PC) != pc_to_line.end()) {
                    i = pc_to_line[PC] - 1;  // -1 because the for-loop will increment i after this
                } else {
                    cerr << "Error: PC " << PC << " not found in pc_to_line map\n";
                    exit(1);
                }
            } else {
                PC = originalPC + 4;
            }
            
    
            break;
        }
    }
    
}

            // Checking if its U format Mnemonics
            if (ans == "")
{
    for (int k = 0; k < Uf.size(); k++)
    {
        if (Mnemonic == Uf[k]) {
            cout << "Cycle " << ++clock_cycles << ":\n";
            fetch(lines[i]);
        
            cout << "Cycle " << ++clock_cycles << ":\n";
            string mnemonic;
            int rd;
            long long imm;
            tie(mnemonic, rd, imm) = decode_U(lines[i]);
        
            cout << "Cycle " << ++clock_cycles << ":\n";
            int result = execute_U(mnemonic, imm, PC);
        
            cout << "Cycle " << ++clock_cycles << ":\n";
            memoryAccessU(mnemonic);
        
            cout << "Cycle " << ++clock_cycles << ":\n";
            writeBackU(mnemonic, rd, result);
        
            tie(ans, binary_ans) = U(lines[i], j, Mnemonic);
            string instruction_output = "0x" + hexa_convert(PC) + " " + ans + " , " + lines[i] + " # " + binary_ans;
            text_segment.push_back(instruction_output);
        
            PC += 4;
            break;
        }
        
    }
}

            // Checking if its UJ format Mnemonics
            if (ans == "")
{
    for (int k = 0; k < Jf.size(); k++) 
        if (Mnemonic == Jf[k]) {
        cout << "Cycle " << ++clock_cycles << ":\n";
        fetch(lines[i]);
    
        cout << "Cycle " << ++clock_cycles << ":\n";
        string mnemonic;
        int rd, offset;
        tie(mnemonic, rd, offset) = decode_J(lines[i], m_label, PC);
        cout << "Decode Stage: Decoded J-type instruction.\n";
        cout << "   Mnemonic: " << mnemonic << ", rd (Ry): x" << rd << ", offset: " << offset << "\n";
    
        cout << "Cycle " << ++clock_cycles << ":\n";
        int targetAddress = execute_J(PC, offset);
        cout << "Execute Stage: Jump target address calculated as " << targetAddress << "\n";
    
        cout << "Cycle " << ++clock_cycles << ":\n";
        memoryAccessJ(mnemonic);
    
        cout << "Cycle " << ++clock_cycles << ":\n";
        int returnAddress = PC + 4;
        writeBackJ(rd, returnAddress, mnemonic);
    
        // Encoding & output instruction
        tie(ans, binary_ans) = J(lines[i], j, Mnemonic, m_label, PC);
        string instruction_output = "0x" + hexa_convert(PC) + " " + ans + " , " + lines[i] + " # " + binary_ans;
        text_segment.push_back(instruction_output);
    
        // ✅ Properly Update PC & i
        PC = targetAddress;
    
        // Use accurate mapping for PC → line number
        if (pc_to_line.find(PC) != pc_to_line.end()) {
            i = pc_to_line[PC] - 1;  // -1 due to the loop's increment
        } else {
            cerr << "Error: PC " << PC << " not found in pc_to_line map\n";
            exit(1);
        }
    
        break;
    }
    
    
}

        }
    }
    // --- STEP 1: PRINT TEXT SEGMENT ---
output << "# --- TEXT SEGMENT ---\n";
for (const auto &entry : text_segment) {
    output << entry << "\n";
}
// Final termination
output << "0x" << hexa_convert(PC) << " Termination of code";

// --- STEP 2: PRINT DATA SEGMENT ---
output << "\n# --- DATA SEGMENT ---\n";
for (const auto &entry : data_segment) {
    output << entry << "\n";
}



}