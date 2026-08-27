#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>

int get_random_value() {
	const int max_value = 100;

	std::srand(std::time(nullptr)); // use current time as seed for random generator
	const int random_value = std::rand() % 100;

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

void write_high_score(const std::string& high_scores_filename, const std::string& user_name, const int attempts_count) {
	// We should open the output file in the append mode - we don't want
	// to erase previous results.
	std::ofstream out_file{high_scores_filename, std::ios_base::app};
	if (!out_file.is_open()) {
		std::cout << "Failed to open file for write: " << high_scores_filename << "!" << std::endl;
		return;
	}

	// Append new results to the table:
	out_file << user_name << ' ';
	out_file << attempts_count;
	out_file << std::endl;
}

void read_high_scores(const std::string& high_scores_filename) {
	std::ifstream in_file{high_scores_filename};
	if (!in_file.is_open()) {
		std::cout << "Failed to open file for read: " << high_scores_filename << "!" << std::endl;
		return;
	}

	std::cout << "High scores table:" << std::endl;

	std::string username;
	int high_score = 0;
	while (true) {
		// Read the username first
		in_file >> username;
		// Read the high score next
		in_file >> high_score;
		// Ignore the end of line symbol
		in_file.ignore();

		if (in_file.fail()) {
			break;
		}

		// Print the information to the screen
		std::cout << username << '\t' << high_score << std::endl;
	}
}

int main() {

	const std::string high_scores_filename = "high_scores.txt";

	const std::string user_name = ask_name();

	const int target_value = get_random_value();

	const int attempts_count = guess_the_number(target_value);

	write_high_score(high_scores_filename, user_name, attempts_count);

	read_high_scores(high_scores_filename);

	return 0;
}
