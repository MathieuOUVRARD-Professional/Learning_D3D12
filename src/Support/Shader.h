#pragma once

#include <Support/WinInclude.h>

#include<fstream>
#include<cstdlib>
#include<filesystem>
#include<string_view> 

class Shader
{
	public:
		Shader(std::string_view name);
		//~Shader();

		inline const char* GetBuffer() const { return m_data.data(); }
		inline const size_t GetSize() const { return m_size; }

	private:
		std::vector<char> m_data;
		size_t m_size = 0;
};
