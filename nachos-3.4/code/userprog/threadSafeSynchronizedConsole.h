#ifndef THREAD_SAFE_SYNCHRONIZED_CONSOLE_H 
#define THREAD_SAFE_SYNCHRONIZED_CONSOLE_H

#include "console.h"
#include "synch.h"

class ThreadSafeSynchronizedConsole
{
	public:
		ThreadSafeSynchronizedConsole();
		~ThreadSafeSynchronizedConsole();
		void PutChar(char ch);
		char GetChar();
		void UpConsoleReadSemaphore();
		void UpConsoleWriteSemaphore();
	private:
		Lock *consoleLock;
		Semaphore *consoleReadSemaphore;
		Semaphore *consoleWriteSemaphore;
		Console *console;
};

#endif