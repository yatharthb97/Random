

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



// Two dimensional distribution

std::string Disribution2DSequerntial(uint64_t draw_size = uint64_t(1e3))
{
    resut_t* x_axis = new double[draw_size/2]();
    resut_t* y_axis = new double[draw_size/2]();

    //Draw numbers

}