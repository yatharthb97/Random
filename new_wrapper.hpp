//Header file for a basic random number generator-seeded by random_device → <random.cpp>
//Also includes Class Distribution

#pragma once

#include "urandom.h"

#include <random>
#include <vector>
#include <chrono>
#include <sstream>
#include <fstream>


/// This block can be used to change the type rnd_error_t to any integer type of your choice.
#define RND_INT_CAST static_cast<unsigned int>
typedef rnd_error_cast_t unsigned int



/** @brief Error codes used within the library. */
enum class rnd_error_t : int
{
	Success = 0,
	EntropyShortage = 1,
	SeedNotFound = 2,
	FileError = 3,
	DistStateNotFound = 4

};

template <class PRNG>
class Rnd_Wrapper
{


private:
	
	Urandom nondet; //! Non-deterministic Source
	
public:

	//Check if compiles
	constexpr typename result_type = Engine::result_type; //! Defination of the return type of the Engine generator
	
	std::vector<Engine::result_type> Seed_list; 	//Stores the last used seed value.
	bool Warmed_up = false; 	//! Flag that indicates whether the engine has warmed up.
	
	PRNG Engine; //! PRNG template parameter
	std::normal_distribution<double> Gauss_dist; //! Gaussian distribution
	std::uniform_real_distribution<double> Uni_dist; //! Uniform distribution

	size_t Discarded_last; //! Last value of the number of States discarded for warm-up of the engine.
	size_t Discard_length; //! Totlal sum of the number of States discarded for warm-up of the engine.

	

	//0. Constructor
	RND(size_t discard = 0): Discard_length(discard)
	{
		//Explicit Initialization
      	Uni_dist.param(std::uniform_real_distribution<double>::param_type(0.0, 1.0));
      	Gauss_dist.param(std::normal_distribution<double>::param_type(0.0, 1.0));
	}

	//1
	/** @brief Warmups up the engine by discarding the given number.
	 * @param No parameter - `Discard_length` if not zero, else Non-deterministic discards. 
	 * @param (optional) - "any" - Non-deterministic discards. 
	 * @param (optional) discard_arg - Number of values discarded. - */
	[[Incomplete]]
	rnd_error_cast_t warm_up(size_t discard_arg = 0, const char* str_t  = "")
	{
		//No option is provided or "any" is passed
		if((discard_length == 0 && discard_arg == 0) || str_t == "any")
		{
			//Discard a random length
			
			nondet.open();
			rnd_error_cast_t advance = nondet.get(); 
			nondet.close();
			
			Discard_length += advance; //Adds to the discard_count
			Engine.discard(advance);
			return advance;
		}

		else if(discard_arg != 0)
		{
			Engine.discard(discard_arg);
			Discard_length += discard_arg;
			return discard_arg;
		}

		//Repeated Invocations will keep discarding equal to discard_length
		else if(Discard_length !=0) //Can be invoked in the case of an Old Seed
		{
			engine.discard(discard_length);
			if(warmed_up)
				discard_length += discard_length;
			return discard_length;
		}

		else
			return 0;
	}

	//2
	/** @brief Returns a vector of all the seed values in the current Seed_list. */
	std::vector<Engine::result_type> get_seed_list() const {
		return Seed_list;
		}

	//3
	/** @brief Returns the first seed value from the seed sequence. Usual use case is debugging by comparing runs without going through the whole sequence of seeds. */
	Engine::result_type get_first_seed() const {
		if(Seed_list.size() > 0)
			return seed_list[0];
		else
			return 0;
		}

	//4
	/** @brief Use the default seed of the generator as the seed. */
	rnd_error_cast_t inline default_seed() __attribute__((flatten)) {
		
		return new_single_seed(Engine::default_seed);
		}

	//5
	/**@brief Uses the current Epoch time for a single seed. */
	rnd_error_cast_t inline seed_by_time() __attribute__((flatten)) {
		
		auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		return new_single_seed(seed);
		
		}

	//6
	/** @brief Seed the Engine with a non-deterministic single seed. */
	rnd_error_cast_t inline new_single_seed(unsigned int x_seed) __attribute__((flatten)) {
		
		if(!Seed_list.is_empty())
		{
			this->free_seed_mem();
		}
		
		seed_list.emplace_back(x_seed); //Write to Seed_list
		Engine.seed(x_seed);
		Warmed_up = false;
		return RND_INT_CAST(this->seed_status());
		}

	//7
	/** @brief Seeds the PRNG with `Engine::state_size` seeds from non-deterministic source. */
	rnd_error_cast_t inline new_seeds() __attribute__((alias("new_state"))) {
		Seed_list.reserve(Engine::state_size);

	    bool open_status = nondet.open();
	    for(unsigned int i = 0; i < Engine::state_size; i++)
	    {
	    	seed_list.emplace_back(nondet.get());
	    }
	    nondet.close();
	    

	    std::seed_seq seeds(std::begin(seed_list), std::end(seed_list));
	    Engine.seed(seeds);
	    Warmed_up = false;

	    return (!open_status)* RND_INT_CAST<unsigned int>(rnd_error_t::FileError) + 
	           (open_status)* RND_INT_CAST<unsigned int>(rnd_error_t::Success);
		}

	//8
	/** @brief Saves seeds to a file. If the file is a json file, the function calls save_state() instead. 
	 * \attention This is a legacy functionality and should only be used in limited cases. */
	rnd_error_cast_t inline save_seeds(std::string filename) {
		std::ifstream fout(filename, std::ios::out);
		if(! fout.is_open())
			return RND_INT_CAST(rnd_error_t::FileError);
		
		if(has_ext(filename, "json"))
		{
			fout.close();
			return this->save_state(filename, false);
		}

		else
		{
			for(unsigned int i = 0; i < Seed_list.size(); i++)
			{
				fout << Seed_list[i] << '\n';
			}
			fout.close();
			return this->seed_status();
		}

		} //End of save_seeds()

	//9
	/** @brief Loads "old" seeds from a file. 
	 * \attention If a json state file is provided, only the "engine_state" is used as seeds — the distribution states are ignored. */
	rnd_error_cast_t old_seeds(std::string filename) {

	    seed_list.reserve(Engine::state_size);

	    std::ifstream seed_file(filename, std::ios::in);
	    
	    if(!seed_file.is_open())
	    {
	    	return RND_INT_CAST(rnd_error_t::FileError);
	    }
		    
	    if(this->has_json_ext(filename))
	    {
	    	json jfile;
	    	jfile << seed_file;

	    	if(jfile.contains("engine_state"))
	    	{
	    		Engine << jfile["engine_state"];
	    		Seed_list = jfile["engine_state"].get<std::vector<Engine::result_type>>();
	   	 		Warmed_up = false;
	    		this->reload_seeds();

	    		return seed_status();
	    	}

	    	else
	    		{ return RNDCAST(rnd_error_t::SeedNotFound); }
	    }

	    else //Assume simple line seperated ASCII values
	    {
	    	unsigned int seeds_read = 0; unsigned int tmp = 0;
			while(seed_file >> tmp && i < Engine::state_size)
			{
				seed_list.emplace_back(tmp);
				seeds_read++; 
			}

		    std::seed_seq seeds(std::begin(seed_list), std::end(seed_list));
		    Engine.seed(seeds);
		    
		    return seed_status();
		}

		}

	//10
	/** @brief Repeatedly calls the passed generating function to generate a seed sequence of length `Engine::state_size`. */
	rnd_error_cast_t generate_seed_with(Engine::result_type (*generating_fn)()) {
		
		this->free_seed_mem();
		std::generate(Seed_list.begin(), Seed_list.end(), generating_fn);

	    std::seed_seq seeds(std::begin(Seed_list), std::end(Seed_list));
	    Engine.seed(seeds);

	    Warmed_up = false;

	    return RND_INT_CAST(this->seed_status());
	    	
		}


	//11
	/** @brief Save state of the Engine and Distributions to a json state file.
	 *  @param fileneame : state file that will be created or overriden. 
	 * @param (optional) include_discard : Passing true includes the Discard_length parameter to the state file.
	 * \attention If the param filename does not have a json extension '.json', the function will append it. */
	rnd_error_cast_t save_state(const std::string filename, bool include_discard = false) {
		using json = nlohmann::json;
		json state_object;
				
		std::ofstream fout(filename, std::ios::out);
		state_object["engine_state"] << std::setw(4) << Engine; //Save engine state
		state_object["gauss_state"] << std::setw(4) << Gauss_dist;
		state_object["uni_state"] << std::setw(4) << Uni_dist;

		if(include_discard)
			{ state_object["discard"] = this->Discard_length; }

		if(! this->has_json_ext(filename))
			{ filename.append(".json") }
		

		std::ofstream fout(filename, std::ios::out);
		if(!fout.is_open())
			return RND_INT_CAST(rnd_error_t::FileError);
		
		fout << state_object;
		fout.close();

		return RND_INT_CAST(rnd_error_t::Success);

		} //End of save_state()

	//12
	/** @brief Restore State of Engine and the distributions from a json state file. The json file must contain three fields: "engine_state", "gauss_state", and "uni_state". If any of these state is missing, the corresponding error is returned.
	 * @param filename : source state file.
	 * @param (optional) include_discard: If the state file contains a "discard" field, the generator is warmed up with that discard value as well. */
	rnd_error_cast_t load_state(const std::string filename,  bool include_discard = false) {

		std::ifstream fin(filename, std::ios::in);
		if(! fin.is_open())
			return RND_INT_CAST(rnd_error_t::FileError);
		
		using json = nlohmann::json;
		json state_object;
		fin >> state_object;

		if(state_object.contains("engine_state"))
		{
			Engine << state_object["engine_state"];
			Seed_list = state_object["engine_state"].get<std::vector<Engine::result_type>>();
		}
		else { return RND_INT_CAST(rnd_error_t::SeedNotFound); }
		

		auto error_state = RND_INT_CAST(rnd_error_t::Success);


		if(state_object.contains("gauss_state"))
			{ Gauss_dist << state_object["gauss_state"]; }
		else {error_state = RND_INT_CAST(rnd_error_t::DistStateNotFound);}


		if(state_object.contains("uni_state"))
			{ Uni_dist << state_object["uni_state"]; }
		else {error_state = RND_INT_CAST(rnd_error_t::DistStateNotFound);}

		if(state_object.contains("discard") && include_discard)
		{	
			this->Discard_length = state_object["discard"]; 
			this->warm_up();	
		}

		fin.close()
		
		if (error_state == RND_INT_CAST(rnd_error_t::DistStateNotFound) && this->seed_status() != rnd_error_t::SeedNotFound)
		{
			return error_state;
		}

		else
			{ return RND_INT_CAST(this->seed_status()); }

		
		} //End of load_state()

	//13
	/** @brief Reloads the Engine with the Seed_list state.
	 * \attention This function does not call warm_up(). */
	rnd_error_cast_t reload_seeds() {

	    std::seed_seq seeds(std::begin(Seed_list), std::end(Seed_list));
	    Engine.seed(seeds);

	    return this->seed_status();
		}

/** defgroup distributions "Distribution Functions". *\
/* @{ */
	
	//14
	/** @brief Return a gaussian distributed random number in range [0,1) from the PRNG Engine */
	double gauss() __attribute__((flatten)) {
		return Gauss_dist(this->Engine());
		}

	//15
	/** @brief Return a uniformly distributed random number in range [0,1) from the PRNG Engine. */
	double uni() __attribute__((flatten)) {
		return Uni_dist(this->Engine());
		}


	//16
	/** @brief Return a gaussian distributed random number in range [0,1) from a non-deterministic engine. */
	double gauss_nondet() __attribute__((flatten)) {
		return Gauss_dist(this->get_nondet());
		}


	//17
	/** @brief Return a uniformly distributed random number in range [0,1) from a non-deterministic engine. */
	double uni_nondet() __attribute__((flatten)) {
		return Uni_dist(this->get_nondet());
		}


	//18
	/** @brief Get raw bytes - sizeof(Engine::result_type) - random number from the  non-determinstic source of the Wrapper. */
	Engine::result_type get_nondet() __attribute__((flatten)) {
		
		if(!nondet.is_open())
		{
			nondet.open();
		}

		return nondet.get();
		}


	//19
	/** @brief Get raw bytes - sizeof(Engine::result_type) - random number from the  non-determinstic source of the Wrapper. Recommended for single use as it closes the non-deterministic stream upon completion. */
	rnd_error_cast_t get_nondet_nclose() __attribute__((flatten)) {
		
		if(!nondet.is_open()) //Open
		{
			nondet.open();
		}

		auto tmp =  nondet.get(); //Read
		
		nondet.close(); //Close

		return tmp;
		}

/* @} */ //End of Group "Distribution Functions"


	//20
	/** @brief This function clears the Seed_list.*/
	void inline free_seed_mem() __attribute__((flatten)){
		std::vector<Engine::result_type> tmp_vec;
		tmp_vec.swap(seed_list);
		//On exit -> seed_list(which is now tmp_vec) is destroyed and freed.
		
		} // End of free_seed_mem()


	//21
	/** @brief Returns the number of seeds in the current seed list. */
	unsigned int inline get_seedlist_size() const {
		return Seed_list.size();
		}


private:
//Helper Functions

	/** @brief Checks whether the file has a given extension extension. */
	bool has_ext(std::string filename, sts::string ext) const {
		return bool(filename.substr(filename.find_last_of(".") + 1) == ext);
		}

	/** @brief Returns the seeding status based on the size of the Seed_list. */
	rnd_error_cast_t seed_status() const {
		if(Seed_list.size() == 0)
			return RND_INT_CAST(rnd_error_t::SeedNotFound);
		else if(Seed_list.size() < Engine::state_size)
			return RND_INT_CAST(Engine::EntropyShortage);
		else
			return RND_INT_CAST(rnd_error_t::Success);
		}
};




