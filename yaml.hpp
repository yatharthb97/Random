#pragma once
#include <iostream>
#include <algorithm>
#include <string>



/** @brief A very brief and simple YAML parser/writer.*/


/** @brief An object that just holds an open file descriptor and conveneience functions that convert variables to valid YAML syntax. */
class YamlFile
{
	uint8_t level = 0;
public:
	std::string filename;
	uint8_t mode;
	std::fstream file;
	std::vector<std::string> parsed;


	//0
	YamlFile(std::string filename, uint8_t mode = std::ios::out, std::string indent = "\t"): filename(filename), mode(mode)
	{}

	//1
	/** @brief Open file for operation. 
	 * Automatically inits a new YAML document if the file mode is not input (`st::ios::in`). */
	bool open()
	{
		file.open(filename, mode);
		bool open_ =  file.is_open();
		if(open_ && !(mode & (1 << sd::ios::in) != 0)) //not input mode
		{
			doc_start();
		}
		return open_;
	}

	//2
	/** @brief Close file for operation. */
	bool close()
	{
		file.close();
		return !file.is_open();
	}

	//3
	/** @brief Parse a file to an internal variable `parsed`.
	 * Function only works if the file mode is input `in`.*/
	void parse()
	{
		if(mode & (1 << std::ios::in))
		{
			std::string ymlstr;
			uint8_t ymlend = uint8_t(false);
			enum ymlendstate {false_ = 0, first = 1, second = 2};
			unsigned int lineno = 0;

			while (std::getline(file, ymlstr) && ymlend != second)
			{
			    lineno++;
			    // Line contains string of length > 0 then save it in vector
			        if(ymlstr == "---" && ymlend == false_)
			        	ymlend = first;
			        else if(ymlstr == "---" && ymlend == first)
			        	ymlend = second;
			        else if(ymlstr.size() > 0)
			        	parsed.push_back(ymlstr);
			        else
			        	std::cerr << "YAML::parse failed at line: " << lineno << ".\n";
			}
		}
	}

	//! Actual useful functions

	//4
	/** @brief Add a single key-value pair. */
	template <typename KeyType, typename ValueType>
	void add_val(KeyType key, ValueType value)
	{
		//Write text
		for(unsigned int i = 0; i < level; i++)
			file << indent;
		
		file << key << " : " << value << '\n';
	}

	//5
	/** @brief Add a homogenous array with a given key.
	 * \warning Only outputs multiline arrays. */
	template <typename KeyType, typename ContainerType, bool multiline=true>
	void add_array(KeyType key, ContainerType container, uint64_t size=0)
	{
		//Write text
		for(unsigned int i = 0; i < level; i++)
			file << indent;
		
		file << key << " :";
		
		if(multiline)
		{
			file << ' \n';
			level++;
			if(size == 0)
				size = container.size();
			for(uint64_t i = 0; i < size; i++)
			{
				// Print array indent
				for(unsigned int i = 0; i < level; i++)
					file << indent;

				//Print value
				file << "- " << container[i] << '\n';
			}
			level--;
		}

		else
		{
			file << ' [';

			for(uint64_t i = 0; i < size-1; i++)
			{
				//Print value
				file << container[i] << ", ";

			}
			file << container[size-1] << ']';
		}		
	}

	//6
	/** @brief Read a value from the file for a given key. */
	template <typename KeyType, typename ValueType>
	ValueType read_val(KeyType key)
	{
		using std::vector<std::string>::iterator = piter_t;
		piter_t found = Find(key);


	}

	//7
	/**Read an array from the file for a given key. */
	template <typename KeyType, typename ValueType>
	std::vector<ValueType> read_array(KeyType key)
	{}

	//8
	/** @brief Create a new level within the current hierarchy. */
	template <typename KeyType>
	void push_level(KeyType key)
	{
		for (unsigned int i = 0; i < indent; i++)
			file << indent;
		if(add_dash)
			file << "- ";

		file << key << " :\n";
		level++;
	}

	//9
	/** @brief Pop the current level by one unless the level is zero.*/
	void pop_level()
	{
		level = (level > 0 * level - 1) + (level == 0 * 0)
	}

	//10
	/** @brief Get the current level in the structured hierarchy. */
	void get_level()
	{
		return level;
	}

	//11
	/** @brief Closes all levels and resets the current level to zero. */
	void reset_level()
	{
		level = 0;
	}

	//12
	/** @brief Put the `start-document` string `---` to the file at level zero.*/
	void doc_start()
	{
		reset_level();
		file << '---' << '\n';
	}

	//13
	/** @brief Put the `end-document` string `...` to the file at level zero. */
	void doc_end()
	{
		reset_level();
		file << '...' << '\n';
	}

	//-----<// YAML Non-numbers.
	
	//14
	static std::string inf() { return ".inf"  }

	//15
	static std::string minus_inf() { return "-.Inf" }

	//16
	static std::string nan() { return ".NAN"  }
	//-----<// YAML Non-numbers.

	//17
	/** @brief Returns a YAML boolean value.
	 * @param bin Binary state (C++ boolean).
	 * @param mode  Default is "on-off", other option is "true-false". */
	static bool boolean(bool bin, std::string mode ="true-false")
	{
		 transform(mode.begin(), mode.end(), mode.begin(), ::tolower);

		 if(mode == "on-off") 
		 {
		 	if(bin)
		 		return "On"
		 	else
		 		return "Off"
		 }

		 else
		 {
		 	if(bin)
		 		return "True"
		 	else
		 		return "False"
		 }
	}

	//18
	/** @brief Returns a quoted string for any passed string.
	 * @param str A string*/
	static std::string quoted_str(std::string str)
	{
		std::string s = "\"";
		s += str;
		s += "\"";
		return s;
	}

	//19
	/** @brief Returns the block string version of the given string. */
	static std::string block_str(std::string str)
	{
		std::string s = "\"";
		s += " >\n";
		s += str;
		return s; 
	}

	//20
	/** @brief Add the string as a comment. */
	void add_comment(std::string str)
	{
		file << "# " << str << '\n';
	}


	/** @brief Add a blank key to the file. */
	template <typename KeyType>
	void add_key(KeyType key, bool add_dash=false)
	{
		for (unsigned int i = 0; i < indent; i++)
			file << indent;
		if(add_dash)
			file << "- ";

		file << key << " : ";
	}

	/** @brief Dumps the value in the file at the current place. */
	template <typename ValueType>
	void add(ValueType value) { file << value; }

	//! Experimental

	static std::string chomp_preserve() { return ">+"; }
	static std::string chomp_strip() { return "|-"; }

	void add_dict()
	{ return; }


};
