#include <iostream>
#include <sstream>

std::string hist_gen()
{
	unsigned int cols = 10;
	unsigned int cols_margin = 4;
	unsigned int rows = 10;

	wchar_t y_arr =  '^';
	wchar_t x_arr =  '>';
	wchar_t y_line = '|';
	wchar_t x_line = '—';
	wchar_t fill =   '#';
	wchar_t fill2 =  '*';
	wchar_t fill3 =  '0';
	wchar_t cross =  'x';


	char hist_matrix[rows][cols] = {' '};

	// Draw features
	hist_matrix[rows-1][cols_margin] = fill3; //Origin
	hist_matrix[0][cols_margin] = y_arr;
	hist_matrix[rows-1][cols-1] = x_arr; //Origin
	
	//Lines
	for(unsigned int r = 1; r < rows-2; r++) { hist_matrix[r][cols_margin+1] = y_line; }
	for(unsigned int c = 1; c < cols-2; c++) { hist_matrix[rows-1][c] = x_line; }




	//Matrix to string
	std::ostringstream buffer;
	for(unsigned int i = 0; i < cols; i++)
	{
		for(unsigned int j = 0; j < cols; j++)
		{
			buffer << hist_matrix[i][j];
		}

		buffer << '\n';
	}

	std::cout << "Histogram\n" << buffer.str() << std::endl;

}

void reload_state()
{
	return;
}

void sample(sample_fn, std::uint64_t samples=100000)
{
	
	//verify that bins is a multiple of 10.


	//Allocate memory
	std::uint64_t count_hist[bins] = {0};
	double hist[bins] = {0.0};

	//Draw random number and generate rt histogram
	double roll = 0;
	for(std::uint64_t i = 0; i < samples; i++)
	{
		roll = this->sample_fn();
		count_hist[unsigned int(round(p*bins))]++;
	}

	for (auto i : count_hist) { std::cout << i << std::endl;} //DEBUG


	//Normalization
	std::uint64_t max_count = *std::max_element(count_hist, count_hist+bins);
	for(unsigned int i = 0; i < bins; i++)
	{
		hist[i] = count_hist[i] * max_cols/max_count;
	}


	// Reload state
	reload_state();


	//Return hist
	return hist;
}
