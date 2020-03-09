#ifndef _THREAD_HANDLE_H_
#define _THREAD_HANDLE_H_
#include <Windows.h>

class ThreadHandle
{
public:
	ThreadHandle();
	virtual ~ThreadHandle();
private:
	HANDLE m_thread;
	static unsigned __stdcall ThreadRunning(LPVOID lpParameter);
	HRESULT InitialThread();
	//when don't use thread anymore, call this function
	void DestroyThread();
	virtual void ThreadExcecute() {};
protected:
	HANDLE m_beginThread;
	HANDLE m_EndThread;
	
};

#endif // !_THREAD_HANDLE_H_



