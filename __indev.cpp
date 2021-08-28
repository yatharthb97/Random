

#include<stdio.h>

bool ends_with(const std::string &filename, const std::string &ext)
{
  return ext.length() <= filename.length() &&
         std::equal(ext.rbegin(), ext.rend(), filename.rbegin());
}

int main()
{
    const char* file_name = "c:\\.directoryname\\file.name.with.too.many.dots.ext";
    GetFileExtension(file_name);
    return 0;
}