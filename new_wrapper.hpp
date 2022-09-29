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
using rnd_error_cast_t = unsigned int;
#define RND_INT_CAST static_cast<rnd_error_cast_t>






/** \brief Describes a wrapper object that takes a `STL Pseudo Random Number Generator`
 *  object as the template arguement and constructs a wrapper object that manages
 *  **seeding, checkpointing, and distribution states**. 
 *  PRNGType - PRNG Engine
 *  DISCARD - Number of states to discard for warm-up (optional). */
template <class PRNGType, size_t DISCARD = 0>
class PRNGWrapper
{


private:
	Urandom nondet; //!< Non-deterministic Source
public:

	/** @brief Error codes used within the library. */
	enum prng_error_t : uintfast8_t
	{
		Success = 0,          //!< Operation Successful
		EntropyShortage = 1,  //!< Seeds available are less than the state size
		SeedNotFound = 2,     //!< Seed(s) Not Found
		FileError = 3,        //!< File Not Found Error
		DistStateNotFound = 4 //!< Distribution(s) states not found

	};

	constexpr PRNGType Engine; //!< PRNG Engine
	std::normal_distribution<double> Gauss_dist; //!< Gaussian distribution
	std::uniform_real_distribution<double> Uni_dist; //!< Uniform distribution

	constexpr typename result_type = Engine::result_type; 
	//!< Defination of the return type of the Engine generator
	
	std::vector<result_type> Seed_list; //!< Stores the last used seed value(s)
	
	//bool Warmed_up = false; //! Flag that indicates whether the engine has warmed up.
	//size_t Discarded_last; //! Last value of the number of States discarded for warm-up of the engine.
	constexpr size_t DiscardLength = DISCARD; //! Totlal sum of the number of States discarded for warm-up of the engine.

	

	//0
	/** \brief Constructor that accepts a default discard length.
	 * @param discard - Overrides the template DISCARD parameter. */
	PRNGWrapper(size_t discard = 0) {
		if(discard != 0)
		{
			DiscardLength = discard;
		}
		
		//Explicit Initialization of ranges
      	Uni_dist.param(std::uniform_real_distribution<double>::param_type(0.0, 1.0));
      	Gauss_dist.param(std::normal_distribution<double>::param_type(0.0, 1.0));
		}


	//1
	/** @brief Set the limits of the gaussian (normal) distribution.
	 * @param lower the lower limit.
	 * @param upper the upper limit. */
	void set_gauss_limits(double lower, double upper) __attribute__((always_inline)) {
		static_assert(upper > lower, "The upper limit must be larger than the lower limit.");
		Gauss_dist.param(std::normal_distribution<double>::param_type(lower, upper));	
		}


	//2
	/** @brief Set the limits of the uniform distribution. 
	 * @param lower the lower limit.
	 * @param upper the upper limit. */
	void set_uni_limits(double lower, double upper) __attribute__((always_inline)) {
		static_assert(upper > lower, "The upper limit must be larger than the lower limit.");
		Uni_dist.param(std::uniform_real_distribution<double>::param_type(lower, upper));	
		}


	//3
	/** @brief Warmups up the engine by discarding the given number.
	 * @param No parameter - `Discard_length` if not zero, else Non-deterministic discards. 
	 * \param (optional) - "any" - Non-deterministic discards. 
	 * \param (optional) discard_arg - Number of values discarded. - */
	[[Incomplete]]
	prng_error_t warm_up(size_t discard_arg = 0, const char* str_t  = "") {
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


	//4
	/** @brief Returns a vector of all the seed values in the current Seed_list. */
	std::vector<Engine::result_type> get_seedlist() const {
		return Seed_list;
		}

	//5
	/** @brief Returns the first seed value from the seed sequence. 
	 * Usual use case is debugging by comparing runs without going through the whole
	 * sequence of seeds. */
	Engine::result_type inline get_first_seed() const {
		if(Seed_list.size() > 0)
			return seed_list[0];
		else
			return 0;
		}

	//6
	/** @brief Use the default seed of the generator as the seed. */
	prng_error_t inline default_seed() __attribute__((flatten)) {
		
		return new_single_seed(Engine::default_seed);
		}

	//7
	/**@brief Uses the current Epoch time for a single seed. */
	prng_error_t inline seed_by_time() __attribute__((flatten)) {
		
		auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		return new_single_seed(seed);
		
		}

	//6
	/** @brief Seed the Engine with a passed single seed.
	 * @param x_seed Single seed value */
	template <typename SeedType>
	prng_error_t inline new_single_seed(SeedType x_seed) {
		
		if(!Seed_list.is_empty())
		{
			this->free_seedlist();
		}
		
		seed_list.emplace_back(x_seed); //Write to Seed_list
		Engine.seed(x_seed);
		Warmed_up = false;
		return this->seed_status();
		}

	//8.0
	/** @brief Seeds the PRNG using `Engine::state_size` seeds from a non-deterministic source.
	 * @param do_discard (optional) Supress engine warm-up by passing `true`. */
	prng_error_t inline new_seeds(bool do_discard = true) {
		
		Seed_list.reserve(Engine::state_size);

	    bool open_status = nondet.open();
	    
	    for(unsigned int i = 0; i < Engine::state_size; i++)
	    {
	    	Seed_list.emplace_back(nondet.get<result_type>());
	    }
	    nondet.close();
	    
	    std::seed_seq seeds(std::begin(Seed_list), std::end(Seed_list));
	    Engine.seed(seeds);
	    
	    Warmed_up = do_discard;
	    Engine.discard(do_discard * DiscardLength);

	    return (!open_status) * prng_error_t::FileError + 
	            (open_status) * prng_error_t::Success;
		}

	//8.1
	/** \brief Alias for `new_seeds()`. */
	prng_error_t inline new_state(bool do_discard = true) __attribute__ ((weak, alias ("new_seeds")));


	//9
	/** \brief Saves seeds to a ASCII file. This function cannot handle `json` files.
	 * \attention This function returns `prng_error_t::FileError` if a `json` file is passed. 
	 * This is done to prevent problems with automatic checkpointing 
	 * \attention This is a legacy functionality and should only be used in limited
	 *  cases. */
	prng_error_t inline save_seeds(std::string filename) {
		
		std::ifstream fout(filename, std::ios::out);
		if(!fout.is_open())
			{ return prng_error_t::FileError; }
		
		if(has_ext(filename, "json"))
		{
			fout.close();
			return prng_error_t::FileError;
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


	//10
	/** @brief Loads "old" seeds from a file. 
	 * \attention If a json state file is provided, only the "engine_state" is used as seeds — the distribution states are ignored. */
	prng_error_t old_seeds(std::string filename) {

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
			while(seed_file >> tmp && seeds_read < Engine::state_size)
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
	prng_error_t generate_seed_with(Engine::result_type (*generating_fn)()) {
		
		this->free_seedlist();
		std::generate(Seed_list.begin(), Seed_list.end(), generating_fn);

	    std::seed_seq seeds(std::begin(Seed_list), std::end(Seed_list));
	    Engine.seed(seeds);

	    Warmed_up = false;

	    return RND_INT_CAST(this->seed_status());
	    	
		}


	//11
	/** \brief Save state of the Engine and Distributions to a json state file.
	 * \param fileneame  state file that will be created or overriden. 
	 * \param (optional) include_discard  Passing true includes the Discard_length parameter to the state file.
	 * \attention If the param filename does not have a json extension '.json', the function will append it. */
	prng_error_t save_state(const std::string filename, bool include_discard = false) {
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
	prng_error_t load_state(const std::string filename,  bool include_discard = false) {

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
	/** \brief Reloads the Engine with the Seed_list state.
	 * \attention This function does not call warm_up(). */
	prng_error_t reload_seeds() {

	    std::seed_seq seeds(std::begin(Seed_list), std::end(Seed_list));
	    Engine.seed(seeds);

	    return this->seed_status();
		}


	//14
	/** @brief Return a gaussian distributed random number in range [0,1)
	 * from the PRNG Engine and the inbuilt Gaussian distribution. */
	double gauss() __attribute__((flatten)) {
		return Gauss_dist(this->Engine());
		}


	//15
	/** @brief Return a uniformly distributed random number in range [0,1)
	 * from the PRNG Engine and the inbuilt Uniform distribution. */
	double uni() __attribute__((flatten)) {
		return Uni_dist(this->Engine());
		}


	//16
	/** @brief Return a gaussian distributed random number in range [0,1) from a
	 *  non-deterministic engine. */
	double gauss_nondet() __attribute__((flatten)) {
		return Gauss_dist(this->get_nondet());
		}


	//17
	/** @brief Return a uniformly distributed random number in range [0,1) from a
	 *  non-deterministic engine. */
	double uni_nondet() __attribute__((flatten)) {
		return Uni_dist(this->get_nondet());
		}


	//18
	/** \brief Get raw bytes - sizeof(Engine::result_type) - random number from a
	 * non-determinstic source.
	 * \warning This function does not close the non-deterministic source. */
	Engine::result_type get_nondet() __attribute__((flatten)) {
		
		if(!nondet.is_open())
		{
			nondet.open();
		}

		return nondet.get<result_type>();
		}


	//19
	/** \brief Get raw bytes - sizeof(Engine::result_type) - random number from the
	 *   non-determinstic source of the Wrapper. Recommended for single use as it closes
	 *  the non-deterministic stream upon completion. */
	Engine::result_type get_nondet_nclose() __attribute__((flatten)) {
		
		if(!nondet.is_open()) //Open
		{
			nondet.open();
		}

		auto tmp =  nondet.get<result_type>(); //Read
		
		nondet.close(); //Close

		return tmp;
		}


	//20
	/** \brief This function clears the Seed_list. */
	void inline free_seedlist() __attribute__((flatten)) {
		std::vector<Engine::result_type> tmp_vec;
		tmp_vec.swap(seed_list);
		//On exit -> seed_list(which is now tmp_vec) is destroyed and freed.
		}


	//21
	/** \brief Returns the number of seeds in the current seed list. */
	unsigned int inline get_seedlist_size() const {
		return Seed_list.size();
		}

	class Experimental
	{
	public:
		template <size_t NoBins>
		std::array<size_t, NoBins> sampler(samples_drawn, bool plot, bool restore_state = true)
		{

		}


		template <size_t SamplesDrawn>
		double performance(size_t SamplesDrawn, double(*rnd_fnc)())
		{
			std::array<double, SamplesDrawn> tmp;

			using namespace std::chrono;
			auto start = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch());

			for(size_t i = 0; i < SamplesDrawn; i++)
			{
				tmp = rnd_fnc()
			}
			auto end = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch());

			return (end - start);
		}

		/** @brief Uses the TestU01 suite to evaluate the statistical properties of the random stream and print/returns a summary string.*/
		void testU01()
		{

		}

		/** @brief Alias of `testU01()` function.*/
		void stat_analysis() __attribute__((weak, alias("testU01")));

	};

private:
//Helper Functions

	/** \brief Checks whether the file has a given extension extension. */
	bool has_ext(std::string filename, sts::string ext) const {
		return bool(filename.substr(filename.find_last_of(".") + 1) == ext);
		}

	/** \brief Returns the seeding status based on the size of the Seed_list. */
	prng_error_t seed_status() const {
		if(Seed_list.size() == 0)
			return prng_error_t::SeedNotFound;
		else if(Seed_list.size() < Engine::state_size)
			return prng_error_t::EntropyShortage;
		else
			return prng_error_t::Success;
		}
};