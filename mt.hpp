//Header file for a basic random number generator-seeded by random_device → <random.cpp>
//Also includes Class Distribution

#define __MT19937_RAND_DEFAULT_WARMUP__ 700000

//Reference - "Improved long-period generators based on linear recurrences modulo 2", F. Panneton, P. L'Ecuyer, M. Matsumoto in AVM TOMS Volume 32 Issue 1, March 2006 Pages 1-16
using Rnd_Wrapper<std::mt19937>(__MT19937_RAND_DEFAULT_WARMUP__) = MT_RND;

#pragma once
#include "urandom.h"
#include <random>
#include <chrono>
#include <sstream>
#include <fstream>

#define __MT19937_RAND_DEFAULT_WARMUP__ 700000
//Reference - "Improved long-period generators based on linear recurrences modulo 2", F. Panneton, P. L'Ecuyer, M. Matsumoto in AVM TOMS Volume 32 Issue 1, March 2006 Pages 1-16


//RND Return Codes
enum class RND_RC : int
{
	Success = 0,
	EntropyShortage = 1,
	SeedNotFound = 2,
	FileError = -1

};

//Extern function
const char* RND_RC_Str(RND_RC x)
{
	if (x == RND_RC::AllSuccess)
		return "AllSuccess";
	else if(x == RND_RC::EntropyShortage)
		return "EntropyShortage";
	else if(x == RND_RC::SeedNotFound)
		return "SeedNotFound";
} // End of RND_RC_Str()



class MT_RND
{

private:
	std::vector<unsigned int> seed_list; //Store Seds
	bool warmed_up;
	size_t discard_length; //Discard Length
	Urandom urandom; //Instantiate with unsigned int type
	

public:

	std::mt19937 engine; //Random MT Engine
	//std::string name; //Optional Name given to the engine


	//Constructor
	MT_RND(/*std::string name = "",*/ 

		  ) : /*name(name),*/ /*discard_length(discard),*/ warmed_up(false)
	{
		discard_length = __MT19937_RAND_DEFAULT_WARMUP__ ;
		//Empty Body --> Allows more complex operations.
	}

	void TimeSeed() //Currently Only Supports the current time.
	{
		auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		SingleSeed(seed);
	}

	void SingleSeed(unsigned int x_seed)
	{
		#if SpS_COMPILETIME_WARNINGS == 1
			static_assert(false, "[WARNING] MT_RND::SingleSeed() produes low entropy - low period engines.\n");
		#endif

		//Free-up vector
		if(!seed_list.empty())
		{
			FreeSeedMem();
		}
		seed_list.emplace_back(x_seed); //Write the seed
		engine.seed(x_seed);
	}

	//Specific Construction of Seed List vector??

	//Needs to be explicitly called
	size_t Discard(size_t discard_arg = 0, const char* str_t  = "")
	{
		if((discard_length == 0 && discard_arg == 0) || str_t == "any")
		{
			//Discard a random length
			
			urandom.Open();
			size_t advance = urandom.get<unsigned int>(); 
			//!!! size_t is not the template arguement for urandom class
			
			discard_length += advance; //Adds to the discard_count
			engine.discard(advance);
			return advance;
		}

		else if(discard_arg != 0)
		{
			engine.discard(discard_arg);
			discard_length += discard_arg;
			return discard_arg;
		}

		//Repeated Invocations will keep discarding equal to discard_length
		else if(discard_length !=0) //Can be invoked in the case of an Old Seed
		{
			engine.discard(discard_length);
			if(warmed_up)
				discard_length += discard_length;
			return discard_length;
		}

		else
			return 0;
	}


	std::vector<unsigned int> getSeedList()
	{
		return seed_list;
	}

	unsigned int FirstSeed() const
	{
		if(seed_list.size() > 0)
			return seed_list[0];
		else
			return 0.0;
	}


	RND_RC NewSeeds() 
	{
	    // Magic number 624: The number of unsigned ints the MT uses as state
	    //std::array<std::mt19937::state_size, unsigned int> rnd_seed_array;

		seed_list.reserve(std::mt19937::state_size);


	    //Open Urandom_handler -> Instantiate
	    urandom.Open();


	    //Read Sequentially
	    for(unsigned int i = 0; i < std::mt19937::state_size; i++)
	    {
	    	seed_list.emplace_back(urandom.get<unsigned int>());
	    }

	    //std::generate(begin(seed_list), end(seed_list), urandom.get());


	    urandom.Close();

	    std::seed_seq seeds(std::begin(seed_list), std::end(seed_list));
	    engine.seed(seeds);


	    engine.discard(discard_length);
	    warmed_up = true;


	    return RND_RC::AllSuccess;
	
	} //End of NewSeed


	//Old Seed Does not Call std::mt19937::discard()
	//What is the file-format? JSON, newline seperated value list, Binary?
	RND_RC OldSeed(std::string &path_to_seed) 
	{
	    // Magic number 624: The number of unsigned ints the MT uses as state
	    //std::array<std::mt19937::state_size, unsigned int> rnd_seed_array = {0};


	    seed_list.reserve(std::mt19937::state_size);

	    std::ifstream seed_file(path_to_seed, std::ios::in); //Open file
	    
	    if(!seed_file.is_open())
	    {
	    	return RND_RC::SeedNotFound;
	    }
		    
		//for(unsigned int i = 0; i < std::mt19937::state_size; i++)

		int i = 0; unsigned int tmp = 0;
		while(seed_file >> tmp && i < std::mt19937::state_size)
		{
			seed_list.emplace_back(tmp);
			i++; 
		}

	    std::seed_seq seeds(std::begin(seed_list), std::end(seed_list));
	    
	    engine.seed(seeds); //PRNG Seeded
	    
	    if(i < std::mt19937::state_size)
	    {
	    	return RND_RC::EntropyShortage;
	    }

	    else
	    	return RND_RC::AllSuccess;

	} //End of Old Seed


	//OK
	void FreeSeedMem()
	{
		//Create a temporary vector
		std::vector<unsigned int> tmp_vec;
		tmp_vec.swap(seed_list); //Swap

		//On exit -> seed_list(which is now tmp_vec) is destroyed and freed.
	}

	//OK
	void SaveSeed(const std::string &filename)
	{
		
		std::ostringstream buffer;

		for(unsigned int i = 0; i < seed_list.size(); i++)
			{buffer << seed_list[i] << '\n';}

		std::ofstream fout(filename, std::ios::out);
		fout << buffer.str();

		//RAII fout close expected
	}

	//Ok
	void SaveState(const std::string &filename)
	{

		std::ofstream fout(filename, std::ios::out);
		fout << engine; //Save engine state

		//RAII fout close expected
	}

	//Ok
	void LoadState(const std::string &filename)
	{

		std::ifstream fin(filename, std::ios::in);
		fin >> engine; //Save engine state

		//RAII fin close expected
	}

	//Ok
	unsigned int SeedSize()
	{
		return seed_list.size();
	}

	//OK
	//Does not close file
	template <typename T>
	T get_nondet()
	{
		if(urandom.is_open())
		{
			urandom.Open();
		}

		return urandom.get<T>();
	}

	//Ok
	//Closes urandom after read
	template <typename T>
	T get_nondet_nclose()
	{
		//Open
		if(!urandom.is_open())
		{
			urandom.Open();
		}

		//Read
		T tmp =  urandom.get<T>();

		//Close
		urandom.Close();

		return tmp;
	}


};




