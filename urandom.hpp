#pragma once
#include<unistd.h>
#include<fcntl.h>
#include <type_traits>


/** @brief Describes a RAII wrapper around the `/dev/urandom` file for linux systems that 
 * reads random bits and constructs type that is passed as the template arguement. */
class Urandom
{

	int FileDesc; //!< File descriptor for open
	bool OpenFlag; //!< Open Status of the urandom file

public:

	/** @brief Constructor -> Sets the read size. */
	Urandom(): FileDesc(-1), OpenFlag(false)
	{}

	/** @brief Destructor -> Closes the file. */
	~Urandom()
	{
		if(OpenFlag)
		{
			close(FileDesc);
		}
	}

	/** @brief Opens the urandom file. */
	bool inline open() __attribute__((always_inline))
	{
		FileDesc = open("/dev/urandom", O_RDONLY, O_NONBLOCK); //Non-blocking mode implemented

		if (FileDesc != -1)
		{
			Open_flag = true;
			return true;
		}

		return false;
	}

	/** @brief Returns the open status of the urandom file object. */
	bool inline is_open() __attribute__((always_inline))
	{
		return OpenFlag;
	}

	/** @brief Close the file without destroying the object. */
	void inline close() __attribute__((always_inline))
	{
		Open_flag = false;
		close(FileDesc);

	}


	/** @brief Reads `sizeof(Type)` number of random bits and return the constructed type. Template specialization */
	template <typename Type>
	Type inline get()
	{
		
		if constexpr (std::is_floating_point<Type>) 
		{}

		else
		{
			Type read_value; //Explicitly use garbage value
			ssize_t rcode = read(FileDesc, &read_value, sizeof(Type));
			return read_value;
		}
		
	}

};



