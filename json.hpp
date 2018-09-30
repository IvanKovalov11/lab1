#include <iostream>
#include <any>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <fstream>

class Json {
private:
	char C = '?';
	enum type { object, array, null };
	type flag = null;
	std::unordered_map<std::string, std::any> map;
	std::vector<std::any> Array;
public:
	Json()
	{
		flag = null;
	}

	bool end_of_stream(std::istringstream &s)
	{
		char t = '?';
		if (s >> t) {
			return false;
		}
		return true;
	}

	Json(const std::string& s)
	{
		std::istringstream stream(s);
		char c;
		stream >> c;
		switch (c)
		{
		case('{'): {
			flag = object;
			map = object_parsing(stream);
			if (!end_of_stream(stream)) throw std::length_error("error");
			break;
		}
		case('['): {
			flag = array;
			Array = array_parsing(stream);
			if (!end_of_stream(stream)) throw std::length_error("error");
			break;
		}
		default:
			throw std::logic_error("input_error");
		}
	}

	bool is_array() const
	{
		if (flag == array) return true;
		return false;
	}

	bool is_object() const
	{
		if (flag == object) return true;
		return false;
	}

	bool is_null() const
	{
		if (flag == null) return true;
		return false;
	}

	const std::any& operator[](const std::string& key) const
	{
		return map.at(key);
	}

	const std::any& operator[](int index) const
	{
		return Array.at(index);
	}

	static Json parse(const std::string& s)
	{
		return Json(s);
	}

	static Json parseFile(const std::string& path_to_file)
	{
		std::string str;
		char c;
		std::ifstream stream(path_to_file);
		if (!stream.is_open())
			throw std::invalid_argument("input_error");
		else
		{
			while (stream >> c)
			{
				str.push_back(c);
			}
			stream.close();
		}
		return Json(str);
	}
private:
	std::any return_type(std::istringstream &stream, bool & success)
	{
		char c;
		if (!(stream >> c)) throw std::runtime_error("time");
		switch (c)
		{
		case('\"'): {
			std::string str;
			while ((stream >> c) && (c != '\"')) {
				str.push_back(c);
			}
			if (c == '\"') success = false;
			return str;
		}
		case('f'): {
			std::string str;
			str.push_back(c);
			while (stream >> c)
			{
				if (c == ',') break;
				if (c == ']') {
					success = false;
					C = ']';
					break;
				}
				if (c == '}') {
					success = false;
					break;
				}
				str.push_back(c);
			}
			if (str != "false") throw std::logic_error("input_lit");
			else return false; // ? 
		}
		case('t'): {
			std::string str;
			str.push_back(c);
			while (stream >> c)
			{
				if (c == ',') break;
				if (c == ']') {
					success = false;
					C = ']';
					break;
				}
				if (c == '}') {
					success = false;
					break;
				}
				str.push_back(c);
			}
			if (str != "true") throw std::logic_error("input_lit");
			else return true; // ? 
		}
		case('['): {
			Json obj;
			auto v_t = array_parsing_in_obj(stream);
			obj.flag = array;
			if (C == ']') return obj;
			obj.Array = v_t;
			if (C == 'F') C = 'E';
			else C = 'f';
			success = false;
			return obj;
		}
		case(','): {
			throw std::logic_error("input_error");
		}
		case('{'): {
			Json obj;
			auto map_t = object_parsing(stream);
			obj.flag = object; // ?
			if (C == '}' || C == 'e') {
				if (C == 'e') C = 'E';
				else C = 'f';
				return obj;
			}
			obj.map = map_t;
			success = false;
			return obj;
		}
		case(']'): {
			C = '0';
			return 0;
		}
		default: {
			std::string str;
			str.push_back(c);
			while (stream >> c) {
				if (c == ',' || c == '}' || c == ']') break;
				str.push_back(c);
			}
			if (c == '}' || c == ']') {
				if (c == '}') C = 'E';
				if (c == ']') C = ']';
				success = false;
			} // !!!!!
			std::stringstream ss(str);
			double temp = 0;
			ss >> temp;
			return temp;
		}
		}
		return 4.0f;
	}

	std::vector<std::any> array_parsing_in_obj(std::istringstream& stream)
	{
		char c = '?';
		bool success = true;
		std::any value;
		std::vector<std::any> v;
		while (success)
		{
			value = return_type(stream, success);
			if (C == '0') return v; // ?
			v.push_back(value);
			if (C == ']' && !success) {
				C = 'f';
				break;
			}
			if (!success) {
				if (!(stream >> c) || c == ',' || c == '}') {
					success = true;
					if (c == '}') {
						C = 'F';
						break;
					}
				}
			}
			value.reset();
		}
		if (c == ']' || c == '?') return v;
		else throw std::logic_error("masssiv");
	}

	void key_in_obj(std::istringstream &stream, std::string& key)
	{
		char c = '?';
		if (!(stream >> c) || (c != '\"')) {
			if (c == '}') {
				C = '}';
				if (!(stream >> c) || c == ',' || c == '}') {
					if (c == '}') C = 'e';
				}
				else throw std::out_of_range("intpu");
				return;
			}
			throw std::logic_error("input_error");
		}
		else {

			while (stream >> c && c != '\"') {
				if (c == ':') throw std::logic_error("input_error");
				key.push_back(c);
			}
		}
	}
	void value_in_obj(std::istringstream &stream, bool &success, std::any& value)
	{
		char c = '?';
		if (!(stream >> c) || c != ':') {
			throw std::logic_error("input_error");
		}
		else {
			value = return_type(stream, success);
		}
	}

	void value_in_arr(std::istringstream &stream, bool& success, std::any& value)
	{
		value = return_type(stream, success);
	}

	std::unordered_map<std::string, std::any> object_parsing(std::istringstream &stream)
	{
		char c = 'n';
		std::string key;
		std::any value;
		std::unordered_map<std::string, std::any> m;
		bool success = true;
		while (success)
		{
			key_in_obj(stream, key);
			if (C == '}' || C == 'e') {
				//C = 'f';
				return m;
			}
			value_in_obj(stream, success, value);
			if (!success && C != 'E') {
				if (!(stream >> c) || c == ',' || c == '}')
					success = true;
				else throw std::out_of_range("intpu");
			}
			m.insert(std::pair<std::string, std::any>(key, value));
			key = "";
			value.reset();
			if (c == '}' || C == 'E') {
				C = 'f';
				break;
			}
		}
		return m;
	}

	std::vector<std::any> array_parsing(std::istringstream &stream)
	{
		char c = 'n';
		std::any value;
		bool success = true;
		while (success)
		{
			value_in_arr(stream, success, value);
			if (!success) {
				if (!(stream >> c) || c == ',' || c == ']')
					success = true;
				else throw std::out_of_range("intpu");
			}
			Array.push_back(value);
			value.reset();
			if (c == ']') break;
		}
		return Array;
	}
};
