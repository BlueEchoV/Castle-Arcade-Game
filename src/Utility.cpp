#include "Utility.h"
#include "SDL.h"
#include <string>
#include <assert.h>

V2 operator+(const V2& a, const V2& b) {
	V2 result = {};
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

V2 operator-(V2& a, V2& b) {
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

V2 operator/(const V2& V2, float scalar) {
	return { V2.x / scalar, V2.y / scalar };
}

void my_Memory_Copy(void* dest, const void* src, size_t count) {
	unsigned char* destination = (unsigned char*)dest;
	unsigned char* source = (unsigned char*)src;
	for (int i = 0; i < count; i++) {
		destination[i] = source[i];
	}
}

float calculate_Distance(float x1, float y1, float x2, float y2) {
	return (float)sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

float calculate_Distance(V2 v_1, V2 v_2) {
	return (float)sqrt((v_2.x - v_1.x) * (v_2.x - v_1.x) + (v_2.y - v_1.y) * (v_2.y - v_1.y));
}


float linear_Interpolation(float left_Point, float right_Point, float percent) {
	// Lerp of T = A * (1 - T) + B * T
	// A is the left side, B is the right side, T is the percentage of the interpolation
	return ((left_Point) * (1 - percent) + (right_Point)*percent);
}

// Operator overload
// Multiply this V2 by this scalar (0.5)
V2 calculate_Center(float w, float h) {
	V2 result = { w / 2, h / 2 };
	return result;
}

V2 calculate_Direction_V2(V2 target, V2 start) {
	V2 result = {};

	result.x = target.x - start.x;
	result.y = target.y - start.y;

	float length = (float)sqrt((result.x * result.x) + (result.y * result.y));

	result.x /= length;
	result.y /= length;

	return result;
}

float random_Float_In_Range(float min, float max) {
	if (min > max) {
		my_Swap(min, max);
	}
	float result = max - min;
	// Random number between 0.0f - 1.0f
	float temp = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	result *= temp;
	result += min;

	return result;
}

V2 random_Vector_In_Range(V2 min, V2 max) {
	return { random_Float_In_Range(min.x, max.x), random_Float_In_Range(min.y, max.y) };
}

size_t file_Last_Modified(std::string file_Name) {
	const char* ptr = file_Name.c_str();
	struct stat file_Buffer = {};

	if (stat(ptr, &file_Buffer) != 0) {
		char errorMessage[256];
		strerror_s(errorMessage, sizeof(errorMessage), errno);
		SDL_Log("ERROR: stat failed for %s, Error: %s", ptr, errorMessage);
	}

	return file_Buffer.st_mtime;
}

// Splits a string using a delimiter and returns a vector of strings
std::vector<std::string> split(const std::string& my_String, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	// Input stream class to operate on strings
	std::istringstream my_Stream(my_String);
	while (std::getline(my_Stream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

void load_CSV_Data(CSV_Data* csv_Data, char* destination, size_t stride, std::span<Type_Descriptor> type_Descriptors) {
	// Check if the file was open prior to calling the function
	bool close_File = false;
	if (!csv_Data->file.is_open()) {
		// Adds flexibility to the function
		csv_Data->file.open(csv_Data->file_Path);
		if (!csv_Data->file.is_open()) {
			SDL_Log("Error loading .csv file");
			return;
		}
		else {
			// Set for closing at the end of the function. Defer has issues here 
			// because defer closes the file when it goes out of scope.
			close_File = true;
		}
	}

	csv_Data->file.seekg(0, std::ios::beg);
	std::string line;
	std::getline(csv_Data->file, line);
	if (line == "") {
		// No rows exist
		return;
	}
	std::vector<std::string> column_Names = split(line, ',');

	int current_Row = 0;
	while (std::getline(csv_Data->file, line)) {
		std::vector<std::string> tokens = split(line, ',');

		// Pointer arithmetic: Calculate a pointer 'write_Ptr' to the destination in memory 
		//					   where the data will be written. The stride determines the offset
		//					   between rows in the destination memory.
		// NOTE: using a uint8_t* ensures that each increment or decrement of the pointer corresponds 
		//		 to one byte.
		char* write_Ptr = destination + (current_Row * stride);
		current_Row++;

		for (int i = 0; i < type_Descriptors.size(); i++) {
			// Grab the descriptor we are currently on in the loop
			Type_Descriptor* type_Descriptor = &type_Descriptors[i];
			// This is for finding the correct token
			int column_Index = get_Column_Index(column_Names, type_Descriptor->column_Name);
			if (column_Index <= -1) {
				continue;
			}
			if (type_Descriptor->variable_Type == DT_INT) {
				// Add the variable offset to get to the correct position in memory
				int* destination_Ptr = (int*)(write_Ptr + type_Descriptor->variable_Offset);
				*destination_Ptr = std::stoi(tokens[column_Index]);
			}
			else if (type_Descriptor->variable_Type == DT_FLOAT) {
				float* destination_Ptr = (float*)(write_Ptr + type_Descriptor->variable_Offset);
				*destination_Ptr = std::stof(tokens[column_Index]);
			}
			else if (type_Descriptor->variable_Type == DT_STRING) {
				std::string* destination_Ptr = (std::string*)(write_Ptr + type_Descriptor->variable_Offset);
				*destination_Ptr = tokens[column_Index];
			}
		}
	}
	if (close_File) {
		csv_Data->file.close();
	}
}

int count_CSV_Rows(std::string file_Name) {
	// ifstream closes the file automatically
	std::ifstream file(file_Name);

	if (!file.is_open()) {
		SDL_Log("Error loading .csv file");
	}

	std::string line;
	std::getline(file, line);
	int total_Rows = 0;
	while (std::getline(file, line)) {
		total_Rows++;
	}

	// assert(total_Rows > 0);
	return total_Rows;
}

int count_CSV_Rows(CSV_Data* csv_Data) {
	std::string line;
	std::getline(csv_Data->file, line);
	int total_Rows = 0;
	while (std::getline(csv_Data->file, line)) {
		total_Rows++;
	}
	return total_Rows;
}

int get_Column_Index(const std::vector<std::string>& column_Names, const std::string& current_Column_Name) {
	int result = -1;
	for (int i = 0; i < column_Names.size(); i++) {
		if (column_Names[i] == current_Column_Name) {
			result = i;
			break;
		}
	}
	assert(result >= 0);
	return result;
}