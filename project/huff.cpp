#include <vector>
#include <deque>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <algorithm>

using namespace std;

// Huffman Tree Node
struct Node
{
    Node(Node *left = nullptr, Node *right = nullptr, char c = '\0', unsigned int f = 0) : left(left), right(right),
                                                                                           c(c), f(f), code()
    {}

    Node(const Node &other) : left(other.left), right(other.right), c(other.c), f(other.f), code(other.code)
    {}

    Node *left;
    Node *right;
    char c;
    unsigned int f;
    string code;
};

/**
 * Comparison class to make min heap
 * Descending order.
 */
struct NodeDescendingComparisonByFrequency
{
    bool operator()(Node *const first, Node *const second)
    {
        return first->f > second->f;
    }
};

/**
 * Comparison class
 */
struct NodeAscendingComparisonByChar
{
    bool operator()(Node *const first, Node *const second)
    {
        return (unsigned int) (first->c) < (unsigned int) (second->c);
    }
};

/**
 * Converts single char to 8 bit binary repr.
 */
string CharToBinaryCode(const char &c)
{
    string bin;
    for (unsigned int y = 0; y < sizeof(char) * 8; y++)
        bin.insert(bin.begin(), (c & (1 << y)) ? '1' : '0');
    return bin;
}

/**
 * Reads input stream and encodes in binary repr.
 */
void ReadBinaryCodesFromStream(ifstream &ifs, deque<char> &ss)
{
    char aux = 0;
    while (ifs.good())
    {
        aux = ifs.get();
        if (!ifs.good())
        {
            break;
        }
        string tmp = CharToBinaryCode(aux);
        ss.insert(ss.end(), tmp.data(), tmp.data() + tmp.length());
    }
}

// converts binary repr to char
char ConvertBinaryCodeToChar(const char *code)
{
    char c = 0;
    for (unsigned int y = 0; y < sizeof(char) * 7; y++)
        c = (c | (code[y] - '0')) << 1;
    return c | (code[7] - '0');
}

// write bit stream to file
void WriteBinaryCodesToStream(deque<char> &ss, ofstream &of)
{
    while (ss.size() > 0)
    {
        of << ConvertBinaryCodeToChar(&ss[0]);
        ss.erase(ss.begin(), ss.begin() + 8);
    }
}

// Serialize Huffman tree, also make encoder mapping
void SerializeHuffmanTree(Node *n, map<char, string> &encoder, deque<char> &ss)
{
    //only left and right links of leaf may be NULL
    if (n->left == nullptr)
    {
        ss.push_back('1');
        string tmp = CharToBinaryCode(n->c);
        ss.insert(ss.end(), tmp.data(), tmp.data() + tmp.length());
        //cout<<cur->c<<" "<<cur->f<<endl;
        encoder[n->c] = n->code;
    } else
    {
        ss.push_back('0');
        n->left->code.append(n->code);
        n->left->code.append("0");
        n->right->code.append(n->code);
        n->right->code.append("1");
        SerializeHuffmanTree(n->left, encoder, ss);
        SerializeHuffmanTree(n->right, encoder, ss);
    }
}

/**
 * Deserialize Huffman Tree
 */
void DeserializeHuffmanTree(Node *n, deque<char> &ss)
{
    char aux = ss.front();
    ss.erase(ss.begin());
    if (aux == '1')
    {
        string tmp(ss.begin(), ss.begin() + 8);
        n->c = ConvertBinaryCodeToChar(&tmp[0]);
        ss.erase(ss.begin(), ss.begin() + 8);
    } else
    {
        n->left = new Node();
        n->right = new Node();
        if (n->left == nullptr || n->right == nullptr)
        {
            cout << "memory allocation error!" << endl;
            exit(1);
        }
        n->left->code.append(n->code);
        n->left->code.append("0");
        n->right->code.append(n->code);
        n->right->code.append("1");
        DeserializeHuffmanTree(n->left, ss);
        DeserializeHuffmanTree(n->right, ss);
    }
}

/*
 * decode bit stream to bytes.
 * recursively traverse tree until leaf.
 */
void DecodeHuffmanTree(Node *n, deque<char> &ss, ofstream &of)
{
    if (n->left == nullptr)
    {
        of << n->c;
    } else
    {
        char aux = ss.front();
        ss.erase(ss.begin());
        if (aux == '0')
        {
            DecodeHuffmanTree(n->left, ss, of);
        } else
        {
            DecodeHuffmanTree(n->right, ss, of);
        }
    }
}

void EncodeInputStream(ifstream &ifs, deque<char> &buffer, map<char, string> &encoder)
{
    while (ifs.good())
    {
        char aux = ifs.get();
        if (!ifs.good())
            break;
        string &tmp = encoder[aux];
        buffer.insert(buffer.end(), tmp.data(), tmp.data() + tmp.length());
    }
}

void CalculateFrequencies(ifstream &ifs, set<Node*, NodeAscendingComparisonByChar> &nodesSortedByFrequencies)
{
    while (ifs.good()) //read file and puts to set
    {
        char aux = ifs.get();
        if (!ifs.good())
        {
            break;
        }
        Node *candidate = new Node(nullptr, nullptr, aux, 1);
        auto ret = nodesSortedByFrequencies.insert(candidate);
        if (!ret.second)
        {
            delete candidate;
            ++(*(ret.first))->f;
        }
    }
}

void BuildHuffmanTree(set<Node *, NodeAscendingComparisonByChar> &nodesSortedByFrequencies, vector<Node *> &nodes)
{    //to use heap push set elements to vector
    for (auto itr = nodesSortedByFrequencies.begin(); itr != nodesSortedByFrequencies.end(); itr++)
    {
        nodes.push_back(*itr);
    }
    nodesSortedByFrequencies.clear();
    make_heap(nodes.begin(), nodes.end(), NodeDescendingComparisonByFrequency());
    while (nodes.size() > 1)
    {
        //pops two least element from heap and push new merged
        //of these nodes
        pop_heap(nodes.begin(), nodes.end(), NodeDescendingComparisonByFrequency());
        Node *first = nodes.back();
        nodes.pop_back();
        pop_heap(nodes.begin(), nodes.end(), NodeDescendingComparisonByFrequency());
        Node *second = nodes.back();
        nodes.pop_back();
        Node *parent = new Node(first, second, '\0', first->f + second->f);
        nodes.push_back(parent);
        push_heap(nodes.begin(), nodes.end(), NodeDescendingComparisonByFrequency());
    }
}

void Encoder(char *in_filename, char *out_filename)
{
    ifstream ifs(in_filename, ifstream::in | ifstream::binary);
    if (!ifs.is_open())
    {
        cout << "file open error!" << endl;
        exit(1);
    }
    set<Node*, NodeAscendingComparisonByChar> nodesSortedByFrequencies;
    vector<Node*> nodes; //holds heap
    CalculateFrequencies(ifs, nodesSortedByFrequencies);
    ifs.close();

    BuildHuffmanTree(nodesSortedByFrequencies, nodes);

    deque<char> buffer; //buffer to hold bit stream
    map<char, string> encoder; //huffman tree encodings
    SerializeHuffmanTree(nodes.front(), encoder, buffer); //write tree to file and set mapping


    ifs.open(in_filename, ifstream::in | ifstream::binary);
    if (!ifs.is_open())
    {
        cout << "file open error!" << endl;
        exit(1);
    }
    EncodeInputStream(ifs, buffer, encoder);

    ofstream ofs(out_filename, ofstream::out | ifstream::binary);
    if (!ofs.is_open())
    {
        cout << "file write error!" << endl;
        exit(1);
    }
    unsigned int length = buffer.size();
    unsigned int trimmedLength = (length / 8) * 8;
    unsigned int padding = length - trimmedLength;
    if (padding != 0)
    {
        padding = 8 - padding;
    }
    while(padding > 0)
    {
        buffer.push_back('0');
        --padding;
    }
    ofs.write((const char *) &length, sizeof(unsigned int));
    WriteBinaryCodesToStream(buffer, ofs);
    ofs.close();
}

void Decoder(char *in_filename, char *out_filename)
{
    Node *root = new Node(); //huffman tree root
    deque<char> buffer; //buffer of '1's and '0's,
    ifstream ifs(in_filename, ifstream::in | ifstream::binary);
    if (!ifs.is_open())
    {
        cout << "file open error!" << endl;
        exit(1);
    }
    unsigned int length; //compressed size
    ifs.read((char *) &length, sizeof(unsigned int));
    unsigned int trimmedLength = (length / 8) * 8;
    unsigned int padding = length - trimmedLength;
    if (padding != 0)
    {
        padding = 8 - padding;
    }
    ReadBinaryCodesFromStream(ifs, buffer);//read all file to buffer converting to bits

    while (padding > 0)
    {
        buffer.erase(buffer.end());
        --padding;
    }

    ofstream ofs(out_filename, ofstream::out | ofstream::binary);
    if (!ofs.is_open())
    {
        cout << "file write error!" << endl;
        exit(1);
    }

    DeserializeHuffmanTree(root, buffer);//read tree

    while (buffer.size() > 0) {
        //decode and write file as readed from tree.
        DecodeHuffmanTree(root, buffer, ofs);
    }
}

/**
 * Handles with arguments.
 */
int main(int argc, char **argv)
{
    if (argc == 4 && argv[1][0] == '-')
    {
        if (argv[1][1] == 'e')
        {
            Encoder(argv[2], argv[3]);
        } else if (argv[1][1] == 'd')
        {
            Decoder(argv[2], argv[3]);
        } else
        {
            cout << "Invalid arguments!" << endl;
            return 1;
        }

    } else
    {
        cout << "Invalid arguments!" << endl;
        return 1;
    }
    return 0;
}
