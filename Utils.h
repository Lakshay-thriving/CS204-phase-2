#ifndef UTILS_H
#define UTILS_H

#include <bits/stdc++.h>
using namespace std;
extern unordered_map<int, int> registers;

extern unordered_map<int, int> execMemory;



// Convert decimal to binary of given bit size (Handles Two's Complement for negative numbers)
inline string dectobin(int n, int size)
{
    string bin = bitset<32>(n).to_string(); // Convert to full 32-bit binary
    return bin.substr(32 - size); // Extract only the required bits
}

// Convert binary to hexadecimal
inline string bintodec(string ans)
{
    bitset<32> b(ans);
    stringstream ss;
    ss << hex << uppercase << b.to_ulong();
    return "0x" + ss.str();
}

// Convert decimal to binary with Two’s Complement
inline string dtb(int n, int size)
{
    string bin = bitset<32>(n).to_string();
    return bin.substr(32 - size);
}

#endif // UTILS_H
