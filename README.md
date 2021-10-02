# `PRNGWrapper` 

## `PRNGWrapper` for C++ STL PRNGs



`PRNGWrapper` is a wrapper around the STL C++ pseudo-random number generators. The inspiration behind the `class` implementation is the correct usage of these modules which suffer from known defects. The other thing that this library accomplished is provide an easy mechanism for checkpointing of the `state` of these generators which is essential for numerical simulations.

## Generate `doxygen` Documentation

```bash
# Install doxygen (https://www.doxygen.nl/manual/install.html)
sudo apt-get install doxygen

cd <Repository Root Folder>

# Generate Documentation
doxygen PRNGWDoxyFile
```

## Features

*  Full initalization (seeding) of the `state_size` of the engine, from non-deterministic source.
  + Using just one seed value results in the initial state that is a small subset of the total state size of the generator. Hence, it directly limits the number of independent and non-overlapping random streams the generator can produce.

+   Provides a mechanism for warm-up of generators
    + Many PRNGs require a warm-up run to recover from bad initalization states. These generators hence produce low-quality streams for a while before settling down. 
+   The C++ standard does not guarantee the initalization states of random distributions, hence, changing the compiler might break reproducibility of the streams. The class implements methods that saves the distribution states as well.
+   Resource initalization is verbose and bulky and requires constructing multiple objects. This `class` encapsulates them into a single object and provides easy to understand functions. 
+   The library also implements commonly used features and an error propagation mechanism.
+   Saves the last set of seeds that were used with the generator.




## Seeding Methods

```C++
//0 .Initalize Object
PRNGWrapper<mt::19937> mt;

//1. Seed the full state_size of the PRNG from non-deterministic source [Recommended]
mt.new_seeds(warmup=70,000);

//2. Seed & Warmup the generator by discarding first 70,000 points
mt.new_seeds(warmup=70,000);

// Same as `new_seeds()`
mt.new_state();

//3. Use the default seed defined by the standard
mt.default_seed();

//4. Use epoch time as single seed
mt.seed_by_time();

//5. Use a user defined single seed
mt.new_single_seed(12345);

//6. Use another function to seed the complete `state_size` of the generator
unsigned int generating_fn() {/*Another generator function*/}
mt.generate_seed_with(generating_fn);
```

 ## Checkpointing

```c++
// Save just the seed values to a file(like its usually done)
mt.save_seeds("file.dat");

//Load seeds from a file - "file.dat"
mt.old_seeds("file.dat");

//Save the Wrapper state (PRNG seeds and distribution seeds to a json file)
mt.save_state("file.json");

//Load a PRNGWrapper state from a json state file
mt.load_state("file.json");

// Reload the seeds that were saved from the last initalization
// Useful when samples are drawn for statistical testing, and the stream can be restarted
mt.reload_seeds();
```

## Generating Numbers

```c++
// All functions return doubles

double rnd = mt.uni(); //Draw a number [0,1) from the uniform distribution
double rnd = mt.gauss(); //Draw a number [0,1) from the gaussian (normal) distribution

// Draw a number [0,1) from non-deterministic source and piped through gaussian (normal) distribution
double rnd = mt.gauss_nondet();  

// Draw a number [0,1) from non-deterministic source and piped through uniform distribution
double rnd = mt.uni_nondet();

 // Returns a non-deterministic number of the raw result type of the PRNG
double rnd = mt.get_nondet();

// Returns a non-deterministic number of the raw result type of the PRNG and 
//closes the non-deterministic source  (recommended for single use case)
double rnd = mt.get_nondet_nclose();
```

##  Saved Seeds

```c++
mt.get_seedlist(); //Returns the last used seeds as a std::vector
mt.free_seedlist(); //Delete the saved seeds
mt.get_seedlist_size(); //Returns the size of the seedlist
```



## Error Propagation

```c++
enum PRNGWrapper::prng_error_t : uintfast8_t
{
	Success = 0,
	EntropyShortage = 1, //!< Seeds available are less than the state size
	SeedNotFound = 2, //!< Seed(s) Not Found
	FileError = 3, //!< File Not Found Error
	DistStateNotFound = 4 //!< Distribution(s) states not found
};
```

