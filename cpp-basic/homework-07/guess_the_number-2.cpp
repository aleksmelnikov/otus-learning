#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <map>
#include <string>

// Bonus task 5: levels of difficulty. Level -> max value of the random number.
const std::map<int, int> levels = {
	{1, 10},
	{2, 50},
	{3, 100},
};

// Parse command line arguments. Returns the max value for the random number.
// A negative result means an error in the arguments (or the -table flag).
int handle_command_line(int argc, char** argv, bool& show_table) {
	show_table = false;
	int max_value = 100;
	bool has_max = false;
	bool has_level = false;

	for (int i = 1; i < argc; ++i) {
		std::string arg{ argv[i] };

		if (arg == "-max") {
			// Bonus task 1: the "-max" argument requires a numeric value
			if (i + 1 >= argc) {
				std::cout << "Wrong usage! The argument '-max' requires a value!" << std::endl;
				return -1;
			}
			max_value = std::stoi(argv[i + 1]);
			has_max = true;
			++i;
		}
		else if (arg == "-table") {
			// Bonus task 2: just show the table and exit
			show_table = true;
		}
		else if (arg == "-level") {
			// Bonus task 5: the "-level" argument requires a numeric value
			if (i + 1 >= argc) {
				std::cout << "Wrong usage! The argument '-level' requires a value!" << std::endl;
				return -1;
			}
			const int level = std::stoi(argv[i + 1]);
			auto it = levels.find(level);
			if (it == levels.end()) {
				std::cout << "Wrong level! Available levels: 1, 2, 3" << std::endl;
				return -1;
			}
			max_value = it->second;
			has_level = true;
			++i;
		}
		else {
			std::cout << "Unknown argument: " << arg << std::endl;
			return -1;
		}
	}

	// "-level" and "-max" simultaneously are an error (bonus task 5)
	if (has_max && has_level) {
		std::cout << "Wrong usage! Use either '-level' or '-max', not both!" << std::endl;
		return -1;
	}

	return max_value;
}

int get_random_value(const int max_value) {
	std::srand(std::time(nullptr)); // use current time as seed for random generator

	const int random_value = std::rand() % max_value;

	return random_value;
}

std::string ask_name() {
	std::cout << "Hi! Enter your name, please:" << std::endl;
	std::string user_name;
	std::cin >> user_name;
	return user_name;
}

int guess_the_number(const int target_value) {
	std::cout << "Enter your guess:" << std::endl;

	int current_value = 0;
	int attempts_count = 0;

	do {
		std::cin >> current_value;
		attempts_count++;

		if (current_value < target_value) {
			std::cout << current_value << " is less than mine" << std::endl;
		}
		else if (current_value > target_value) {
			std::cout << current_value << " is greater than mine" << std::endl;
		}
		else {
			std::cout << "you win! attempts = " << attempts_count << std::endl;
			break;
		}

	} while (true);

	return attempts_count;
}

// Read the table into a map holding the best (min) attempts for each user.
// Bonus task 3: only minimum values are kept.
std::map<std::string, int> read_best_scores(const std::string& high_scores_filename) {
	std::map<std::string, int> best;

	std::ifstream in_file{high_scores_filename};
	if (!in_file.is_open()) {
		return best;
	}

	std::string username;
	int high_score = 0;
	while (true) {
		in_file >> username;
		in_file >> high_score;
		in_file.ignore();

		if (in_file.fail()) {
			break;
		}

		auto it = best.find(username);
		if (it == best.end() || high_score < it->second) {
			best[username] = high_score;
		}
	}

	return best;
}

// Print the table with only the best result for each user.
void print_best_scores(const std::map<std::string, int>& best) {
	std::cout << "High scores table:" << std::endl;
	for (const auto& entry : best) {
		std::cout << entry.first << '\t' << entry.second << std::endl;
	}
}

// Bonus task 4: rewrite the table keeping the best result for each user.
// If the current result is better than the previous one, the previous is replaced.
void write_high_score(const std::string& high_scores_filename, const std::string& user_name, const int attempts_count) {
	auto best = read_best_scores(high_scores_filename);

	auto it = best.find(user_name);
	if (it == best.end() || attempts_count < it->second) {
		best[user_name] = attempts_count;
	}

	std::ofstream out_file{high_scores_filename};
	if (!out_file.is_open()) {
		std::cout << "Failed to open file for write: " << high_scores_filename << "!" << std::endl;
		return;
	}

	for (const auto& entry : best) {
		out_file << entry.first << ' ' << entry.second << std::endl;
	}
}

int main(int argc, char** argv) {

	const std::string high_scores_filename = "high_scores.txt";
	bool show_table = false;

	const int max_value = handle_command_line(argc, argv, show_table);
	if (max_value < 0) {
		return -1;
	}

	// Bonus task 2: print the table right away and exit
	if (show_table) {
		print_best_scores(read_best_scores(high_scores_filename));
		return 0;
	}

	const std::string user_name = ask_name();

	const int target_value = get_random_value(max_value);

	const int attempts_count = guess_the_number(target_value);

	write_high_score(high_scores_filename, user_name, attempts_count);

	print_best_scores(read_best_scores(high_scores_filename));

	return 0;
}
