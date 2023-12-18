#include "huffman_tree.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <algorithm>

Huffman_Tree::Huffman_Tree(const char *filepath)
{
    fp_in = fopen(filepath, "r");
    if (fp_in == nullptr)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    this->filepath = filepath;
    return;
}

/*******************************************************************************
* functionality:

* args:
*
*******************************************************************************/
void Huffman_Tree::init()
{
    /*check if it's been encoded*/
    fread(&file_header, 10, 1, fp_in);
    if (file_header.signature[0] == 0x77 && file_header.signature[1] == 0x88) //  if has the signature, than it's compressed
    {
        is_compressed = true;
        std::cout << "A compressed file!" << std::endl;
    }

    if (is_compressed) //  it's compressed, read huffman-code
    {
        read_code_from_fp_in();
        //  init the bit_codes2
        for (auto &it : bit_codes)
            bit_codes2[it.second] = it.first;
    }
    else // it's a normal file, get huffman-code solution
    {
        memset(&file_header, 0, sizeof(Huff_Header));
        file_header.signature[0] = 0x77;
        file_header.signature[1] = 0x88;
        encode_char();  //  get huffman-code sulotion of a file
        reverse_code(); //  get reverse code
    }
    return;
}

//  DES sort by frequency
bool comp(const std::pair<char, int> &a, const std::pair<char, int> &b)
{
    if (a.second > b.second)
        return true;
    return false;
}

/*******************************************************************************
* functionality:
        count the char frequency, store the pair into a vecotr, and sort DSC
* args:
* 		pathname of the file
*******************************************************************************/
void Huffman_Tree::count_char()
{
    char *cur = nullptr;
    //  read blocks until EOF
    fseek(fp_in, 0, SEEK_SET);
    while (!feof(fp_in))
    {
        cur = buf_en;
        memset(buf_en, 0, BUF_SIZE);
        fread(buf_en, BUF_SIZE, 1, fp_in);
        while (*cur != '\0' && cur < buf_en + 1024)
        {
            count[*cur]++; //   char statistics
            ++cur;
            ++file_header.total_char_num;
        }
    }
    file_header.total_code_num = count.size();

    //  store frequency into a vector
    for (auto &it : count)
    {
        weight.emplace_back(it);
    }
    std::sort(weight.begin(), weight.end(), comp);
    return;
}

bool comp2(TNode<std::pair<char, int>> *a, TNode<std::pair<char, int>> *b)
{
    if ((*a).val.second > (*b).val.second)
        return true;
    return false;
}

/*******************************************************************************
* functionality:
        construct huffmantree from the vector
* args:
* 		null
*******************************************************************************/
void Huffman_Tree::construct_huff()
{
    if (is_constructed)
        return;
    //  init the forests
    std::vector<TNode<std::pair<char, int>> *> trees;
    for (auto &it : weight)
    {
        TNode<std::pair<char, int>> *tmp = huff.new_node(it);
        trees.emplace_back(tmp);
    }
    std::sort(trees.begin(), trees.end(), comp2);

    //  construct the huffman tree
    TNode<std::pair<char, int>> *parent = nullptr;
    while (trees.size() != 1)
    {
        //  get the least two
        auto left = trees.end() - 1;
        auto right = left - 1;

        //  construct parent
        std::pair<char, int> tmp{'\0', 0};
        tmp.second += (*left)->val.second;
        tmp.second += (*right)->val.second;
        parent = huff.new_node(tmp);
        parent->left = *left;
        parent->right = *right;

        //  erase the child from trees, add parent to trees
        trees.pop_back();
        trees.pop_back();
        trees.emplace_back(parent);
        avg_codelen += parent->val.second;

        //  sort the trees
        std::sort(trees.begin(), trees.end(), comp2);
    }
    huff.update_root(parent);
    is_constructed = true;
    return;
    /*  use an unordered_map to store tress, but lots of bugs exist */
    // std::unordered_multimap<std::pair<char, int>, TNode<std::pair<char, int>>> parents{0};
    // while (weight.size() != 1)
    // {
    //     //  construct left and right child, and erase them each time
    //     auto it = weight.end();
    //     if ((*(--it)).first != '\0')
    //         left = huff.new_node(*it);
    //     else
    //     {
    //         auto node = parents.find(*it);
    //         left = &(*node).second;
    //         parents.erase(node);
    //     }
    //     weight.erase(it);

    //     it = weight.end();
    //     if ((*(--it)).first != '\0')
    //         right = huff.new_node(*(it));
    //     else
    //     {
    //         auto node = parents.find(*it);
    //         left = &(*node).second;
    //         parents.erase(node);
    //     }
    //     weight.erase(it);

    //     //  construct parent
    //     std::pair<char, int> tmp{'\0', 0};
    //     tmp.first = '\0';
    //     tmp.second += left->val.second;
    //     tmp.second += right->val.second;
    //     parent = huff.new_node(tmp);
    //     parent->left = left;
    //     parent->right = right;
    //     //  add parent node to (a ds) for later use
    //     parents.emplace(tmp, parent);

    //     //  add the new weight to vector, and sort again
    //     weight.emplace_back(tmp);
    //     std::sort(weight.begin(), weight.end(), comp);
    // }
}

/*******************************************************************************
* functionality:
        get the code of the leaves node, store them into an unordered_map
* args:
* 		root of the huffmantree, code records the string
*******************************************************************************/
void Huffman_Tree::post_order(const TNode<std::pair<char, int>> *root, std::string &code, uint16_t &bit_code, int &codelen)
{
    if (root->left == nullptr && root->right == nullptr)
    {
        codes[(root->val).first] = code;
        bit_codes[(root->val).first].first = bit_code;
        bit_codes[(root->val).first].second = codelen;
        // code.erase(code.end() - 1);
        return;
    }
    //  left
    code.push_back('0');
    bit_code <<= 1;
    if (root->left)
        post_order(root->left, code, bit_code, ++codelen);
    bit_code >>= 1;
    --codelen;
    code.erase(code.end() - 1);

    //  right
    code.push_back('1');
    bit_code <<= 1;
    ++bit_code;
    if (root->right)
        post_order(root->right, code, bit_code, ++codelen);
    bit_code >>= 1;
    --codelen;
    code.erase(code.end() - 1);
    return;
}

/*******************************************************************************
* functionality:
        get a huffman encoding solution of a file
        open a file, counts the frequency of chars, construct a huffmantree for
        the chars, and get the encoding of chars in string forms;
        store into codes(unordered_map)
* args:
*       pathname of the file
*******************************************************************************/
void Huffman_Tree::encode_char()
{
    count_char();
    construct_huff();
    const TNode<std::pair<char, int>> *root = huff.get_root();
    std::string code;
    uint16_t bit_code = 0;
    int codelen = 0;
    post_order(root, code, bit_code, codelen);
    return;
}

/*******************************************************************************
* functionality:
        reverse bit-codes; like from 10110 to 01101
* args:
*       NULL
*******************************************************************************/
void Huffman_Tree::reverse_code()
{
    uint16_t mask = 0;
    for (auto &it : bit_codes)
    {
        reverse_bit_codes[it.first].second = it.second.second;
        //  reverse each bit
        for (int i = 0; i < it.second.second; i++)
        {
            mask = 1;
            mask &= (it.second.first >> i);
            mask <<= ((it.second.second - i) - 1); //  low bit reverse to high bit
            reverse_bit_codes[it.first].first |= mask;
        }
    }
}

/*******************************************************************************
* functionality:
        write a code to a byte-buffer.
        return true on no bits left in byte, otherwise return false
        e.g. write 10011 to a byte-buffer
        return false, code_remain_len=0, byte_remain_len=3
* args:
*                  byte:   pointer to the byte-buffer
                   code:   code to be written
        code_remain_len:   return the remain len of code
        byte_remain_len:   return teh remain len of byte
*******************************************************************************/
bool Huffman_Tree::byte_write(char *byte, uint16_t &code, int &code_remain_len, int &byte_remain_len)
{
    if (byte_remain_len == 8) //    it's an empty byte
    {
        *byte |= code;
        if (code_remain_len == 8) // a byte-long code
        {
            code_remain_len = 0;
            byte_remain_len = 8;
            return true;
        }
        else if (code_remain_len < 8) // code shorter than a byte
        {
            byte_remain_len = 8 - code_remain_len;
            code_remain_len = 0;
            return false;
        }
        else //  code longer than a byte
        {
            code_remain_len -= 8;
            byte_remain_len = 8;
            code >>= 8;
            return true;
        }
    }
    else //  not an empty byte
    {
        uint16_t temp = code << (8 - byte_remain_len);
        *byte |= temp;
        if (code_remain_len == byte_remain_len) //  code fits the bits
        {
            code_remain_len = 0;
            byte_remain_len = 8;
            return true;
        }
        else if (code_remain_len < byte_remain_len) //  code still can't fill the byte
        {
            byte_remain_len -= code_remain_len;
            code_remain_len = 0;
            return false;
        }
        else //  code overflow the byte
        {
            code_remain_len -= byte_remain_len;
            code >>= byte_remain_len;
            byte_remain_len = 8;
            return true;
        }
    }
}

void Huffman_Tree::encode_file()
{
    if (is_compressed)
    {
        std::cout << "already compressed!" << std::endl;
        return;
    }
    std::cout << "encoding..." << std::endl;

    fp_out = fopen(((filepath + ".huff").c_str()), "w");
    if (fp_out == nullptr)
    {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

    //  encode blocks, and write to output file, until EOF
    clearerr(fp_in);
    fseek(fp_in, 0, SEEK_SET);
    int byte_remain_len = 8;

    char *c = buf_en;   //  char in read buffer
    char *cur = buf_de; //  byte-buffer in write buffer

    memcpy(buf_de, &file_header, sizeof(Huff_Header)); //    write header
    cur += sizeof(Huff_Header);

    Code tmp[100];
    int i = 0;
    for (auto &it : bit_codes) //    write code to header
    {
        tmp[i].c = it.first;
        tmp[i].code = it.second.first;
        tmp[i].code_len = it.second.second;
        ++i;
        if ((cur + i * 4) > buf_de + BUF_SIZE) // write buffer is full
        {
            memcpy(cur, tmp, (i - 1) * 4);
            fwrite(buf_de, BUF_SIZE, 1, fp_out);
            memset(buf_de, 0, BUF_SIZE);
            tmp[0].c = tmp[i - 1].c;
            tmp[0].code = tmp[i - 1].code;
            tmp[0].code_len = tmp[i - 1].code_len;
            i = 1;
        }
        if (i == 100) //  code array is full
        {
            memcpy(cur, tmp, i * 4);
            cur += i * 4;
            i = 0;
        }
    }
    if (i != 0)
    {
        memcpy(cur, tmp, i * 4);
        cur += i * 4;
    }
    std::cout << "header size: " << file_header.total_code_num * 4 + 12 << std::endl;
    std::cout << "data size: " << file_header.total_char_num * (static_cast<float>(avg_codelen) / file_header.total_char_num) / 8 << std::endl;

    while (!feof(fp_in))
    {
        memset(buf_en, 0, BUF_SIZE);
        fread(buf_en, BUF_SIZE, 1, fp_in);
        c = buf_en; //   当初忘了在这里重置指针，导致写入数据不完整；耗费了很多调试时间

        while (c <= buf_en + BUF_SIZE && *c != '\0') //  encode a block
        {
            uint16_t code_to_write = reverse_bit_codes[*c].first;
            int code_remain_len = reverse_bit_codes[*c].second;

            while (code_remain_len) //  write a code to buffer
            {
                if (byte_write(cur, code_to_write, code_remain_len, byte_remain_len))
                    ++cur;
                if (cur == buf_de + BUF_SIZE) //  write buffer is full
                {
                    fwrite(buf_de, BUF_SIZE, 1, fp_out);
                    memset(buf_de, 0, BUF_SIZE);
                    cur = buf_de;
                }
            }
            ++c;
        }
    }
    (*cur == 0) ? fwrite(buf_de, cur - buf_de, 1, fp_out) : fwrite(buf_de, cur - buf_de + 1, 1, fp_out);
    is_constructed = true;
}

/*******************************************************************************
* functionality:
        show information about huffman encoding solution
* args:
* 		null
*******************************************************************************/
void Huffman_Tree::show_info()
{
    std::cout << std::endl
              << "char counts: " << weight.size() << std::endl
              << "total char counts: " << file_header.total_char_num << std::endl
              << "***************************************************" << std::endl;
    for (auto &it : weight)
    {
        std::cout << it.first << " (0x " << std::hex << static_cast<int>(it.first) << ")"
                  << ": " << std::dec << it.second << std::endl;
    }
    std::cout << std::endl
              << "encoding solution:" << std::endl
              << "***************************************************" << std::endl;
    std::cout << "WPL/avg code length: " << static_cast<float>(avg_codelen) / file_header.total_char_num << std::endl;
    for (auto &it : codes)
    {
        std::cout << it.first << ": " << it.second << std::endl;
    }
    std::cout << std::endl
              << "bits code:" << std::endl
              << "***************************************************" << std::endl;
    for (auto &it : bit_codes)
    {
        std::cout << it.first << ": " << it.second.first << " length: " << it.second.second << std::endl;
    }
    std::cout << std::endl
              << "reverse bits code:" << std::endl
              << "***************************************************" << std::endl;
    std::cout << "size of reverse: " << reverse_bit_codes.size() << std::endl;
    for (auto &it : reverse_bit_codes)
    {
        std::cout << it.first << ": " << it.second.first << " length: " << it.second.second << std::endl;
    }
    return;
}

/*******************************************************************************
* functionality:
        read huffman-code solution from an compressed file
* args:
* 		NULL
*******************************************************************************/
void Huffman_Tree::read_code_from_fp_in()
{
    fseek(fp_in, sizeof(Huff_Header), SEEK_SET);
    fread(buf_de, BUF_SIZE, 1, fp_in);
    char *cur = buf_de;
    Code tmp;
    for (int i = 0; i < file_header.total_code_num; i++)
    {
        memcpy(&tmp, cur, sizeof(Code));
        bit_codes[tmp.c].first = tmp.code;
        bit_codes[tmp.c].second = tmp.code_len;
        cur += sizeof(Code);
        if (cur >= buf_de + BUF_SIZE) // read out of buffer
        {
            fread(buf_de, BUF_SIZE, 1, fp_in);
            cur = buf_de;
        }
    }
}

/*******************************************************************************
* functionality:
        decode a compressed file into a new file
* args:
* 		NULL
*******************************************************************************/
void Huffman_Tree::decode_file()
{
    if (!is_compressed)
    {
        std::cout << "normal file, can't be decoded!" << std::endl;
        return;
    }
    fp_out = fopen((filepath + ".de").c_str(), "w");
    if (fp_out == nullptr)
    {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }
    long off = file_header.total_code_num * sizeof(Code) + sizeof(Huff_Header);
    clearerr(fp_in);
    fseek(fp_in, off, SEEK_SET);
    memset(buf_de, 0, BUF_SIZE);

    char *cur = buf_en;
    char *save = buf_de;
    uint16_t code = 0;
    uint32_t counter = file_header.total_char_num;
    char c = '\0';
    int len = 0;

    while (!feof(fp_in)) //   decode file
    {
        cur = buf_en;
        fread(buf_en, BUF_SIZE, 1, fp_in);
        while (counter != 0 && cur < buf_en + BUF_SIZE) //   decode block
        {
            // printf("%ld\n", cur - buf_en);
            for (int i = 0; i < 8; ++i) //   decode byte
            {
                ++len;
                code <<= 1;
                code |= ((*cur) >> i) & 1;
                auto it = bit_codes2.find({code, len});
                if (it != bit_codes2.end()) //    satisfy a code
                {
                    *save = it->second;
                    code = 0;
                    len = 0;
                    --counter;
                    ++save;
                    if (counter == 0)
                        break;

                    if (save == buf_de + BUF_SIZE)
                    {
                        fwrite(buf_de, BUF_SIZE, 1, fp_out);
                        memset(buf_de, 0, BUF_SIZE);
                        save = buf_de;
                    }
                }
            }
            ++cur;
        }
    }
    fwrite(buf_de, save - buf_de, 1, fp_out);
    return;
}