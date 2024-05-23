#pragma once
#include <cmath>
#include <stdlib.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <span>
#include <stdint.h>

// I need stable indices for this to work
struct Handle {
	// Bit fields (unsigned int index : 10;)
	// uint16_t is just way better
	uint64_t index;
	uint64_t generation;
};
struct Generation {
	bool slot_Taken = false;
	// Default generation 1
	uint16_t generation = 1;
};
int count_Active_Handles(Generation generations[], int size);
bool compare_Handles(Handle handle_1, Handle handle_2);

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
// Suppress compiler warnings
#define REF(V) ((void)V)

// DEFER
template <typename T>
struct ExitScope
{
	T lambda;
	ExitScope(T lambda) : lambda(lambda) { }
	~ExitScope() { lambda(); }
	// NO_COPY(ExitScope);
};

struct ExitScopeHelp
{
	template<typename T>
	ExitScope<T> operator+(T t) { return t; }
};

#define _SG_CONCAT(a, b) a ## b
#define SG_CONCAT(a, b) _SG_CONCAT(a, b)
#define DEFER auto SG_CONCAT(defer__, __LINE__) = ExitScopeHelp() + [&]()

struct V2{
	float x;
	float y;
};

template<typename T>
void my_Swap(T& a, T& b) {
	T temp = a;
	a = b;
	b = temp;
}

V2 operator+(const V2& a, const V2& b);
V2 operator-(V2& a, V2& b);
V2 operator/(const V2& V2, float scalar);

void my_Memory_Copy(void* dest, const void* src, size_t count);

float calculate_Distance(float x1, float y1, float x2, float y2);
float calculate_Distance(V2 v_1, V2 v_2);

float linear_Interpolation(float left_Point, float right_Point, float percent);

V2 calculate_Center(float w, float h);
V2 calculate_Direction_V2(V2 target, V2 start);

float random_Float_In_Range(float min, float max);

V2 random_Vector_In_Range(V2 min, V2 max);

size_t file_Last_Modified(std::string file_Name);

enum Data_Type {
	// You can use int for bool as well (0 or non 0)
	DT_INT,
	DT_FLOAT,
	DT_STRING
};

struct Type_Descriptor {
	Data_Type variable_Type;
	int variable_Offset;
	std::string column_Name;
};

// Macro named FIELD
// data_Type: This is the type of the field, passed as an argument to the macro.
// offsetof(struct_Type, name): This macro is used to determine the offset of a member within a struct.
//					  It returns the byte offset of name within the struct_Type. This assumes 
//					  that name is a member of the struct_Type.
// #name: This is a preprocessor operator that turns the name into a string literal.
#define FIELD(struct_Type, data_Type, name) { data_Type, offsetof(struct_Type, name), #name }

struct CSV_Data {
	std::string file_Path;
	size_t last_Modified_Time;
	std::ifstream file;
	// store the rows here? OR, a return value as well or an out parameter
	// 65,536 values (rows)
	// 64 bit registers
	uint16_t rows;
};

int count_CSV_Rows(CSV_Data* csv_Data);
CSV_Data create_Open_CSV_File(std::string file_Path);
void close_CSV_File(CSV_Data* csv_Data);
// Making these const refs is MUCH better performance wise because there is a significant amount of memory not being copied in
int get_Column_Index(const std::vector<std::string>& column_Names, const std::string& current_Column_Name);
std::vector<std::string> split(const std::string& my_String, char delimiter);
void load_CSV_Data(CSV_Data* csv_Data, char* destination, size_t stride, std::span<Type_Descriptor> type_Descriptors);