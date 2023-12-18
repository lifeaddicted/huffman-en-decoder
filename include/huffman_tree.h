#ifndef __HUFFMAN_TREE_H__
#define __HUFFMAN_TREE_H__

#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdio>

#include "binary_tree.h"

#define BUF_SIZE 1024

//  file header struct
struct Huff_Header
{
    uint32_t total_char_num{0};
    uint32_t total_code_num{0};
    u_char signature[2]{0};
};
using Huff_Header = struct Huff_Header;

//  struct of codes, for write to the encoded file's header
struct Code
{
    char c{0};
    uint8_t code_len{0};
    uint16_t code{0};
};
using Code = struct Code;

//  hashfunc for bit_codes2
struct hash_pair
{
    size_t operator()(const std::pair<uint16_t, int> &p) const
    {
        auto hash1 = std::hash<uint16_t>{}(p.first + p.second);
        // auto hash2 = std::hash<uint16_t>{}();
        return hash1;
    }
};

/*  usage: Huffman_Tree is a huffman-code encoder and decoder of a file
    it has a FILE* for an encode/decode file*/
class Huffman_Tree
{
private:
    std::map<char, int> count;                //  frequency of chars in file
    std::vector<std::pair<char, int>> weight; //  same as count

    Binary_Tree<std::pair<char, int>> huff;                                   //    huffman tree of the chars
    std::unordered_map<char, std::string> codes;                              //    huffman-codes in string forms
    std::unordered_map<char, std::pair<uint16_t, int>> bit_codes;             //    huffman-codes in uint16_t forms(for encoding)
    std::unordered_map<std::pair<uint16_t, int>, char, hash_pair> bit_codes2; //    huffman-codes in uint16_t forms(for decoding)
    std::unordered_map<char, std::pair<uint16_t, int>> reverse_bit_codes;     //    reverse the huffman-codes for write(High bits in low position)
    /*  hashtable stores the char-huffmancodes pairs,
        the huffmancodes store with a 16bit uint, and the code length*/

    char buf_en[BUF_SIZE]{0}; //  buffer for encode
    char buf_de[BUF_SIZE]{0}; //  buffer for decode

    //  file attributes
    Huff_Header file_header;
    FILE *fp_in = nullptr;
    FILE *fp_out = nullptr;
    std::string filepath;
    bool is_constructed = false;
    bool is_compressed = false;
    uint32_t avg_codelen = 0;

public:
    Huffman_Tree(const char *filepath);
    ~Huffman_Tree() { fclose(fp_in); };
    void init();
    void show_info();

    void encode_file();

    void decode_file();

private:
    void count_char();
    void construct_huff();
    void encode_char();
    void reverse_code();

    void read_code_from_fp_in();

    bool byte_write(char *byte, uint16_t &code, int &code_remain_len, int &byte_remain_len);

    void post_order(const TNode<std::pair<char, int>> *root, std::string &code, uint16_t &bit_code, int &codelen);
};

#endif