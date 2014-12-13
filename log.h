#include <algorithm>

#ifndef LOG_H
#define LOG_H

class Log {
	public:
		static int BUFFER_SIZE;
		static Log* Get();

		void Write(const char* msg, int length);
		char* buffer;
	private:
		Log();
		static Log* Instance;
};

int Log::BUFFER_SIZE = 256;

Log* Log::Instance = 0;

Log::Log() {
	buffer = new char[Log::BUFFER_SIZE];
	for(int i = 0; i < BUFFER_SIZE; i += 1) {
		buffer[i] = 0;
	}
}

void Log::Write(const char* msg, int length) {
	length = std::min(length, BUFFER_SIZE);
	for(int i = 0; i < length; i += 1) {
		buffer[i] = msg[i];
	}
	for(int i = length; i < BUFFER_SIZE; i += 1) {
		buffer[i] = 0;
	}
}

Log* Log::Get() {
	if(Instance == 0) {
		Instance = new Log();
	}
	return Instance;
}

#endif