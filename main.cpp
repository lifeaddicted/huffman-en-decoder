/*******************************************************************************
 *		File Name:		main.cpp
 *		Discription:	file compression with huffman encoding
 *
 *
 *		Author:			Zj Nong
 *		Mail:			594201810@qq.com
 *******************************************************************************/
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <set>

#include "huffman_tree.h"

int main()
{
    Huffman_Tree huf("../txta.huff");
    huf.init();
    huf.encode_file();
    huf.decode_file();
    // huf.show_info();
    return 0;
}
