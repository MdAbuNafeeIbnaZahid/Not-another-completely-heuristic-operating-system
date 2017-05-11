#include "threadSafeSynchronizedConsole.h"

static void ThreadSafeSynchronizedConsoleReadHandler(int arg)
{
	ThreadSafeSynchronizedConsole *myConsole = (ThreadSafeSynchronizedConsole*) arg;
	myConsole->UpConsoleReadSemaphore();
}

static void ThreadSafeSynchronizedConsoleWriteHandler(int arg)
{
	ThreadSafeSynchronizedConsole *myConsole = (ThreadSafeSynchronizedConsole*) arg;
	myConsole->UpConsoleWriteSemaphore();
}

ThreadSafeSynchronizedConsole::ThreadSafeSynchronizedConsole()
{
	consoleLock = new Lock("ThreadSafeSynchronizedConsole Lock");
	consoleReadSemaphore = new Semaphore("consoleReadSemaphore", 0);
	consoleWriteSemaphore = new Semaphore("consoleWriteSemaphore", 0);
	console = new Console(NULL, NULL, ThreadSafeSynchronizedConsoleReadHandler, 
	ThreadSafeSynchronizedConsoleWriteHandler, (int)this );
}

ThreadSafeSynchronizedConsole::~ThreadSafeSynchronizedConsole()
{
	delete consoleLock;
	delete consoleReadSemaphore;
	delete consoleWriteSemaphore;
	delete console;
}

void ThreadSafeSynchronizedConsole::PutChar(char ch)
{
	console->PutChar(ch);
	consoleWriteSemaphore->P(); 
}

char ThreadSafeSynchronizedConsole::GetChar()
{
	consoleReadSemaphore->P();
	char ch = console->GetChar();
	return ch;
}

void ThreadSafeSynchronizedConsole::UpConsoleReadSemaphore()
{
	consoleReadSemaphore->V();
}

void ThreadSafeSynchronizedConsole::UpConsoleWriteSemaphore()
{
	consoleWriteSemaphore->V();
}

void ThreadSafeSynchronizedConsole::AcquireConsoleLock()
{
	consoleLock->Acquire();
}

void ThreadSafeSynchronizedConsole::ReleaseConsoleLock()
{
	consoleLock->Release();
}