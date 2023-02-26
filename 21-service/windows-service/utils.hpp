#include <fstream>
// 通常,不要将这样的函数放在hpp文件中
void WriteLog(const char* str)
{
    std::ofstream outfile;
    outfile.open("E:\\ServiceOutFile.txt", std::ios::out|std::ios::app);
    outfile << str << std::endl;
    return;
}