/// Buffered reader to implement fast istream-like operations.
/// It preloads chunks of data from a reader (possibly an ifstream) and permits to access it directly.
/// It currently also implements fast reading of base 10 values as double.
/// \note On Windows, files opened as binary even for text reading have better performance.
/// \author Francois-R.Boyer@PolyMtl.ca
/// \date 2020-03
/// \file
#pragma once
#include <fstream>
#include <iostream>
#include <type_traits>
#include <vector>

template <typename IStreamT = std::ifstream>
class BufferedReader {
public:
	using size_type = std::size_t;

	static constexpr size_type default_buffer_size = 0x10000;
	using char_type = typename std::decay_t<IStreamT>::char_type;
	
	BufferedReader(IStreamT& stream, size_type buffer_size = default_buffer_size) :
		stream_(stream),
		buffer_(buffer_size) {}

	template <typename IStreamU = IStreamT>
	BufferedReader(std::enable_if_t<!std::is_reference_v<IStreamU>, IStreamU&&> stream, size_type buffer_size = default_buffer_size) :
		stream_(std::move(stream)),
		buffer_(buffer_size) {}

	BufferedReader(const std::string& filename, std::ios::openmode mode = std::ios::in, size_type buffer_size = default_buffer_size) :
		stream_(filename, mode),
		buffer_(buffer_size) {}
		
	bool isEnd() { return !hasCharsLeft(1); }

	operator bool() { return !isEnd(); }

	const char_type* getPointer() { return &buffer_[current_position_]; }

	size_type charsLeftInBuffer() {
		return chars_in_buffer_ - current_position_;
	}

	//NOTE: For text files, this will be inexact if the number of chars in file is different from the number of chars when reading (i.e. \r\n on Windows), but it will be monotonic increasing up to the total number of chars in file. Useful for file reading progression, not to seek back to a previous value.
	std::streamoff tellg() {
		return std::streamoff{stream_.tellg()} - charsLeftInBuffer();
	}

    std::streamoff getFileSize() {
	    std::streampos oldPos = stream_.tellg();
	    stream_.seekg(0, std::ios::end);
	    std::streamoff size = stream_.tellg();
	    stream_.seekg(oldPos);
	    return size;
    }

	const IStreamT& getStream() const { return stream_; }

	// --- Methods above this line do not invalidate pointers to data in buffer, those below do. ---
	// For the methods below, you can use ensureCharsLeft or hasCharsLeft with a value at least as large as the requested data to ensure the pointers are not invalidated.

	/// To peek (read a value without moving read pointer) a binary value as the specified type.
	/// This version throws on failure.
	/// \note This invalidates references and pointers returned by data access methods if the data is not currently in the buffer.
	template <typename T>
	std::enable_if_t<!std::is_pointer_v<T>, const T&> peekAs() {
		ensureCharsLeft(sizeof(T));
		return *reinterpret_cast<const T*>(getPointer());
	}	

	/// To peek (read a value without moving read pointer) a binary value as the specified type.
	/// This version returns nullptr on failure.
	/// \note This invalidates references and pointers returned by data access methods if the data is not currently in the buffer.
	template <typename T>
	std::enable_if_t<std::is_pointer_v<T>, const std::remove_pointer_t<T>*> peekAs() {
		using Elem = std::remove_pointer_t<T>;
		return hasCharsLeft(sizeof(Elem))
			? reinterpret_cast<const Elem*>(getPointer())
			: nullptr;
	}

	/// Move the read head.
	/// It does not support going backwards.
	/// \note This invalidates references and pointers returned by data access methods if the data is not currently in the buffer.
	BufferedReader& operator+= (size_type advance_by) {
		current_position_ += advance_by;
		if (current_position_ >= chars_in_buffer_) {
			if (current_position_ > chars_in_buffer_)
				stream_.seekg(current_position_ - chars_in_buffer_, std::ios::cur);
			current_position_ = 0;
			chars_in_buffer_ = 0;
		}
		return *this;
	}

	/// Reads (moving read pointer) a binary value as the specified type.
	/// The value is copied, not returned by reference (see note).
	/// \note This invalidates references and pointers returned by data access methods if the data is not currently in the buffer.
	template <typename T>
	std::enable_if_t<!std::is_pointer_v<T>, T> readAs() {
		T result = peekAs<T>();
		*this += sizeof(T);
		return result;
	}
	
	/// \note This invalidates references and pointers returned by data access methods if the data is not currently in the buffer.
	void ensureCharsLeft(size_type n_chars) {
		if (!hasCharsLeft(n_chars))
			throw std::ios::failure("Insufficient chars left to read");
	}
	
	/// \note This invalidates references and pointers returned by data access methods if the data is not currently in the buffer.
	bool hasCharsLeft(size_type n_chars) {
		if (n_chars <= charsLeftInBuffer())
			return true;
		
		if (n_chars > buffer_.size())
			buffer_.resize(n_chars);
		
		shiftBufferToCurrentPosition();
		fillBuffer();
		return chars_in_buffer_ >= n_chars;
	}
		
protected:
	void fillBuffer() {
		stream_.read(&buffer_[chars_in_buffer_], buffer_.size() - chars_in_buffer_);
		chars_in_buffer_ += size_type(stream_.gcount());
		if (chars_in_buffer_ < buffer_.size())
			buffer_[chars_in_buffer_] = '\0'; // Add a null char at end, to help some kind of text processing. This is done only at end of file. Before the end, use hasCharsLeft to be certain there are enough chars to parse.
	}

	/// \note This always invalidates references and pointers returned by data access methods (except if current_position is already zero and calling this method is a waste of time).
	void shiftBufferToCurrentPosition() {
		chars_in_buffer_ -= current_position_;
		memcpy(&buffer_[0], &buffer_[current_position_], chars_in_buffer_ * sizeof(char_type));
		current_position_ = 0;
	}

	IStreamT stream_;
	std::vector<char_type> buffer_;
	size_type current_position_ = 0, chars_in_buffer_ = 0;
};


template <typename CharT>
bool is_digit(CharT c) { return c >= '0' && c <= '9'; };

template <typename CharT>
bool is_sign(CharT c) { return c == '-' || c == '+'; };

template <typename CharT>
bool is_space(CharT c) { return c==' ' || c=='\n' || c=='\r' || c=='\t'; }; // simplified is_space as std::isspace is slower.


template <typename StreamT>
BufferedReader<StreamT>& ws(BufferedReader<StreamT>& reader)
{
	while (!reader.isEnd()) {
		auto* chars = reader.getPointer();
		auto n_chars = reader.charsLeftInBuffer();
		for (decltype(n_chars) i = 0; i < n_chars; ++i)
			if (!is_space(chars[i])) {
				reader += i;
				return reader;
			}
		reader += n_chars;
	}
	return reader;
}


/// Similar to std::strtoll, but faster, assuming fixed base 10 and no sign nor error reporting.
template <typename CharT>
int64_t peekInteger(const CharT* text_begin, const CharT* text_end, const CharT** value_end_out = nullptr)
{
    int64_t value = 0;
	const auto* current = text_begin;  CharT c;
    for (current = text_begin; current < text_end && is_digit(c = *current); ++current)
        value = value*10 + (c - '0');

	if (value_end_out)
		*value_end_out = current;

	return value;
}


/// Fast reading of decimal values (not in scientific notation) with maximum of 20.19 digits, using BufferedReader.
/// From our experiments it is 15 times faster that istream >> double, reading several million values from a fast M2 SSD or cached in memory.
template <typename StreamT>
double readDouble(BufferedReader<StreamT>& reader, int64_t relative_to_value = 0)
{
	static constexpr std::size_t longest_integer_chars = 20;
	static constexpr std::size_t longest_fraction_chars = 19;
	static constexpr std::size_t longest_value_chars = 1+longest_integer_chars+1+longest_fraction_chars+1; // sign+int.frac +separator
	static const double negativePow10[] = { 1E0, 1E-1, 1E-2, 1E-3, 1E-4, 1E-5, 1E-6, 1E-7, 1E-8, 1E-9, 1E-10, 1E-11, 1E-12, 1E-13, 1E-14, 1E-15, 1E-16, 1E-17, 1E-18, 1E-19 };
	auto is_fraction_separator = [](auto c) { return c == '.'; };

	reader.hasCharsLeft(longest_value_chars);
	const char* text_begin = reader.getPointer();
	bool is_negative = *text_begin == '-';
	const char* text_current = text_begin + is_sign(*text_begin);

	int64_t value_integer = peekInteger(text_current, text_current+longest_integer_chars, &text_current);
	double value_fractional = 0;
	if (is_fraction_separator(*text_current)) {
		++text_current;
		if (is_digit(*text_current)) {
			const char* fraction_begin = text_current;
			int64_t value_fraction = peekInteger(fraction_begin, fraction_begin + longest_fraction_chars, &text_current);
			value_fractional = value_fraction * negativePow10[text_current - fraction_begin];
		}
	}
	double value = (value_integer - relative_to_value) + value_fractional;
	if (is_negative)
		value = -value;
	reader += text_current - text_begin;
	return value; 
}


template <typename StreamT>
BufferedReader<StreamT>& operator>> (BufferedReader<StreamT>& reader, double& result)
{
	ws(reader);
	result = readDouble(reader);
	return reader;
}
