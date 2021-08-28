


#pragma once
#include<unistd.h>
#include<fcntl.h>


template <typename Typename>
class Urandom
{

	int FileDesc; //File descriptor for open
	bool Open_flag;
	unsigned int Read_Size_Bytes;

public:

	//Constructor -> Sets the read size
	Urandom(): FileDesc(-1), Open_flag(false), Read_Size_Bytes(sizeof(Typename))
	{}


	~Urandom()
	{
		if(Open_flag)
		{
			close(FileDesc);
		}
	}

	bool inline open() __attribute_((always_inline))
	{
		FileDesc = open("/dev/urandom", O_RDONLY, O_NONBLOCK); //Non-blocking mode implemented

		if (FileDesc != -1)
		{
			Open_flag = true;
			return true;
		}

		return false;
	}

	bool inline is_open() __attribute__((always_inline))
	{
		return Open_flag;
	}


	void inline close()
	{
		Open_flag = false;
		close(FileDesc);

	}

	Typename inline get()
	{
		Typename read_value; //Explicitly use garbage value
		ssize_t rcode = read(FileDesc, &read_value, sizeof(Typename));
		return read_value;
	}

	template <typename Type2>
	Type2 inline get()
	{
		Type2 read_value; //Explicitly use garbage value
		ssize_t rcode = read(FileDesc, &read_value, sizeof(Typename));
		return read_value;
	}

};



